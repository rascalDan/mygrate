#include "updateDatabase.h"
#include "pqConn.h"
#include "typeMapper.h"
#include "writePqCopyStrm.h"
#include <compileTimeFormatter.h>
#include <cstdint>
#include <dbRecordSet.h>
#include <eventSourceBase.h>
#include <helpers.h>
#include <input/mysqlConn.h>
#include <input/mysqlRecordSet.h>
#include <input/replStream.h>
#include <input/sql/selectColumns.h>
#include <input/sql/selectIndexes.h>
#include <input/sql/showMasterStatus.h>
#include <memory>
#include <mysql_types.h>
#include <output/pq/sql/insertColumn.h>
#include <output/pq/sql/insertSource.h>
#include <output/pq/sql/insertTable.h>
#include <output/pq/sql/selectColumns.h>
#include <output/pq/sql/selectSource.h>
#include <output/pq/sql/selectSourceSchema.h>
#include <output/pq/sql/selectTables.h>
#include <output/pq/sql/updateSourcePosition.h>
#include <output/pq/sql/updateSourceRotate.h>
#include <row.h>
#include <stdexcept>
#include <streamSupport.h>

namespace MyGrate::Output::Pq {
	constexpr uint8_t STANDALONE {1};

#ifndef __cpp_aggregate_paren_init
	ColumnDef::ColumnDef(std::string n, std::size_t o, bool p) : name {std::move(n)}, ordinal(o), is_pk(p) { }
#endif

	UpdateDatabase::UpdateDatabase(const char * const str, uint64_t s) : UpdateDatabase {PqConn {str}, s} { }

	UpdateDatabase::UpdateDatabase(PqConn && conn, uint64_t s) :
		UpdateDatabase {std::forward<PqConn>(conn), s, output::pq::sql::selectSourceSchema::execute(&conn, s)}
	{
	}

	UpdateDatabase::UpdateDatabase(PqConn && conn, uint64_t s, RecordSetPtr cfg) :
		PqConn {std::move(conn)}, source {s}, schema(cfg->at(0, 0)),
		database(cfg->at(0, 1)), selected {tables.end()}, table_map {nullptr, nullptr}
	{
		auto trecs = output::pq::sql::selectTables::execute(this, source);
		auto crecs = output::pq::sql::selectColumns::execute(this, source);
		for (auto t {0U}; t < trecs->rows(); t++) {
			tables.emplace(trecs->at(t, 0), std::make_unique<TableOutput>(*crecs, trecs->at(t, 0)));
		}
	}

	EventSourceBasePtr
	UpdateDatabase::getSource()
	{
		auto srcrec = output::pq::sql::selectSource::execute(this, source);
		verify<ConfigError>(srcrec->rows() == 1, "Wrong number of source config rows");
		return (*srcrec)[0].create<Input::ReplicationStream, 7>();
	}

	TableOutput::TableOutput(const RecordSet & crecs, std::string_view name) : keys {0}
	{
		for (auto c {0U}; c < crecs.rows(); c++) {
			if (crecs.at(c, 0) == name) {
				const auto & cd = columns.emplace_back(crecs[c].create<ColumnDef, 3, 1>());
				if (cd->is_pk) {
					keys += 1;
				}
			}
		}
	}

	UpdateDatabase
	UpdateDatabase::createNew(PqConn * pq, const char * host, const char * username, const char * password,
			unsigned short port, const char * db, int sid, const char * schema)
	{
		Input::MySQLConn my {host, username, password, port};
		auto ms = input::sql::showMasterStatus::execute(&my);
		auto source_id = Tx {pq}([&]() {
			pq->query(scprintf<"CREATE SCHEMA IF NOT EXISTS %?">(schema).c_str());
			return **output::pq::sql::insertSource::execute(
					pq, host, username, password, port, db, ms->at(0, 0), ms->at(0, 1), sid, schema);
		});
		return UpdateDatabase(pq->connstr.c_str(), source_id);
	}

