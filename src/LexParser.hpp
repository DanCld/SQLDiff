/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifndef LEXPARSER_HPP
#define LEXPARSER_HPP

#include <istream>

#include "SQLParserHelper.hpp"

namespace sqlfileparser
{

	const SQLTableListManager& lexParse(std::istream& input);

} // namespace

#endif