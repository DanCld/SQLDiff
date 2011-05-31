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
sep	[ \t\n]+

%%

create{sep}table{sep} { BEGIN TABLENAME; }

\n { }

. { }

<TABLENAME>{alpha} { sm.addNewTable(yytext); }
<TABLENAME>\( { BEGIN TABLEFIELD; }
<TABLENAME>\) { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLENAME"); }
<TABLENAME>\n { }
<TABLENAME>. { }

<TABLEFIELD>{sep} { }
<TABLEFIELD>primary { sm.commitPrimary(yytext); BEGIN FDEFINITION; }
<TABLEFIELD>foreign { sm.commitForeign(yytext); BEGIN FDEFINITION; }
<TABLEFIELD>index { sm.commitIndex(yytext); BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { sm.addNewField(yytext); fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>[\r\n] { }
<TABLEFIELD>. { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLEFIELD"); }

<FDEFINITION>{alphaext} { fp.addToBuffer(yytext); }
<FDEFINITION>\( { fp.addToBuffer(yytext);  BEGIN FDEFINITIONP; }
<FDEFINITION>{sep} { fp.addToBuffer(" "); }
<FDEFINITION>, { sm.commitField(fp.buffer()); BEGIN TABLEFIELD ; }
<FDEFINITION>\) { sm.commitField(fp.buffer()); BEGIN ENDTABLE; }
<FDEFINITION>[\'\"] { fp.addToBuffer(yytext); BEGIN FDEFINITIONS; }
<FDEFINITION>[\r\n] { }
<FDEFINITION>. { }

<FDEFINITIONP>\) { fp.addToBuffer(yytext); BEGIN FDEFINITION; }
<FDEFINITIONP>[\r\n] { }
<FDEFINITIONP>. { fp.addToBuffer(yytext); }

<FDEFINITIONS>[\'\"] { fp.addToBuffer(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS>[\r\n] { }
<FDEFINITIONS>. { fp.addToBuffer(yytext); }

<ENDTABLE>; { sm.commitTable(); BEGIN INITIAL; }
<ENDTABLE>{sep} { }
<ENDTABLE>[\r\n] { }
<ENDTABLE>. { }

%%

namespace sqlfileparser
{

const SQLTableListManager& lexParse()
{
	yyFlexLexer lex;
	while (lex.yylex()) { }

	return sm;
}

} //namespace
