#include "SQLParserHelper.hpp"
#include "LexParser.hpp"
#include "SQLFileParser.hpp"

using namespace sqlfileparser;

int main()
{
	const SQLTableListManager& sm = lexParse();
	sm.print();

	return 0;
}
