#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GeraCodigo.h"

typedef struct stringList {
  char *s;
  struct stringList *prox;
} StringList;


/*
* declaracoes
*/
char* tipo_base_string(Base_TAG tipo_base);
char *tipo_string(Tipo *tipo);
static int gen_id();
static int gen_stringid();
static int gen_label();
static void gera_declaracoes_iniciais();
static void gera_codigo_varglobais(Definicao *defs);
static void gera_codigo_strings();

static void gera_codigo_funcoes(Definicao *defs);
static FILE* output = NULL;
static StringList *strLst = NULL;
static char *lastStringToPrint = NULL;

static int gera_codigo_expcte(Constante *c);
static int gera_codigo_expunaria(Exp *e);
static int gera_codigo_expvar(Exp *e);
static int gera_codigo_exparith(Exp *e);
static int gera_codigo_expcmp(Exp *e);
static int gera_codigo_expor(Exp *e);
static int gera_codigo_expand(Exp *e);
static int gera_codigo_expas(Exp *e);
static int gera_codigo_expnew(Exp *e);
static int gera_codigo_expchamada(Exp *e);
static int gera_codigo_expressao(Exp *e);
static void gera_codigo_bloco(Bloco *b);
/*
* implementacoes
*/

static int gen_id() {
    static int i = 0;
    return i++;
}

static int gen_label() {
    static int i = 0;
    return i++;
}

static int gen_stringid() {
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

char *tipo_string(Tipo *tipo) {

  char *str;
  char *strbase;
  char aststr[200] = "";

  str = (char*) malloc(sizeof(char)*200);
  if (str == NULL) {printf("Falta de memoria\n");exit(1);}
  str[0] ='\0';

  strbase = tipo_base_string(tipo->tipo_base);
  while(tipo->de != NULL){
    sprintf(aststr,"%s*",aststr);
    tipo = tipo->de;
  }
  sprintf(str,"%s%s",strbase,aststr);

  return str;
}

static char* traduz_escapes_string(const char * str, int *len) {

  //char *escapen = "\\0A";
  //char *escapet = "\\09";
  //char *escapecb = "\\5C";
  char *nvstr;
  int i,j, length;

  length = strlen(str);
  nvstr = (char*) malloc((length*3+1)*sizeof(char));
  if (nvstr == NULL) {printf("Falta de memoria\n");exit(1);}
  (*len) = 0;
  for (j = 0, i = 0; i < length; i++) {

    switch(str[i]) {
          case '\n':
            nvstr[j] = '\\';
            j++;
            nvstr[j] = '0';
            j++;
            nvstr[j] = 'A';
            j++;
            (*len)++;
            break;
          case '\t':
            nvstr[j] = '\\';
            j++;
            nvstr[j] = '0';
            j++;
            nvstr[j] = '9';
            j++;
            (*len)++;
            break;
          case '\\':
            nvstr[j] = '\\';
            j++;
            nvstr[j] = '5';
            j++;
            nvstr[j] = 'C';
            j++;
            (*len)++;
            break;
          default:
            nvstr[j] = str[i];
            j++;
            (*len)++;
        }
    }
  nvstr[j] = '\0';
  (*len)++;
  return nvstr;
}

static int gera_codigo_expcte(Constante *c) {

  if(c->tag == CDEC){
    int idtemp = gen_id();
    fprintf(output,"\t%%t%d = add i32 0, %d\n",idtemp,c->val.i);
    return idtemp;
  }
  else if (c->tag == CREAL){
    int idtemp = gen_id();
    fprintf(output,"\t%%t%d = fadd float 0.0, %f\n",idtemp,c->val.f);
    return idtemp;
  }
  else if (c->tag == CSTRING) {
    int idtemp, originalStringLen;
    char *originalString;
    char snum[200];
    char *finalString;
    char *strPrint;
    StringList *nv;

    idtemp = gen_stringid();
    originalString = traduz_escapes_string(c->val.s,&originalStringLen);
    //originalStringLen = strlen(originalString)+1;

    sprintf(snum,"@.s%d = private unnamed_addr constant [%d x i8] c\"",idtemp,originalStringLen);

    // +4 pelo \00 e "
    finalString = (char*) malloc((strlen(snum)+strlen(originalString)+4)*sizeof(char));

    sprintf(finalString,"%s%s\\00\"",snum,originalString);
    //printf("%s",finalString);
    nv = (StringList*) malloc(sizeof(StringList));
    if (nv == NULL) {printf("Falta de memoria\n");exit(1);}
    nv->s = finalString;
    nv->prox = strLst;
    strLst = nv;

    strPrint = (char*) malloc(200*sizeof(char));
    if (strPrint == NULL) {printf("Falta de memoria\n");exit(1);}

    sprintf(strPrint,"call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([%d x i8], [%d x i8]* @.s%d, i32 0, i32 0))\n",originalStringLen,originalStringLen,idtemp);
    if (lastStringToPrint != NULL) free(lastStringToPrint);
    lastStringToPrint = strPrint;

    return idtemp;
  }
  return -1;
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
    char *strtipo = tipo_string(v->tipo);
    int idtemp;

    idtemp = gen_id();
    //%1 = load i32, i32* %lol
    if (v->u.vvar.escopo == EscopoGlobal)
      fprintf(output,"\t%%t%d = load %s, %s* @t%d\n",idtemp,strtipo,strtipo,v->u.vvar.d.def->idTemp);
    else
      fprintf(output,"\t%%t%d = load %s, %s* %%t%d\n",idtemp,strtipo,strtipo,v->u.vvar.d.def->idTemp);

    if (v->tipo->tipo_base == bChar){
      //%idtemp = sext i8 %lastid to i32
      int lastid = idtemp;
      idtemp = gen_id();
      fprintf(output,"\t%%t%d = sext i8 %%t%d to i32\n",idtemp,lastid);
    }


    return idtemp;
  }
  else if (v->tag == vAcesso){
    int idvar,idindex,idtemp;
    Exp *expvar, *expindex;
    expvar = v->u.vacesso.expvar;
    expindex = v->u.vacesso.expindex;

    char *str_base = tipo_base_string(expvar->tipo->tipo_base);
    char *str_tipo = tipo_string(expvar->tipo);
    idvar = gera_codigo_expressao(expvar);
    idindex = gera_codigo_expressao(expindex);
    //%11 = load float*, float** %fvec, align 8
    //%12 = getelementptr inbounds float, float* %11, i64 3
    idtemp = gen_id();
    fprintf(output,"\t%%t%d = getelementptr %s,%s %%t%d, i32 %%t%d\n",idtemp,str_base,str_tipo,idvar,idindex);

    return idtemp;
  }
  return -1;
}

