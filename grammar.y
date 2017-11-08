%code requires {
  #include <stdio.h>
  #include <string.h>
  #if !defined(tree_h)
  #include "tree.h"
  #define tree_h
  #endif
}

%code {
  int   yylex(void);
  void  yyerror(const char *str);
  int   yywrap(void);

  Programa* nodePrograma = NULL;
}

%{

%}

%union{
  int int_val;
  char *str_val;
  float float_val;
  Programa *prog;
  Definicao* def;
  DefVar* defVar;
  DefFunc* defFunc;
  Tipo* t;
/*
  ParametroL* params;
  Exp* exp;

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
%token	<str_val> TK_ID
%token	TK_EQUAL
%token  TK_NOTEQUAL
%token	TK_LESSEQUAL
%token	TK_GREATEREQUAL
%token	TK_AND
%token	TK_OR
%token	TK_STRING

%type<prog> programa
%type<def>  lista_definicoes definicao
%type<defVar> def_variavel
%type<defFunc> def_funcao
%type<t> tipo
%type<int_val> tipo_primitivo
%type<str_val> ID

%%

programa:               lista_definicoes {  $$ = (Programa*)malloc(sizeof(Programa));
                                            $$->defs = $1;
                                            nodePrograma = $$;}
                    ;

lista_definicoes:       definicao {$$ = $1;}
                    |   definicao lista_definicoes {$$ = $1; $$->prox = $2;}
                    ;

definicao:              def_variavel {  $$ = (Definicao*)malloc(sizeof(Definicao));
                                        $$->u.v = $1;
                                        $$->tag = DVar;
                                        $$->prox = NULL;
                                     }
                    |   def_funcao {    $$ = (Definicao*)malloc(sizeof(Definicao));
                                        $$->u.f = $1;
                                        $$->tag = DFunc;
                                        $$->prox = NULL;}
                    ;

def_variavel:           ID ':' tipo ';' {  $$ = (DefVar*)malloc(sizeof(DefVar));
                                              $$->id = $1;
                                              printf("%s",$1);
                                              $$->tipo = $3;
                                              $$->escopo = EscopoGlobal;
                                              }
                    ;

tipo:                   tipo_primitivo {  $$ = (Tipo*)malloc(sizeof(Tipo));
                                          $$->tag = base;
                                          //printf("base %d\n",$1);
                                          $$->tipo_base = $1;
                                          $$->de = NULL;
                                          }

                    |   tipo '[' ']'  {   $$ = (Tipo*)malloc(sizeof(Tipo));
                                          $$->tag = array;
                                          $$->de = $1;
                                      }
                    ;

tipo_primitivo:         TK_INT    {$$ = bInt;}
                    |   TK_FLOAT  {$$ = bFloat;}
                    |   TK_CHAR   {$$ = bChar;}
                    |   TK_VOID   {$$ = bVoid;}
                    ;

def_funcao:             ID '(' parametros ')' ':' tipo bloco {$$ = (DefFunc*)malloc(sizeof(DefFunc));}
                    ;

parametros:             /* vazio */
                    |   lista_params
                    ;

lista_params:           parametro
                    |   parametro ',' lista_params
                    ;

parametro:              ID ':' tipo
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

variavel:               ID
                    |   expressao_base '[' expressao ']'
                    ;

chamada:                ID '(' lista_exp ')'
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

ID: TK_ID           { $$ = yylval.str_val;
                      printf("id:%s\n",$$);
                    }
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
