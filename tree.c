#include <stdio.h>
#include <stdlib.h>
#if !defined(tree_h)
#include "tree.h"
#define tree_h
#endif

static char* strFormatoTipo(Base_TAG x) {
    switch (x){
      case 0: return "INT";
      case 1: return "FLOAT";
      case 2: return "CHAR";
      case 3: return "VOID";
      default: return "ERROTIPO";
    }
}




void print_bloco (Bloco *b){
    if (b == NULL) return;
    DefVarL *aux = b->declVars;
    printf("sdss\n" );
    while (aux != NULL){
      Base_TAG base = aux->v->tipo->tipo_base;
      printf("decl:  %s %s\n",aux->v->id,strFormatoTipo(base));
      aux = aux->prox;
    }

}
