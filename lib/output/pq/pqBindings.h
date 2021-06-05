#ifndef MYGRATE_OUTPUT_PQ_PQBINDINGS
#define MYGRATE_OUTPUT_PQ_PQBINDINGS

#include <dbTypes.h>
#include <helpers.h>
#include <initializer_list>
#include <variant>
#include <vector>

namespace MyGrate::Output::Pq {
	struct Bindings {
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		explicit Bindings(const std::initializer_list<DbValue> & vs)
		{
			bufs.reserve(vs.size());
			values.reserve(vs.size());
			lengths.reserve(vs.size());
			for (const auto & v : vs) {
				v.visit(*this);
			}
		}
		template<Stringable T>
		void
		operator()(const T & v)
		{
			bufs.emplace_back(std::to_string(v));
			const auto & vw {bufs.back()};
			values.emplace_back(vw.data());
			lengths.emplace_back(vw.length());
		}
		template<Viewable T>
		void
		operator()(const T & v)
		{
			values.emplace_back(v.data());
			lengths.emplace_back(v.size());
		}
		template<typename T>
		void
		operator()(const T &)
		{
			throw std::runtime_error("Not implemented");
		}
		void
		operator()(const std::nullptr_t &)
		{
			values.emplace_back(nullptr);
			lengths.emplace_back(0);
		}

		std::vector<std::string> bufs;
		std::vector<const char *> values;
		std::vector<int> lengths;
	};
}

#endif
