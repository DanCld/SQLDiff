/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#ifndef SQLPARSERHELPER_HPP
#define SQLPARSERHELPER_HPP

#include <set>
#include <deque>
#include <map>
#include <string>
#include <ostream>
#include <memory>

namespace sqlfileparser
{

typedef std::deque<std::string> TableNodeList;
typedef std::map<std::string, std::string> TableNodeMap;
typedef std::set<std::pair<std::string, std::string> > TableIndexList;

struct SQLTable {

	std::string name, tabletype;

/* the fields are kept into 2 data structures; the first one is indexed
   for the use of the algorythm while the second is used for the output
   ordering
*/
	TableNodeList fields;

	TableNodeMap indexedfields;

/* the order isn't relevant for constraints / keys / indexes so any
   output order will do
*/

	TableIndexList primary, foreign, noindex, index, unique, fulltext, spatial;

/* we can't put the struct into an indexed container without providing
   a comparison operator
*/

	int operator<(const SQLTable& other) const;

	void clear();

	void print(std::ostream&) const;
};

typedef std::deque<SQLTable> SQLTableRawList;
typedef std::set<SQLTable> SQLTableList;

enum MgrState {
	DUMMY = 0,
	FIELD,
	PRIMARY,
	FOREIGN,
	INDEX,
	UNIQUE,
	FULLTEXT,
	SPATIAL
};

class SQLTableListManager
{
	public:

		SQLTableListManager();

		void addNewTable(const std::string& tname);

		void commitTable();

		void addNewField(const std::string& tfield);

		void setState(MgrState state);
		
		const MgrState getState() const { return lastState_; }

		void addPrimaryKeyFromField();

		void commit();

		void scrapCommit();

		void addTableType();

		void clear();

		void print(std::ostream& out) const;

/* the "good practice" says that we should export private members
   through public methods so we can control the access policy
*/

		const SQLTableList& tlist() const { return tlist_; }

		const SQLTableRawList& rawtlist() const { return rawtlist_; }

		const std::string& tempTable() const { return temptable_.name; }

		std::string& tempConstraint() { return tempconstraint_; }
		
		std::string& tempContents() { return tempcontents_; }

		std::string& tempModifier() { return fieldmodifier_; }

	private:

		void commitField();

		void commitPrimary();

		void commitForeign();

		void commitIndex();

		void commitUnique();

		void commitFulltext();

		void commitSpatial();

		SQLTableList tlist_;

		SQLTableRawList rawtlist_;

		SQLTable temptable_;

		std::string tempfield_;

		std::string tempconstraint_;

		std::string tempcontents_;

		std::string fieldmodifier_;

		MgrState lastState_;
};

typedef std::shared_ptr<SQLTableListManager> SQLTableListManagerPtr;

} // namespace

#endif
