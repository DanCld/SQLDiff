/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifndef SQLPARSERHELPER_HPP
#define SQLPARSERHELPER_HPP

#include <set>
#include <deque>
#include <map>
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

		std::string::size_type size() const { return buffer_.size(); }

	private:

		std::string buffer_;

};

typedef std::deque<std::string> TableNodeList;
typedef std::map<std::string, std::string> TableNodeMap;
typedef std::set<std::string> TableIndexList;

struct SQLTable {

	std::string name, tabletype;

	TableNodeList fields;
	
	TableNodeMap indexedfields;
	
	TableIndexList primary, foreign, index;

	int operator<(const SQLTable& other) const;

	void clear();

	void print() const;
};

typedef std::deque<SQLTable> SQLTableRawList;
typedef std::set<SQLTable> SQLTableList;

enum MgrState {
	DUMMY = 0,
	FIELD,
	PRIMARY,
	FOREIGN,
	INDEX
};

class SQLTableListManager
{
	public:

		SQLTableListManager();

		void addNewTable(const std::string& tname);

		void commitTable();

		void addNewField(const std::string& tfield);

		void setState(MgrState state);

		void commit(const std::string& contents);

		void addTableType(const std::string& ttype);

		void clear();

		void print() const;

		const SQLTableList& tlist() const { return tlist_; }

		const SQLTableRawList& rawtlist() const { return rawtlist_; }

	private:

		void commitField(const std::string& contents);

		void commitPrimary(const std::string& contents);

		void commitForeign(const std::string& contents);

		void commitIndex(const std::string& contents);

		SQLTableList tlist_;

		SQLTableRawList rawtlist_;

		SQLTable temptable_;

		std::string tempfield_;

		MgrState lastState_;
};

} // namespace

#endif
