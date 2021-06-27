#define BOOST_TEST_MODULE TypeMappers
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "helpers.h"
#include <compileTimeFormatter.h>
#include <output/pq/typeMapper.h>

using M = MyGrate::Output::Pq::TypeMapper::RegexMapper;

namespace std {
	ostream &
	operator<<(ostream & s, const MyGrate::Output::Pq::TypeMapper::ObsRegex & r)
	{
		return s << r.src;
	}
	ostream &
	operator<<(ostream & s, const M & m)
	{
		return MyGrate::scprintf<"{ type: %?, column: %?, target: %? }">(s, m.typeMatch, m.columnMatch, m.targetType);
	}
}

using regex_mapping_test_data = std::tuple<M, std::string_view, std::string_view, std::optional<std::string>>;
using namespace MyGrate::Output::Pq;
BOOST_DATA_TEST_CASE(regex_mapper,
		boost::unit_test::data::make<regex_mapping_test_data>({
				// Types
				{{R"(int\(\d+\))"_r, {}, "int"}, "int(10)", "any", "int"},
				{{R"(int(\(\d+\))?)"_r, {}, "int"}, "int", "any", "int"},
				{{R"(int\(\d+\))"_r, {}, "int"}, "int(1)", "any", "int"},
				{{R"(int\(\d+\))"_r, {}, "int"}, "varchar(1)", "any", {}},
				{{R"(varchar\(\d+\))"_r, {}, ""}, "varchar(1)", "any", "varchar(1)"},
				{{R"(varchar\(\d+\))"_r, {}, ""}, "varchar(10)", "any", "varchar(10)"},
				{{R"(varchar\(\d+\))"_r, {}, ""}, "int(10)", "any", {}},
				{{R"(varchar\(\d{4,}\))"_r, {}, "text"}, "varchar(1024)", "any", "text"},
				// Names
				{{{}, R"(.*\.id)"_r, "bigint"}, "int(10)", "all.id", "bigint"},
				{{{}, R"(.*\.name)"_r, "text"}, "int(10)", "all.name", "text"},
		}),
		m, inType, inCol, exp)
{
	BOOST_CHECK_EQUAL(m.map(inType, inCol), exp);
}

using mapping_test_data = std::tuple<std::string_view, std::string_view>;
BOOST_DATA_TEST_CASE(default_type_mapping,
		boost::unit_test::data::make<mapping_test_data>({
				// https://dev.mysql.com/doc/refman/8.0/en/numeric-types.html
				{"bit(10)", "bit(10)"},
				{"tinyint", "smallint"},
				{"tinyint(1)", "boolean"},
				{"tinyint(2)", "smallint"},
				{"smallint", "smallint"},
				{"smallint(1)", "smallint"},
				{"smallint(2)", "smallint"},
				{"mediumint", "int"},
				{"mediumint(5)", "int"},
				{"int", "int"},
				{"int(10)", "int"},
				{"bigint", "bigint"},
				{"bigint(10)", "bigint"},
				{"tinyint unsigned", "smallint"},
				{"tinyint(1) unsigned", "boolean"},
				{"tinyint(2) unsigned", "smallint"},
				{"smallint unsigned", "smallint"},
				{"smallint(1) unsigned", "smallint"},
				{"smallint(2) unsigned", "smallint"},
				{"mediumint unsigned", "int"},
				{"mediumint(5) unsigned", "int"},
				{"int unsigned", "int"},
				{"int(10) unsigned", "int"},
				{"bigint unsigned", "bigint"},
				{"bigint(10) unsigned", "bigint"},
				{"decimal", "decimal"},
				{"decimal(1,2)", "decimal(1,2)"},
				{"decimal(1)", "decimal(1)"},
				{"numeric", "numeric"},
				{"numeric(1,2)", "numeric(1,2)"},
				{"numeric(1)", "numeric(1)"},
				{"float", "float"},
				{"float(1,2)", "float"},
				{"real", "double"},
				{"real(1,2)", "double"},
				{"double", "double"},
				{"double(1,2)", "double"},
				{"double precision", "double"},
				{"double precision(1,2)", "double"},
				// https://dev.mysql.com/doc/refman/8.0/en/date-and-time-types.html
				{"date", "date"},
				{"datetime", "timestamp without time zone"},
				{"datetime(1)", "timestamp without time zone"},
				{"timestamp", "timestamp without time zone"},
				{"timestamp(1)", "timestamp without time zone"},
				{"time", "time without time zone"},
				{"time(1)", "time without time zone"},
				{"year", "smallint"},
				{"time(1)", "time without time zone"},
				{"year", "smallint"},
				// https://dev.mysql.com/doc/refman/8.0/en/string-types.html
				{"binary", "bytea"},
				{"binary(10)", "bytea"},
				{"varbinary", "bytea"},
				{"varbinary(10)", "bytea"},
				{"char", "char"},
				{"char(10)", "char(10)"},
				{"varchar", "varchar"},
				{"varchar(10)", "varchar(10)"},
				{"blob", "bytea"},
				{"blob(100)", "bytea"},
				{"tinyblob", "bytea"},
				{"mediumblob", "bytea"},
				{"longblob", "bytea"},
				{"text", "text"},
				{"text(100)", "text"},
				{"tinytext", "text"},
				{"mediumtext", "text"},
				{"longtext", "text"},
				// enum
				// set
				// https://dev.mysql.com/doc/refman/8.0/en/json.html
				{"json", "json"},
		}),
		inType, exp)
{
	TypeMapper tm;
	BOOST_CHECK_EQUAL(tm.map(inType, {}), exp);
}
