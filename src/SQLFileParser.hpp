/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifndef SQLFILEPARSER_HPP
#define SQLFILEPARSER_HPP

#include <ostream>

#include "SQLParserHelper.hpp"

namespace sqlfileparser
{

typedef std::string TableDropCommands;
typedef std::map<std::string, std::string> TableCommandsMap;
typedef std::map<std::string, std::string> FieldCommand;
typedef std::map<std::string, FieldCommand> FieldCommandsMap;
typedef std::map<std::string, std::string> FieldDropCommandsMap;
typedef std::map<std::string, std::string> KeyCommandsMap;

class SQLFileParser {

	public:

		SQLFileParser(const SQLTableListManagerPtr& psm1, const SQLTableListManagerPtr& psm2);

		void print(std::ostream& out);

	private:

		void parseTables();

		void parseFields(const SQLTable& ref1, const SQLTable& ref2);

		void parsePrimary(const SQLTable& ref1, const SQLTable& ref2);

		void parseForeign(const SQLTable& ref1, const SQLTable& ref2);

		void parseIndex(const SQLTable& ref1, const SQLTable& ref2);

		void parseUnique(const SQLTable& ref1, const SQLTable& ref2);

		void parseFullText(const SQLTable& ref1, const SQLTable& ref2);

		void parseSpatial(const SQLTable& ref1, const SQLTable& ref2);


/* these are just mysql query printers */

		void printCreateTableCommand(const SQLTable& ref);

		void printDropTableCommand(const SQLTable& ref);

		void printAlterModifyCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield);

		void printAlterDropCommand(const SQLTable& ref, const std::string& rfield);

		void printAlterAddCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield);

		void printAlterDropPrimaryCommand(const SQLTable& ref);

		void printAlterAddPrimaryCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterDropForeignCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterAddForeignCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterDropIndexCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterAddIndexCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterDropUniqueCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterAddUniqueCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterDropFullTextCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterAddFullTextCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterDropSpatialCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);

		void printAlterAddSpatialCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc);


/* these data structures must be allocated and initialized outside this class;
   please note that we keep shared pointers to them
*/

		const SQLTableListManagerPtr psm1_, psm2_;

		TableCommandsMap tableCommands_;

		TableDropCommands tableDropCommands_;

		FieldCommandsMap fieldCommands_;

		FieldDropCommandsMap fieldDropCommands_;

		KeyCommandsMap keyCommands_;

};

} // namespace

#endif

