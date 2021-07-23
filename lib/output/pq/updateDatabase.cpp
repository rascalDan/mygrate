#include "updateDatabase.h"
#include "pqConn.h"
#include "typeMapper.h"
#include <compileTimeFormatter.h>
#include <cstdint>
#include <dbRecordSet.h>
#include <eventSourceBase.h>
#include <helpers.h>
#include <input/mysqlConn.h>
#include <input/mysqlRecordSet.h>
#include <input/replStream.h>
#include <input/sql/selectColumns.h>
#include <input/sql/showMasterStatus.h>
#include <memory>
#include <output/pq/sql/insertColumn.h>
#include <output/pq/sql/insertSource.h>
#include <output/pq/sql/insertTable.h>
#include <output/pq/sql/selectColumns.h>
#include <output/pq/sql/selectSource.h>
#include <output/pq/sql/selectSourceSchema.h>
#include <output/pq/sql/selectTables.h>
#include <row.h>
#include <stdexcept>
#include <streamSupport.h>

namespace MyGrate::Output::Pq {
	ColumnDef::ColumnDef(std::string n, std::size_t o, bool p) : name {std::move(n)}, ordinal(o), is_pk(p) { }

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

	TableOutput::TableOutput(const RecordSet & crecs, std::string_view name)
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
		auto cols = input::sql::selectColumns::execute(conn, tableName);
		verify<std::logic_error>(cols->rows() > 0, "Table has no rows");
		auto tableDef {std::make_unique<TableOutput>()};
		Tx {this}([&] {
			const auto table_id = **output::pq::sql::insertTable::execute(this, tableName, source);
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
					ct << " primary key";
					tableDef->keys += 1;
				}
				tableDef->columns.push_back(std::make_unique<ColumnDef>(col[0], tableDef->columns.size() + 1, col[3]));
			}
			ct << ")";
			this->query(ct.str().c_str());
		});
		tables.emplace(tableName, std::move(tableDef));
	}

	struct WritePqCopyStream {
		explicit WritePqCopyStream(FILE * o) : out {o} { }
		WritePqCopyStream(const WritePqCopyStream &) = delete;
		WritePqCopyStream(WritePqCopyStream &&) = delete;
		WritePqCopyStream & operator=(const WritePqCopyStream &) = delete;
		WritePqCopyStream & operator=(WritePqCopyStream &&) = delete;

		~WritePqCopyStream()
		{
			fputc('\n', out);
		}

		void
		nextField()
		{
			fputc('\t', out);
		}

		void operator()(std::nullptr_t) const
		{
			fputs("\\N", out);
		}
#define BASIC_PRINT(T, fmt) \
	void operator()(T v) const \
	{ \
		fprintf(out, fmt, v); \
	}
		BASIC_PRINT(double, "%f")
		BASIC_PRINT(float, "%f")
		BASIC_PRINT(int8_t, "%hhd")
		BASIC_PRINT(uint8_t, "%hhu")
		BASIC_PRINT(int16_t, "%hd")
		BASIC_PRINT(uint16_t, "%hu")
		BASIC_PRINT(int32_t, "%d")
		BASIC_PRINT(uint32_t, "%u")
		BASIC_PRINT(int64_t, "%ld")
		BASIC_PRINT(uint64_t, "%lu")
#undef BASIC_PRINT
		void operator()(timespec) const
		{
			throw std::logic_error("timespec not implemented");
		}
		void
		operator()(Date v) const
		{
			fprintf(out, "%d-%d-%d", v.year, v.month, v.day);
		}
		void
		operator()(Time v) const
		{
			fprintf(out, "%d:%d:%d", v.hour, v.minute, v.second);
		}
		void
		operator()(DateTime v) const
		{
			operator()(static_cast<Date &>(v));
			fputc('T', out);
			operator()(static_cast<Time &>(v));
		}
		void
		operator()(std::string_view v) const
		{
			auto pos {v.begin()};
			while (pos != v.end()) {
				auto esc = std::find_if(pos, v.end(), [](unsigned char c) {
					return std::iscntrl(c);
				});
				if (esc != pos) {
					fwrite(pos, esc - pos, 1, out);
					pos = esc;
				}
				while (pos != v.end()) {
					fprintf(out, "\\%03o", *pos);
					pos++;
				}
			}
		}
		void operator()(BitSet) const
		{
			throw std::logic_error("bitset not implemented");
		}
		void
		operator()(Blob v) const
		{
			fputs("\\\\x", out);
			std::for_each(v.begin(), v.end(), [this](auto b) {
				fprintf(out, "%02hhx", (uint8_t)b);
			});
		}

		FILE * out;
	};

	void
	UpdateDatabase::copyTableContent(Input::MySQLConn * conn, const char * table)
	{
		auto out = beginBulkUpload(schema.c_str(), table);
		auto sourceSelect = [this](auto table) {
			std::stringstream sf;
			unsigned int ordinal {0};
			for (const auto & col : tables.at(table)->columns) {
				scprintf<"%? %?">(sf, !ordinal++ ? "SELECT " : ", ", col->name);
			}
			sf << " FROM " << table;
			return sf.str();
		};
		auto stmt {conn->prepare(sourceSelect(table).c_str(), 0)};
		stmt->execute({});
		auto sourceCursor {stmt->cursor()};

		const auto cols = sourceCursor->columns();
		while (sourceCursor->fetch()) {
			WritePqCopyStream cs {out};
			for (auto ordinal {0U}; ordinal < cols; ordinal += 1) {
				if (ordinal) {
					cs.nextField();
				}
				sourceCursor->at(ordinal).visit(cs);
			}
		}

		fclose(out);
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
	UpdateDatabase::updateRow(MariaDB_Event_Ptr e)
	{
		if (selected != tables.end()) {
			auto & out = selected->second;
			verify<std::runtime_error>(
					e->event.rows.column_count == out->columns.size(), "Incorrect number of columns in row data");
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
			std::vector<DbValue> updateValues;
			updateValues.reserve(out->columns.size() + out->keys);
			RowPair rp {e->event.rows, table_map->event.table_map};
			std::copy(rp.second.begin(), rp.second.end(), std::back_inserter(updateValues));
			std::copy_if(rp.first.begin(), rp.first.end(), std::back_inserter(updateValues),
					[c = out->columns.begin()](auto &&) mutable {
						return (c++)->get()->is_pk;
					});
			out->update->execute(updateValues);
			verify<ReplicationError>(out->update->rows() == 1, "Wrong number of rows updated.");
		}
	}

	void
	UpdateDatabase::deleteRow(MariaDB_Event_Ptr e)
	{
		if (selected != tables.end()) {
			auto & out = selected->second;
			verify<std::runtime_error>(
					e->event.rows.column_count == out->columns.size(), "Incorrect number of columns in row data");
			if (!out->deleteFrom) {
				std::stringstream ou;
				std::size_t kordinal {0};

				scprintf<"DELETE FROM %?.%? ">(ou, schema, selected->first);
				for (const auto & col : out->columns) {
					if (col->is_pk) {
						scprintf<"%? %? = $%?">(
								ou, kordinal == out->columns.size() ? " WHERE " : " AND ", col->name, kordinal + 1);
						kordinal++;
					}
				}

				out->deleteFrom = prepare(ou.str().c_str(), kordinal);
			}
			std::vector<DbValue> updateValues;
			updateValues.reserve(out->keys);
			Row rp {e->event.rows, table_map->event.table_map};
			std::copy_if(rp.begin(), rp.end(), std::back_inserter(updateValues),
					[c = out->columns.begin()](auto &&) mutable {
						return (c++)->get()->is_pk;
					});
			out->deleteFrom->execute(updateValues);
			verify<ReplicationError>(out->deleteFrom->rows() == 1, "Wrong number of rows deleted.");
		}
	}

	void
	UpdateDatabase::insertRow(MariaDB_Event_Ptr e)
	{
		if (selected != tables.end()) {
			auto & out = selected->second;
			verify<std::runtime_error>(
					e->event.rows.column_count == out->columns.size(), "Incorrect number of columns in row data");
			if (!out->insertInto) {
				std::stringstream ou;
				std::size_t ordinal {0}, kordinal {out->columns.size()};

				scprintf<"INSERT INTO %?.%? ">(ou, schema, selected->first);
				for (const auto & col : out->columns) {
					scprintf<"%? %?">(ou, !ordinal ? "(" : ", ", col->name);
				}
				ou << ") VALUES";
				for (const auto & col : out->columns) {
					scprintf<"%? $%?">(ou, !ordinal ? "(" : ", ", col->name);
				}
				ou << ")";

				out->insertInto = prepare(ou.str().c_str(), kordinal);
			}
			std::vector<DbValue> updateValues;
			updateValues.reserve(out->columns.size());
			Row rp {e->event.rows, table_map->event.table_map};
			std::copy(rp.begin(), rp.end(), std::back_inserter(updateValues));
			out->insertInto->execute(updateValues);
			verify<ReplicationError>(out->insertInto->rows() == 1, "Wrong number of rows updated.");
		}
	}
}
