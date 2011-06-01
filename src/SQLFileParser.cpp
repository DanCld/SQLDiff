/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include <iostream>
#include <sstream>
#include <string>

#include "SQLFileParser.hpp"

namespace sqlfileparser
{

SQLFileParser::SQLFileParser(const SQLTableListManager& sm1, const SQLTableListManager& sm2, std::ostream& out)
:sm1_(sm1),
sm2_(sm2),
out_(out)
{
}

void
SQLFileParser::parseTables() const
{
	SQLTableList::const_iterator v1_it = sm1_.tlist().begin();
	SQLTableList::const_iterator v2_it = sm2_.tlist().begin();

	while( v1_it != sm1_.tlist().end() || v2_it != sm2_.tlist().end() )
	{
		if ( v2_it == sm2_.tlist().end() )
		{
			printDropTableCommand(*v1_it);
			v1_it++;
			continue;
		}

		if ( v1_it == sm1_.tlist().end() )
		{
			printCreateTableCommand(*v2_it);
			v2_it++;
			continue;
		}

		if ( v1_it->name < v2_it->name )
		{
			printDropTableCommand(*v1_it);
			v1_it++;
			continue;
		}

		if ( v1_it->name > v2_it->name )
		{
			printCreateTableCommand(*v2_it);
			v2_it++;
			continue;
		}

		parseFields(*(v1_it++), *(v2_it++));
	}
}

void
SQLFileParser::parseFields(const SQLTable& ref1, const SQLTable& ref2) const
{
	TableNodeMap::const_iterator fit1 = ref1.indexedfields.begin();
	TableNodeMap::const_iterator fit2 = ref2.indexedfields.begin();

	while( fit1 != ref1.indexedfields.end() || fit2 != ref2.indexedfields.end() )
	{
		if ( fit2 == ref2.indexedfields.end() )
		{
			printAlterDropCommand(ref1, fit1->first);
			fit1++;
			continue;
		}

		if ( fit1 == ref1.indexedfields.end() )
		{
			printAlterAddCommand(ref2, *fit2);
			fit2++;
			continue;
		}

		if ( fit1->first < fit2->first )
		{
			printAlterDropCommand(ref1, fit1->first);
			fit1++;
			continue;
		}

		if ( fit1->first > fit2->first )
		{
			printAlterAddCommand(ref2, *fit2);
			fit2++;
			continue;
		}

		if ( fit1->second != fit2->second )
		{
			printAlterModifyCommand(ref2, *fit2);
		}

		fit1++;
		fit2++;
	}

}

void
SQLFileParser::printCreateTableCommand(const SQLTable& ref) const
{
	out_ << "create table " << ref.name << std::endl;

	std::ostringstream ostr_;
	
	for(TableNodeList::const_iterator fit = ref.fields.begin(); fit != ref.fields.end(); ++fit)
	{
		ostr_ << std::endl << "\t" << *fit << " " << ref.indexedfields.at(*fit) << ",";
	}
	
	for(TableIndexList::const_iterator pit = ref.primary.begin(); pit != ref.primary.end(); ++pit)
	{
		ostr_ << std::endl << "\t" << "primary key " << *pit << ",";
	}
	
	for(TableIndexList::const_iterator oit = ref.foreign.begin(); oit != ref.foreign.end(); ++oit)
	{
		ostr_ << std::endl << "\t" << "foreign key " << *oit << ",";
	}
	
	for(TableIndexList::const_iterator iit = ref.index.begin(); iit != ref.index.end(); ++iit)
	{
		ostr_ << std::endl << "\t" << "index " << *iit << ",";
	}

	std::string tmpbuf(ostr_.str());
	tmpbuf.erase(tmpbuf.length()-1, std::string::npos);
	out_ << "(" << tmpbuf << std::endl << ") type " << ref.tabletype << ";" << std::endl << std::endl;
}

void
SQLFileParser::printDropTableCommand(const SQLTable& ref) const
{
	out_ << "drop table " << ref.name << ";" << std::endl << std::endl;
}

void
SQLFileParser::printAlterModifyCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield) const
{
	out_ << "alter table " << ref.name << " modify column " << rfield.first << " " << rfield.second << ";" << std::endl << std::endl;
}

void
SQLFileParser::printAlterDropCommand(const SQLTable& ref, const std::string& rfield) const
{
	out_ << "alter table " << ref.name << " drop column " << rfield << ";" << std::endl << std::endl;
}

void
SQLFileParser::printAlterAddCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield) const
{
	std::string previous;
	for(TableNodeList::const_iterator fit = ref.fields.begin() ; fit != ref.fields.end() ; ++fit)
	{
		if (rfield.first == *fit) break;
		previous.assign(*fit);
	}

	std::string tmpbuf;
	if (previous.size() > 0) tmpbuf.assign("after " + previous);
	else tmpbuf.assign("first");
	
	out_ << "alter table " << ref.name << " add column " << rfield.first << " " << rfield.second <<  " " << tmpbuf << ";" << std::endl << std::endl;
}


} //namespace
