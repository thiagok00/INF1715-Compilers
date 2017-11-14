%code requires {
  #include <stdio.h>
  #include <string.h>
  #if !defined(ArvoreSintaticaAbstrata_h)
  #include "ArvoreSintaticaAbstrata.h"
  #define ArvoreSintaticaAbstrata_h
  #endif
  #if !defined(TabelaSimbolos_h)
  #include "TabelaSimbolos.h"
  #define TabelaSimbolos_h
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
  ParametroL* paramsL;
  Bloco* bloco;
  DefVarL* defvars;
  CMD *cmd;
  CMDL *cmdL;
  Exp* exp;
  Constante* cons;
  Var *var;
  ExpL* expl;
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
%type<paramsL> parametros lista_params parametro
%type<bloco> bloco
%type<defvars> lista_def_var lista_def_var2
%type<cmdL> lista_comandos
%type<cmd> comando
%type<exp> expressao exp_or exp_and exp_cmp exp_add exp_mult exp_unaria exp_as expressao_base chamada
%type<cons> constante
%type<expl> lista_exp lista_exp2
%type<var> variavel

%%

programa:               lista_definicoes {  $$ = (Programa*)malloc(sizeof(Programa));
                                            $$->defs = $1;
                                            nodePrograma = $$;}
                    ;

lista_definicoes:       definicao {$$ = $1;}
                    |   definicao lista_definicoes {$$ = $1; $$->prox = $2;}
                    ;

definicao:              def_variavel {  $$ = (Definicao*)malloc(sizeof(Definicao));
                                        $1->escopo = EscopoGlobal;
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
                                              $$->tipo = $3;
                                              }
                    ;

tipo:                   tipo_primitivo {  $$ = (Tipo*)malloc(sizeof(Tipo));
                                          $$->tag = base;
                                          $$->tipo_base = $1;
                                          $$->de = NULL;
                                          }
                    |   tipo '[' ']'  {   $$ = (Tipo*)malloc(sizeof(Tipo));
                                          $$->tag = array;
                                          $$->tipo_base = $1->tipo_base;
                                          $$->de = $1;
                                      }
                    ;

tipo_primitivo:         TK_INT    {$$ = bInt;}
                    |   TK_FLOAT  {$$ = bFloat;}
                    |   TK_CHAR   {$$ = bChar;}
                    |   TK_VOID   {$$ = bVoid;}
                    ;

def_funcao:             ID '(' parametros ')' ':' tipo bloco {$$ = (DefFunc*)malloc(sizeof(DefFunc));
                                                              $$->tiporet = $6;
                                                              $$->id = $1;
                                                              $$->params = $3;
                                                              $$->bloco = $7;
                                                             }
                    ;

parametros:             /* vazio */ {$$ = NULL;}
                    |   lista_params {$$ = $1;}
                    ;

lista_params:           parametro {$$ = $1;}
                    |   parametro ',' lista_params {$$ = $1;
                                                    $$->prox = $3;
                                                    }
                    ;

parametro:              ID ':' tipo { $$ = (ParametroL*)malloc(sizeof(ParametroL));
                                      $$->id = $1;
                                      $$->tipo = $3;
                                      $$->prox = NULL;
    }
                    ;

bloco:                  '{' lista_def_var lista_comandos '}'  {$$ = (Bloco*)malloc(sizeof(Bloco));
                                                               if($2 != NULL)
                                                                  $$->declVars = $2->prim;
                                                               else
                                                                  $$->declVars = $2;
                                                               $$->cmds = $3;
                                                              }
                    ;

lista_def_var:          /*vazio*/ {$$ = NULL;}
                    |   lista_def_var lista_def_var2 {

                                                        if ($1 != NULL) {
                                                          $1->prox = $2;
                                                          $$ = $2 ;
                                                          $2->prim = $1->prim;
                                                        }
                                                        else{$$ = $2;$$->prim = $$;}
                                                    }
                    ;
lista_def_var2: def_variavel {
                                $$ = (DefVarL*)malloc(sizeof(DefVarL));
                                $1->escopo = EscopoLocal;
                                $$->v = $1;
                                $$->prox = NULL;
                                $$->prim = NULL;
}
                ;
lista_comandos:         /*vazio*/ {$$ = NULL;}
                    |   comando lista_comandos {$$ = (CMDL*)malloc(sizeof(CMDL));
                                                $$->c = $1;
                                                $$->prox = $2;
                                               }
                    ;

comando:                bloco { $$ = (CMD*)malloc(sizeof(CMD));
                                $$->tag = CMD_BLOCK;
                                $$->u.bloco = $1;
                              }
                    |   '@' expressao ';'{ $$ = (CMD*)malloc(sizeof(CMD));
                                           $$->tag = CMD_PRINT;
                                           $$->u.exp = $2;
                                        }
                    |   TK_RETURN expressao ';' { $$ = (CMD*)malloc(sizeof(CMD));
                                                  $$->tag = CMD_RETURN;
                                                  $$->u.exp = $2;
                                                }
                    |   TK_RETURN ';' { $$ = (CMD*)malloc(sizeof(CMD));
                                        $$->tag = CMD_RETURNVOID;
                                      }
                    |   variavel '=' expressao ';' { $$ = (CMD*)malloc(sizeof(CMD));
                                                     $$->tag = CMD_ATR;
                                                     $$->u.atr.var = $1;
                                                     $$->u.atr.exp = $3;
                                                   }
                    |   TK_IF  expressao  bloco { $$ = (CMD*)malloc(sizeof(CMD));
                                                  $$->tag = CMD_IF;
                                                  $$->u.cmdif.exp = $2;
                                                  $$->u.cmdif.bloco = $3;
                                                }
                    |   TK_IF  expressao  bloco  TK_ELSE  bloco { $$ = (CMD*)malloc(sizeof(CMD));
                                                                  $$->tag = CMD_IFELSE;
                                                                  $$->u.cmdifelse.exp = $2;
                                                                  $$->u.cmdifelse.blocoif = $3;
                                                                  $$->u.cmdifelse.blocoelse = $5;
                                                                }
                    |   TK_WHILE    expressao    bloco { $$ = (CMD*)malloc(sizeof(CMD));
                                                         $$->tag = CMD_WHILE;
                                                         $$->u.cmdwhile.exp = $2;
                                                         $$->u.cmdwhile.bloco = $3;
                                                       }
                    |   chamada ';' { $$ = (CMD*)malloc(sizeof(CMD));
                                      $$->tag = CMD_CHAMADA;
                                      $$->u.exp = $1;

                                    }
                    ;

variavel:               ID {     $$ = (Var*)malloc(sizeof(Var));
                                 $$->tag = vVar;
                                 $$->u.vvar.id = $1;
                                 $$->tipo = NULL;
                           }
                    |   expressao_base '[' expressao ']' {  $$ = (Var*)malloc(sizeof(Var));
                                                            $$->tag = vAcesso;
                                                            $$->u.vacesso.expvar = $1;
                                                            $$->u.vacesso.expindex = $3;
                                                            $$->tipo = NULL;
                    }
                    ;

chamada:                ID '(' lista_exp ')' {  $$ = (Exp*)malloc(sizeof(Exp));
                                                $$->tag = EXP_CHAMADA;
                                                $$->u.expchamada.idFunc = $1;
                                                $$->u.expchamada.params = $3;
                                                $$->tipo = NULL;
                                             }
                    ;

lista_exp:              /* vazio*/ {$$ = NULL;}
                    |   lista_exp2 { $$ = $1;}
                    ;

lista_exp2:             expressao { $$ = (ExpL*)malloc(sizeof(ExpL));
                                    $$->e = $1;
                                    $$->prox = NULL;
                                  }
                    |   expressao ',' lista_exp2 {$$ = (ExpL*)malloc(sizeof(ExpL));
                                                        $$->e = $1;
                                                        $$->prox = $3;}
                    ;

expressao:              exp_or {$$ = $1;}
                    ;

exp_or:                 exp_and {$$ = $1;}
                    |   exp_or TK_OR exp_and  { $$ = (Exp*)malloc(sizeof(Exp));
                                                $$->tag = EXP_BIN;
                                                $$->u.expbin.expesq = $1;
                                                $$->u.expbin.expdir = $3;
                                                $$->u.expbin.opbin = opor;
                                              }
                    ;

exp_and:                exp_cmp {$$ = $1;}
                    |   exp_and TK_AND exp_cmp  { $$ = (Exp*)malloc(sizeof(Exp));
                                                $$->tag = EXP_BIN;
                                                $$->u.expbin.expesq = $1;
                                                $$->u.expbin.expdir = $3;
                                                $$->u.expbin.opbin = opand;
                                              }
                    ;

exp_cmp:                exp_add {$$ = $1;}
                    |   exp_cmp TK_EQUAL exp_add  { $$ = (Exp*)malloc(sizeof(Exp));
                                                    $$->tag = EXP_BIN;
                                                    $$->u.expbin.expesq = $1;
                                                    $$->u.expbin.expdir = $3;
                                                    $$->u.expbin.opbin = equal;
                                                    }
                    |   exp_cmp TK_NOTEQUAL exp_add  { $$ = (Exp*)malloc(sizeof(Exp));
                                                    $$->tag = EXP_BIN;
                                                    $$->u.expbin.expesq = $1;
                                                    $$->u.expbin.expdir = $3;
                                                    $$->u.expbin.opbin = notequal;
                                                    }
                    |   exp_cmp TK_LESSEQUAL exp_add  { $$ = (Exp*)malloc(sizeof(Exp));
                                                    $$->tag = EXP_BIN;
                                                    $$->u.expbin.expesq = $1;
                                                    $$->u.expbin.expdir = $3;
                                                    $$->u.expbin.opbin = lessequal;
                                                    }
                    |   exp_cmp TK_GREATEREQUAL exp_add  { $$ = (Exp*)malloc(sizeof(Exp));
                                                    $$->tag = EXP_BIN;
                                                    $$->u.expbin.expesq = $1;
                                                    $$->u.expbin.expdir = $3;
                                                    $$->u.expbin.opbin = greaterequal;
                                                    }
                    |   exp_cmp'<' exp_add  { $$ = (Exp*)malloc(sizeof(Exp));
                                                    $$->tag = EXP_BIN;
                                                    $$->u.expbin.expesq = $1;
                                                    $$->u.expbin.expdir = $3;
                                                    $$->u.expbin.opbin = less;
                                                    }
                    |   exp_cmp '>' exp_add  { $$ = (Exp*)malloc(sizeof(Exp));
                                                    $$->tag = EXP_BIN;
                                                    $$->u.expbin.expesq = $1;
                                                    $$->u.expbin.expdir = $3;
                                                    $$->u.expbin.opbin = greater;
                                                    }
                    ;

exp_add:                exp_mult {$$ = $1;}
                    |   exp_add '+' exp_mult { $$ = (Exp*)malloc(sizeof(Exp));
                                                $$->tag = EXP_BIN;
                                                $$->u.expbin.expesq = $1;
                                                $$->u.expbin.expdir = $3;
                                                $$->u.expbin.opbin = opadd;
                                              }
                    |   exp_add '-' exp_mult { $$ = (Exp*)malloc(sizeof(Exp));
                                                $$->tag = EXP_BIN;
                                                $$->u.expbin.expesq = $1;
                                                $$->u.expbin.expdir = $3;
                                                $$->u.expbin.opbin = opsub;
                                              }
                    ;

exp_mult:               exp_unaria {$$ = $1;}
                    |   exp_mult '*' exp_unaria { $$ = (Exp*)malloc(sizeof(Exp));
                                                  $$->tag = EXP_BIN;
                                                  $$->u.expbin.expesq = $1;
                                                  $$->u.expbin.expdir = $3;
                                                  $$->u.expbin.opbin = opmult;
                                                }
                    |   exp_mult '/' exp_unaria { $$ = (Exp*)malloc(sizeof(Exp));
                                                  $$->tag = EXP_BIN;
                                                  $$->u.expbin.expesq = $1;
                                                  $$->u.expbin.expdir = $3;
                                                  $$->u.expbin.opbin = opdiv;
                                                }
                    ;

exp_unaria:             exp_as {$$ = $1;}
                    |   '-'   exp_unaria  { $$ = (Exp*)malloc(sizeof(Exp));
                                            $$->tag = EXP_UNARIA;
                                            $$->u.expunaria.exp = $2;
                                            $$->u.expunaria.opun = opmenos;
                                          }
                    |   '!'   exp_unaria  { $$ = (Exp*)malloc(sizeof(Exp));
                                            $$->tag = EXP_UNARIA;
                                            $$->u.expunaria.exp = $2;
                                            $$->u.expunaria.opun = opnot;
                                          }
                    ;


exp_as:                 expressao_base {$$ = $1;}
                    |   expressao_base TK_AS tipo { $$ = (Exp*)malloc(sizeof(Exp));
                                                    $$->tag = EXP_AS;
                                                    $$->u.expnewas.exp = $1;
                                                    $$->u.expnewas.tipo = $3;
                                                  }
                    ;

expressao_base:         constante { $$ = (Exp*)malloc(sizeof(Exp));
                                    $$->tag = EXP_CTE;
                                    $$->u.expcte = $1;
                                  }
                    |   variavel  { $$ = (Exp*)malloc(sizeof(Exp));
                                    $$->tag = EXP_VAR;
                                    $$->u.expvar = $1;
                                  }
                    |   '(' expressao ')' {$$ = $2;}
                    |   chamada { $$ = $1;}
                    |   TK_NEW tipo '[' expressao ']'{  $$ = (Exp*)malloc(sizeof(Exp));
                                                        $$->tag = EXP_NEW;
                                                        $$->u.expnewas.tipo = $2;
                                                        $$->u.expnewas.exp = $4;
                                                     }
                    ;

constante:              TK_DEC {  $$ = (Constante*)malloc(sizeof(Constante));
                                  $$->tag = CDEC;
                                  $$->val.i = yylval.int_val;
                               }
                    |   TK_REAL {  $$ = (Constante*)malloc(sizeof(Constante));
                                   $$->tag = CREAL;
                                   $$->val.f = yylval.float_val;
                                }
                    |   TK_STRING {  $$ = (Constante*)malloc(sizeof(Constante));
                                     $$->tag = CSTRING;
                                     $$->val.s = yylval.str_val;
                                  }
                    ;

ID: TK_ID           { $$ = yylval.str_val; }
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
  tipa_arvore(nodePrograma);
  print_tree(nodePrograma);
  return 0;
}
