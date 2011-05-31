/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include <iostream>

#include "SQLParserHelper.hpp"

namespace sqlfileparser
{

TextScannerHelper::TextScannerHelper()
:buffer_()
{
}

void
TextScannerHelper::addToBuffer(const char* piece)
{
	buffer_.append(piece);
}

void
TextScannerHelper::resetBuffer()
{
	buffer_.clear();
}

int
SQLTable::operator<(const SQLTable& other) const
{
	return name < other.name;
}

void
SQLTable::clear()
{
	name.clear();
	fields.clear();
	primary.clear();
	foreign.clear();
	index.clear();
}

void
SQLTable::print() const
{
	std::cout << "TABLE: " << name << std::endl;
	
	for(TableNodeMap::const_iterator fit = fields.begin(); fit != fields.end(); ++fit)
	{
		std::cout << "FIELD: " << fit->first << " " << fit->second << std::endl;
	}
	
	for(TableIndexList::const_iterator pit = primary.begin(); pit != primary.end(); ++pit)
	{
		std::cout << "PRIMARY: " << *pit << std::endl;
	}
	
	for(TableIndexList::const_iterator oit = foreign.begin(); oit != foreign.end(); ++oit)
	{
		std::cout << "FOREIGN: " << *oit << std::endl;
	}
	
	for(TableIndexList::const_iterator iit = index.begin(); iit != index.end(); ++iit)
	{
		std::cout << "INDEX: " << *iit << std::endl;
	}
}

SQLTableListManager::SQLTableListManager()
:tlist_(),
temptable_(),
tempfield_()
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
	temptable_.clear();
}

void
SQLTableListManager::addNewField(const std::string& tfield)
{
	tempfield_.assign(tfield);
}

void
SQLTableListManager::commitField(const std::string& contents)
{
	temptable_.fields.push_back(std::make_pair<std::string,std::string>(tempfield_, contents));
}

void
SQLTableListManager::commitPrimary(const std::string& contents)
{
	temptable_.primary.insert(contents);
}

void 
SQLTableListManager::commitForeign(const std::string& contents)
{
	temptable_.foreign.insert(contents);
}

void
SQLTableListManager::commitIndex(const std::string& contents)
{
	temptable_.index.insert(contents);
}

void
SQLTableListManager::print() const
{
	std::cout << "---------------------------------" << std::endl;
	for(SQLTableList::const_iterator it = tlist_.begin() ; it != tlist_.end() ; ++it)
	{
		it->print();
		std::cout << "---------------------------------" << std::endl;
	}
}

} //namespace
