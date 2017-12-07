# INF1715 - Compilers
A compiler for the monga language, written for the class INF1715-Compilers at PUC-Rio.

## Monga
http://www.inf.puc-rio.br/~roberto/comp/lang.html
A Linguagem Monga

A linguagem Monga é uma linguagem procedural simples. Ela oferece os tipos char, int, float, e arrays; estruturas de controle while e if-then-else; variáveis globais e locais; e funções com parâmetros e retorno de valores. Além disso, ela gera código objeto compatível com C, o que permite que programas Monga possam chamar funções das bibliotecas padrão de C.

Léxico

Comentários em Monga começam com # e terminam no final da linha. Identificadores são como em C. Strings são escritos entre aspas duplas ("..."), e podem conter os escapes \n (fim de linha), \t (tab), \" (aspas), e \\ (contrabarra). Numerais, tanto inteiros como ponto flutuante, podem ser escritos em decimal ou em hexa (começando com 0x).

A lista de palavras reservadas segue abaixo:

as     char    else    float   if
int    new     return  void    while
Sintaxe

A sintaxe da linguagem segue abaixo, em EBNF. Note que { X } significa uma lista de zero ou mais ocorrências de X e [ X ] significa um X opcional. Itens entre plicks (aspas simples) ou em maiúsculas denotam terminais (tokens), outros nomes denotam não-terminais.
programa : { definicao }
definicao : def-variavel | def-funcao
def-variavel : ID ':' tipo ';'
tipo : INT | CHAR | FLOAT | VOID | tipo '[' ']'
def-funcao : ID '(' parametros ')' ':' tipo bloco
parametros : [ parametro { ',' parametro } ]
parametro : ID ':' tipo
bloco : '{' { def-variavel } { comando } '}'
comando : IF exp bloco [ ELSE bloco ]
        | WHILE exp bloco
        | var '=' exp ';'
        | RETURN [ exp ] ';'
        | chamada ';'
	| '@' exp ';'
	| bloco
var : ID | exp '[' exp ']'
exp : NUMERAL | LITERAL
	| var
	| '(' exp ')'
	| chamada
	| exp AS tipo
	| NEW tipo '[' exp ']'
	| '-' exp
	| exp '+' exp
	| exp '-' exp
	| exp '*' exp
	| exp '/' exp
	| exp '==' exp
	| exp '~=' exp
	| exp '<=' exp
	| exp '>=' exp
	| exp '<' exp
	| exp '>' exp
	| '!' exp
	| exp '&&' exp
	| exp '||' exp
chamada : ID '(' lista-exp ')'
lista-exp : [ exp { ',' exp } ]
  
  ### Depedencies
    flex bison clang-3.8
  ### Usage
  make
	./out < tests/you_test.mmg
	clang-3.8 -o monga monga.ll
	./monga
