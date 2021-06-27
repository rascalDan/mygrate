#ifndef MYGRATE_OUTPUT_PQ_TYPEMAPPER_H
#define MYGRATE_OUTPUT_PQ_TYPEMAPPER_H

#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace MyGrate::Output::Pq {
	class TypeMapper {
	public:
		class ObsRegex : public std::regex {
		public:
			explicit ObsRegex(const std::string & src, std::regex_constants::syntax_option_type sot = {}) :
				std::regex {src, sot}, src {src}
			{
			}
			const std::string src;
		};

		struct Mapping {
			virtual std::optional<std::string_view> map(std::string_view t, std::string_view n) const = 0;
		};
		using MappingPtr = std::unique_ptr<Mapping>;

		struct RegexMapper : public Mapping {
			RegexMapper(std::optional<ObsRegex>, std::optional<ObsRegex>, std::string);

			std::optional<std::string_view> map(std::string_view t, std::string_view n) const override;

			std::optional<ObsRegex> typeMatch;
			std::optional<ObsRegex> columnMatch;
			std::string targetType;
		};

		TypeMapper();

		std::string_view map(std::string_view t, std::string_view n) const;

	private:
		std::vector<MappingPtr> mappings;
	};

	TypeMapper::ObsRegex operator""_r(const char * input, std::size_t);
}

#endif
