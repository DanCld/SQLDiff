bin_PROGRAMS = sqlFileParser

LexParser.cpp: LexParser.l
	$(LEX) -o LexParser.cpp LexParser.l

sqlFileParser_SOURCES = \
	main.cpp \
	SQLFileParser.cpp SQLFileParser.hpp \
	SQLParserHelper.cpp SQLParserHelper.hpp \
	LexParser.cpp LexParser.hpp

sqlFileParser_CPPFLAGS = -std=c++0x -Wall

sqlFileParser_LDADD = $(LEXLIB)

CLEANFILES = \
	rm LexParser.cpp
