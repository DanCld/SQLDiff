build:
	flex -o LexParser.cpp LexParser.l
	g++ -Wall SQLFileParser.cpp SQLParserHelper.cpp LexParser.cpp -o sqlFileParser

clean:
	rm -f sqlFileParser LexParser.cpp
