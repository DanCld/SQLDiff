%{
/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include "SQLParserHelper.hpp"

#include <stdexcept>

using namespace sqlfileparser;

static TextScannerHelper fp;
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
<TABLEFIELD>primary{sep}key { sm.setState(PRIMARY); fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>foreign{sep}key { sm.setState(FOREIGN); fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>index{sep} { sm.setState(INDEX); fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { sm.addNewField(yytext); fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>[\`\r\n] { }
<TABLEFIELD>. { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLEFIELD"); }

<FDEFINITION>{alphaext} { fp.addToBuffer(yytext); }
<FDEFINITION>\( { fp.addToBuffer(yytext);  BEGIN FDEFINITIONP; }
<FDEFINITION>{csep}, { sm.commit(fp.buffer()); BEGIN TABLEFIELD ; }
<FDEFINITION>{sep} { if (fp.size() > 0) fp.addToBuffer(" "); }
<FDEFINITION>\) { sm.commit(fp.buffer()); BEGIN ENDTABLE; }
<FDEFINITION>[\'\"] { fp.addToBuffer(yytext); BEGIN FDEFINITIONS; }
<FDEFINITION>{endln} { }
<FDEFINITION>. { }

<FDEFINITIONP>\) { fp.addToBuffer(yytext); BEGIN FDEFINITION; }
<FDEFINITIONP>{endln} { }
<FDEFINITIONP>. { fp.addToBuffer(yytext); }

<FDEFINITIONS>[\'\"] { fp.addToBuffer(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS>{endln} { }
<FDEFINITIONS>. { fp.addToBuffer(yytext); }

<ENDTABLE>; { sm.addTableType(fp.buffer()); sm.commitTable(); BEGIN INITIAL; }
<ENDTABLE>type { fp.resetBuffer(); }
<ENDTABLE>{alphaext} { fp.addToBuffer(yytext); }
<ENDTABLE>{sep} { if (fp.size() > 0) fp.addToBuffer(" "); }
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
