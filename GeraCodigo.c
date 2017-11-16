#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include "GeraCodigo.h"

/*
* declaracoes
*/
char* tipo_base_string(Base_TAG tipo_base);
static int gen_id();
static void gera_declaracoes_iniciais();
static void gera_codigo_varglobais(Definicao *defs);

static void gera_codigo_funcoes(Definicao *defs);
static FILE* output = NULL;


static int gera_codigo_expcte(Constante *c);
static int gera_codigo_expunaria(Exp *e);
static int gera_codigo_expvar(Exp *e);
static int gera_codigo_expbin(Exp *e);
static int gera_codigo_expas(Exp *e);
static int gera_codigo_expressao(Exp *e);
/*
* implementacoes
*/

static int gen_id() {
    static int i = 0;
    return i++;
}

char* tipo_base_string(Base_TAG tipo_base) {

    switch(tipo_base){
      case bInt:
        return "i32";
      case bChar:
        return "i8";
      case bFloat:
        return "float";
      default: return "i32";
    }
}

static int gera_codigo_expcte(Constante *c) {
  int idtemp = gen_id();

  if(c->tag == CDEC){
    fprintf(output,"\t%%t%d = add i32 0, %d\n",idtemp,c->val.i);
  }
  else if (c->tag == CREAL){
    fprintf(output,"\t%%t%d = fadd float 0.0, %f\n",idtemp,c->val.f);
  }
  //TODO CONSTANTES STRING
  return idtemp;
}

static int gera_codigo_expunaria(Exp *e) {
    int idtemp =-1;
    int idexp;

    idexp = gera_codigo_expressao(e->u.expunaria.exp);

    if(e->u.expunaria.opun == opnot){

      if(e->u.expunaria.exp->tipo->tipo_base == bInt){
        int lastid = idexp;
        idtemp = gen_id();
        //  %t(idtemp) = icmp ne i32 %t(lastid), 0
        fprintf(output,"\t%%t%d = icmp ne i32 %%t%d, 0\n",idtemp,lastid);
        lastid = idtemp;
        idtemp = gen_id();
        //  %t(idtemp) = xor i1 %t(lastid), true
        fprintf(output,"\t%%t%d = xor i1 %%t%d, true\n",idtemp,lastid);
        lastid = idtemp;
        idtemp = gen_id();
        //  %t(idtemp) = zext i1 %t(lastid) to i32
        fprintf(output,"\t%%t%d = zext i1 %%t%d to i32\n",idtemp,lastid);
        return idtemp;
      }
      else if(e->u.expunaria.exp->tipo->tipo_base == bFloat){
        int lastid = idexp;
        idtemp = gen_id();
        //%2 = fcmp une float %1, 0.000000e+00
        fprintf(output,"\t%%t%d = fcmp une float %%t%d, 0.000000e+00\n",idtemp,lastid);
        lastid = idtemp;
        idtemp = gen_id();
        //%3 = xor i1 %2, true
        fprintf(output,"\t%%t%d = xor i1 %%t%d, true\n",idtemp,lastid);
        lastid = idtemp;
        idtemp = gen_id();
        //%4 = zext i1 %3 to i32
        fprintf(output,"\t%%t%d = zext i1 %%t%d to i32\n",idtemp,lastid);
        lastid = idtemp;
        idtemp = gen_id();
        //%5 = sitofp i32 %4 to float
        fprintf(output,"\t%%t%d = sitofp i32 %%t%d to float\n",idtemp,lastid);
        return idtemp;
      }
    }
    else if (e->u.expunaria.opun == opmenos) {
      if (e->u.expunaria.exp->tipo->tipo_base == bInt){
        int lastid = idexp;
        idtemp = gen_id();
        //%2 = sub nsw i32 0, %1
        fprintf(output,("\t%%t%d = sub nsw i32 0, %%t%d\n"),idtemp,lastid);
        return idtemp;
      }
      else if(e->u.expunaria.exp->tipo->tipo_base == bFloat){
        int lastid = idexp;
        idtemp = gen_id();
        //%4 = fsub float -0.000000e+00, %3
        fprintf(output,"\t%%t%d = fsub float -0.000000e+00, %%t%d\n",idtemp,lastid);
        return idtemp;
      }
    }


    return idtemp;

}

