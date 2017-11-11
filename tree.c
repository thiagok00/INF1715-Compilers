#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(tree_h)
#include "tree.h"
#define tree_h
#endif

/*
*  declarations
*/
//static char*  strFormatoTipo(Base_TAG x);
static char*  getStringEscopo(EscopoTag escopo);

static void   print_variavel(Var *v, int nivel);
static void   print_constante(Constante *c, int nivel);
static void   print_exp_unaria(Exp *e, int nivel);
static void   print_exp_binaria(Exp *e, int nivel);
static void   print_exp(Exp *e, int nivel);
static void   print_cmd(CMD *c,int nivel);
static void   print_bloco (Bloco *b, int nivel);
static void   print_defvar(DefVar * dvar,int nivel);
static void   print_deffunc(DefFunc * dfunc,int nivel);
static void   print_defs(Definicao* d, int nivel);
/*
*  implementations
*/
/*static void print_padding(int nivel) {
    while (nivel) {
      printf("\t");
      nivel--;
    }
}
*/
static char* strFormatoTipo(Base_TAG x) {
    switch (x){
      case bInt: return "INT";
      case bFloat: return "FLOAT";
      case bChar: return "CHAR";
      case bVoid: return "VOID";
      default: return "TIPOERRADO";
    }
}

char* getStringTipo (Tipo *t) {

    Tipo *aux = t;
    char *strbase = strFormatoTipo(t->tipo_base);
    char *arrayde = "array de ";
    char *str;
    int arrayCounts = 0;

    if(t == NULL) {
      printf("TIPO NULL");
      return NULL;
    }
    while (aux->de != NULL) {
      arrayCounts++;
      aux = aux->de;
    }

    str = (char*)malloc((strlen(arrayde)*arrayCounts + strlen(strbase) + 1) * sizeof(char));
    if (str == NULL){printf("falta de memoria"); exit(1);}
    str[0] = '\0';
    while(arrayCounts) {
      strcat(str,arrayde);
      arrayCounts--;
    }
    strcat(str,strbase);
    return str;
}

static char* getStringEscopo(EscopoTag escopo) {
  switch (escopo) {
    case EscopoGlobal:
      return "Global";
    case EscopoLocal:
      return "Local";
    case EscopoFunc:
      return "EscopoFunc";
  }
  return "[Erro Escopo]";
}

static void print_constante(Constante *c,int nivel) {
   if (c == NULL) return;
   printf("%d\t [CONSTANTE] ",nivel);
   switch (c->tag) {
     case CDEC:
        printf("%d",c->val.i);
     break;
     case CREAL:
        printf("%f",c->val.f);
     break;
     case CSTRING:
        printf("%s",c->val.s);
     break;
   }
   printf("\n");
}

static void print_variavel(Var* v, int nivel) {
  if (v == NULL) return;

  if (v->tipo == NULL) {
    //ainda nao costurado
    printf("%d\t [Variavel] %s\n",nivel,v->id);
  }
  else {
    printf("%d\t [Variavel %s] %s : %s\n",nivel,getStringEscopo(v->escopo),v->id,getStringTipo(v->tipo));
  }
}

static void print_exp_unaria(Exp *e, int nivel){
  if (e == NULL || e->tag != EXP_UNARIA) return;
  switch(e->u.expunaria.opun){
      case opnot:
        printf("%d\t [EXP NOT]",nivel);
      break;
      case opmenos:
        printf("%d\t [EXP MENOS]",nivel);
      break;
  }
  print_exp(e->u.expunaria.exp,nivel+1);
}

