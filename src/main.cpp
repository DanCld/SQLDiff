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

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 3)
		{
			throw std::runtime_error("too few parameters; usage: " + std::string(argv[0]) + " version1.sql version2.sql [ upgrade.sql ]");
		}

		std::ifstream inp1(argv[1]), inp2(argv[2]);

		if (!inp1.good())
		{
			throw std::runtime_error("cannot open file " + std::string(argv[1]) + " for reading.");
		}

		if (!inp2.good())
		{
			throw std::runtime_error("cannot open file " + std::string(argv[2]) + " for reading.");
		}

	/* performing 2 local copies of the static variable in the lex parsing module;
		we're not modifying them so the "good practice" says we should keep them
		const; if we were not copying them then the main class would be "fed" with
		2 references to the same data structure
	*/

		const SQLTableListManager sm1(lexParse(inp1));
		const SQLTableListManager sm2(lexParse(inp2));

#ifdef DEBUG

		std::ofstream debug1("debug1.txt");
		sm1.print(debug1);

		std::ofstream debug2("debug2.txt");
		sm2.print(debug2);

#endif

		SQLFileParser sqlParser(sm1, sm2);

		if (argc == 4)
		{
			std::ofstream out;
			out.open(argv[3]);
			if (!out.good())
			{
				throw std::runtime_error("cannot open file " + std::string(argv[3]) + " for writing.");
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
