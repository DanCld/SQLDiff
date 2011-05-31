%{
/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

#include "SQLFileParser.hpp"

#include <iostream>
#include <stdexcept>

using namespace sqlfileparser;

TextScannerHelper fp;

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

<TABLENAME>{alpha} { std::cout << "TABLE: " << yytext << std::endl; }
<TABLENAME>\( { BEGIN TABLEFIELD; }
<TABLENAME>\) { throw std::runtime_error("Unexpected character " + std::string(yytext) + " in context TABLENAME"); }
<TABLENAME>\n { }
<TABLENAME>. { }

<TABLEFIELD>{sep} { }
<TABLEFIELD>primary { std::cout << "PRIMARY: " << yytext << " | " ; fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>foreign { std::cout << "FOREIGN: " << yytext << " | " ; fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>index { std::cout << "INDEX: " << yytext << " | " ; fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>{alpha} { std::cout << "FIELD: " << yytext << " | " ; fp.resetBuffer(); BEGIN FDEFINITION; }
<TABLEFIELD>. { }

<FDEFINITION>{alphaext} { fp.addToBuffer(yytext); }
<FDEFINITION>\( { fp.addToBuffer(yytext);  BEGIN FDEFINITIONP; }
<FDEFINITION>{sep} { fp.addToBuffer(" "); }
<FDEFINITION>, { std::cout << fp.buffer() << std::endl ; BEGIN TABLEFIELD ; }
<FDEFINITION>\) { std::cout << fp.buffer() << std::endl ; BEGIN ENDTABLE; }
<FDEFINITION>[\'\"] { fp.addToBuffer(yytext); BEGIN FDEFINITIONS; }
<FDEFINITION>. { }

<FDEFINITIONP>\) { fp.addToBuffer(yytext); BEGIN FDEFINITION; }
<FDEFINITIONP>\n { }
<FDEFINITIONP>. { fp.addToBuffer(yytext); }

<FDEFINITIONS>[\'\"] { fp.addToBuffer(yytext); BEGIN FDEFINITION; }
<FDEFINITIONS>\n { }
<FDEFINITIONS>. { fp.addToBuffer(yytext); }

<ENDTABLE>; { std::cout << std::endl << std::endl; BEGIN INITIAL; }
<ENDTABLE>{sep} { }
<ENDTABLE>. { }

%%

TextScannerHelper::TextScannerHelper()
:buffer_()
{
}

void
TextScannerHelper::addToBuffer(const char* piece)
{
	buffer_.append(piece);
}

void
TextScannerHelper::resetBuffer()
{
	buffer_.clear();
}

int main()
{
	yyFlexLexer lex;
	while (lex.yylex()) { }

	return 0;
}
