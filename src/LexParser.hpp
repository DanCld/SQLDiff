/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifndef LEXPARSER_HPP
#define LEXPARSER_HPP

#include <istream>

#include "SQLParserHelper.hpp"

namespace sqlfileparser
{

/* returning a smart pointer reference
*/

	SQLTableListManagerPtr& lexParse(std::istream& input, bool skipModifiedTimestamps = false);

} // namespace

#endif
