#ifndef MYGRATE_OUTPUT_PQ_PQBINDINGS
#define MYGRATE_OUTPUT_PQ_PQBINDINGS

#include <compileTimeFormatter.h>
#include <concepts>
#include <dbTypes.h>
#include <helpers.h>
#include <initializer_list>
#include <streamSupport.h>
#include <variant>
#include <vector>

namespace MyGrate::Output::Pq {
	struct Bindings {
		explicit Bindings(const std::span<const DbValue> vs)
		{
			bufs.reserve(vs.size());
			values.reserve(vs.size());
			lengths.reserve(vs.size());
			formats.reserve(vs.size());
			for (const auto & v : vs) {
				v.visit(*this);
			}
		}
		void
		operator()(const Stringable auto & v)
		{
			addBuf(std::to_string(v));
		}
		void
		operator()(const Viewable auto & v)
		{
			values.emplace_back(v.data());
			lengths.emplace_back(v.size());
			formats.push_back(1);
		}
		void
		operator()(const auto & v)
		{
			addBuf(scprintf<"%?">(v));
		}
		void
		operator()(const timespec &)
		{
			throw std::logic_error("Not implemented");
		}
		void
		operator()(const std::nullptr_t &)
		{
			values.emplace_back(nullptr);
			lengths.emplace_back(0);
			formats.push_back(0);
		}

		std::vector<std::string> bufs;
		std::vector<const char *> values;
		std::vector<int> lengths;
		std::vector<int> formats;

	private:
		void
		addBuf(std::string str)
		{
			const auto & vw = bufs.emplace_back(std::move(str));
			values.emplace_back(vw.data());
			lengths.emplace_back(vw.length());
			formats.push_back(0);
		}
	};
}

#endif