	void
	UpdateDatabase::addTable(Input::MySQLConn * conn, const char * tableName)
	{
		// Assumes a readonly or transaction supporting table
		conn->query("SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ");
		conn->query(scprintf<"LOCK TABLE %? READ">(tableName).c_str());
		Tx {conn}([&] {
			auto pos = *(*input::sql::showMasterStatus::execute(conn))[0].create<MySQL::ReplicationPosition, 2>();
			conn->query("UNLOCK TABLES");
			// Consistent unlocked view of table during transaction
			auto cols = input::sql::selectColumns::execute(conn, tableName);
			verify<std::logic_error>(cols->rows() > 0, "Table has no rows");
			auto tableDef {std::make_unique<TableOutput>()};
			Tx {this}([&] {
				const auto table_id
						= **output::pq::sql::insertTable::execute(this, tableName, source, pos.filename, pos.position);
				std::stringstream ct;
				scprintf<"CREATE TABLE %?.%?(">(ct, schema, tableName);
				TypeMapper tm;
				for (auto col : *cols) {
					output::pq::sql::insertColumn::execute(this, col[0], col.currentRow(), table_id);
					if (col.currentRow()) {
						ct << ',';
					}
					scprintf<"%? %?">(ct, col[0], tm.map(col[2], scprintf<"%?.%?">(tableName, col[0])));
					if (!col[1]) {
						ct << " not null";
					}
					if (col[3]) {
						tableDef->keys += 1;
					}
					tableDef->columns.push_back(
							std::make_unique<ColumnDef>(col[0], tableDef->columns.size() + 1, col[3]));
				}
				ct << ")";
				this->query(ct.str().c_str());
				this->copyTableContent(conn, tableName, tableDef);
				this->copyIndexes(conn, tableName);
			});
			tables.emplace(tableName, std::move(tableDef));
		});
	}

	void
	UpdateDatabase::copyTableContent(Input::MySQLConn * conn, const char * table, const TableDefPtr & tableDef)
	{
		auto out = beginBulkUpload(schema.c_str(), table);
		auto sourceSelect = [&tableDef](auto table) {
			std::stringstream sf;
			unsigned int ordinal {0};
			for (const auto & col : tableDef->columns) {
				scprintf<"%? %?">(sf, !ordinal++ ? "SELECT " : ", ", col->name);
			}
			sf << " FROM " << table;
			return sf.str();
		};
		auto stmt {conn->prepare(sourceSelect(table).c_str(), 0)};
		stmt->execute({});
		auto sourceCursor {stmt->cursor()};

		const auto cols = sourceCursor->columns();
		WritePqCopyStream cs {out};
		while (sourceCursor->fetch()) {
			for (auto ordinal {0U}; ordinal < cols; ordinal += 1) {
				if (ordinal) {
					cs.nextField();
				}
				sourceCursor->at(ordinal).visit(cs);
			}
			cs.nextRecord();
		}

		fclose(out);
	}

	void
	UpdateDatabase::copyIndexes(Input::MySQLConn * conn, const char * tableName)
	{
		auto idxs = input::sql::selectIndexes::execute(conn, tableName);
		for (const auto idx : *idxs) {
			const auto [name, columns, nonunique]
					= *idx.create<std::tuple<std::string_view, std::string_view, bool>, 3, 1>();
			if (nonunique) {
				query(scprintf<"CREATE INDEX %? ON %?.%?(%?)">(name, schema, tableName, columns).c_str());
			}
			else if (name == "PRIMARY") {
				query(scprintf<"ALTER TABLE %?.%? ADD CONSTRAINT pk_%? PRIMARY KEY(%?)">(
						schema, tableName, name, columns)
								.c_str());
			}
			else {
				query(scprintf<"CREATE UNIQUE INDEX %? ON %?.%?(%?)">(name, schema, tableName, columns).c_str());
			}
		}
	}

	void
	UpdateDatabase::tableMap(MariaDB_Event_Ptr e)
	{
		if (*e->event.table_map.database == database) {
			selected = tables.find(*e->event.table_map.table);
			table_map = std::move(e);
		}
		else {
			selected = tables.end();
			table_map.reset();
		}
	}

	void
	UpdateDatabase::beforeEvent(const MariaDB_Event_Ptr &)
	{
		if (!intx) {
			beginTx();
		}
	}

	void
	UpdateDatabase::afterEvent(const MariaDB_Event_Ptr & e)
	{
		if (!intx) {
			output::pq::sql::updateSourcePosition::execute(this, e->next_event_pos, source);
			commitTx();
		}
	}

	void
	UpdateDatabase::verifyRow(const MariaDB_Event_Ptr & e, const TableDefPtr & out)
	{
		verify<std::runtime_error>(
				e->event.rows.column_count == out->columns.size(), "Incorrect number of columns in row data");
	}

	void
	UpdateDatabase::copyKeys(const Row & r, const TableDefPtr & td, std::back_insert_iterator<Row> && out)
	{
		std::copy_if(r.begin(), r.end(), out, [c = td->columns.begin()](auto &&) mutable {
			return (c++)->get()->is_pk;
		});
	}

