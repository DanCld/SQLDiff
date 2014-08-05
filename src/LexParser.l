%{
/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

/* this file will be expanded into a .cpp by flex
*/

#include "SQLParserHelper.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>


using namespace sqlfileparser;

namespace
{

static SQLTableListManagerPtr psm;

static unsigned long line;
static int parantLevel;
static bool wasInt;
static bool skipTimestamps;
static bool lastFieldTimestamp;

} // anonymous namespace

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
<TABLENAME>\`{alpha}\` { psm->addNewTable(yytext); }
<TABLENAME>{alpha} { psm->addNewTable(yytext); }
<TABLENAME>\( { wasInt = false; lastFieldTimestamp = false; BEGIN TABLEFIELD; }
<TABLENAME>\) {
	std::ostringstream linestr;
	linestr << line;
	throw std::runtime_error("Unexpected character \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context TABLENAME");
}

<TABLENAME>[\r]+ { }
<TABLENAME>\n { line++; }
<TABLENAME>. { }

<TABLEFIELD>(?i:constraint{csep}) { BEGIN FCONSTRAINT; }
<TABLEFIELD>(?i:primary{sep}key{csep}) { psm->setState(PRIMARY); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:foreign{sep}key{csep}) { psm->setState(FOREIGN); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:key{csep}) { psm->setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:index{csep}) { psm->setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:unique{csep}) { psm->setState(UNIQUE); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:fulltext{csep}) { psm->setState(FULLTEXT); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:spatial{csep}) { psm->setState(SPATIAL); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:check{csep}) {
	std::ostringstream linestr;
	linestr << line;
	std::cerr << "WARNING: check ignored (table: \"" << psm->tempTable() << "\", line " << linestr.str() << ")" << std::endl;
	BEGIN SKIPLINE;
}
<TABLEFIELD>\`{alpha}\` { psm->addNewField(yytext); wasInt = false; lastFieldTimestamp = false; BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { psm->addNewField(yytext); wasInt = false; lastFieldTimestamp = false; BEGIN FDEFINITION; }
<TABLEFIELD>{sep} { }
<TABLEFIELD>[\r]+ { }
<TABLEFIELD>\n { line++; }
<TABLEFIELD>. {
	std::ostringstream linestr;
	linestr << line; 
	throw std::runtime_error("Unexpected character \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context TABLEFIELD");
}

<FCONSTRAINT>(?i:primary{sep}key) { psm->setState(PRIMARY); BEGIN FDEFINITION; }
<FCONSTRAINT>(?i:foreign{sep}key) { psm->setState(FOREIGN); BEGIN FDEFINITION; }
<FCONSTRAINT>(?i:unique{csep}) { psm->setState(UNIQUE); BEGIN FDEFINITION; }
<FCONSTRAINT>\`{alpha}\` { 
	if (psm->tempConstraint().size() > 0)
	{
		std::ostringstream linestr;
		linestr << line; 
		throw std::runtime_error("Unexpected token \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context FCONSTRAINT");
	}
	psm->tempConstraint().assign(yytext);
}
<FCONSTRAINT>{alpha} { 
	if (psm->tempConstraint().size() > 0)
	{
		std::ostringstream linestr;
		linestr << line; 
		throw std::runtime_error("Unexpected token \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context FCONSTRAINT");
	}
	psm->tempConstraint().assign(yytext);
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
<FDEFINITION>(?i:primary{sep}key{csep}) { psm->addPrimaryKeyFromField(); }
<FDEFINITION>(?i:not{sep}null{csep}) { psm->tempModifier().assign("not null"); }
<FDEFINITION>(?i:null{csep}) { if (psm->getState() != FIELD) psm->tempContents().append("null"); }
<FDEFINITION>int{csep}\( { psm->tempContents().append("int"); wasInt = true; BEGIN SKIPPAR; }
<FDEFINITION>smallint{csep}\( { psm->tempContents().append("smallint"); wasInt = true; BEGIN SKIPPAR; }
<FDEFINITION>bigint{csep}\( { psm->tempContents().append("bigint"); wasInt = true; BEGIN SKIPPAR; }
<FDEFINITION>tinyint{csep}\( { psm->tempContents().append("tinyint"); wasInt = true; BEGIN SKIPPAR; }
<FDEFINITION>text{csep}\( { psm->tempContents().append("text"); BEGIN SKIPPAR; }
<FDEFINITION>double { psm->tempContents().append(yytext); wasInt = true; }
<FDEFINITION>boolean { psm->tempContents().append("tinyint"); }
<FDEFINITION>false { psm->tempContents().append("0"); }
<FDEFINITION>true { psm->tempContents().append("1"); }
<FDEFINITION>{dtime} {
	if (!skipTimestamps)
	{
		std::ostringstream linestr;
		linestr << line;
		std::cerr << "WARNING: datetime initializers may be adjusted to the MySQL time zone and appear as differences between versions! (line " + linestr.str() + ", '" << yytext << "')" << std::endl;
	}
	psm->tempContents().append(yytext);
	lastFieldTimestamp = true;
}
<FDEFINITION>{alphaext} {
/* convert field definition wording to lowercase, except when the wording is between quotes */
	std::string tmp_yytext(yytext);
	std::transform(tmp_yytext.begin(), tmp_yytext.end(), tmp_yytext.begin(), ::tolower);
	psm->tempContents().append(tmp_yytext);
}
<FDEFINITION>\( {
	if (psm->tempContents().size() > 0 && psm->tempContents().at(psm->tempContents().size()-1) != ' ') psm->tempContents() += ' ';
	psm->tempContents().append(yytext);  BEGIN FDEFINITIONP;
}
<FDEFINITION>{csep}, {
	if (skipTimestamps && lastFieldTimestamp)
	{
		psm->scrapCommit();
		lastFieldTimestamp = false;
	}
	else
	{
		psm->commit();
	}
	BEGIN TABLEFIELD;
}
<FDEFINITION>{sep} { if (psm->tempContents().size() > 0) psm->tempContents().append(" "); }
<FDEFINITION>\) {
	if (skipTimestamps && lastFieldTimestamp)
	{
		psm->scrapCommit();
		lastFieldTimestamp = false;
	}
	else
	{
		psm->commit();
	}
	psm->tempContents().clear();
	BEGIN ENDTABLE;
}
<FDEFINITION>[\'] {
	if (!wasInt)
	{
		psm->tempContents().append(yytext);
		BEGIN FDEFINITIONS1;
	}
}
<FDEFINITION>[\"] {
	if (!wasInt)
	{
		psm->tempContents().append(yytext);
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
		psm->tempContents().append(yytext);
		BEGIN FDEFINITION;
	}
	else parantLevel--;
}
<FDEFINITIONP>,{csep} { if (parantLevel == 0) psm->tempContents().append(","); }
<FDEFINITIONP>[\r]+ { }
<FDEFINITIONP>\n { line++; }
<FDEFINITIONP>` { }
<FDEFINITIONP>. { if (parantLevel == 0) psm->tempContents().append(yytext); }

<FDEFINITIONS1>[\'] { psm->tempContents().append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS1>{dtime} {
	if (!skipTimestamps)
	{
		std::ostringstream linestr;
		linestr << line;
		std::cerr << "WARNING: datetime initializers may be adjusted to the MySQL time zone and appear as differences between versions! (line " + linestr.str() + ", '" << yytext << "')" << std::endl;
	}
	lastFieldTimestamp = true;
	psm->tempContents().append(yytext);
}
<FDEFINITIONS1>[\r]+ { }
<FDEFINITIONS1>\n { line++; }
<FDEFINITIONS1>. { psm->tempContents().append(yytext); }

<FDEFINITIONS2>[\"] { psm->tempContents().append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS2>{dtime} {
	if (!skipTimestamps)
	{
		std::ostringstream linestr;
		linestr << line;
		std::cerr << "WARNING: datetime initializers may be adjusted to the MySQL time zone and appear as differences between versions! (line " + linestr.str() + ", '" << yytext << "')" << std::endl;
	}
	lastFieldTimestamp = true;
	psm->tempContents().append(yytext);
}
<FDEFINITIONS2>[\r]+ { }
<FDEFINITIONS2>\n { line++; }
<FDEFINITIONS2>. { psm->tempContents().append(yytext); }

<ENDTABLE>(?i:create{sep}table) {
	std::ostringstream linestr;
	linestr << line;
	throw std::runtime_error("Unexpected token \"" + std::string(yytext) + "\" on line " + linestr.str() + " in context ENDTABLE (missing \";\" ?)");
}
<ENDTABLE>; { psm->addTableType(); psm->commitTable(); BEGIN INITIAL; }
<ENDTABLE>{alphaexteq} { psm->tempContents().append(yytext); }
<ENDTABLE>{sep} { if (psm->tempContents().size() > 0) psm->tempContents().append(" "); }
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

SQLTableListManagerPtr&
lexParse(std::istream& input, bool skipModifiedTimestamps)
{
	psm.reset(new SQLTableListManager);
	parantLevel = 0;
	wasInt = false;
	line = 1;
	skipTimestamps = skipModifiedTimestamps;
	lastFieldTimestamp = false;

	yyFlexLexer lex(&input);
	while (lex.yylex());

	return psm;
}

} //namespace

