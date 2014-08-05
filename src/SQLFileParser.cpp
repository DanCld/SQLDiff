/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include <sstream>
#include <string>

#include "SQLFileParser.hpp"

namespace sqlfileparser
{

SQLFileParser::SQLFileParser(const SQLTableListManagerPtr& psm1, const SQLTableListManagerPtr& psm2)
:psm1_(psm1),
psm2_(psm2),
tableCommands_(),
tableDropCommands_(),
fieldCommands_(),
fieldDropCommands_(),
keyCommands_()
{
	parseTables();
}

void
SQLFileParser::print(std::ostream& out)
{

/* We generated the result into indexed structures but we need to print everything
   out in the order we received the input (that is the table order in the second .sql file
*/

	for(SQLTableRawList::const_iterator it = psm2_->rawtlist().begin() ; it != psm2_->rawtlist().end() ; ++it)
	{

/* create table statements, if any
*/

		TableCommandsMap::const_iterator tit = tableCommands_.find(it->name);
		if (tit != tableCommands_.end())
		{
			out << tit->second;
			continue;
		}

/* alter table add column / modify column, if any
*/

		FieldCommandsMap::const_iterator fit = fieldCommands_.find(it->name);
		if (fit != fieldCommands_.end())
		{
			for(TableNodeList::const_iterator mit = it->fields.begin() ; mit != it->fields.end() ; ++mit)
			{
				FieldCommand::const_iterator sit = fit->second.find(*mit);
				if (sit != fit->second.end())
				{
					out << sit->second;
				}
			}
		}

/* alter table add constraints, keys, indexes... , if any
   alter table drop constraints, keys, indexes... , if any
*/

		KeyCommandsMap::const_iterator pit = keyCommands_.find(it->name);
		if (pit != keyCommands_.end())
		{
			out << pit->second;
		}

/* alter table drop column, if any (mind the order: first the constraint, then the column!)
*/

		FieldDropCommandsMap::const_iterator fdit = fieldDropCommands_.find(it->name);
		if (fdit != fieldDropCommands_.end())
		{
			out << fdit->second;
		}
	}

/* last: drop table statements
*/

	out << tableDropCommands_;
}

void
SQLFileParser::parseTables()
{
	SQLTableList::const_iterator v1_it = psm1_->tlist().begin();
	SQLTableList::const_iterator v2_it = psm2_->tlist().begin();

/* we go through both indexed structures at once (complexity O(n))
*/

	while( v1_it != psm1_->tlist().end() || v2_it != psm2_->tlist().end() )
	{
		if ( v2_it == psm2_->tlist().end() )
		{
			printDropTableCommand(*v1_it);
			v1_it++;
			continue;
		}

		if ( v1_it == psm1_->tlist().end() )
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

		fieldCommands_.insert(std::make_pair(v1_it->name, FieldCommand()));
		fieldDropCommands_.insert(std::make_pair(v1_it->name, std::string()));
		parseFields(*(v1_it), *(v2_it));

		keyCommands_.insert(std::make_pair(v1_it->name, std::string()));
		parsePrimary(*(v1_it), *(v2_it));
		parseForeign(*(v1_it), *(v2_it));
		parseIndex(*(v1_it), *(v2_it));
		parseUnique(*(v1_it), *(v2_it));
		parseFullText(*(v1_it), *(v2_it));
		parseSpatial(*(v1_it), *(v2_it));

		v1_it++;
		v2_it++;
	}
}

void
SQLFileParser::parseFields(const SQLTable& ref1, const SQLTable& ref2)
{
	TableNodeMap::const_iterator fit1 = ref1.indexedfields.begin();
	TableNodeMap::const_iterator fit2 = ref2.indexedfields.begin();

/* we go through both indexed structures at once (complexity O(n))
*/

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
SQLFileParser::parsePrimary(const SQLTable& ref1, const SQLTable& ref2)
{
	TableIndexList::const_iterator fit1 = ref1.primary.begin();
	TableIndexList::const_iterator fit2 = ref2.primary.begin();

	while( fit1 != ref1.primary.end() || fit2 != ref2.primary.end() )
	{
		if ( fit2 == ref2.primary.end() )
		{
			printAlterDropPrimaryCommand(ref1);
			fit1++;
			continue;
		}

		if ( fit1 == ref1.primary.end() )
		{
			printAlterAddPrimaryCommand(ref2, *fit2);
			fit2++;
			continue;
		}

/* Iterators point to std::pair<std::string, std::string> ; the condition below works if both
   first and second elements of the pair are different. In our situation it may trigger a 
   primary key replacement if the primary key field name changes or the constraint identification
   symbol changes.
*/

		if ( *fit1 != *fit2 )
		{
			printAlterDropPrimaryCommand(ref1);
			printAlterAddPrimaryCommand(ref2, *fit2);
		}

		fit1++;
		fit2++;
	}
}

void
SQLFileParser::parseForeign(const SQLTable& ref1, const SQLTable& ref2)
{
	TableIndexList::const_iterator fit1 = ref1.foreign.begin();
	TableIndexList::const_iterator fit2 = ref2.foreign.begin();

	while( fit1 != ref1.foreign.end() || fit2 != ref2.foreign.end() )
	{
		if ( fit2 == ref2.foreign.end() )
		{
			printAlterDropForeignCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1 == ref1.foreign.end() )
		{
			printAlterAddForeignCommand(ref2, *fit2);
			fit2++;
			continue;
		}

		if ( fit1->first < fit2->first )
		{
			printAlterDropForeignCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1->first > fit2->first )
		{
			printAlterAddForeignCommand(ref2, *fit2);
			fit2++;
			continue;
		}

/* the condition below forces the replacement of the foreign key if the constraint
   identification symbol changes
*/

		if (fit1->first == fit2->first && fit2->second.size() > 0 && fit1->second != fit2->second)
		{
			printAlterDropForeignCommand(ref1, *fit1);
			printAlterAddForeignCommand(ref2, *fit2);
		}

		fit1++;
		fit2++;
	}
}

void
SQLFileParser::parseIndex(const SQLTable& ref1, const SQLTable& ref2)
{
	TableIndexList::const_iterator fit1 = ref1.index.begin();
	TableIndexList::const_iterator fit2 = ref2.index.begin();

	while( fit1 != ref1.index.end() || fit2 != ref2.index.end() )
	{
		if ( fit2 == ref2.index.end() )
		{
			printAlterDropIndexCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1 == ref1.index.end() )
		{
			printAlterAddIndexCommand(ref2, *fit2);
			fit2++;
			continue;
		}

/* for indexes the second part of the pair is not used
*/

		if ( fit1->first < fit2->first )
		{
			printAlterDropIndexCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1->first > fit2->first )
		{
			printAlterAddIndexCommand(ref2, *fit2);
			fit2++;
			continue;
		}

		fit1++;
		fit2++;
	}
}

void
SQLFileParser::parseUnique(const SQLTable& ref1, const SQLTable& ref2)
{
	TableIndexList::const_iterator fit1 = ref1.unique.begin();
	TableIndexList::const_iterator fit2 = ref2.unique.begin();

	while( fit1 != ref1.unique.end() || fit2 != ref2.unique.end() )
	{
		if ( fit2 == ref2.unique.end() )
		{
			printAlterDropUniqueCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1 == ref1.unique.end() )
		{
			printAlterAddUniqueCommand(ref2, *fit2);
			fit2++;
			continue;
		}

		if ( fit1->first < fit2->first )
		{
			printAlterDropUniqueCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1->first > fit2->first )
		{
			printAlterAddUniqueCommand(ref2, *fit2);
			fit2++;
			continue;
		}

/* the condition below forces the replacement of the unique key if the constraint
   identification symbol changes
*/

		if (fit1->first == fit2->first && fit2->second.size() > 0 && fit1->second != fit2->second)
		{
			printAlterDropUniqueCommand(ref1, *fit1);
			printAlterAddUniqueCommand(ref2, *fit2);
		}

		fit1++;
		fit2++;
	}
}

void
SQLFileParser::parseFullText(const SQLTable& ref1, const SQLTable& ref2)
{
	TableIndexList::const_iterator fit1 = ref1.fulltext.begin();
	TableIndexList::const_iterator fit2 = ref2.fulltext.begin();

	while( fit1 != ref1.fulltext.end() || fit2 != ref2.fulltext.end() )
	{
		if ( fit2 == ref2.fulltext.end() )
		{
			printAlterDropFullTextCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1 == ref1.fulltext.end() )
		{
			printAlterAddFullTextCommand(ref2, *fit2);
			fit2++;
			continue;
		}

/* for fulltext indexes the second part of the pair is not used
*/

		if ( fit1->first < fit2->first )
		{
			printAlterDropFullTextCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1->first > fit2->first )
		{
			printAlterAddFullTextCommand(ref2, *fit2);
			fit2++;
			continue;
		}

		fit1++;
		fit2++;
	}
}

void
SQLFileParser::parseSpatial(const SQLTable& ref1, const SQLTable& ref2)
{
	TableIndexList::const_iterator fit1 = ref1.spatial.begin();
	TableIndexList::const_iterator fit2 = ref2.spatial.begin();

	while( fit1 != ref1.spatial.end() || fit2 != ref2.spatial.end() )
	{
		if ( fit2 == ref2.spatial.end() )
		{
			printAlterDropSpatialCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1 == ref1.spatial.end() )
		{
			printAlterAddSpatialCommand(ref2, *fit2);
			fit2++;
			continue;
		}

/* for spatial indexes the second part of the pair is not used
*/

		if ( fit1->first < fit2->first )
		{
			printAlterDropSpatialCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( fit1->first > fit2->first )
		{
			printAlterAddSpatialCommand(ref2, *fit2);
			fit2++;
			continue;
		}

		fit1++;
		fit2++;
	}
}


/* output generators */

void
SQLFileParser::printCreateTableCommand(const SQLTable& ref)
{
	std::ostringstream mstr_;
	
	mstr_ << "create table " << ref.name
		<< std::endl;

	std::ostringstream ostr_;

	for(TableNodeList::const_iterator fit = ref.fields.begin(); fit != ref.fields.end(); ++fit)
	{
		ostr_ << std::endl << "\t"
			<< *fit << " " << ref.indexedfields.at(*fit)
			<< ",";
	}

	for(TableIndexList::const_iterator pit = ref.primary.begin(); pit != ref.primary.end(); ++pit)
	{
		ostr_ << std::endl << "\t"
			<< ((pit->second.size() > 0)?("constraint " + pit->second + " "):"")
			<< "primary key " << pit->first
			<< ",";
	}

	for(TableIndexList::const_iterator oit = ref.foreign.begin(); oit != ref.foreign.end(); ++oit)
	{
		ostr_ << std::endl << "\t"
			<< ((oit->second.size() > 0)?("constraint " + oit->second + " "):"")
			<< "foreign key " << oit->first
			<< ",";
	}

	for(TableIndexList::const_iterator iit = ref.index.begin(); iit != ref.index.end(); ++iit)
	{
		ostr_ << std::endl << "\t"
			<< "index " << ((iit->second.size() > 0)?iit->second + " ":"") << "(" << iit->first << ")"
			<< ",";
	}

	for(TableIndexList::const_iterator uit = ref.unique.begin(); uit != ref.unique.end(); ++uit)
	{
		ostr_ << std::endl << "\t"
			<< "unique " << ((uit->second.size() > 0)?uit->second + " ":"") << "(" << uit->first << ")"
			<< ",";
	}

	for(TableIndexList::const_iterator ftit = ref.fulltext.begin(); ftit != ref.fulltext.end(); ++ftit)
	{
		ostr_ << std::endl << "\t"
			<< "fulltext " << ((ftit->second.size() > 0)?ftit->second + " ":"") << "(" << ftit->first << ")"
			<< ",";
	}

	for(TableIndexList::const_iterator sit = ref.spatial.begin(); sit != ref.spatial.end(); ++sit)
	{
		ostr_ << std::endl << "\t"
			<< "spatial " << ((sit->second.size() > 0)?sit->second + " ":"") << "(" << sit->first << ")"
			<< ",";
	}

	std::string tmpbuf(ostr_.str());
	tmpbuf.erase(tmpbuf.length()-1, std::string::npos);

	mstr_ << "(" << tmpbuf << std::endl << ") "
		<< ref.tabletype << ";"
		<< std::endl << std::endl;

	tableCommands_.insert(std::make_pair(ref.name, mstr_.str()));
}

void
SQLFileParser::printDropTableCommand(const SQLTable& ref)
{
	std::ostringstream mstr_;

	mstr_ << "drop table " << ref.name << ";"
		<< std::endl << std::endl;

	tableDropCommands_.append(mstr_.str());
}

void
SQLFileParser::printAlterModifyCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield)
{
	std::ostringstream mstr_;

	mstr_<< "alter table " << ref.name
		<< " modify column " << rfield.first << " " << rfield.second << ";"
		<< std::endl << std::endl; 

	fieldCommands_.at(ref.name).insert(std::make_pair(rfield.first, mstr_.str()));
}

void
SQLFileParser::printAlterDropCommand(const SQLTable& ref, const std::string& rfield)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " drop column " << rfield << ";"
		<< std::endl << std::endl;

	fieldDropCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterAddCommand(const SQLTable& ref, const std::pair<std::string, std::string>& rfield)
{
	std::ostringstream mstr_;

	std::string previous;
	for(TableNodeList::const_iterator fit = ref.fields.begin() ; fit != ref.fields.end() ; ++fit)
	{
		if (rfield.first == *fit) break;
		previous.assign(*fit);
	}

	std::string tmpbuf;
	if (previous.size() > 0) tmpbuf.assign("after " + previous);
	else tmpbuf.assign("first");
	
	mstr_ << "alter table " << ref.name
		<< " add column " << rfield.first << " " << rfield.second <<  " " << tmpbuf << ";"
		<< std::endl << std::endl;

	fieldCommands_.at(ref.name).insert(std::make_pair(rfield.first, mstr_.str()));
}

void
SQLFileParser::printAlterDropPrimaryCommand(const SQLTable& ref)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " drop primary key;"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterAddPrimaryCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " add"
		<< ((desc.second.size() > 0)?" constraint " + desc.second:"")
		<< " primary key " << desc.first << ";"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterDropForeignCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	if (desc.second.size() > 0)
	{
		mstr_ << "alter table " << ref.name
			<< " drop foreign key " << desc.second << ";"
			<< std::endl << std::endl;
	}
	else
	{
		/* foreign key dropping can't be automatically implemented as it requires an identifier created
		internally by the InnoDB engine */

		mstr_ << "# foreign key dropping can't be automatically implemented as it " << std::endl;
		mstr_ << "# requires an identifier created internally by the InnoDB engine." << std::endl;
		mstr_ << "# alter table " << ref.name
			<< " drop foreign key " << "??fk_symbol??" << "; // description: " << desc.first
			<< std::endl << std::endl;
	}

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterAddForeignCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " add"
		<< ((desc.second.size() > 0)?" constraint " + desc.second:"")
		<< " foreign key " << desc.first << ";"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterDropIndexCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " drop index " << desc.second << ";"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterAddIndexCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " add index " << ((desc.second.size() > 0)?desc.second + " ":"") << "(" << desc.first << ");"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterDropUniqueCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " drop key " << desc.second << ";"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterAddUniqueCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " add unique " << ((desc.second.size() > 0)?desc.second + " ":"") << "(" << desc.first << ");"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterDropFullTextCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " drop key " << desc.second << ";"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterAddFullTextCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " add fulltext " << ((desc.second.size() > 0)?desc.second + " ":"") << "(" << desc.first << ");"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterDropSpatialCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " drop key " << desc.second << ";"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterAddSpatialCommand(const SQLTable& ref, const std::pair<std::string, std::string>& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " add spatial " << ((desc.second.size() > 0)?desc.second + " ":"") << "(" << desc.first << ");"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

} //namespace