static int gera_codigo_expvar(Exp *e) {

  Var *v = e->u.expvar;
  if (v->tag == vVar){
    char *strtipo = tipo_base_string(v->tipo->tipo_base);
    int idtemp;

    idtemp = gen_id();
    //%1 = load i32, i32* %lol
    fprintf(output,"\t%%t%d = load %s, %s* %%%s\n",idtemp,strtipo,strtipo,v->u.vvar.id);
    if (v->tipo->tipo_base == bChar){
      //%idtemp = sext i8 %lastid to i32
      int lastid = idtemp;
      idtemp = gen_id();
      fprintf(output,"\t%%t%d = sext i8 %%t%d to i32\n",idtemp,lastid);
    }


    return idtemp;
  }
  else if (v->tag == vAcesso){
    //TODO ACESSO ARRAY
    return -1;
  }
  return -1;
}

static int gera_codigo_expbin(Exp *e) {

  int idesq, iddir, idtemp=-1;
  int flag = 0;
  char *op, *strtipobase;
  Base_TAG tipoexp;

  tipoexp = e->u.expbin.expesq->tipo->tipo_base; //assumindo que os dois tipos sao iguais, garantido pela tipagem
  strtipobase = tipo_base_string(tipoexp);

  idesq = gera_codigo_expressao(e->u.expbin.expesq);
  iddir = gera_codigo_expressao(e->u.expbin.expdir);


  if (e->u.expbin.opbin == opadd){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fadd";
    else
      op = "add";
  }
  if (e->u.expbin.opbin == opsub){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fsub";
    else
      op = "sub nsw";
  }
  if (e->u.expbin.opbin == opmult){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fmul";
    else
      op = "mul nsw";
  }
  if (e->u.expbin.opbin == opdiv) {
    flag = 1;
    if (tipoexp == bFloat)
      op = "fdiv";
    else
      op = "sdiv";
  }
  if (flag){
    idtemp = gen_id();
    //%t(idtemp) = fsub float %idesq, %iddir
    fprintf(output,"\t%%t%d = %s %s %%t%d, %%t%d\n",idtemp,op,strtipobase,idesq,iddir);
    return idtemp;
  }
  if (e->u.expbin.opbin == equal){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fcmp oeq";
    else
      op = "icmp eq";
  }
  if (e->u.expbin.opbin == notequal){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fcmp une";
    else
      op = "icmp ne";
  }
  if (e->u.expbin.opbin == greater ){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fcmp ogt";
    else
      op = "icmp sgt";
  }
  if (e->u.expbin.opbin == less){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fcmp olt";
    else
      op = "icmp slt";
  }
  if (e->u.expbin.opbin == greaterequal){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fcmp oge";
    else
      op = "icmp sge";
  }
  if (e->u.expbin.opbin == lessequal){
    flag = 1;
    if (tipoexp == bFloat)
      op = "fcmp ole";
    else
      op = "icmp sge";
  }
  if (flag){
    int lastid;
    idtemp = gen_id();
    //%t(idtemp) = op tipo %t(idesq), %t(iddir)
    fprintf(output,"\t%%t%d = %s %s %%t%d, %%t%d\n",idtemp,op,strtipobase,idesq,iddir);
    lastid = idtemp;
    idtemp = gen_id();
    //%t(idtemp) = zext i1 %t(lastid) to i32
    fprintf(output,"\t%%t%d = zext i1 %%t%d to i32\n",idtemp,lastid);

    return idtemp;
  }

  //TODO && e ||

  return idtemp;
}

static int gera_codigo_expas(Exp *e) {

  int idexp;

  idexp = gera_codigo_expressao(e->u.expnewas.exp);
  if (e->u.expnewas.exp->tipo->tipo_base == bInt && e->tipo->tipo_base == bFloat) {
      int idtemp = gen_id();
      //%2 = sitofp i32 %1 to float
      fprintf(output,"\t%%t%d = sitofp i32 %%t%d to float\n",idtemp,idexp);
      return idtemp;
  }
  //nao precisa de conversao
  return idexp;
}

static int gera_codigo_expressao(Exp *e) {


    switch(e->tag) {
      case EXP_CTE:
        return gera_codigo_expcte(e->u.expcte);
      case EXP_UNARIA:
        return gera_codigo_expunaria(e);
      case EXP_VAR:
        return gera_codigo_expvar(e);
      case EXP_BIN:
        return gera_codigo_expbin(e);
      case EXP_AS:
        return gera_codigo_expas(e);
        default:
          return -1;
        break;
    }

}

