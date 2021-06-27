#include "typeMapper.h"
#include <compileTimeFormatter.h>

namespace MyGrate::Output::Pq {
	TypeMapper::RegexMapper::RegexMapper(std::optional<ObsRegex> t, std::optional<ObsRegex> c, std::string tt) :
		typeMatch {std::move(t)}, columnMatch {std::move(c)}, targetType {std::move(tt)}
	{
	}

	std::optional<std::string_view>
	TypeMapper::RegexMapper::map(std::string_view typeName, std::string_view columnName) const
	{
		if ((!typeMatch || std::regex_match(typeName.begin(), typeName.end(), *typeMatch))
				&& (!columnMatch || std::regex_match(columnName.begin(), columnName.end(), *columnMatch))) {
			if (targetType.empty()) {
				return {typeName};
			}
			return {targetType};
		}
		return {};
	}

	TypeMapper::TypeMapper()
	{
		const auto add = [this](auto r, auto t) {
			mappings.emplace_back(std::make_unique<RegexMapper>(r, std::nullopt, t));
		};
		// https://dev.mysql.com/doc/refman/8.0/en/numeric-types.html
		add(R"(bit\(\d+\))"_r, "");
		add(R"(tinyint\(1\)( (un)?signed)?)"_r, "boolean");
		add(R"((tiny|small)int(\(\d+\))?( (un)?signed)?)"_r, "smallint");
		add(R"((medium)?int(\(\d+\))?( (un)?signed)?)"_r, "int");
		add(R"(bigint(\(\d+\))?( (un)?signed)?)"_r, "bigint");
		add(R"((numeric|decimal)(\(\d+(,\d+)?\))?)"_r, "");
		add(R"(float(\(\d+,\d+\))?)"_r, "float");
		add(R"((real|double( precision)?)(\(\d+,\d+\))?)"_r, "double");
		// https://dev.mysql.com/doc/refman/8.0/en/date-and-time-types.html
		add(R"(date)"_r, "date");
		add(R"(datetime)"_r, "timestamp without time zone");
		add(R"(datetime(\(\d+\))?)"_r, "timestamp without time zone"); // drops precision
		add(R"(timestamp)"_r, "timestamp without time zone");
		add(R"(timestamp(\(\d+\))?)"_r, "timestamp without time zone"); // drops precision
		add(R"(time)"_r, "time without time zone");
		add(R"(time(\(\d+\))?)"_r, "time without time zone"); // drops precision
		add(R"(year)"_r, "smallint");
		// https://dev.mysql.com/doc/refman/8.0/en/string-types.html
		add(R"((var)?binary(\(\d+\))?)"_r, "bytea");
		add(R"((var)?char(\(\d+\))?)"_r, "");
		add(R"((tiny||medium|long)blob(\(\d+\))?)"_r, "bytea");
		add(R"((tiny||medium|long)text(\(\d+\))?)"_r, "text");
		add(R"(enum(.*))"_r, "text"); // not really
		// set
		// https://dev.mysql.com/doc/refman/8.0/en/json.html
		add(R"(json)"_r, "json");
	}

	std::string_view
	TypeMapper::map(std::string_view t, std::string_view n) const
	{
		for (const auto & m : mappings) {
			if (const auto rt {m->map(t, n)}) {
				return *rt;
			}
		}
		throw std::runtime_error(scprintf<"No mapping for %? (%?)">(n, t));
	}

	TypeMapper::ObsRegex operator""_r(const char * input, std::size_t)
	{
		return MyGrate::Output::Pq::TypeMapper::ObsRegex {input, std::regex::icase};
	}
}
