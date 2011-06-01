/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifndef SQLFILEPARSER_HPP
#define SQLFILEPARSER_HPP

#include <ostream>
#include "SQLParserHelper.hpp"

namespace sqlfileparser
{

typedef std::map<std::string, std::string> TableCommandsMap;
typedef std::map<std::string, std::string> FieldCommand;
typedef std::map<std::string, FieldCommand> FieldCommandsMap;
typedef std::map<std::string, std::string> KeyCommandsMap;

class SQLFileParser {

	public:

		SQLFileParser(const SQLTableListManager& sm1, const SQLTableListManager& sm2, std::ostream& out);

		void parseTables();

	private:

		void parseFields(const SQLTable& ref1, const SQLTable& ref2);

		void parsePrimary(const SQLTable& ref1, const SQLTable& ref2);

		void parseForeign(const SQLTable& ref1, const SQLTable& ref2);

		void parseIndex(const SQLTable& ref1, const SQLTable& ref2);

		void printCreateTableCommand(const SQLTable& ref);

		void printDropTableCommand(const SQLTable& ref);

		void printAlterModifyCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield);

		void printAlterDropCommand(const SQLTable& ref, const std::string& rfield);

		void printAlterAddCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield);

		void printAlterDropPrimaryCommand(const SQLTable& ref);

		void printAlterAddPrimaryCommand(const SQLTable& ref, const std::string& desc);

		void printAlterDropForeignCommand(const SQLTable& ref, const std::string& desc);

		void printAlterAddForeignCommand(const SQLTable& ref, const std::string& desc);

		void printAlterDropIndexCommand(const SQLTable& ref, const std::string& desc);

		void printAlterAddIndexCommand(const SQLTable& ref, const std::string& desc);

		const SQLTableListManager &sm1_, &sm2_;

		TableCommandsMap tableCommands_;

		FieldCommandsMap fieldCommands_;

		KeyCommandsMap keyCommands_;

		mutable std::ostream& out_;
};

} // namespace

#endif
