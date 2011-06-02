%{
/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include "SQLParserHelper.hpp"

#include <stdexcept>

using namespace sqlfileparser;

static std::string bufString;

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
<TABLEFIELD>primary{sep}key { sm.setState(PRIMARY); bufString.clear(); BEGIN FDEFINITION; }
<TABLEFIELD>foreign{sep}key { sm.setState(FOREIGN); bufString.clear(); BEGIN FDEFINITION; }
<TABLEFIELD>index{csep} { sm.setState(INDEX); bufString.clear(); BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { sm.addNewField(yytext); bufString.clear(); BEGIN FDEFINITION; }
<TABLEFIELD>[\`\r\n] { }
<TABLEFIELD>. { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLEFIELD"); }

<FDEFINITION>{alphaext} { bufString.append(yytext); }
<FDEFINITION>\( { bufString.append(yytext);  BEGIN FDEFINITIONP; }
<FDEFINITION>{csep}, { sm.commit(bufString); BEGIN TABLEFIELD ; }
<FDEFINITION>{sep} { if (bufString.size() > 0) bufString.append(" "); }
<FDEFINITION>\) { sm.commit(bufString); BEGIN ENDTABLE; }
<FDEFINITION>[\'\"] { bufString.append(yytext); BEGIN FDEFINITIONS; }
<FDEFINITION>{endln} { }
<FDEFINITION>. { }

<FDEFINITIONP>\) { bufString.append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONP>{endln} { }
<FDEFINITIONP>. { bufString.append(yytext); }

<FDEFINITIONS>[\'\"] { bufString.append(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS>{endln} { }
<FDEFINITIONS>. { bufString.append(yytext); }

<ENDTABLE>; { sm.addTableType(bufString); sm.commitTable(); BEGIN INITIAL; }
<ENDTABLE>type { bufString.clear(); }
<ENDTABLE>{alphaext} { bufString.append(yytext); }
<ENDTABLE>{sep} { if (bufString.size() > 0) bufString.append(" "); }
<ENDTABLE>{endln} { }
<ENDTABLE>. { }

%%

namespace sqlfileparser
{

const SQLTableListManager& lexParse(std::istream& input)
{
	bufString.clear();
	sm.clear();

	yyFlexLexer lex(&input);
	while (lex.yylex()) { }

	return sm;
}

} //namespace
