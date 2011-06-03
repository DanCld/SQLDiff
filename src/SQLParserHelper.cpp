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
	unique.clear();
	fulltext.clear();
	spatial.clear();
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

	for(TableIndexList::const_iterator uit = unique.begin(); uit != unique.end(); ++uit)
	{
		out << "UNIQUE: " << uit->first << std::endl;
	}

	for(TableIndexList::const_iterator ftit = fulltext.begin(); ftit != fulltext.end(); ++ftit)
	{
		out << "FULLTEXT: " << ftit->first << std::endl;
	}

	for(TableIndexList::const_iterator sit = spatial.begin(); sit != spatial.end(); ++sit)
	{
		out << "SPATIAL: " << sit->first << std::endl;
	}

	out << "TYPE: " << tabletype << std::endl;
}

SQLTableListManager::SQLTableListManager()
:tlist_(),
temptable_(),
tempfield_(),
tempconstraint_(),
tempcontents_(),
fieldmodifier_(),
lastState_(DUMMY)
{
}

void
SQLTableListManager::addNewTable(const std::string& tname)
{
/* cut the ` character
*/

	std::string::size_type first=tname.find_first_not_of('`'), last=tname.find_last_not_of('`');

	temptable_.name.assign(tname.substr(first, last - first + 1));
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
/* remove the ` character and go to lowercase
*/

	std::string::size_type first=tfield.find_first_not_of('`'), last=tfield.find_last_not_of('`');

	tempfield_.assign(tfield.substr(first, last - first + 1));

	std::transform(tempfield_.begin(), tempfield_.end(), tempfield_.begin(), ::tolower);

	lastState_ = FIELD;
	tempcontents_.clear();
	fieldmodifier_.clear();
}

void
SQLTableListManager::setState(MgrState state)
{
	lastState_ = state;
	tempcontents_.clear();
	fieldmodifier_.clear();
}

void
SQLTableListManager::addPrimaryKeyFromField()
{
	temptable_.primary.insert(std::make_pair<std::string, std::string>("(" + tempfield_ + ")", ""));
}

void
SQLTableListManager::commit()
{
	std::string::size_type first=tempcontents_.find_first_not_of(' '), last=tempcontents_.find_last_not_of(' ');
	tempcontents_.assign(tempcontents_.substr(first, last - first + 1));
	
	if (fieldmodifier_.size() > 0)
	{
		tempcontents_.append(" " + fieldmodifier_);
	}
	else
	{
		tempcontents_.append(" null");
	}

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
		case UNIQUE:
		{
			commitUnique();
			break;
		}
		case FULLTEXT:
		{
			commitFulltext();
			break;
		}
		case SPATIAL:
		{
			commitSpatial();
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
	std::transform(tempcontents_.begin(), tempcontents_.end(), tempcontents_.begin(), ::tolower);
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
	
	std::string::size_type first=tempcontents_.find_first_of('('), last=tempcontents_.find_first_of(')');
	std::string indexfield(tempcontents_.substr(first + 1, last - first - 1));

	temptable_.foreign.insert(std::make_pair<std::string, std::string>(tempcontents_, tempconstraint_));

	TableIndexList::iterator it = temptable_.index.find(std::make_pair<std::string, std::string>(indexfield, ""));
	if (it != temptable_.index.end())
	{
		temptable_.index.erase(it);
		temptable_.noindex.insert(std::make_pair<std::string, std::string>(indexfield, ""));
	}
}

void
SQLTableListManager::commitIndex()
{
/* for index name (field) just keep the (field)
*/
	std::string::size_type first=tempcontents_.find_first_of('('), last=tempcontents_.find_last_of(')');
	tempcontents_.assign(tempcontents_.substr(first + 1, last - first - 1));

	TableIndexList::iterator it = temptable_.noindex.find(std::make_pair<std::string, std::string>(tempcontents_, ""));
	if (it == temptable_.index.end())
	{
		temptable_.index.insert(std::make_pair<std::string, std::string>(tempcontents_, ""));
	}
}

void
SQLTableListManager::commitUnique()
{
/* for index name (field) just keep the (field)
*/
	std::string::size_type first=tempcontents_.find_first_of('('), last=tempcontents_.find_last_of(')');
	tempcontents_.assign(tempcontents_.substr(first + 1, last - first - 1));

	temptable_.unique.insert(std::make_pair<std::string, std::string>(tempcontents_, tempconstraint_));
}

void
SQLTableListManager::commitFulltext()
{
/* for index name (field) just keep the (field)
*/
	std::string::size_type first=tempcontents_.find_first_of('('), last=tempcontents_.find_last_of(')');
	tempcontents_.assign(tempcontents_.substr(first + 1, last - first - 1));

	temptable_.fulltext.insert(std::make_pair<std::string, std::string>(tempcontents_, ""));
}

void
SQLTableListManager::commitSpatial()
{
/* for index name (field) just keep the (field)
*/
	std::string::size_type first=tempcontents_.find_first_of('('), last=tempcontents_.find_last_of(')');
	tempcontents_.assign(tempcontents_.substr(first + 1, last - first - 1));

	temptable_.spatial.insert(std::make_pair<std::string, std::string>(tempcontents_, ""));
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
	fieldmodifier_.clear();
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
