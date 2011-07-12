/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifndef LEXPARSER_HPP
#define LEXPARSER_HPP

#include <istream>

#include "SQLParserHelper.hpp"

namespace sqlfileparser
{

/* returning a const reference to a static variable declared in the .cpp file
*/

	const SQLTableListManager& lexParse(std::istream& input, bool skipModifiedTimestamps = false);

} // namespace

#endif