static int gera_codigo_exparith(Exp *e) {

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
  return idtemp;
}

static int gera_codigo_expcmp(Exp *e) {

  int idesq, iddir, idtemp=-1;
  char *op, *strtipobase;
  Base_TAG tipoexp;
  int lastid;

  tipoexp = e->u.expbin.expesq->tipo->tipo_base; //assumindo que os dois tipos sao iguais, garantido pela tipagem
  strtipobase = tipo_base_string(tipoexp);

  idesq = gera_codigo_expressao(e->u.expbin.expesq);
  iddir = gera_codigo_expressao(e->u.expbin.expdir);

  if (e->u.expbin.opbin == equal){
    if (tipoexp == bFloat)
      op = "fcmp oeq";
    else
      op = "icmp eq";
  }
  if (e->u.expbin.opbin == notequal){
    if (tipoexp == bFloat)
      op = "fcmp une";
    else
      op = "icmp ne";
  }
  if (e->u.expbin.opbin == greater ){
    if (tipoexp == bFloat)
      op = "fcmp ogt";
    else
      op = "icmp sgt";
  }
  if (e->u.expbin.opbin == less){
    if (tipoexp == bFloat)
      op = "fcmp olt";
    else
      op = "icmp slt";
  }
  if (e->u.expbin.opbin == greaterequal){
    if (tipoexp == bFloat)
      op = "fcmp oge";
    else
      op = "icmp sge";
  }
  if (e->u.expbin.opbin == lessequal){
    if (tipoexp == bFloat)
      op = "fcmp ole";
    else
      op = "icmp sge";
  }

  idtemp = gen_id();
  //%t(idtemp) = op tipo %t(idesq), %t(iddir)
  fprintf(output,"\t%%t%d = %s %s %%t%d, %%t%d\n",idtemp,op,strtipobase,idesq,iddir);
  lastid = idtemp;
  idtemp = gen_id();
  //%t(idtemp) = zext i1 %t(lastid) to i32
  fprintf(output,"\t%%t%d = zext i1 %%t%d to i32\n",idtemp,lastid);

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

static int gera_codigo_expor(Exp *e) {

  return -1;
}
static int gera_codigo_expand(Exp *e) {

  int idexp,idtemp,labelTrue,labelFalse;

  idexp = gera_codigo_expressao(e->u.expbin.expesq);
  idtemp = gen_id();
  //  %t(idtemp) = icmp ne i32 %t(idexp), 0
  //  br i1 %3, label %4, label %8
  printf("\t%%t%d = icmp ne i32 %%t%d, 0",idtemp,idexp);
  labelTrue = gen_label();
  labelFalse = gen_label();
  printf("\tbr i1 %%t%d, label %%%d, label %%%d",idtemp,labelTrue,labelFalse);



  return 0;
}

static int gera_codigo_expchamada(Exp *e) {
  int idtemp = -1;
  Tipo *tiporet = e->u.expchamada.def->tiporet;
  ExpL *expl;
  char paramString[200] = "";

  expl = e->u.expchamada.params;
  while (expl != NULL) {
    int expid;
    Tipo *exptipo;

    expid = gera_codigo_expressao(expl->e);
    exptipo = expl->e->tipo;
    if(expl->prox != NULL)
      sprintf(paramString,"%s%s %%t%d, ",paramString,tipo_string(exptipo),expid);
    else
      sprintf(paramString,"%s%s %%t%d",paramString,tipo_string(exptipo),expid);
    expl = expl->prox;
}

if(tiporet->tipo_base == bVoid){
  fprintf(output,"\tcall void @%s(",e->u.expchamada.idFunc);
}
else{
  //TODO ARRAYS
  idtemp = gen_id();
  fprintf(output,"\t%%t%d = call %s @%s(",idtemp,tipo_string(tiporet),e->u.expchamada.idFunc);
}
//fprintf(output,")\n");

fprintf(output,"%s)\n",paramString);
return idtemp;;
}

static int gera_codigo_expnew(Exp *e) {

  int expid,idtemp,lastid,mult;

  expid = gera_codigo_expressao(e->u.expnewas.exp);
  idtemp = gen_id();
  if(e->tipo->tipo_base == bChar)
    mult = 1;
  else
    mult = 4;
  fprintf(output, "\t%%t%d = mul nsw i32 %%t%d, %d \n",idtemp,expid,mult);
  //%5 = sext i32 %4 to i64
  // lastid = idtemp;
  // idtemp = gen_id();
  // fprintf(output,"\t%%t%d = sext i32 %%t%d to i64\n",idtemp,lastid);

  //%t(expid) = call i8* @malloc(i64 10)
  lastid = idtemp;
  idtemp = gen_id();
  fprintf(output,"\t%%t%d = call i8* @malloc(i32 %%t%d)\n",idtemp,lastid);
  //%t(idtemp) = bitcast i8* %t(lastid) to i32*
  if(e->tipo->tipo_base != bChar) {
    lastid = idtemp;
    idtemp = gen_id();
    fprintf(output,"\t%%t%d = bitcast i8* %%t%d to i32*\n",idtemp,lastid);
  }
  return idtemp;
}


static int gera_codigo_expressao(Exp *e) {

    switch(e->tag) {
      case EXP_CTE:
        return gera_codigo_expcte(e->u.expcte);
      case EXP_UNARIA:
        return gera_codigo_expunaria(e);
      case EXP_VAR:
        return gera_codigo_expvar(e);
      case EXP_ARITH:
        return gera_codigo_exparith(e);
      case EXP_CMP:
        return gera_codigo_expcmp(e);
      case EXP_AS:
        return gera_codigo_expas(e);
      case EXP_OR:
        return gera_codigo_expor(e);
      case EXP_AND:
        return gera_codigo_expand(e);
      case EXP_CHAMADA:
        return gera_codigo_expchamada(e);
      case EXP_NEW:
        return gera_codigo_expnew(e);
      default:
        return -1;
        break;
    }

}

static void gera_codigo_while(CMD *c){
  int idtemp, idexp;
  int labelTrue,labelCmp,labelFinal;

  labelCmp = gen_label();
  labelTrue = gen_label();
  labelFinal = gen_label();

  fprintf(output,"\tbr label %%L%d\n",labelCmp);
  fprintf(output,"L%d:\n",labelCmp);
  idexp = gera_codigo_expressao(c->u.cmdwhile.exp);
  idtemp = gen_id();
  //  %4 = icmp ne i32 %3, 0
  fprintf(output,"\t%%t%d = icmp ne i32 %%t%d, 0\n",idtemp,idexp);
  //br i1 %4, label %5, label %6
  fprintf(output,"\tbr i1 %%t%d, label %%L%d, label %%L%d\n",idtemp,labelTrue,labelFinal);
  fprintf(output,"L%d:\n",labelTrue);
  gera_codigo_bloco(c->u.cmdwhile.bloco);
  //br label %2
  fprintf(output,"\tbr label %%L%d\n",labelCmp);

  fprintf(output,"L%d:\n",labelFinal);

}

static void gera_codigo_ifelse(CMD *c){
  int idtemp, idexp;
  int labelTrue,labelFalse,labelFinal;
  idexp = gera_codigo_expressao(c->u.cmdifelse.exp);
  idtemp = gen_id();
  //  %3 = icmp ne i32 %2, 0
  fprintf(output,"\t%%t%d = icmp ne i32 %%t%d, 0\n",idtemp,idexp);
  //br i1 %3, label %4, label %5
  labelTrue = gen_label();
  labelFalse = gen_label();
  labelFinal = gen_label();
  fprintf(output,"\tbr i1 %%t%d, label %%L%d, label %%L%d\n",idtemp,labelTrue,labelFalse);

  //label true
  fprintf(output,"L%d:\n",labelTrue);
  gera_codigo_bloco(c->u.cmdifelse.blocoif);
  fprintf(output,"\tbr label %%L%d\n",labelFinal);

  //label false
  fprintf(output,"L%d:\n",labelFalse);
  gera_codigo_bloco(c->u.cmdifelse.blocoelse);
  fprintf(output,"\tbr label %%L%d\n",labelFinal);

  fprintf(output,"L%d:\n",labelFinal);
}
static void gera_codigo_if(CMD *c){
  int idtemp, idexp;
  int labelTrue,labelFinal;
  idexp = gera_codigo_expressao(c->u.cmdif.exp);
  idtemp = gen_id();
  //  %3 = icmp ne i32 %2, 0
  fprintf(output,"\t%%t%d = icmp ne i32 %%t%d, 0\n",idtemp,idexp);
  //br i1 %3, label %4, label %5
  labelTrue = gen_label();
  labelFinal = gen_label();
  fprintf(output,"\tbr i1 %%t%d, label %%L%d, label %%L%d\n",idtemp,labelTrue,labelFinal);
  fprintf(output,"L%d:\n",labelTrue);
  gera_codigo_bloco(c->u.cmdif.bloco);
  fprintf(output,"\tbr label %%L%d\n",labelFinal);
  fprintf(output,"L%d:\n",labelFinal);
}
static void gera_codigo_bloco(Bloco *b) {

  DefVarL* defsvar = b->declVars;
  CMDL *cmds = b->cmds;
  while (defsvar != NULL){
    DefVar *v = defsvar->v;
    int idTemp = gen_id();
    v->idTemp = idTemp;

    fprintf(output,"\t%%t%d = alloca %s\n",v->idTemp,tipo_string(v->tipo));

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
            char *varstrtipo = tipo_string(varatr->tipo);
            int valexp = gera_codigo_expressao(c->u.atr.exp);

            if(varatr->tipo->tipo_base == bChar && varatr->tipo->tag == base){
              idtemp = gen_id();
              //%2 = trunc i32 %1 to i8
              fprintf(output,"\t%%t%d = trunc i32 %%t%d to i8\n",idtemp,valexp);
            }
            else {
              idtemp = valexp;
            }
            //store i32 %2, i32* %lol
            if (varatr->u.vvar.escopo == EscopoGlobal)
              fprintf(output, "\tstore %s %%t%d, %s* @t%d\n",varstrtipo,idtemp,varstrtipo,varatr->u.vvar.d.def->idTemp);
            else
              fprintf(output, "\tstore %s %%t%d, %s* %%t%d\n",varstrtipo,idtemp,varstrtipo,varatr->u.vvar.d.def->idTemp);

          }
          //TODO ARRAY
        break;
        case CMD_PRINT:
          idtemp = gera_codigo_expressao(c->u.e);
          if (c->u.e->tipo->tag == base && c->u.e->tipo->tipo_base == bInt) {
            int lastid = idtemp;
            idtemp = gen_id();
            //%2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i32 %1)
            fprintf(output,"\t%%t%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4  x i8], [4 x i8]* @.intprint, i32 0, i32 0), i32 %%t%d)",idtemp,lastid);
        }
          else if (c->u.e->tipo->tag == base && c->u.e->tipo->tipo_base == bFloat) {
            int lastid = idtemp;
            idtemp = gen_id();
            //%2 = fpext float %1 to double
            fprintf(output,"\t%%t%d = fpext float %%t%d to double\n",idtemp,lastid);
            lastid = idtemp;
            idtemp = gen_id();
            //%3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), double %2)
            fprintf(output,"\t%%t%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.floatprint, i32 0, i32 0), double %%t%d)\n",idtemp,lastid);
          }
        else if (c->u.e->tipo->tag == array && c->u.e->tipo->tipo_base == bChar){
           int idTemp = gen_id();
           fprintf(output,"\t%%t%d = %s",idTemp,lastStringToPrint);
        }
        break;
        case CMD_RETURNVOID:
          fprintf(output,"\tret void\n");
        break;
        case CMD_RETURN:
          idtemp = gera_codigo_expressao(c->u.e);
          fprintf(output,"\tret %s %%t%d\n",tipo_string(c->u.e->tipo),idtemp);
        break;
        case CMD_IF:
          gera_codigo_if(c);
        break;
        case CMD_IFELSE:
          gera_codigo_ifelse(c);
        break;
        case CMD_WHILE:
          gera_codigo_while(c);
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
        ParametroL *params = defs->u.f->params, *paramsAux;

        char paramList[400] = "";

        paramsAux = params;
        while(paramsAux != NULL) {
          int idtemp = gen_id();
          char *strtipo = tipo_string(paramsAux->tipo);
          paramsAux->idtemp = idtemp;
          if(paramsAux->prox == NULL)
            sprintf(paramList,"%s%s %%t%d",paramList,strtipo,idtemp);
          else
            sprintf(paramList,"%s%s %%t%d, ",paramList,strtipo,idtemp);
          paramsAux = paramsAux->prox;
        }

        char* tiporetstr = tipo_string(f->tiporet);
        fprintf(output,"define %s @%s(%s) {\n",tiporetstr,f->id,paramList);

        while (params != NULL) {
          //TODO ARRAYS
          int lastid;
          int idtemp = gen_id();
          char *strtipo = tipo_string(params->tipo);

          lastid = params->idtemp;
          params->idtemp = idtemp;

          fprintf(output,"\t%%t%d = alloca %s\n",idtemp,strtipo);
          fprintf(output,"\tstore %s %%t%d, %s* %%t%d\n",strtipo,lastid,strtipo,idtemp);
          params = params->prox;
        }

        gera_codigo_bloco(f->bloco);
        fprintf(output, "\t}\n");
    }
    defs = defs->prox;
  }
}

