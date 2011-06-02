/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include <sstream>
#include <string>

#include "SQLFileParser.hpp"

namespace sqlfileparser
{

SQLFileParser::SQLFileParser(const SQLTableListManager& sm1, const SQLTableListManager& sm2)
:sm1_(sm1),
sm2_(sm2),
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
	for(SQLTableRawList::const_iterator it = sm2_.rawtlist().begin() ; it != sm2_.rawtlist().end() ; ++it)
	{
		TableCommandsMap::const_iterator tit = tableCommands_.find(it->name);
		if (tit != tableCommands_.end())
		{
			out << tit->second;
			continue;
		}

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

		KeyCommandsMap::const_iterator pit = keyCommands_.find(it->name);
		if (pit != keyCommands_.end())
		{
			out << pit->second;
		}

		FieldDropCommandsMap::const_iterator fdit = fieldDropCommands_.find(it->name);
		if (fdit != fieldDropCommands_.end())
		{
			out << fdit->second;
		}
	}

	out << tableDropCommands_;
}

void
SQLFileParser::parseTables()
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

		fieldCommands_.insert(std::make_pair<std::string, FieldCommand>(v1_it->name, FieldCommand()));
		fieldDropCommands_.insert(std::make_pair<std::string, std::string>(v1_it->name, std::string()));
		parseFields(*(v1_it), *(v2_it));

		keyCommands_.insert(std::make_pair<std::string, std::string>(v1_it->name, std::string()));
		parsePrimary(*(v1_it), *(v2_it));
		parseForeign(*(v1_it), *(v2_it));
		parseIndex(*(v1_it), *(v2_it));

		v1_it++;
		v2_it++;
	}
}

void
SQLFileParser::parseFields(const SQLTable& ref1, const SQLTable& ref2)
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

		if ( *fit1 < *fit2 )
		{
			printAlterDropForeignCommand(ref1, *fit1);
			fit1++;
			continue;
		}

		if ( *fit1 > *fit2 )
		{
			printAlterAddForeignCommand(ref2, *fit2);
			fit2++;
			continue;
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
			printAlterDropIndexCommand(ref1, fit1->first);
			fit1++;
			continue;
		}

		if ( fit1 == ref1.index.end() )
		{
			printAlterAddIndexCommand(ref2, fit2->first);
			fit2++;
			continue;
		}

		if ( *fit1 < *fit2 )
		{
			printAlterDropIndexCommand(ref1, fit1->first);
			fit1++;
			continue;
		}

		if ( *fit1 > *fit2 )
		{
			printAlterAddIndexCommand(ref2, fit2->first);
			fit2++;
			continue;
		}

		fit1++;
		fit2++;
	}
}

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
			<< "index " << iit->first
			<< ",";
	}

	std::string tmpbuf(ostr_.str());
	tmpbuf.erase(tmpbuf.length()-1, std::string::npos);

	mstr_ << "(" << tmpbuf << std::endl << ") "
		<< ref.tabletype << ";"
		<< std::endl << std::endl;

	tableCommands_.insert(std::make_pair<std::string, std::string>(ref.name, mstr_.str()));
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

	fieldCommands_.at(ref.name).insert(std::make_pair<std::string, std::string>(rfield.first, mstr_.str()));
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

	fieldCommands_.at(ref.name).insert(std::make_pair<std::string, std::string>(rfield.first, mstr_.str()));
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
		mstr_ << " alter table " << ref.name
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
SQLFileParser::printAlterDropIndexCommand(const SQLTable& ref, const std::string& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " drop index " << desc << ";"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

void
SQLFileParser::printAlterAddIndexCommand(const SQLTable& ref, const std::string& desc)
{
	std::ostringstream mstr_;

	mstr_ << "alter table " << ref.name
		<< " add index " << desc << ";"
		<< std::endl << std::endl;

	keyCommands_.at(ref.name).append(mstr_.str());
}

} //namespace
