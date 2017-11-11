#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabelaSimbolos.h"

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
static Simbolo* proc_id_func(Simbolo *s, const char *id);
static int cmp_tipo_base(Tipo *t1, Tipo *t2);

int verifica_tipo(Tipo *t, Base_TAG base);
static void tipa_variavel(Simbolo *s, Var *v, int nivelEscopo);
static void tipa_expvar(Simbolo *s, Exp *e, int nivelEscopo);
static void tipa_expconstante (Exp *e);
static void tipa_expnew (Exp *e);
static Simbolo* tipa_expressao(Simbolo *s, Exp *e, int nivelEscopo);
static void tipa_comando(Simbolo*s, CMD *cmd, int nivelEscopo);
static Simbolo* tipa_bloco(Simbolo *s, Bloco *b,int nivelEscopo);

//para debug apenas, printa estrutura auxilar usada para costurar a arvore
static void print_simbolos(Simbolo *s);


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

/*
*  retorn a -1 se o t1 é menos expressivo -2 se é mais expressivo e 0 se são iguais
*/
static int cmp_tipo_base(Tipo *t1, Tipo *t2) {
    if(t1->tipo_base < t2->tipo_base)
      return -1;
    else if (t1->tipo_base > t2->tipo_base)
      return 1;
    return 0;
}

static Simbolo * destroi_simbolo_final(Simbolo *s){
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
            printf("[Erro] Redefinicao da variavel %s\n",v->id);
            exit(1);
          }
          p = p->prox;
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
    Simbolo *nv = cria_no_final(s,nivelEscopo);
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

int verifica_tipo(Tipo *t, Base_TAG base) {
  if(t->tag == array)
    return 0;
  if(t->tipo_base == base)
    return 1;
  return 0;
}

static void tipa_variavel(Simbolo *s, Var *v, int nivelEscopo) {
    ParametroL *p;
    while (s != NULL){
      if (s->tag == SVar) {
        if(strcmp(s->u.v->id,v->id) == 0) {
          v->u.def = s->u.v;
          v->tipo = s->u.v->tipo;
          v->escopo = v->u.def->escopo;
          if(v->tipo->tipo_base == bVoid) {
            printf("[Erro] %s nao pode ser declarada com tipo void\n",v->id);
            exit(1);
          }
          return;
        }
      }
      else if (s->tag == SParams) {
        p = s->u.p;
        while (p != NULL) {
          if (strcmp(p->id,v->id) == 0){
            v->escopo = EscopoFunc;
            v->u.defp = p;
            v->tipo = p->tipo;
            if(v->tipo->tipo_base == bVoid) {
              printf("[Erro] %s nao pode ser declarada com tipo void\n",v->id);
              exit(1);
            }
            return;
          }
          p = p->prox;
        }
      } /* fim else if */
      s = s->ant;
    }
    printf("[Erro] variavel %s nao declarada\n",v->id);
    exit(1);
}

static void tipa_expvar(Simbolo *s, Exp *e, int nivelEscopo) {
  Tipo *t = NULL;
  tipa_variavel(s,e->u.expvar,nivelEscopo);

  //promovendo expvar char para int
  if (e->u.expvar->tipo->tipo_base == bChar) {
    t = (Tipo *)malloc(sizeof(Tipo));
    if (t == NULL) {printf("falta de memoria\n");exit(1);}
    t->tag = base;
    t->tipo_base = bInt;
    t->de = NULL;
    e->tipo = t;
  }
  else {
    e->tipo = e->u.expvar->tipo;
  }
}

