#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TabelaSimbolos.h"

typedef enum SIM_TAG { SVar, SFunc, SParams } SIM_Tag;
typedef struct simbolo
{
  SIM_Tag tag;
  union {
    DefVar *v;
    DefFunc *f;
    ParametroL *p;
  }u;
  int escopo;
  struct simbolo *prim;
  struct simbolo *ant;
  struct simbolo *prox;
} Simbolo;

static Simbolo * destroi_simbolo_final(Simbolo *s);
static Simbolo* cria_no_final(Simbolo *s, int nivelEscopo);
static Simbolo* cria_no_var(Simbolo *s, DefVar *v, int nivelEscopo);
static Simbolo* cria_no_func(Simbolo *s, DefFunc *f, int nivelEscopo);
static Simbolo* cria_no_param(Simbolo *s, ParametroL *p, int nivelEscopo);
static Simbolo* proc_no_func(Simbolo *s, const char *id);


static int verifica_tipo(Tipo *t, Base_TAG base);
static int cmp_tipo_base(Tipo *t1, Tipo *t2);
static int cmp_tipo_array(Tipo *t1, Tipo *t2);
static void tipa_variavel(Simbolo *s, Var *v, int nivelEscopo);
static Exp* promove_tipo(Exp *e, Base_TAG tipo_base);


static Exp* promove_tipo(Exp *e, Base_TAG tipo_base);
static void tipa_expvar(Simbolo *s, Exp *e, int nivelEscopo);
static void tipa_expchamada(Simbolo *s,Exp *e, int nivelEscopo);
static void tipa_expacesso(Simbolo *s,Exp *e, int nivelEscopo);
static void tipa_expconstante(Simbolo *s, Exp *e, int nivelEscopo);
static void tipa_expas(Simbolo *s, Exp *e, int nivelEscopo);
static void tipa_expnew(Simbolo *s, Exp *e, int nivelEscopo);
static void tipa_expbin(Simbolo*s, Exp *e, int nivelEscopo);
static void tipa_expunaria(Simbolo *s, Exp *e, int nivelEscopo);
static void tipa_expressao(Simbolo *s, Exp *e, int nivelEscopo);

static void checa_tipo_cmdatr(Simbolo *s, CMD *cmd, int nivelEscopo);
static void checa_tipo_expcond(Simbolo*s, Exp*e, int nivelEscopo);
static void tipa_comando(Simbolo*s, CMD *cmd, int nivelEscopo);

static Simbolo* tipa_bloco(Simbolo *s, Bloco *b,int nivelEscopo);

//para debug apenas, printa estrutura auxilar usada para costurar a arvore
static void print_simbolos(Simbolo *s);

/*
* implementations
*/

static void print_simbolos(Simbolo *s) {
    if (s == NULL) return;
    s = s->prim;
    printf("Printando Simbolos\n");
    while (s != NULL) {
        ParametroL *aux;
        switch (s->tag) {
          case SVar:
            printf("%d [NoVar] %s -> ",s->escopo,s->u.v->id);
          break;
          case SFunc:
            printf("%d [NoFunc] %s -> ",s->escopo,s->u.f->id);
          break;
          case SParams:
            aux = s->u.p;
            while(aux != NULL) {
              printf("%d [NoParam] %s -> ",s->escopo,aux->id);
              aux = aux->prox;
            }
          break;
        }
      s = s->prox;
    }
    printf("\n");
}

static Simbolo * destroi_simbolo_final(Simbolo *s) {
  Simbolo *aux = s;
  if (s->ant == NULL) {
    s = NULL;
  }
  else{
    s->ant->prox = s->prox;
    s = s->ant;
  }
  free(aux);
  return s;
}

static Simbolo* cria_no_final (Simbolo *s, int nivelEscopo) {
    Simbolo *nv = (Simbolo*)malloc(sizeof(Simbolo));
    if(nv == NULL) {
      printf("sem memoria costura\n");
      exit(1);
    }

    nv->escopo = nivelEscopo;
    nv->prox = NULL;
    if (s == NULL){
        nv->prim = nv;
        nv->ant = NULL;
    }
    else {
      nv->prim = s->prim;
      nv->ant = s;
      s->prox = nv;
    }
    return nv;
}

