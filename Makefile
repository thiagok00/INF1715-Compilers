main:
	flex lexico.lex
	bison -d -v grammar.y
	cc -Wall lex.yy.c grammar.tab.c tree.c tabelaSimbolos.c -o out

testsyntax: main
	python test.py unittest

saveresult:
	python test.py saveresults

testast:
	out

clean:
	rm out
	rm lex.yy.c
	rm grammar.tab.c
	rm grammar.tab.h
	rm grammar.output
