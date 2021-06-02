#include <boost/lexical_cast/bad_lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <cstdlib>
#include <eventSourceBase.h>
#include <iostream>
#include <memory>
#include <output/pq/updateDatabase.h>
#include <string>

namespace po = boost::program_options;

int
main(int argc, char ** argv)
{
	std::string pgconn;
	uint64_t sourceid {};
	bool help {};
	po::options_description opts("MyGrate");
	opts.add_options()("postgresql,p", po::value(&pgconn)->required(), "Target PostgreSQL connection string")(
			"sourceid,s", po::value(&sourceid)->default_value(1), "Source identifier")("help,h",
			po::value(&help)->zero_tokens(), "Help");

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(opts).run(), vm);

	if (vm.count("help")) {
		std::cout << opts;
		return EXIT_SUCCESS;
	}
	po::notify(vm);

	MyGrate::Output::Pq::UpdateDatabase ud {pgconn.c_str(), sourceid};
	auto src {ud.getSource()};
	src->readEvents(ud);

	return EXIT_SUCCESS;
}
