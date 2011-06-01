build:
	flex -o LexParser.cpp LexParser.l
	g++ -g3 -O0 -Wall main.cpp SQLFileParser.cpp SQLParserHelper.cpp LexParser.cpp -o sqlFileParser

clean:
	rm -f sqlFileParser LexParser.cpp