static Simbolo* cria_no_var(Simbolo *s, DefVar *v, int nivelEscopo) {
    Simbolo *aux,*nv;

    aux = s;
    while(aux != NULL && aux->escopo == nivelEscopo) {
      if (aux->tag == SVar) {
          if (strcmp(aux->u.v->id, v->id) == 0) {
            printf("[Erro] Redefinicao da variavel %s\n",v->id);
            exit(1);
          }
      }
      else if (aux->tag == SParams) {
        ParametroL *p = aux->u.p;
        while(p != NULL) {
          if(strcmp(p->id, v->id) == 0) {
            printf("[Erro] Redefinicao da variavel \"%s\"\n",v->id);
            exit(1);
          }
          p = p->prox;
        }
      }
      else if (aux->tag == SFunc) {
        if (strcmp(aux->u.f->id, v->id) == 0) {
          printf("[Erro] Redefinicao de \"%s\" para um simbolo diferente\n",v->id);
          exit(1);
        }
      }
      aux = aux->ant;
    }

    nv = cria_no_final(s,nivelEscopo);
    nv->tag = SVar;
    nv->u.v = v;
    return nv;
}

static Simbolo* cria_no_func(Simbolo *s, DefFunc *f, int nivelEscopo) {
    Simbolo *aux,*nv;
    aux = s;
    while(aux != NULL /*&& aux->escopo == nivelEscopo*/) {
        if (aux->tag == SVar) {
            if (strcmp(aux->u.v->id, f->id) == 0) {
              printf("[Erro] Redefinicao de \"%s\" para um simbolo diferente\n",f->id);
              exit(1);
            }
        }
        else if (aux->tag == SFunc) {
          if (strcmp(aux->u.f->id, f->id) == 0) {
              printf("[Erro] Redefinicao da funcao \"%s\"\n",f->id);
              exit(1);
          }
      }
      aux = aux->ant;
    }
    nv = cria_no_final(s,nivelEscopo);
    nv->tag = SFunc;
    nv->u.f = f;
    return nv;
}

static Simbolo* cria_no_param(Simbolo *s, ParametroL *p, int nivelEscopo) {
    Simbolo *nv = cria_no_final(s,nivelEscopo);
    nv->tag = SParams;
    nv->u.p = p;
    return nv;
}

static Simbolo* proc_no_func(Simbolo *s, const char *id) {

  while (s != NULL) {
    if (s->tag == SFunc) {
      if (strcmp(s->u.f->id,id) == 0){
        return s;
      }
    }
    s = s->ant;
  }
  return NULL;
}

/*
* AUXILIAR TIPOS
*/
static int verifica_tipo(Tipo *t, Base_TAG base) {
  if(t->tag == array)
    return 0;
  if(t->tipo_base == base)
    return 1;
  return 0;
}

static int cmp_tipo_base(Tipo *t1, Tipo *t2) {
  if(t1->tipo_base == bChar && t2->tipo_base == bInt) return 0; //equivalencia de char e int
  if(t1->tipo_base == bInt && t2->tipo_base == bChar) return 0; //equivalencia de char e int
    if(t1->tipo_base < t2->tipo_base)
      return -1;
    else if (t1->tipo_base > t2->tipo_base)
      return 1;
    return 0;
}

static int cmp_tipo_array(Tipo *t1, Tipo *t2){

  if (t1->tag != array || t2->tag != array) return -1;
  if (cmp_tipo_base(t1,t2) == 0) {
    Tipo *aux1,*aux2;
    aux1 = t1->de;
    aux2 = t2->de;
    while (aux1 != NULL && aux2 != NULL) {
      aux1 = aux1->de;
      aux2 = aux2->de;
    }
    if (aux1 != NULL || aux2 != NULL)
      return -1;
    else
      return 0;
  }
  return -1;
}

static void tipa_variavel(Simbolo *s, Var *v, int nivelEscopo) {
    ParametroL *p;
    while (s != NULL){
      if (s->tag == SVar) {
        if(strcmp(s->u.v->id,v->u.vvar.id) == 0) {
          v->u.vvar.d.def = s->u.v;
          v->tipo = s->u.v->tipo;
          v->u.vvar.escopo = v->u.vvar.d.def->escopo;
          if(v->tipo->tipo_base == bVoid) {
            printf("[Erro] %s nao pode ser declarada com tipo void\n",v->u.vvar.id);
            exit(1);
          }
          return;
        }
      }
      else if (s->tag == SParams) {
        p = s->u.p;
        while (p != NULL) {
          if (strcmp(p->id,v->u.vvar.id) == 0){
            v->u.vvar.escopo = EscopoFunc;
            v->u.vvar.d.defp = p;
            v->tipo = p->tipo;
            if(v->tipo->tipo_base == bVoid) {
              printf("[Erro] %s nao pode ser declarada com tipo void\n",v->u.vvar.id);
              exit(1);
            }
            return;
          }
          p = p->prox;
        }
      } /* fim else if */
      s = s->ant;
    }
    printf("[Erro] variavel %s nao declarada\n",v->u.vvar.id);
    exit(1);
}

