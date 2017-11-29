main:
	flex Tokens.lex
	bison -d -v Gramatica.y
	cc -Wall lex.yy.c Gramatica.tab.c ArvoreSintaticaAbstrata.c TabelaSimbolos.c GeraCodigo.c -o out

compmonga: main
	./out < tests/test_arith.mmg
	clang-3.8 -o monga monga.ll
	./monga
	./out < tests/test_cmp.mmg
	clang-3.8 -o monga monga.ll
	./monga
	./out < tests/test_expas.mmg
	clang-3.8 -o monga monga.ll
	./monga

geraex:
	clang-3.8 -emit-llvm -S -O0 example.c

rodaex:
	clang-3.8 -o example example.ll
	./example

clean:
	rm out
	rm lex.yy.c
	rm Gramatica.tab.c
	rm Gramatica.tab.h
	rm Gramatica.output
	rm monga
	rm monga.ll
	rm example.ll
	rm
