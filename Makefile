main:
	flex Tokens.lex
	bison -d -v Gramatica.y
	cc -Wall lex.yy.c Gramatica.tab.c ArvoreSintaticaAbstrata.c TabelaSimbolos.c -o out

clean:
	rm out
	rm lex.yy.c
	rm Gramatica.tab.c
	rm Gramatica.tab.h
	rm Gramatica.output
