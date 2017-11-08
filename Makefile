main:
	flex lexico.lex
	bison -d -v grammar.y
	cc lex.yy.c grammar.tab.c -o out

testsyntax: main
	python test.py unittest

saveresult:
	python test.py saveresults

clean:
	rm out
	rm lex.yy.c
	rm grammar.tab.c
	rm grammar.tab.h
	rm grammar.output
