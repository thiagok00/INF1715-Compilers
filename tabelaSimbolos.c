#include <stdio.h>
#include <stdlib.h>
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

//para debug apenas
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
            printf("%d [NoFUnc] %s -> ",s->escopo,s->u.f->id);
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

static Simbolo* cria_no_var(Simbolo *s, DefVar *v, int nivelEscopo){
    Simbolo *nv = cria_no_final(s,nivelEscopo);
    nv->tag = SVar;
    nv->u.v = v;
    return nv;
}
static Simbolo* cria_no_func(Simbolo *s, DefFunc *f, int nivelEscopo){
    Simbolo *nv = cria_no_final(s,nivelEscopo);
    nv->tag = SFunc;
    nv->u.f = f;
    return nv;
}
static Simbolo* cria_no_param(Simbolo *s, ParametroL *p, int nivelEscopo){
    Simbolo *nv = cria_no_final(s,nivelEscopo);
    nv->tag = SParams;
    nv->u.p = p;
    return nv;
}

static void costura_expressao(Simbolo *s, Exp *e, int nivelEscopo){

}

static void costura_bloco(Simbolo *s, Bloco *b,int nivelEscopo) {

    DefVarL *auxvars;
    CMDL *auxcmds;

    if (b == NULL) return;

    auxvars = b->declVars;
    while (auxvars != NULL) {
      s = cria_no_var(s,auxvars->v,nivelEscopo);
      auxvars = auxvars->prox;
    }

    auxcmds = b->cmds;
    while(auxcmds != NULL) {
      CMD *cmd = auxcmds->c;
      switch (cmd->tag) {
        case CMD_BLOCK:
          costura_bloco(s,cmd->u.bloco,nivelEscopo+1);
        break;
        case CMD_ATR:
          costura_expressao(s,cmd->u.atr.expvar,nivelEscopo);
          costura_expressao(s,cmd->u.atr.exp,nivelEscopo);
        break;
        case CMD_WHILE:
          costura_expressao(s,cmd->u.cmdwhile.exp,nivelEscopo);
          costura_bloco(s,cmd->u.cmdwhile.bloco,nivelEscopo+1);
        break;
        case CMD_IF:
          costura_expressao(s,cmd->u.cmdif.exp,nivelEscopo);
          costura_bloco(s,cmd->u.cmdif.bloco,nivelEscopo+1);
        break;
        case CMD_IFELSE:
          costura_expressao(s,cmd->u.cmdifelse.exp,nivelEscopo);
          costura_bloco(s,cmd->u.cmdifelse.blocoif,nivelEscopo+1);
          costura_bloco(s,cmd->u.cmdifelse.blocoelse,nivelEscopo+1);
        break;
        case CMD_RETURN:
        case CMD_PRINT:
        case CMD_CHAMADA:
          costura_expressao(s,cmd->u.exp,nivelEscopo);
        break;
        default:
        break;
      }
      auxcmds = auxcmds->prox;
    }

    print_simbolos(s);
    //desempilha variaveis do escopo
    while (s != NULL && s->escopo >= nivelEscopo) {
      s = destroi_simbolo_final(s);
    }
    //return s;
}


void costura_arvore(Programa *p) {
    Definicao *auxdef;
    Simbolo *s = NULL;
    int nivelEscopo = 0;

    printf("Costurando Arvore\n");
    if (p == NULL) return;

    //todas variaveis globais
    auxdef = p->defs;
    while (auxdef != NULL) {
        if (auxdef->tag == DVar) {
          s = cria_no_var(s,auxdef->u.v ,nivelEscopo);
        }
        auxdef = auxdef->prox;
      }
    auxdef = p->defs;
    while (auxdef != NULL) {
        if (auxdef->tag == DFunc) {
          s = cria_no_func(s,auxdef->u.f,nivelEscopo);
          s = cria_no_param(s,auxdef->u.f->params,nivelEscopo+1);
          costura_bloco(s,auxdef->u.f->bloco,nivelEscopo+1);
        }
        auxdef = auxdef->prox;
    }
}