static void gera_codigo_varglobais(Definicao *defs) {
  //TODO arrays
  while (defs != NULL){
    if (defs->tag == DVar){
        int idTemp = gen_id();
        char* tipostr = tipo_string(defs->u.v->tipo);
        defs->u.v->idTemp = idTemp;
        if(defs->u.v->tipo->tipo_base == bFloat) {
          fprintf(output, "@t%d = common global %s 0.000000e+00\n",idTemp,tipostr);
        }
        else {
          fprintf(output, "@t%d = common global %s 0\n",idTemp,tipostr);
        }
    }
    defs = defs->prox;
  }
  fprintf(output,"\n");
}

void geracodigo_arvore(Programa *p){
  output = fopen("monga.ll","w");
  strLst = NULL;
  if (output == NULL){
    printf("Nao conseguiu abrir arquivo de saida\n");
    exit(1);
  }
  gera_declaracoes_iniciais();
  gera_codigo_varglobais(p->defs);
  gera_codigo_funcoes(p->defs);

  gera_codigo_strings();
}

static void gera_codigo_strings() {
  printf("\n");
  while(strLst != NULL) {
    fprintf(output,"%s\n",strLst->s);
    strLst = strLst->prox;
  }
}

static void gera_declaracoes_iniciais() {
  fprintf(output,"target triple = \"x86_64-pc-linux-gnu\"");
	fprintf(output, "declare i8* @malloc(i32)\n" );
	fprintf(output, "declare i32 @printf(i8* nocapture readonly, ...)\n" );
	fprintf(output, "declare i32 @puts(i8* nocapture readonly)\n" );
  fprintf(output, "\n");
  fprintf(output, "@.intprint = private unnamed_addr constant [4 x i8] c\"%%d\\0A\\00\"\n");
	fprintf(output, "@.floatprint = private unnamed_addr constant [4 x i8] c\"%%f\\0A\\00\"\n" );
  fprintf(output,"\n");
}
