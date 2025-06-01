CC = g++
LEX = flex
BISON = bison
CFLAGS = -std=c++23

all: lexer.tab.cc parser.tab.cc ast.hh ast.cc
	$(CC) $(CFLAGS) $^ -o mlang

lexer.tab.cc: mlang.l
	$(LEX) -o lexer.tab.cc mlang.l

parser.tab.cc: mlang.y
	$(BISON) -Wcounterexamples -d -o parser.tab.cc mlang.y

clean:
	rm -f lexer.tab.cc parser.tab.cc parser.tab.hh mlang

