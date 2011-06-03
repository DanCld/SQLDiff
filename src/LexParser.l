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

using namespace sqlfileparser;

/* the static (pre-allocated on heap) variable should always be declared in the .cpp;
   we must ensure that is allocated once and initialized (cleared) before every use;
   we must return it outside this module by the means of a reference to prevent extra
   copying
*/

static unsigned long line;
static SQLTableListManager sm;

%}

%option c++ noyywrap

%x TABLENAME
%x TABLEFIELD
%x FCONSTRAINT
%x FDEFINITION
%x FDEFINITIONP
%x FDEFINITIONS
%x SKIPLINE
%x SKIPLINEP
%x SKIPLINES

%x ENDTABLE

alpha	[a-zA-Z][a-zA-Z0-9_]*
alphaext	[a-zA-Z0-9_]+
alphaexteq	[a-zA-Z0-9_`=]+
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
<TABLENAME>\( { BEGIN TABLEFIELD; }
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
<TABLEFIELD>\`{alpha}\` { sm.addNewField(yytext); BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { sm.addNewField(yytext); BEGIN FDEFINITION; }
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

<FDEFINITION>(?i:primary{sep}key{csep}) { sm.addPrimaryKeyFromField(); }
<FDEFINITION>{alphaext} { sm.tempContents().append(yytext); }
<FDEFINITION>\( { sm.tempContents().append(yytext);  BEGIN FDEFINITIONP; }
<FDEFINITION>{csep}, { sm.commit(); BEGIN TABLEFIELD ; }
<FDEFINITION>{sep} { if (sm.tempContents().size() > 0) sm.tempContents().append(" "); }
<FDEFINITION>\) { sm.commit(); sm.tempContents().clear(); BEGIN ENDTABLE; }
<FDEFINITION>[\'\"] { sm.tempContents().append(yytext); BEGIN FDEFINITIONS; }
<FDEFINITION>[\r]+ { }
<FDEFINITION>\n { line++; }
<FDEFINITION>. { }

<FDEFINITIONP>\) { sm.tempContents().append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONP>[\r]+ { }
<FDEFINITIONP>\n { line++; }
<FDEFINITIONP>. { sm.tempContents().append(yytext); }

<FDEFINITIONS>[\'\"] { sm.tempContents().append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS>[\r]+ { }
<FDEFINITIONS>\n { line++; }
<FDEFINITIONS>. { sm.tempContents().append(yytext); }

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

const SQLTableListManager& lexParse(std::istream& input)
{
	line = 1;
	sm.clear();

	yyFlexLexer lex(&input);
	while (lex.yylex()) { }

	return sm;
}

} //namespace