static void gera_codigo_bloco(Bloco *b) {

  DefVarL* defsvar = b->declVars;
  CMDL *cmds = b->cmds;
  while (defsvar != NULL){
    DefVar *v = defsvar->v;
    //TODO ARRAY
    fprintf(output,"\t%%%s = alloca %s\n",v->id,tipo_base_string(v->tipo->tipo_base));

    defsvar = defsvar->prox;
  }
  while (cmds != NULL) {
      int idtemp = -1;
      Var *varatr = NULL;

      CMD *c = cmds->c;
      switch(c->tag){
        case CMD_ATR:
          varatr = c->u.atr.var;
          if (varatr->tag == vVar) {
            char *varstrtipo = tipo_base_string(varatr->tipo->tipo_base);
            int valexp = gera_codigo_expressao(c->u.atr.exp);

            if(varatr->tipo->tipo_base == bChar){
              idtemp = gen_id();
              //%2 = trunc i32 %1 to i8
              fprintf(output,"\t%%t%d = trunc i32 %%t%d to i8\n",idtemp,valexp);
            }
            else {
              idtemp = valexp;
            }
            //store i32 %2, i32* %lol
            fprintf(output, "\tstore %s %%t%d, %s* %%%s\n",varstrtipo,idtemp,varstrtipo,varatr->u.vvar.id);

          }
          //TODO ARRAY
        break;
        case CMD_PRINT:
          idtemp = gera_codigo_expressao(c->u.e);
          if (c->u.e->tipo->tipo_base == bInt) {
            int lastid = idtemp;
            idtemp = gen_id();
            //%2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i32 %1)
            fprintf(output,"\t%%t%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.intprint, i32 0, i32 0), i32 %%t%d)",idtemp,lastid);
        }
          else if (c->u.e->tipo->tipo_base == bFloat) {
            int lastid = idtemp;
            idtemp = gen_id();
            //%2 = fpext float %1 to double
            fprintf(output,"\t%%t%d = fpext float %%t%d to double\n",idtemp,lastid);
            lastid = idtemp;
            idtemp = gen_id();
            //%3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), double %2)
            fprintf(output,"\t%%t%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.floatprint, i32 0, i32 0), double %%t%d)\n",idtemp,lastid);
          }
        break;
        default:
        break;
      }
      cmds = cmds->prox;
  }/* fim while */

}

static void gera_codigo_funcoes(Definicao *defs) {
  while (defs != NULL){
    if (defs->tag == DFunc){
        DefFunc *f = defs->u.f;
        //TODO parametros
        char* tiporetstr = tipo_base_string(f->tiporet->tipo_base);
        fprintf(output,"define %s @%s() {\n",tiporetstr,f->id);
        gera_codigo_bloco(f->bloco);
        //ret void
        fprintf(output,"\tret i32 0\n}\n");
    }
    defs = defs->prox;
  }
}

static void gera_codigo_varglobais(Definicao *defs) {
  //TODO arrays
  while (defs != NULL){
    if (defs->tag == DVar){
        char* tipostr = tipo_base_string(defs->u.v->tipo->tipo_base);
        if(defs->u.v->tipo->tipo_base == bFloat)
          fprintf(output, "@%s = common global %s 0.000000e+00\n",defs->u.v->id,tipo_base_string(defs->u.v->tipo->tipo_base));
        else
          fprintf(output, "@%s = common global %s 0\n",defs->u.v->id,tipo_base_string(defs->u.v->tipo->tipo_base));
    }
    defs = defs->prox;
  }
  fprintf(output,"\n");
}

void geracodigo_arvore(Programa *p){
  output = fopen("monga.ll","w");
  if (output == NULL){
    printf("Nao conseguiu abrir arquivo de saida\n");
    exit(1);
  }
  gera_declaracoes_iniciais();
  gera_codigo_varglobais(p->defs);
  gera_codigo_funcoes(p->defs);
}

static void gera_declaracoes_iniciais() {
	//fprintf(output, "declare noalias i8* @malloc(i64)\n" );
  fprintf(output,"target triple = \"x86_64-pc-linux-gnu\"");
	fprintf(output, "declare i8* @malloc(i32)\n" ); //malloc
	fprintf(output, "declare i32 @printf(i8* nocapture readonly, ...)\n" );
	fprintf(output, "declare i32 @puts(i8* nocapture readonly)\n" );

  fprintf(output, "\n");

  fprintf(output, "@.intprint = private unnamed_addr constant [3 x i8] c\"%%d\\00\"");
  //@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
  //@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
	fprintf(output, "@.floatprint = private unnamed_addr constant [4 x i8] c\"%%f\\0A\\00\"" );
	fprintf(output, "@.charprint = private unnamed_addr constant [3 x i8] c\"%%c\\00\"\n" );
	//fprintf(output, "@.strprint = private unnamed_addr constant [3 x i8] c\"%%s\\00\"\n" );

  fprintf(output,"\n");
}
