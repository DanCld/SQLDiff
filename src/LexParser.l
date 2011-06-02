%{
/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include "SQLParserHelper.hpp"

#include <iostream>
#include <stdexcept>

using namespace sqlfileparser;

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
alphaext	[a-zA-Z0-9_`]+
sep	[ \t]+
endln	[\r\n]+
csep	[ \t]*

%%

(?i:create{sep}table) { BEGIN TABLENAME; }

\n { }

. { }

<TABLENAME>(?i:if{sep}not{sep}exists) { }
<TABLENAME>{alpha} { sm.addNewTable(yytext); }
<TABLENAME>\( { BEGIN TABLEFIELD; }
<TABLENAME>\) { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLENAME"); }
<TABLENAME>[\`\r\n] { }
<TABLENAME>. { }

<TABLEFIELD>{sep} { }
<TABLEFIELD>(?i:constraint) { BEGIN FCONSTRAINT; }
<TABLEFIELD>(?i:primary{sep}key) { sm.setState(PRIMARY); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:foreign{sep}key) { sm.setState(FOREIGN); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:key{csep}) { sm.setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:index{csep}) { sm.setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:unique{csep}) { std::cerr << "WARNING: unique constraints not yet supported! (table: " << sm.tempTable() << " )" << std::endl; BEGIN SKIPLINE; }
<TABLEFIELD>(?i:fulltext{csep}) { std::cerr << "WARNING: fulltext constraints not yet supported! (table: " << sm.tempTable() << " )" << std::endl; BEGIN SKIPLINE; }
<TABLEFIELD>(?i:spatial{csep}) { std::cerr << "WARNING: spatial constraints not yet supported (table: " << sm.tempTable() << " )" << std::endl; BEGIN SKIPLINE; }
<TABLEFIELD>(?i:check{csep}) { std::cerr << "WARNING: checks not yet supported (table: " << sm.tempTable() << " )" << std::endl; BEGIN SKIPLINE; }

<TABLEFIELD>{alpha} { sm.addNewField(yytext); BEGIN FDEFINITION; }
<TABLEFIELD>[\`\r\n] { }
<TABLEFIELD>. { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLEFIELD"); }

<FCONSTRAINT>{sep} { }
<FCONSTRAINT>(?i:primary{sep}key) { sm.setState(PRIMARY); BEGIN FDEFINITION; }
<FCONSTRAINT>(?i:foreign{sep}key) { sm.setState(FOREIGN); BEGIN FDEFINITION; }
<FCONSTRAINT>(?i:unique{csep}) { std::cerr << "WARNING: unique constraints not yet supported! (table: " << sm.tempTable() << " )" << std::endl; BEGIN SKIPLINE; }
<FCONSTRAINT>{alpha} { std::cerr << "WARNING: constraints with symbols not yet supported! (table: " << sm.tempTable() << " )" << std::endl; }
<FCONSTRAINT>[\`\r\n] { }
<FCONSTRAINT>. { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context FCONSTRAINT"); }

<FDEFINITION>{alphaext} { sm.tempContents().append(yytext); }
<FDEFINITION>\( { sm.tempContents().append(yytext);  BEGIN FDEFINITIONP; }
<FDEFINITION>{csep}, { sm.commit(); BEGIN TABLEFIELD ; }
<FDEFINITION>{sep} { if (sm.tempContents().size() > 0) sm.tempContents().append(" "); }
<FDEFINITION>\) { sm.commit(); BEGIN ENDTABLE; }
<FDEFINITION>[\'\"] { sm.tempContents().append(yytext); BEGIN FDEFINITIONS; }
<FDEFINITION>{endln} { }
<FDEFINITION>. { }

<FDEFINITIONP>\) { sm.tempContents().append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONP>{endln} { }
<FDEFINITIONP>. { sm.tempContents().append(yytext); }

<FDEFINITIONS>[\'\"] { sm.tempContents().append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS>{endln} { }
<FDEFINITIONS>. { sm.tempContents().append(yytext); }

<ENDTABLE>; { sm.addTableType(); sm.commitTable(); BEGIN INITIAL; }
<ENDTABLE>(?i:type) { sm.tempContents().clear(); }
<ENDTABLE>{alphaext} { sm.tempContents().append(yytext); }
<ENDTABLE>{sep} { if (sm.tempContents().size() > 0) sm.tempContents().append(" "); }
<ENDTABLE>{endln} { }
<ENDTABLE>. { }

<SKIPLINE>\( { BEGIN SKIPLINEP; }
<SKIPLINE>[\'\"] { BEGIN SKIPLINES; }
<SKIPLINE>, { BEGIN TABLEFIELD; }
<SKIPLINE>\) { BEGIN ENDTABLE; }
<SKIPLINE>{endln} { }
<SKIPLINE>. { }

<SKIPLINEP>\) { BEGIN SKIPLINE; }
<SKIPLINEP>{endln} { }
<SKIPLINEP>. { }

<SKIPLINES>[\'\"] { BEGIN SKIPLINE; }
<SKIPLINES>{endln} { }
<SKIPLINES>. { }

%%

namespace sqlfileparser
{

const SQLTableListManager& lexParse(std::istream& input)
{
	sm.clear();

	yyFlexLexer lex(&input);
	while (lex.yylex()) { }

	return sm;
}

} //namespace