static void print_exp_binaria(Exp *e, int nivel){
  if (e == NULL || e->tag != EXP_BIN) return;
  switch(e->u.expbin.opbin){

    case opadd:
      printf("%d\t [EXP ADD : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case opsub:
      printf("%d\t [EXP SUB : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case opmult:
      printf("%d\t [EXP MULT : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case opdiv:
      printf("%d\t [EXP DIV : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case opand:
      printf("%d\t [EXP AND : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case opor:
      printf("%d\t [EXP OR : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case equal:
      printf("%d\t [EXP EQUAL : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case notequal:
      printf("%d\t [EXP NOT EQUAL : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case less:
      printf("%d\t [EXP LESS : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case greater:
      printf("%d\t [EXP GREATER : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case lessequal:
      printf("%d\t [EXP LESS EQUAL : %s]\n",nivel, getStringTipo(e->tipo));
    break;
    case greaterequal:
      printf("%d\t [EXP GREATER EQUAL : %s]\n",nivel, getStringTipo(e->tipo));
    break;
  }
   print_exp(e->u.expbin.expesq,nivel+1);
   print_exp(e->u.expbin.expdir,nivel+1);
}
static void print_exp(Exp *e, int nivel) {
      ExpL *auxparam;
      int nivelparams;

      if(e == NULL) return;
      switch (e->tag) {
        case EXP_BIN:
          print_exp_binaria(e,nivel);
        break;
        case EXP_UNARIA:
          print_exp_unaria(e,nivel);
        break;
        case EXP_CTE:
          printf("%d\t [Exp Constante : %s]\n",nivel, getStringTipo(e->tipo));
          print_constante(e->u.expcte,nivel+1);
        break;
        case EXP_VAR:
          printf("%d\t [Exp Variavel : %s]\n",nivel,getStringTipo(e->tipo));
          print_variavel(e->u.expvar,nivel+1);
        break;
        case EXP_ACESSO:
          printf("%d\t [Exp Acesso : %s]\n",nivel, getStringTipo(e->tipo));
          print_exp(e->u.expacesso.expvar,nivel+1);
          print_exp(e->u.expacesso.expindex,nivel+1);
        break;
        case EXP_CHAMADA:
          auxparam = e->u.expchamada.params;
          nivelparams = nivel;
          if (e->tipo == NULL)
            printf("%d\t [Exp Chamada : %s] %s\n",nivel, getStringTipo(e->tipo), e->u.expchamada.idFunc);
          else
            printf("%d\t [Exp Chamada : %s] %s\n",nivel, getStringTipo(e->tipo),e->u.expchamada.idFunc);
          while (auxparam != NULL) {
            nivelparams++;
            print_exp(auxparam->e,nivel);
            auxparam = auxparam->prox;
          }
        break;
        case EXP_NEW:
          printf("%d [Exp\t New : %s]\n",nivel, getStringTipo(e->tipo));
          print_exp(e->u.expnewas.exp,nivel+1);
          printf("%d\t [Tipo] %s\n",nivel+1,getStringTipo(e->u.expnewas.tipo));
        break;
        case EXP_AS:
          printf("%d\t [Exp As : %s]\n",nivel, getStringTipo(e->tipo));
          print_exp(e->u.expnewas.exp,nivel+1);
          printf("%d\t [Tipo : %s]\n",nivel+1,getStringTipo(e->u.expnewas.tipo));
        break;
        default:
          printf("EXP N RECONHECIDA\n   ");
      }

}

static void print_cmd(CMD *c,int nivel) {
    if(c == NULL) return;
    switch (c->tag) {

      case CMD_ATR:
        printf("%d\t [CMD ATRIBUIÇAO]\n",nivel);
        print_exp(c->u.atr.expvar,nivel+1);
        print_exp(c->u.atr.exp,nivel+1);
      break;
      case CMD_WHILE:
        printf("%d\t [CMD WHILE]\n",nivel);
        print_exp(c->u.cmdwhile.exp,nivel+1);
        print_bloco(c->u.cmdwhile.bloco,nivel+1);
      break;
      case CMD_IF:
        printf("%d\t [CMD IF]\n",nivel);
        print_exp(c->u.cmdif.exp,nivel+1);
        print_bloco(c->u.cmdif.bloco,nivel+1);
      break;
      case CMD_IFELSE:
        printf("%d\t [CMD IF ELSE]\n",nivel);
        print_exp(c->u.cmdifelse.exp,nivel+1);
        print_bloco(c->u.cmdifelse.blocoif,nivel+1);
        print_bloco(c->u.cmdifelse.blocoelse,nivel+1);
      break;
      case CMD_RETURN:
        printf("%d\t [CMD RETURN]\n",nivel);
        print_exp(c->u.exp,nivel+1);
      break;
      case CMD_RETURNVOID:
        printf("%d\t [CMD RETURN VOID]\n",nivel);
      break;
      case CMD_CHAMADA:
        printf("%d\t [CMD CHAMADA]\n",nivel);
        print_exp(c->u.exp,nivel+1);
      break;
      case CMD_BLOCK:
        printf("%d\t [CMD BLOCK]\n",nivel);
        print_bloco(c->u.bloco,nivel+1);
      break;
      case CMD_PRINT:
        printf("%d\t [CMD PRINT]\n",nivel);
        print_exp(c->u.exp,nivel+1);
      break;
      case CMD_SKIP:
        printf("%d\t [CMD SKIP]\n",nivel);
      break;
      default:
        printf("TAG CMD ERRADO");
    }
}


static void print_bloco (Bloco *b, int nivel) {
    DefVarL *aux = b->declVars;
    CMDL *auxcmd = b->cmds;
    int nivelaux = nivel;

    if (b == NULL) return;
    printf ("%d\t [Bloco]\n",nivel);
    while (aux != NULL){
      nivelaux++;
      printf("%d\t [Def Var] %s : %s\n",nivelaux,aux->v->id,getStringTipo(aux->v->tipo));
      aux = aux->prox;
    }
    nivelaux = nivel;
    while (auxcmd != NULL) {
      nivelaux++;
      print_cmd(auxcmd->c,nivelaux);
      auxcmd = auxcmd->prox;
    }

}

static void print_defvar(DefVar * dvar,int nivel) {
    if (dvar == NULL) return;

    char *tipostr = getStringTipo(dvar->tipo);
    if(dvar->escopo == 0 ) //Global def
      printf("%d\t [Def Variavel Global] %s : %s\n",nivel,dvar->id,tipostr);
    else //EscopoLocal
      printf("%d\t [Def Variavel Local] %s : %s\n",nivel,dvar->id,tipostr);
}

static void print_deffunc(DefFunc * dfunc,int nivel) {

    char *tiporetstr;
    ParametroL* aux = dfunc->params;
    int parametroCount = 0;
    if(dfunc == NULL) return;
    tiporetstr = getStringTipo(dfunc->tiporet);
    printf("%d\t [Def Funcao] %s : %s \n",nivel,dfunc->id,tiporetstr);
    while ( aux != NULL) {
      char *tipoparam = getStringTipo(aux->tipo);
      printf("%d\t [Parametro %d] %s : %s\n",nivel,parametroCount+1,aux->id,tipoparam);
      parametroCount++;
      aux = aux->prox;
    }
    print_bloco(dfunc->bloco,nivel+1  );
}

static void print_defs(Definicao* d, int nivel) {
    Definicao *aux = d;
    while (aux != NULL) {
      printf("%d\t [Definicao]\n",nivel);
      if(aux->tag == DVar){
          print_defvar(aux->u.v,nivel+1);
      }
      else if (aux->tag == DFunc) {
        print_deffunc(aux->u.f,nivel+1);
      }
      nivel++;
      aux = aux->prox;
    }
}

void print_tree (Programa *p) {
  int nivel = 0;
  if (p != NULL){
    printf("Nivel\t No\n");
    printf("%d\t [Programa]\n",nivel);
    print_defs(p->defs,nivel+1);
  }
  else
    printf("[Programa vazio]\n");
}