/*
* TIPAGEM EXPRESSOES
*/

static Exp* promove_tipo(Exp *e, Base_TAG tipo_base) {

    Exp * expas = NULL;
    Tipo *nvtipo = NULL;

    nvtipo = (Tipo*)malloc(sizeof(Tipo));
    if (nvtipo == NULL) {printf("memoria insuficiente\n");exit(1);}

    nvtipo->tag = base;
    nvtipo->tipo_base = tipo_base;
    nvtipo->de = NULL;

    expas = (Exp*)malloc(sizeof(Exp));
    if (expas == NULL) {printf("memoria insuficiente\n");exit(1);}
    expas->tag = EXP_AS;
    expas->tipo = nvtipo;
    expas->u.expnewas.exp = e;
    expas->u.expnewas.tipo = nvtipo;

    return expas;
}

static void tipa_expvar(Simbolo *s, Exp *e, int nivelEscopo) {

  Tipo *t;
  if(e->u.expvar->tag == vVar) {
    tipa_variavel(s,e->u.expvar,nivelEscopo);

    //promovendo expvar char para int
    t = (Tipo *)malloc(sizeof(Tipo));
    if (t == NULL) {printf("falta de memoria\n");exit(1);}
    t->de = NULL;
    t->tag = base;
    if(e->u.expvar->tipo->tipo_base == bChar){
      t->tipo_base = bInt;
    }
    else {
      t->tipo_base = e->tipo->tipo_base;
    }
    e->tipo = t;
  }
  else if (e->u.expvar->tag == vAcesso) {
    tipa_expacesso(s,e,nivelEscopo);
  }
}

static void tipa_expchamada(Simbolo *s,Exp *e, int nivelEscopo) {
  Simbolo *aux = NULL;
  ExpL *explist = NULL;
  ParametroL *paramlist = NULL;;

  //achando definicao da funcao
  aux = proc_no_func(s,e->u.expchamada.idFunc);
  if (aux != NULL) {
    e->u.expchamada.def = aux->u.f;
    e->tipo = aux->u.f->tiporet;
  }
  else {
    printf("[Erro] funcao %s nao declarada\n",e->u.expchamada.idFunc);
    exit(1);
  }
  //comparando parametros da chamada(explist) com a declaracao(paramlist)
  paramlist = e->u.expchamada.def->params;
  explist = e->u.expchamada.params;
  while (explist != NULL) {
    if (paramlist == NULL){
      printf("[Erro] argumentos a mais em chamada de \"%s\"\n",e->u.expchamada.idFunc);
      exit(1);
    }
    tipa_expressao(s,explist->e,nivelEscopo);
    if (explist->e->tipo->tag != paramlist->tipo->tag) {
      printf("[Erro] tipo de parametro na chamada de funcao \"%s\" inesperado\n",e->u.expchamada.idFunc);
      exit(1);
    }
    if(explist->e->tipo->tag == base){
      int resp = cmp_tipo_base(explist->e->tipo,paramlist->tipo);
      //se tipo da chamada for menos expressivo faz a conversao para tipo mais expressivo
      if (resp < 0){
        Exp *expas = promove_tipo(explist->e,paramlist->tipo->tipo_base);
        expas->u.expnewas.exp = explist->e;
        explist->e = expas;
      }
      //se tipo da chamada for mais expressivo da erro de tipagem
      else if (resp > 0) {
        printf("[Erro] tipo de parametro na chamada de funcao \"%s\" inesperado\n",e->u.expchamada.idFunc);
        exit(1);
      }
    }
    else if (cmp_tipo_array(explist->e->tipo,paramlist->tipo) != 0) {
      printf("[Erro] tipo de parametro na chamada de funcao \"%s\" inesperado\n",e->u.expchamada.idFunc);
      exit(1);
    }
    explist = explist->prox;
    paramlist = paramlist->prox;
  }
  if (paramlist != NULL) {
    printf("[Erro] faltou argumentos em chamada de \"%s\"\n",e->u.expchamada.idFunc);
    exit(1);
  }
  return;
}

