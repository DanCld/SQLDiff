%{
/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

/* this file will be expanded into a .cpp one by the flex program
*/

#include "SQLParserHelper.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

using namespace sqlfileparser;

/* the static (pre-allocated on heap) variable should always be declared in the .cpp;
   we must ensure that is allocated once and initialized (cleared) before every use;
   we must return it outside this module by the means of a reference to prevent extra
   copying
*/

static unsigned long line;
static SQLTableListManager sm;
static int parantLevel;
static bool wasInt;
static bool skipTimestamps;
static bool lastFieldTimestamp;

%}

%option c++ noyywrap

%x TABLENAME
%x TABLEFIELD
%x FCONSTRAINT
%x FDEFINITION
%x FDEFINITIONP
%x FDEFINITIONS1
%x FDEFINITIONS2
%x SKIPPAR
%x SKIPLINE
%x SKIPLINEP
%x SKIPLINES

%x ENDTABLE

alpha	[a-zA-Z][a-zA-Z0-9_]*
alphaext	[a-zA-Z0-9_]+
alphaexteq	[a-zA-Z0-9_`=]+
dtime [1-9][0-9]{3}-[0-9]{2}-[0-9]{2}[ ][0-9]{2}\:[0-9]{2}\:[0-9]{2}
sep	[ \t]+
csep	[ \t]*

%%

(?i:create{sep}table) { BEGIN TABLENAME; }
[\r]+ { }
\n { line++; }
. { }

<TABLENAME>(?i:if{sep}not{sep}exists) { }
<TABLENAME>\`{alpha}\` { sm.addNewTable(yytext); }
<TABLENAME>{alpha} { sm.addNewTable(yytext); }
<TABLENAME>\( { wasInt = false; lastFieldTimestamp = false; BEGIN TABLEFIELD; }
<TABLENAME>\) {
	std::ostringstream linestr;
	linestr << line;
	throw std::runtime_error("Unexpected character \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context TABLENAME");
}

<TABLENAME>[\r]+ { }
<TABLENAME>\n { line++; }
<TABLENAME>. { }

<TABLEFIELD>(?i:constraint) { BEGIN FCONSTRAINT; }
<TABLEFIELD>(?i:primary{sep}key) { sm.setState(PRIMARY); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:foreign{sep}key) { sm.setState(FOREIGN); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:key{csep}) { sm.setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:index{csep}) { sm.setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:unique{csep}) { sm.setState(UNIQUE); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:fulltext{csep}) { sm.setState(FULLTEXT); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:spatial{csep}) { sm.setState(SPATIAL); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:check{csep}) {
	std::ostringstream linestr;
	linestr << line;
	std::cerr << "WARNING: check ignored (table: \"" << sm.tempTable() << "\", line " << linestr.str() << ")" << std::endl;
	BEGIN SKIPLINE;
}
<TABLEFIELD>\`{alpha}\` { sm.addNewField(yytext); wasInt = false; lastFieldTimestamp = false; BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { sm.addNewField(yytext); wasInt = false; lastFieldTimestamp = false; BEGIN FDEFINITION; }
<TABLEFIELD>{sep} { }
<TABLEFIELD>[\r]+ { }
<TABLEFIELD>\n { line++; }
<TABLEFIELD>. {
	std::ostringstream linestr;
	linestr << line; 
	throw std::runtime_error("Unexpected character \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context TABLEFIELD");
}

<FCONSTRAINT>(?i:primary{sep}key) { sm.setState(PRIMARY); BEGIN FDEFINITION; }
<FCONSTRAINT>(?i:foreign{sep}key) { sm.setState(FOREIGN); BEGIN FDEFINITION; }
<FCONSTRAINT>(?i:unique{csep}) { sm.setState(UNIQUE); BEGIN FDEFINITION; }
<FCONSTRAINT>\`{alpha}\` { 
	if (sm.tempConstraint().size() > 0)
	{
		std::ostringstream linestr;
		linestr << line; 
		throw std::runtime_error("Unexpected token \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context FCONSTRAINT");
	}
	sm.tempConstraint().assign(yytext);
}
<FCONSTRAINT>{alpha} { 
	if (sm.tempConstraint().size() > 0)
	{
		std::ostringstream linestr;
		linestr << line; 
		throw std::runtime_error("Unexpected token \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context FCONSTRAINT");
	}
	sm.tempConstraint().assign(yytext);
}
<FCONSTRAINT>{sep} { }
<FCONSTRAINT>[\r]+ { }
<FCONSTRAINT>\n { line++; }
<FCONSTRAINT>. {
	std::ostringstream linestr;
	linestr << line;
	throw std::runtime_error("Unexpected character \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context FCONSTRAINT");
}

<FDEFINITION>(?i:key{csep}) { }
<FDEFINITION>(?i:default{sep}null{csep}) { }
<FDEFINITION>(?i:default{sep}\'\'{csep}) { }
<FDEFINITION>(?i:primary{sep}key{csep}) { sm.addPrimaryKeyFromField(); }
<FDEFINITION>(?i:not{sep}null{csep}) { sm.tempModifier().assign("not null"); }
<FDEFINITION>(?i:null{csep}) { }
<FDEFINITION>int{csep}\( { sm.tempContents().append("int"); wasInt = true; BEGIN SKIPPAR; }
<FDEFINITION>smallint{csep}\( { sm.tempContents().append("smallint"); wasInt = true; BEGIN SKIPPAR; }
<FDEFINITION>bigint{csep}\( { sm.tempContents().append("bigint"); wasInt = true; BEGIN SKIPPAR; }
<FDEFINITION>tinyint{csep}\( { sm.tempContents().append("tinyint"); wasInt = true; BEGIN SKIPPAR; }
<FDEFINITION>text{csep}\( { sm.tempContents().append("text"); BEGIN SKIPPAR; }
<FDEFINITION>double { sm.tempContents().append(yytext); wasInt = true; }
<FDEFINITION>boolean { sm.tempContents().append("tinyint"); }
<FDEFINITION>false { sm.tempContents().append("0"); }
<FDEFINITION>true { sm.tempContents().append("1"); }
<FDEFINITION>{dtime} {
	if (!skipTimestamps)
	{
		std::ostringstream linestr;
		linestr << line;
		std::cerr << "WARNING: datetime initializers may be adjusted to the MySQL time zone and appear as differences between versions! (line " + linestr.str() + ", '" << yytext << "')" << std::endl;
	}
	sm.tempContents().append(yytext);
	lastFieldTimestamp = true;
}
<FDEFINITION>{alphaext} {
/* convert field definition wording to lowercase, except when the wording is between quotes */
	std::string tmp_yytext(yytext);
	std::transform(tmp_yytext.begin(), tmp_yytext.end(), tmp_yytext.begin(), ::tolower);
	sm.tempContents().append(tmp_yytext);
}
<FDEFINITION>\( {
	if (sm.tempContents().size() > 0 && sm.tempContents().at(sm.tempContents().size()-1) != ' ') sm.tempContents() += ' ';
	sm.tempContents().append(yytext);  BEGIN FDEFINITIONP;
}
<FDEFINITION>{csep}, {
	if (skipTimestamps && lastFieldTimestamp)
	{
		sm.scrapCommit();
		lastFieldTimestamp = false;
	}
	else
	{
		sm.commit();
	}
	BEGIN TABLEFIELD;
}
<FDEFINITION>{sep} { if (sm.tempContents().size() > 0) sm.tempContents().append(" "); }
<FDEFINITION>\) { 
	if (skipTimestamps && lastFieldTimestamp)
	{
		sm.scrapCommit();
		lastFieldTimestamp = false;
	}
	else
	{
		sm.commit();
	}
	sm.tempContents().clear();
	BEGIN ENDTABLE;
}
<FDEFINITION>` { }
<FDEFINITION>[\'] {
	if (!wasInt)
	{
		sm.tempContents().append(yytext);
		BEGIN FDEFINITIONS1;
	}
}
<FDEFINITION>[\"] {
	if (!wasInt)
	{
		sm.tempContents().append(yytext);
		BEGIN FDEFINITIONS2;
	}
}
<FDEFINITION>[\r]+ { }
<FDEFINITION>\n { line++; }
<FDEFINITION>. { }

<FDEFINITIONP>\( { parantLevel++; }
<FDEFINITIONP>\) {
	if (parantLevel == 0)
	{
		sm.tempContents().append(yytext);
		BEGIN FDEFINITION;
	}
	else parantLevel--;
}
<FDEFINITIONP>,{csep} { if (parantLevel == 0) sm.tempContents().append(","); }
<FDEFINITIONP>[\r]+ { }
<FDEFINITIONP>\n { line++; }
<FDEFINITIONP>` { }
<FDEFINITIONP>. { if (parantLevel == 0) sm.tempContents().append(yytext); }

<FDEFINITIONS1>[\'] { sm.tempContents().append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS1>{dtime} {
	if (!skipTimestamps)
	{
		std::ostringstream linestr;
		linestr << line;
		std::cerr << "WARNING: datetime initializers may be adjusted to the MySQL time zone and appear as differences between versions! (line " + linestr.str() + ", '" << yytext << "')" << std::endl;
	}
	lastFieldTimestamp = true;
	sm.tempContents().append(yytext);
}
<FDEFINITIONS1>[\r]+ { }
<FDEFINITIONS1>\n { line++; }
<FDEFINITIONS1>. { sm.tempContents().append(yytext); }

<FDEFINITIONS2>[\"] { sm.tempContents().append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS2>{dtime} {
	if (!skipTimestamps)
	{
		std::ostringstream linestr;
		linestr << line;
		std::cerr << "WARNING: datetime initializers may be adjusted to the MySQL time zone and appear as differences between versions! (line " + linestr.str() + ", '" << yytext << "')" << std::endl;
	}
	lastFieldTimestamp = true;
	sm.tempContents().append(yytext);
}
<FDEFINITIONS2>[\r]+ { }
<FDEFINITIONS2>\n { line++; }
<FDEFINITIONS2>. { sm.tempContents().append(yytext); }

<ENDTABLE>(?i:create{sep}table) {
	std::ostringstream linestr;
	linestr << line;
	throw std::runtime_error("Unexpected token \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context ENDTABLE (missing \";\" ?)");
}
<ENDTABLE>; { sm.addTableType(); sm.commitTable(); BEGIN INITIAL; }
<ENDTABLE>{alphaexteq} { sm.tempContents().append(yytext); }
<ENDTABLE>{sep} { if (sm.tempContents().size() > 0) sm.tempContents().append(" "); }
<ENDTABLE>[\r]+ { }
<ENDTABLE>\n { line++; }
<ENDTABLE>. { }

<SKIPPAR>\) { BEGIN FDEFINITION; }
<SKIPPAR>[\r]+ { }
<SKIPPAR>\n { line++; }
<SKIPPAR>. { }

<SKIPLINE>\( { BEGIN SKIPLINEP; }
<SKIPLINE>[\'\"] { BEGIN SKIPLINES; }
<SKIPLINE>, { BEGIN TABLEFIELD; }
<SKIPLINE>\) { BEGIN ENDTABLE; }
<SKIPLINE>[\r]+ { }
<SKIPLINE>\n { line++; }
<SKIPLINE>. { }

<SKIPLINEP>\) { BEGIN SKIPLINE; }
<SKIPLINEP>[\r]+ { }
<SKIPLINEP>\n { line++; }
<SKIPLINEP>. { }

<SKIPLINES>[\'\"] { BEGIN SKIPLINE; }
<SKIPLINES>[\r]+ { }
<SKIPLINES>\n { line++; }
<SKIPLINES>. { }

%%

namespace sqlfileparser
{

const SQLTableListManager& lexParse(std::istream& input, bool skipModifiedTimestamps)
{
	parantLevel = 0;
	wasInt = false;
	line = 1;
	sm.clear();
	skipTimestamps = skipModifiedTimestamps;
	lastFieldTimestamp = false;

	yyFlexLexer lex(&input);
	while (lex.yylex()) { }

	return sm;
}

} //namespace
