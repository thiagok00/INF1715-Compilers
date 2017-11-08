%{
#include <stdio.h>
#include <string.h>
#include "tree.h"

int   yylex(void);
void  yyerror(const char *str);
int   yywrap(void);

%}

%union{
  int i;
  char *s;
  float f;
/*
  Programa *prog;
  Definicao* def;
  DefVar* defVar;
  DefFunc* defFunc;
  ParametroL* params;
  Exp* exp;
  Tipo* type;
  CMDL* cmd;
  Bloco* bloco;
  DefVarL* defvars;
  ExpL* expl;
  Constante* cons;
  */
}

%token	TK_INT
%token	TK_FLOAT
%token	TK_CHAR
%token	TK_VOID
%token	TK_NEW
%token	TK_IF
%token	TK_ELSE
%token	TK_WHILE
%token	TK_AS
%token	TK_RETURN
%token	TK_DEC
%token	TK_REAL
%token	TK_ID
%token	TK_EQUAL
%token  TK_NOTEQUAL
%token	TK_LESSEQUAL
%token	TK_GREATEREQUAL
%token	TK_AND
%token	TK_OR
%token	TK_STRING



%%

programa:               lista_definicoes
                    ;

lista_definicoes:       definicao
                    |   definicao lista_definicoes
                    ;

definicao:              def_variavel
                    |   def_funcao
                    ;

def_variavel:           TK_ID ':' tipo ';'
                    ;

tipo:                   tipo_primitivo
                    |   tipo '[' ']'
                    ;

tipo_primitivo:         TK_INT
                    |   TK_FLOAT
                    |   TK_CHAR
                    |   TK_VOID
                    ;

def_funcao:             TK_ID '(' parametros ')' ':' tipo bloco
                    ;

parametros:             /* vazio */
                    |   lista_params
                    ;

lista_params:           parametro
                    |   parametro ',' lista_params
                    ;

parametro:              TK_ID ':' tipo
                    ;

bloco:                  '{' lista_def_var lista_comandos '}'
                    ;

lista_def_var:          /*vazio*/
                    |   lista_def_var def_variavel
                    ;

lista_comandos:         /*vazio*/
                    |   comando lista_comandos
                    ;

comando:                bloco
                    |   '@' expressao ';'
                    |   TK_RETURN expressao ';'
                    |   TK_RETURN ';'
                    |   variavel '=' expressao ';'
                    |   TK_IF  expressao  bloco
                    |   TK_IF  expressao  bloco  TK_ELSE  bloco
                    |   TK_WHILE    expressao    bloco
                    |   chamada ';'
                    ;

variavel:               TK_ID
                    |   expressao_base '[' expressao ']'
                    ;

chamada:                TK_ID '(' lista_exp ')'
                    ;

lista_exp:              /* vazio*/
                    |   lista_exp2
                    ;

lista_exp2:             expressao
                    |   expressao ',' lista_exp2
                    ;

expressao:              exp_or
                    ;

exp_or:                 exp_and
                    |   exp_or TK_OR exp_and
                    ;

exp_and:                exp_cmp
                    |   exp_and TK_AND exp_cmp
                    ;

exp_cmp:                exp_add
                    |   exp_cmp TK_EQUAL exp_add
                    |   exp_cmp TK_NOTEQUAL exp_add
                    |   exp_cmp TK_LESSEQUAL exp_add
                    |   exp_cmp TK_GREATEREQUAL exp_add
                    |   exp_cmp'<' exp_add
                    |   exp_cmp '>' exp_add
                    ;

exp_add:                exp_mult
                    |   exp_add '+' exp_mult
                    |   exp_add '-' exp_mult
                    ;

exp_mult:               exp_unaria
                    |   exp_mult '*' exp_unaria
                    |   exp_mult '/' exp_unaria
                    ;

exp_unaria:             exp_as
                    |   '-'   exp_unaria
                    |   '!'   exp_unaria
                    ;


exp_as:                 expressao_base
                    |   expressao_base TK_AS tipo
                    ;

expressao_base:         TK_DEC
                    |   TK_REAL
                    |   TK_STRING
                    |   variavel
                    |   '(' expressao ')'
                    |   chamada
                    |   TK_NEW tipo '[' expressao ']'
                    ;

%%

void yyerror(const char *str) {
  fprintf(stderr,"error: %s\n",str);
}

int yywrap(void) {
  return 1;
}

int main(void) {
  yyparse();
  return 0;
}
