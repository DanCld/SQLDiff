/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include <fstream>
#include <iostream>

#include "LexParser.hpp"
#include "SQLFileParser.hpp"

using namespace sqlfileparser;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " version1.sql version2.sql [ upgrade.sql ]" << std::endl;
		return -1;
	}

	std::ifstream inp1(argv[1]), inp2(argv[2]);

	if (!inp1.good())
	{
		std::cout << "Cannot open file " << argv[1] << " for reading." << std::endl;
		return -2;
	}

	if (!inp2.good())
	{
		std::cout << "Cannot open file " << argv[2] << " for reading." << std::endl;
		return -3;
	}

/* performing 2 local copies of the static variable in the lex parsing module;
   we're not modifying them so the "good practice" says we should keep them
   const; if we were not copying them then the main class would be "fed" with
   2 references to the same data structure
*/

	const SQLTableListManager sm1(lexParse(inp1));
	const SQLTableListManager sm2(lexParse(inp2));

	SQLFileParser sqlParser(sm1, sm2);

	if (argc == 4)
	{
		std::ofstream out;
		out.open(argv[3]);
		if (!out.good())
		{
			std::cout << "Cannot open file " << argv[3] << " for writing." << std::endl;
			return -4;
		}
		sqlParser.print(out);
	}
	else
	{
		sqlParser.print(std::cout);
	}

	return 0;
}
