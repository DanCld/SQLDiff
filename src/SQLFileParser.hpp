/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifndef SQLFILEPARSER_HPP
#define SQLFILEPARSER_HPP

#include <ostream>
#include "SQLParserHelper.hpp"

namespace sqlfileparser
{

class SQLFileParser {

	public:

		SQLFileParser(const SQLTableListManager& sm1, const SQLTableListManager& sm2, std::ostream& out);

		void parseTables() const;

	private:

		void parseFields(const SQLTable& ref1, const SQLTable& ref2) const;

		void printCreateTableCommand(const SQLTable& ref) const;

		void printDropTableCommand(const SQLTable& ref) const;

		void printAlterModifyCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield) const;

		void printAlterDropCommand(const SQLTable& ref, const std::string& rfield) const;

		void printAlterAddCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield) const;

		const SQLTableListManager &sm1_, &sm2_;

		mutable std::ostream& out_;
};

} // namespace

#endif
