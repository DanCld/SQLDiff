%{
/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include "SQLParserHelper.hpp"

#include <stdexcept>

using namespace sqlfileparser;

static SQLTableListManager sm;

%}

%option c++ noyywrap

%x TABLENAME
%x TABLEFIELD
%x FDEFINITION
%x FDEFINITIONP
%x FDEFINITIONS
%x ENDTABLE

alpha	[a-zA-Z][a-zA-Z0-9_]*
alphaext	[a-zA-Z0-9_]+
sep	[ \t]+
endln	[\r\n]+
csep	[ \t]*

%%

(?i:create{sep}table) { BEGIN TABLENAME; }

\n { }

. { }

<TABLENAME>{alpha} { sm.addNewTable(yytext); }
<TABLENAME>\( { BEGIN TABLEFIELD; }
<TABLENAME>\) { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLENAME"); }
<TABLENAME>[\`\r\n] { }
<TABLENAME>. { }

<TABLEFIELD>{sep} { }
<TABLEFIELD>primary{sep}key { sm.setState(PRIMARY); BEGIN FDEFINITION; }
<TABLEFIELD>foreign{sep}key { sm.setState(FOREIGN); BEGIN FDEFINITION; }
<TABLEFIELD>index{csep} { sm.setState(INDEX); BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { sm.addNewField(yytext); BEGIN FDEFINITION; }
<TABLEFIELD>[\`\r\n] { }
<TABLEFIELD>. { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLEFIELD"); }

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
<ENDTABLE>type { sm.tempContents().clear(); }
<ENDTABLE>{alphaext} { sm.tempContents().append(yytext); }
<ENDTABLE>{sep} { if (sm.tempContents().size() > 0) sm.tempContents().append(" "); }
<ENDTABLE>{endln} { }
<ENDTABLE>. { }

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