static void tipa_expacesso(Simbolo *s,Exp *e, int nivelEscopo) {

  tipa_expressao(s,e->u.expvar->u.vacesso.expvar,nivelEscopo);
  tipa_expressao(s,e->u.expvar->u.vacesso.expindex,nivelEscopo);

  if (verifica_tipo(e->u.expvar->u.vacesso.expindex->tipo,bInt)) {
    Tipo *nvtipo = NULL;

    nvtipo = (Tipo*) malloc(sizeof(Tipo));
    if (nvtipo == NULL) {printf("memoria insuficiente");exit(1);}
    nvtipo->tag = base;
    nvtipo->tipo_base = e->u.expvar->u.vacesso.expvar->tipo->tipo_base;
    nvtipo->de = NULL;
    e->tipo = nvtipo;
  }
  else {
    printf("[Erro Tipo] Esperado index tipo int encontrando %s\n",getStringTipo (e->u.expvar->u.vacesso.expindex->tipo));
    exit(1);
  }
}

static void tipa_expconstante(Simbolo *s, Exp *e, int nivelEscopo) {
    Tipo *t;
    t = (Tipo*) malloc(sizeof(Tipo));
    if (t == NULL) {printf("falta de memoria\n");exit(1);}
    t->tag = base;
    t->de = NULL;
    e->tipo = t;
    switch (e->u.expcte->tag) {
      case CDEC:
        t->tipo_base = bInt;
      break;
      case CREAL:
        t->tipo_base = bFloat;
      break;
      case CSTRING:
        t->tipo_base = bVoid; /* TODO encarar constantes string como tipo void mesmo?*/
      break;
    }
}

static void tipa_expas(Simbolo *s, Exp *e, int nivelEscopo) {
  //TODO o que eu deveria aceitar no AS? "x as int[]" qual o tipo disso?
  tipa_expressao(s,e->u.expnewas.exp,nivelEscopo);
  e->tipo = e->u.expnewas.exp->tipo;
}

static void tipa_expnew(Simbolo *s, Exp *e, int nivelEscopo) {
  Tipo *t;
  tipa_expressao(s,e->u.expnewas.exp,nivelEscopo);
  t = (Tipo*) malloc(sizeof(Tipo));
  if (t == NULL) {printf("falta de memoria\n");exit(1);}
  t->tag = base;
  t->tipo_base = bVoid;
  t->de = NULL;
  e->tipo = t;
}

static void tipa_expbin(Simbolo*s, Exp *e, int nivelEscopo) {
  Tipo *nv;
  tipa_expressao(s, e->u.expbin.expesq,nivelEscopo);
  tipa_expressao(s, e->u.expbin.expdir,nivelEscopo);

  if (e->u.expbin.expesq->tipo->tag != base || e->u.expbin.expdir->tipo->tag != base){
    printf("[Erro] tipo invalido em soma\n");exit(1);
  }

  if (verifica_tipo(e->u.expbin.expesq->tipo,bVoid) || verifica_tipo(e->u.expbin.expdir->tipo,bVoid)){
    printf("[Erro] Operacao nao pode ser feita com tipo Void\n");
    exit(1);
  }

  switch (e->u.expbin.opbin) {
    case opadd:
    case opsub:
    case opmult:
    case opdiv:
      //promovendo tipo "menor" para tipo "maior"
      if(cmp_tipo_base(e->u.expbin.expesq->tipo, e->u.expbin.expdir->tipo) > 0){
        Exp *expas = promove_tipo(e->u.expbin.expdir,e->u.expbin.expesq->tipo->tipo_base);
        e->u.expbin.expdir = expas;
        e->tipo = e->u.expbin.expesq->tipo;
      }
      else if (cmp_tipo_base(e->u.expbin.expesq->tipo, e->u.expbin.expdir->tipo) < 0){
        Exp *expas = promove_tipo(e->u.expbin.expesq,e->u.expbin.expdir->tipo->tipo_base);
        e->u.expbin.expesq = expas;
        e->tipo = e->u.expbin.expdir->tipo;
      }
      else {
        e->tipo = e->u.expbin.expesq->tipo;
      }
    break;
    case equal:
    case notequal:
    case less:
    case lessequal:
    case greater:
    case greaterequal:

      nv = (Tipo*) malloc(sizeof(Tipo));
      if (nv == NULL) {printf("falta de memoria\n");exit(1);}
      nv->tag = base;
      nv->tipo_base = bInt;
      nv->de = NULL;
      if(cmp_tipo_base(e->u.expbin.expesq->tipo, e->u.expbin.expdir->tipo) > 0){
        Exp *expas = promove_tipo(e->u.expbin.expdir,e->u.expbin.expesq->tipo->tipo_base);
        e->u.expbin.expdir = expas;
        e->tipo = e->u.expbin.expesq->tipo;
      }
      else if (cmp_tipo_base(e->u.expbin.expesq->tipo, e->u.expbin.expdir->tipo) < 0){
        Exp *expas = promove_tipo(e->u.expbin.expesq,e->u.expbin.expdir->tipo->tipo_base);
        e->u.expbin.expesq = expas;
        e->tipo = e->u.expbin.expdir->tipo;
      }
      else {
        e->tipo = e->u.expbin.expesq->tipo;
      }
      e->tipo = nv;
    break;
    case opand:
    case opor:
      if (verifica_tipo(e->u.expbin.expesq->tipo,bInt) == 0) {
        printf("[Erro] expressao esperava valor int e encontrou %s",getStringTipo(e->u.expbin.expesq->tipo));
        exit(1);
      }
      if (verifica_tipo(e->u.expbin.expdir->tipo,bInt) == 0 ) {
        printf("[Erro] expressao esperava valor int e encontrou %s",getStringTipo(e->u.expbin.expdir->tipo));
      }

      nv = (Tipo*) malloc(sizeof(Tipo));
      if (nv == NULL) {printf("falta de memoria\n");exit(1);}
      nv->tag = base;
      nv->tipo_base = bInt;
      nv->de = NULL;
      e->tipo = nv;
    break;
  }
}

