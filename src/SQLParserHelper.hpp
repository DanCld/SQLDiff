#ifndef SQLPARSERHELPER_HPP
#define SQLPARSERHELPER_HPP

/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include <set>
#include <deque>
#include <string>

namespace sqlfileparser
{

class TextScannerHelper
{
	public:

		TextScannerHelper();

		const std::string& buffer() const { return buffer_; }

		void addToBuffer(const char* piece);

		void resetBuffer();

	private:

		std::string buffer_;

};

typedef std::deque<std::pair<std::string, std::string> > TableNodeMap;
typedef std::set<std::string> TableIndexList;

struct SQLTable {

	std::string name;

	TableNodeMap fields;
	
	TableIndexList primary, foreign, index;

	int operator<(const SQLTable& other) const;

	void clear();

	void print() const;
};

typedef std::set<SQLTable> SQLTableList;

class SQLTableListManager
{
	public:

		SQLTableListManager();

		void addNewTable(const std::string& tname);

		void commitTable();

		void addNewField(const std::string& tfield);
		
		void commitField(const std::string& contents);

		void commitPrimary(const std::string& contents);

		void commitForeign(const std::string& contents);

		void commitIndex(const std::string& contents);

		void print() const;

	private:

		SQLTableList tlist_;

		SQLTable temptable_;

		std::string tempfield_;
};

} // namespace

#endif