static Simbolo* proc_id_func(Simbolo *s, const char *id) {

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

static Simbolo* tipa_chamada(Simbolo *s,Exp *e, int nivelEscopo) {
  Simbolo *aux = NULL;
  ExpL *explist = NULL;
  aux = proc_id_func(s,e->u.expchamada.idFunc);
  if (aux != NULL) {
    e->u.expchamada.def = aux->u.f;
    e->tipo = aux->u.f->tiporet;
  }
  else {
    printf("[Erro] funcao %s nao declarada\n",e->u.expchamada.idFunc);
    exit(1);
  }
  //TODO tipar parametros!
  explist = e->u.expchamada.params;
  while (explist != NULL) {
    s = tipa_expressao(s,explist->e,nivelEscopo+1);
    explist = explist->prox;
  }
  return s;
}

static void tipa_expacesso(Exp *e) {
  if (verifica_tipo(e->u.expacesso.expindex->tipo,bInt)) {
    e->tipo = e->u.expacesso.expvar->tipo;
  }
  else {
    printf("[Erro Tipo] Esperado index tipo int encontrando %s\n",getStringTipo (e->u.expacesso.expindex->tipo));
    exit(1);
  }
}
static void tipa_expconstante (Exp *e) {
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

static void tipa_expnew (Exp *e) {
  Tipo *t;
  t = (Tipo*) malloc(sizeof(Tipo));
  if (t == NULL) {printf("falta de memoria\n");exit(1);}
  t->tag = base;
  t->tipo_base = bVoid;
  t->de = NULL;
  e->tipo = t;
}

static Simbolo* tipa_expbin (Simbolo*s, Exp *e, int nivelEscopo){
  Tipo *menor, *maior, *nv;
  s = tipa_expressao(s, e->u.expbin.expesq,nivelEscopo);
  s = tipa_expressao(s, e->u.expbin.expdir,nivelEscopo);
  if (verifica_tipo(e->u.expbin.expesq->tipo,bVoid) || verifica_tipo(e->u.expbin.expdir->tipo,bVoid)){
    printf("[Erro] Operacao nao pode ser feita com tipo Void\n");
    exit(1);
  }
  switch (e->u.expbin.opbin) {
    case opadd:
    case opsub:
    case opmult:
    case opdiv:
      if(cmp_tipo_base(e->u.expbin.expesq->tipo, e->u.expbin.expdir->tipo) > 0){
        e->tipo = e->u.expbin.expesq->tipo;
      }
      else if (cmp_tipo_base(e->u.expbin.expesq->tipo, e->u.expbin.expdir->tipo) < 0){
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
        e->tipo = e->u.expbin.expesq->tipo;
      }
      else if (cmp_tipo_base(e->u.expbin.expesq->tipo, e->u.expbin.expdir->tipo) < 0){
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
  return s;
}

static Simbolo* tipa_expressao(Simbolo *s, Exp *e, int nivelEscopo) {
  //Simbolo *aux = NULL;
  if (e == NULL) return s;

  switch (e->tag) {

    case EXP_BIN:
      s = tipa_expbin(s,e,nivelEscopo);
    break;
    case EXP_UNARIA:
      s = tipa_expressao(s,e->u.expunaria.exp,nivelEscopo);
      e->tipo = e->u.expunaria.exp->tipo;
    break;
    case EXP_VAR:
      tipa_expvar(s,e,nivelEscopo);
    break;
    case EXP_ACESSO:
      s = tipa_expressao(s,e->u.expacesso.expvar,nivelEscopo);
      s = tipa_expressao(s,e->u.expacesso.expindex,nivelEscopo);
      tipa_expacesso(e);
    break;
    case EXP_CHAMADA:
      tipa_chamada(s,e,nivelEscopo);
    break;
    case EXP_AS:
        s = tipa_expressao(s,e->u.expnewas.exp,nivelEscopo);
        e->tipo = e->u.expnewas.exp->tipo;
        break;
    case EXP_NEW:
        s = tipa_expressao(s,e->u.expnewas.exp,nivelEscopo);
        tipa_expnew(e);
    break;
    case EXP_CTE:
        tipa_expconstante(e);
    break;
  }
  return s;
}

static void tipa_comando(Simbolo*s, CMD *cmd, int nivelEscopo) {
  switch (cmd->tag) {
    case CMD_BLOCK:
      tipa_bloco(s,cmd->u.bloco,nivelEscopo+1);
    break;
    case CMD_ATR:
      tipa_expressao(s,cmd->u.atr.expvar,nivelEscopo);
      s = tipa_expressao(s,cmd->u.atr.exp,nivelEscopo);
    break;
    case CMD_WHILE:
      s = tipa_expressao(s,cmd->u.cmdwhile.exp,nivelEscopo);
      tipa_bloco(s,cmd->u.cmdwhile.bloco,nivelEscopo+1);
    break;
    case CMD_IF:
      s = tipa_expressao(s,cmd->u.cmdif.exp,nivelEscopo);
      tipa_bloco(s,cmd->u.cmdif.bloco,nivelEscopo+1);
    break;
    case CMD_IFELSE:
      s = tipa_expressao(s,cmd->u.cmdifelse.exp,nivelEscopo);
      tipa_bloco(s,cmd->u.cmdifelse.blocoif,nivelEscopo+1);
      tipa_bloco(s,cmd->u.cmdifelse.blocoelse,nivelEscopo+1);
    break;
    case CMD_RETURN:
    case CMD_PRINT:
    case CMD_CHAMADA:
      s = tipa_expressao(s,cmd->u.exp,nivelEscopo);
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