static void tipa_expunaria(Simbolo *s, Exp *e, int nivelEscopo) {
  tipa_expressao(s,e->u.expunaria.exp,nivelEscopo);
  //o que deveria aceitar aqui? !x[] !new int [10]??
  //aceitando apenas tipos base
  if (e->u.expunaria.exp->tipo->tag != base && e->u.expunaria.exp->tipo->tipo_base != bVoid){
    printf("[Erro] tipo inesperado para operador unario\n");
    exit(1);
  }
  e->tipo = e->u.expunaria.exp->tipo;
}

static void tipa_expressao(Simbolo *s, Exp *e, int nivelEscopo) {
  //Simbolo *aux = NULL;
  if (e == NULL)
    return;

  switch (e->tag) {

    case EXP_BIN:
      tipa_expbin(s,e,nivelEscopo);
    break;
    case EXP_UNARIA:
      tipa_expunaria(s,e,nivelEscopo);
    break;
    case EXP_VAR:
      tipa_expvar(s,e,nivelEscopo);
    break;
    case EXP_CHAMADA:
      tipa_expchamada(s,e,nivelEscopo);
    break;
    case EXP_AS:
      tipa_expas(s,e,nivelEscopo);
    break;
    case EXP_NEW:
      tipa_expnew(s,e,nivelEscopo);
    break;
    case EXP_CTE:
      tipa_expconstante(s,e,nivelEscopo);
    break;
  }
  return;
}

static void checa_tipo_cmdatr(Simbolo *s, CMD *cmd, int nivelEscopo) {
  tipa_variavel(s,cmd->u.atr.var,nivelEscopo);
  tipa_expressao(s,cmd->u.atr.exp,nivelEscopo);

  if(cmd->u.atr.var->tipo->tipo_base == bVoid && cmd->u.atr.exp->tipo->tipo_base == bVoid){
    printf("[Erro] atribuicao invalida de tipo \"%s\" para tipo \"%s\"\n",getStringTipo(cmd->u.atr.exp->tipo),getStringTipo(cmd->u.atr.var->tipo));
    exit(1);
  }

  if(cmd->u.atr.var->tipo->tag == base && cmd->u.atr.exp->tipo->tag == base) {
    int resp = cmp_tipo_base(cmd->u.atr.exp->tipo, cmd->u.atr.var->tipo);
    if (resp < 0){
      //promove tipo da expressao para o tipo da variavel
      Exp *expas;
      expas = promove_tipo(cmd->u.atr.exp,cmd->u.atr.var->tipo->tipo_base);
      expas->u.expnewas.exp = cmd->u.atr.exp;
      cmd->u.atr.exp = expas;
    }
    if (resp > 0) {
      //tipo da expressao é mais expressivo que da variavel, nao converte
      printf("[Erro] atribuicao invalida de tipo \"%s\" para tipo \"%s\"\n",getStringTipo(cmd->u.atr.exp->tipo),getStringTipo(cmd->u.atr.var->tipo));
      exit(1);
    }
  }
  else if (cmd->u.atr.var->tipo->tag == array && cmd->u.atr.exp->tipo->tag == array) {
    if(cmp_tipo_array(cmd->u.atr.var->tipo,cmd->u.atr.exp->tipo) != 0){
      printf("[Erro] atribuicao invalida de tipo \"%s\" para tipo \"%s\"\n",getStringTipo(cmd->u.atr.exp->tipo),getStringTipo(cmd->u.atr.var->tipo));
      exit(1);
    }
  }
  else {
    printf("[Erro] atribuicao invalida de tipo \"%s\" para tipo \"%s\"\n",getStringTipo(cmd->u.atr.exp->tipo),getStringTipo(cmd->u.atr.var->tipo));
    exit(1);
  }
  return;
}

