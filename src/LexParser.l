%{
/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

/* this file will be expanded into a .cpp one by the flex program
*/

#include "SQLParserHelper.hpp"

#include <iostream>
#include <stdexcept>

using namespace sqlfileparser;

/* the static (pre-allocated on heap) variable should always be declared in the .cpp;
   we must ensure that is allocated once and initialized (cleared) before every use;
   we must return it outside this module by the means of a reference to prevent extra
   copying
*/

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
endln	[\r\n]+
csep	[ \t]*

%%

(?i:create{sep}table) { BEGIN TABLENAME; }
{endln} { }
. { }

<TABLENAME>(?i:if{sep}not{sep}exists) { }
<TABLENAME>\`{alpha}\` { sm.addNewTable(yytext); }
<TABLENAME>{alpha} { sm.addNewTable(yytext); }
<TABLENAME>\( { BEGIN TABLEFIELD; }
<TABLENAME>\) { throw std::runtime_error("Unexpected character \"" + std::string(yytext) + "\" in context TABLENAME"); }
<TABLENAME>{endln} { }
<TABLENAME>. { }

<TABLEFIELD>(?i:constraint) { BEGIN FCONSTRAINT; }
<TABLEFIELD>(?i:primary{sep}key) { sm.setState(PRIMARY); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:foreign{sep}key) { sm.setState(FOREIGN); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:key{csep}) { sm.setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:index{csep}) { sm.setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>(?i:unique{csep}) { std::cerr << "WARNING: unique constraints not yet supported! (table: \"" << sm.tempTable() << "\")" << std::endl; BEGIN SKIPLINE; }
<TABLEFIELD>(?i:fulltext{csep}) { std::cerr << "WARNING: fulltext constraints not yet supported! (table: \"" << sm.tempTable() << "\")" << std::endl; BEGIN SKIPLINE; }
<TABLEFIELD>(?i:spatial{csep}) { std::cerr << "WARNING: spatial constraints not yet supported (table: \"" << sm.tempTable() << "\")" << std::endl; BEGIN SKIPLINE; }
<TABLEFIELD>(?i:check{csep}) { std::cerr << "WARNING: checks not yet supported (table: \"" << sm.tempTable() << "\")" << std::endl; BEGIN SKIPLINE; }
<TABLEFIELD>\`{alpha}\` { sm.addNewField(yytext); BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { sm.addNewField(yytext); BEGIN FDEFINITION; }
<TABLEFIELD>{sep} { }
<TABLEFIELD>{endln} { }
<TABLEFIELD>. { throw std::runtime_error("Unexpected character \"" + std::string(yytext) + "\" in context TABLEFIELD"); }

<FCONSTRAINT>(?i:primary{sep}key) { sm.setState(PRIMARY); BEGIN FDEFINITION; }
<FCONSTRAINT>(?i:foreign{sep}key) { sm.setState(FOREIGN); BEGIN FDEFINITION; }
<FCONSTRAINT>(?i:unique{csep}) { std::cerr << "WARNING: unique constraints not yet supported! (table: \"" << sm.tempTable() << "\")" << std::endl; BEGIN SKIPLINE; }
<FCONSTRAINT>\`{alpha}\` { 
	if (sm.tempConstraint().size() > 0)
	{
		throw std::runtime_error("Unexpected token \"" + std::string(yytext) + "\" in context FCONSTRAINT");
	}
	sm.tempConstraint().assign(yytext);
}
<FCONSTRAINT>{alpha} { 
	if (sm.tempConstraint().size() > 0)
	{
		throw std::runtime_error("Unexpected token \"" + std::string(yytext) + "\" in context FCONSTRAINT");
	}
	sm.tempConstraint().assign(yytext);
}
<FCONSTRAINT>{sep} { }
<FCONSTRAINT>{endln} { }
<FCONSTRAINT>. { throw std::runtime_error("Unexpected character \"" + std::string(yytext) + "\" in context FCONSTRAINT"); }

<FDEFINITION>(?i:primary{sep}key) { sm.addPrimaryKeyFromField(); }
<FDEFINITION>{alphaext} { sm.tempContents().append(yytext); }
<FDEFINITION>\( { sm.tempContents().append(yytext);  BEGIN FDEFINITIONP; }
<FDEFINITION>{csep}, { sm.commit(); BEGIN TABLEFIELD ; }
<FDEFINITION>{sep} { if (sm.tempContents().size() > 0) sm.tempContents().append(" "); }
<FDEFINITION>\) { sm.commit(); sm.tempContents().clear(); BEGIN ENDTABLE; }
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
<ENDTABLE>{alphaexteq} { sm.tempContents().append(yytext); }
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
