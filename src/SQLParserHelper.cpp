/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include <stdexcept>
#include <algorithm>

#include "SQLParserHelper.hpp"

namespace sqlfileparser
{

int
SQLTable::operator<(const SQLTable& other) const
{
	return name < other.name;
}

void
SQLTable::clear()
{
	name.clear();
	tabletype.clear();
	fields.clear();
	indexedfields.clear();
	primary.clear();
	foreign.clear();
	index.clear();
}

void
SQLTable::print(std::ostream& out) const
{
	out << "TABLE: " << name << std::endl;

	for(TableNodeList::const_iterator fit = fields.begin(); fit != fields.end(); ++fit)
	{
		out << "FIELD: " << *fit << " " << indexedfields.at(*fit) << std::endl;
	}

	for(TableIndexList::const_iterator pit = primary.begin(); pit != primary.end(); ++pit)
	{
		out << "PRIMARY: " << pit->first << std::endl;
	}

	for(TableIndexList::const_iterator oit = foreign.begin(); oit != foreign.end(); ++oit)
	{
		out << "FOREIGN: " << oit->first << std::endl;
	}

	for(TableIndexList::const_iterator iit = index.begin(); iit != index.end(); ++iit)
	{
		out << "INDEX: " << iit->first << std::endl;
	}

	out << "TYPE: " << tabletype << std::endl;
}

SQLTableListManager::SQLTableListManager()
:tlist_(),
temptable_(),
tempfield_(),
tempconstraint_(),
tempcontents_(),
lastState_(DUMMY)
{
}

void
SQLTableListManager::addNewTable(const std::string& tname)
{
	temptable_.name.assign(tname);
}

void
SQLTableListManager::commitTable()
{
	tlist_.insert(temptable_);
	rawtlist_.push_back(temptable_);

	temptable_.clear();
}

void
SQLTableListManager::addNewField(const std::string& tfield)
{
	tempfield_.assign(tfield);
	lastState_ = FIELD;
	tempcontents_.clear();
}

void
SQLTableListManager::setState(MgrState state)
{
	lastState_ = state;
	tempcontents_.clear();
}

void
SQLTableListManager::addPrimaryKeyFromField()
{
	temptable_.primary.insert(std::make_pair<std::string, std::string>("(" + tempfield_ + ")", ""));
}

void
SQLTableListManager::commit()
{
	std::string::size_type last=tempcontents_.find_last_not_of(' ');
	tempcontents_.assign(tempcontents_.substr(0, last + 1));

	std::transform(tempcontents_.begin(), tempcontents_.end(), tempcontents_.begin(), ::tolower);

	switch(lastState_)
	{
		case FIELD:
		{
			commitField();
			break;
		}
		case PRIMARY:
		{
			commitPrimary();
			break;
		}
		case FOREIGN:
		{
			commitForeign();
			break;
		}
		case INDEX:
		{
			commitIndex();
			break;
		}
		default:
		{
			throw std::logic_error("SQLTableListManager:commit() called on DUMMY last state!");
		}
	}
	lastState_ = DUMMY;
	tempconstraint_.clear();
}

void
SQLTableListManager::addTableType()
{
	temptable_.tabletype.assign(tempcontents_);
}

void
SQLTableListManager::commitField()
{
	temptable_.fields.push_back(tempfield_);
	temptable_.indexedfields.insert(std::make_pair<std::string,std::string>(tempfield_, tempcontents_));
}

void
SQLTableListManager::commitPrimary()
{
	temptable_.primary.insert(std::make_pair<std::string, std::string>(tempcontents_, tempconstraint_));
}

void 
SQLTableListManager::commitForeign()
{
	temptable_.foreign.insert(std::make_pair<std::string, std::string>(tempcontents_, tempconstraint_));
}

void
SQLTableListManager::commitIndex()
{
	temptable_.index.insert(std::make_pair<std::string, std::string>(tempcontents_, ""));
}

void
SQLTableListManager::clear()
{
	tlist_.clear();
	rawtlist_.clear();
	temptable_.clear();
	tempfield_.clear();
	tempconstraint_.clear();
	tempcontents_.clear();
	lastState_ = DUMMY;
}

void
SQLTableListManager::print(std::ostream& out) const
{
	out << "---------------------------------" << std::endl;
	for(SQLTableList::const_iterator it = tlist_.begin() ; it != tlist_.end() ; ++it)
	{
		it->print(out);
		out << "---------------------------------" << std::endl;
	}
}

} //namespace
