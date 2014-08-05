/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "LexParser.hpp"
#include "SQLFileParser.hpp"

using namespace sqlfileparser;

int
main(int argc, char* argv[])
{
	try
	{
		if (argc < 3 || argc > 5)
		{
			throw std::runtime_error("Wrong number of parameters; usage: " + std::string(argv[0]) + " [--skip-modified-timestamps] version1.sql version2.sql [ upgrade.sql ]");
		}

		int pstart = 1;
		const std::string skip_modified("--skip-modified-timestamps");
		bool skipModifiedTimestampsFunction = false;

		if (skip_modified.compare(0, 2, argv[1], 2) == 0)
		{
			if (skip_modified.compare(argv[1]) == 0)
			{
				skipModifiedTimestampsFunction = true;
				pstart++;
			}
			else
			{
				throw std::runtime_error("Unknown option: " + std::string(argv[1]));
			}
		}

		std::ifstream inp1(argv[pstart]), inp2(argv[pstart + 1]);

		if (!inp1.good())
		{
			throw std::runtime_error("cannot open file " + std::string(argv[pstart]) + " for reading.");
		}

		if (!inp2.good())
		{
			throw std::runtime_error("cannot open file " + std::string(argv[pstart + 1]) + " for reading.");
		}

		SQLTableListManagerPtr psm1 = lexParse(inp1, skipModifiedTimestampsFunction), psm2 = lexParse(inp2, skipModifiedTimestampsFunction);

#ifdef DEBUG

		std::ofstream debug1("debug1.txt");
		psm1->print(debug1);

		std::ofstream debug2("debug2.txt");
		psm2->print(debug2);

#endif

		SQLFileParser sqlParser(psm1, psm2);

		if (argc == pstart + 3)
		{
			std::ofstream out;
			out.open(argv[pstart + 2]);
			if (!out.good())
			{
				throw std::runtime_error("cannot open file " + std::string(argv[pstart + 2]) + " for writing.");
			}
			sqlParser.print(out);
		}
		else
		{
			sqlParser.print(std::cout);
		}

	}
	catch(std::exception &ex)
	{
		std::cerr << "Caught exception: " << ex.what() << std::endl;
		return -1;
	}
	
	return 0;
}