static void checa_tipo_expcond(Simbolo*s, Exp*e, int nivelEscopo) {

  tipa_expressao(s,e,nivelEscopo);
  if (e->tipo->tag != base && e->tipo->tipo_base != bInt) {
      printf("[Erro] tipo condicao inesperado, ao inves de \"int\" foi dado \"%s\"",getStringTipo(e->tipo));
  }
}

static void tipa_comando(Simbolo*s, CMD *cmd, int nivelEscopo) {
  switch (cmd->tag) {
    case CMD_BLOCK:
      s = tipa_bloco(s,cmd->u.bloco,nivelEscopo+1);
    break;
    case CMD_ATR:
      checa_tipo_cmdatr(s,cmd,nivelEscopo);
    break;
    case CMD_WHILE:
      checa_tipo_expcond(s,cmd->u.cmdwhile.exp,nivelEscopo);
      s = tipa_bloco(s,cmd->u.cmdwhile.bloco,nivelEscopo+1);
    break;
    case CMD_IF:
      checa_tipo_expcond(s,cmd->u.cmdif.exp,nivelEscopo);
      s = tipa_bloco(s,cmd->u.cmdif.bloco,nivelEscopo+1);
    break;
    case CMD_IFELSE:
      checa_tipo_expcond(s,cmd->u.cmdifelse.exp,nivelEscopo);
      s = tipa_bloco(s,cmd->u.cmdifelse.blocoif,nivelEscopo+1);
      s = tipa_bloco(s,cmd->u.cmdifelse.blocoelse,nivelEscopo+1);
    break;
    case CMD_RETURN:
      //TODO checa tipo
    case CMD_PRINT:
      //TODO checa tipo
    case CMD_CHAMADA:
      tipa_expressao(s,cmd->u.exp,nivelEscopo);
    break;
    default:
    break;
  }

}

static Simbolo* tipa_bloco(Simbolo *s, Bloco *b,int nivelEscopo) {

    DefVarL *auxvars;
    CMDL *auxcmds;

    if (b == NULL) return s;

    auxvars = b->declVars;
    while (auxvars != NULL) {
      s = cria_no_var(s,auxvars->v,nivelEscopo);
      auxvars = auxvars->prox;
    }

    auxcmds = b->cmds;
    while(auxcmds != NULL) {
      CMD *cmd = auxcmds->c;
      tipa_comando(s,cmd,nivelEscopo);
      auxcmds = auxcmds->prox;
    }

    //print_simbolos(s);
    //desempilha variaveis do escopo
    while (s != NULL && s->escopo >= nivelEscopo) {
      s = destroi_simbolo_final(s);
    }
    return s;
}

void tipa_arvore(Programa *p) {

    Definicao *auxdef;
    Simbolo *s = NULL;
    int nivelEscopo = 0;

    if (p == NULL) return;

    //todas variaveis globais e declaracoes de funcoes
    auxdef = p->defs;
    while (auxdef != NULL) {
        if (auxdef->tag == DVar) {
          s = cria_no_var(s,auxdef->u.v ,nivelEscopo);
        }
        if (auxdef->tag == DFunc) {
          s = cria_no_func(s,auxdef->u.f,nivelEscopo);
        }
        auxdef = auxdef->prox;
      }
      print_simbolos(s);
    auxdef = p->defs;
    while (auxdef != NULL) {
        if (auxdef->tag == DFunc) {
          //s = cria_no_func(s,auxdef->u.f,nivelEscopo);
          s = cria_no_param(s,auxdef->u.f->params,nivelEscopo+1);

          s = tipa_bloco(s,auxdef->u.f->bloco,nivelEscopo+1);
        }
        auxdef = auxdef->prox;
    }
}
