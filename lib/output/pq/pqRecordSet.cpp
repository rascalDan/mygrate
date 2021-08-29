#include "pqRecordSet.h"
#include "dbTypes.h"
#include "pqStmt.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <helpers.h>
#include <libpq-fe.h>
#include <server/catalog/pg_type_d.h>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace MyGrate::Output::Pq {
	PqRecordSet::PqRecordSet(ResPtr r) : res {std::move(r)} { }

	std::size_t
	PqRecordSet::rows() const
	{
		return boost::numeric_cast<std::size_t>(PQntuples(res.get()));
	}

	std::size_t
	PqRecordSet::columns() const
	{
		return boost::numeric_cast<std::size_t>(PQnfields(res.get()));
	}

	DbValue
	PqRecordSet::at(std::size_t row, std::size_t col) const
	{
		if (PQgetisnull(res.get(), static_cast<int>(row), static_cast<int>(col))) {
			return nullptr;
		}
		const auto value {PQgetvalue(res.get(), static_cast<int>(row), static_cast<int>(col))};
		const auto size {static_cast<size_t>(PQgetlength(res.get(), static_cast<int>(row), static_cast<int>(col)))};
		const auto type {PQftype(res.get(), static_cast<int>(col))};
		switch (type) {
			// case BITOID: TODO bool
			// case BOOLOID: TODO bool
			// case BOOLARRAYOID:
			case VARBITOID:
			case BYTEAOID:
				// This is wrong :)
				return Blob {reinterpret_cast<const std::byte *>(value), size};
			case INT2OID:
				return static_cast<int16_t>(std::strtol(value, nullptr, 10));
			case INT4OID:
				return static_cast<int32_t>(std::strtol(value, nullptr, 10));
			case INT8OID:
				return std::strtol(value, nullptr, 10);
			case FLOAT4OID:
				return std::strtof(value, nullptr);
			case FLOAT8OID:
			case CASHOID:
			case NUMERICOID:
				return std::strtod(value, nullptr);
			case DATEOID: {
				tm tm {};
				const auto end = strptime(value, "%F", &tm);
				verify<std::runtime_error>(end && !*end, "Invalid date string");
				return Date {tm};
			}
			case TIMEOID: {
				tm tm {};
				const auto end = strptime(value, "%T", &tm);
				verify<std::runtime_error>(end && !*end, "Invalid time string");
				return Time {tm};
			}
			case TIMESTAMPOID: {
				tm tm {};
				const auto end = strptime(value, "%F %T", &tm);
				verify<std::runtime_error>(end && !*end, "Invalid timestamp string");
				return DateTime {tm};
			}
			// case TIMESTAMPTZOID: Maybe add TZ support?
			// case INTERVALOID: Maybe add interval support?
			// case TIMETZOID: Maybe add TZ support?
			case VOIDOID:
				return nullptr;
			default:
				return std::string_view {value, size};
		}
	}
}