	void
	UpdateDatabase::updateRow(MariaDB_Event_Ptr e)
	{
		if (selected != tables.end()) {
			auto & out = selected->second;
			verifyRow(e, out);
			if (!out->update) {
				std::stringstream ou;
				std::size_t ordinal {0}, kordinal {out->columns.size()};

				scprintf<"UPDATE %?.%? ">(ou, schema, selected->first);
				for (const auto & col : out->columns) {
					scprintf<"%? %? = $%?">(ou, !ordinal ? " SET " : ", ", col->name, ordinal + 1);
					ordinal++;
				}
				for (const auto & col : out->columns) {
					if (col->is_pk) {
						scprintf<"%? %? = $%?">(
								ou, kordinal == out->columns.size() ? " WHERE " : " AND ", col->name, kordinal + 1);
						kordinal++;
					}
				}

				out->update = prepare(ou.str().c_str(), kordinal);
			}
			beforeEvent(e);
			auto rows {Row::fromRowsEvent(e->event.rows, table_map->event.table_map)};
			verify<ReplicationError>(rows.size() % 2 == 0, "Odd number of update rows");
			for (auto rp = rows.begin(); rp != rows.end(); rp++) {
				copyKeys(*rp, out, std::back_inserter(*rp));
				out->update->execute(*rp);
				verify<ReplicationError>(out->update->rows() == 1, "Wrong number of rows updated.");
			}
			afterEvent(e);
		}
	}

	void
	UpdateDatabase::deleteRow(MariaDB_Event_Ptr e)
	{
		if (selected != tables.end()) {
			auto & out = selected->second;
			verifyRow(e, out);
			if (!out->deleteFrom) {
				std::stringstream ou;
				std::size_t kordinal {0};

				scprintf<"DELETE FROM %?.%? ">(ou, schema, selected->first);
				for (const auto & col : out->columns) {
					if (col->is_pk) {
						scprintf<"%? %? = $%?">(ou, !kordinal ? " WHERE " : " AND ", col->name, kordinal + 1);
						kordinal++;
					}
				}

				out->deleteFrom = prepare(ou.str().c_str(), kordinal);
			}
			beforeEvent(e);
			for (auto & r : Row::fromRowsEvent(e->event.rows, table_map->event.table_map)) {
				Row keys;
				copyKeys(r, out, std::back_inserter(keys));
				out->deleteFrom->execute(keys);
				verify<ReplicationError>(out->deleteFrom->rows() == 1, "Wrong number of rows deleted.");
			}
			afterEvent(e);
		}
	}

	void
	UpdateDatabase::insertRow(MariaDB_Event_Ptr e)
	{
		if (selected != tables.end()) {
			auto & out = selected->second;
			verifyRow(e, out);
			if (!out->insertInto) {
				std::stringstream ou;
				std::size_t ordinal {0}, vordinal {0};

				scprintf<"INSERT INTO %?.%? ">(ou, schema, selected->first);
				for (const auto & col : out->columns) {
					scprintf<"%? %?">(ou, !ordinal++ ? "(" : ", ", col->name);
				}
				ou << ") VALUES";
				for (const auto & col : out->columns) {
					scprintf<"%? $%?">(ou, !vordinal++ ? "(" : ", ", vordinal);
					(void)col;
				}
				ou << ")";

				out->insertInto = prepare(ou.str().c_str(), out->columns.size());
			}
			beforeEvent(e);
			for (const auto & r : Row::fromRowsEvent(e->event.rows, table_map->event.table_map)) {
				out->insertInto->execute(r);
				verify<ReplicationError>(out->insertInto->rows() == 1, "Wrong number of rows updated.");
			}
			afterEvent(e);
		}
	}

	void
	UpdateDatabase::rotate(MariaDB_Event_Ptr e)
	{
		if ((e->flags & LOG_EVENT_ARTIFICIAL_F) == 0) {
			beforeEvent(e);
			output::pq::sql::updateSourceRotate::execute(
					this, *e->event.rotate.filename, e->event.rotate.position, source);
			afterEvent(e);
		}
	}

	void
	UpdateDatabase::gtid(MariaDB_Event_Ptr e)
	{
		beforeEvent(e);
		if (!(e->event.gtid.flags & STANDALONE)) {
			intx = true;
		}
		afterEvent(e);
	}

	void
	UpdateDatabase::xid(MariaDB_Event_Ptr e)
	{
		beforeEvent(e);
		intx = false;
		afterEvent(e);
	}
}
