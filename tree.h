/*
  Duvidas em relaçao as structs com mesmo campo como if while (que tem exp e bloco) operadores unarios e operadores binarios:
  criar uma struct só que nem feito nas expressoes ou algo separar como parecido em statements??
*/

/*
*  Constante
*/
typedef enum CTE_TAG { CDEC, CREAL, CSTRING } CTE_TAG;
typedef struct Constante
{
	CTE_TAG tag;
	union {
		const char* s;
		int i;
		double d;
	} val;
} Constante;

/*
* Tipo
*/
typedef enum Tipo_TAG { pInt, pFloat, pChar, array} Tipo_TAG;

typedef union Tipo
{
	Tipo_TAG tag;
  struct {
    Tipo_TAG tag;
    union Tipo *tipo;
  } array;
} Tipo;

/*
* Definicao Variavel
*/
//typedef enum EscopoTag { EscopoGlobal,EscopoLocal } EscopoTag;

typedef struct DefVar
{
  Tipo* tipo;
  char* id;
	//EscopoVar escopoTag;
} DefVar;

typedef struct DefVarL
{
	DefVar* dv;
	struct DefVarL* prox;
} DefVarL;

/*
* Bloco
*/
typedef struct Bloco {
  DefVarL *declVars;
  struct CMDL *cmds;
}Bloco;

/*
* Declaracao Funcao
*/
typedef struct ParametroL
{
	Tipo *t;
	char *id; //TODO: faz sentido guardar o nome do param?
	struct ParametroL *prox;
} ParametroL;

typedef struct DefFunc
{
  Tipo *tiporet;
  char *id;
  ParametroL *params;
  Bloco *bloco;
} DefFunc;

/*
* Definicoes
*/
typedef union definicao {
    DefVar *defVar;
    DefFunc *defFunc;
}Definicao;

typedef struct definicaoL {
    Definicao *def;
    struct definicaoL *prox;
}DefinicaoL;

typedef struct programa {
	DefinicaoL* defs;
} Programa;

/*
* Variavel
*/
typedef struct Var
{
  Tipo *tipo;
	const char *id;
  // e o valor??
	//DefVar* declaracao; pra costurar depois (?)
} Var;



/*
* Expressao
*/
typedef enum EXP_TAG {
	EXP_ADD,
	EXP_SUB,
	EXP_MULT,
	EXP_DIV,
	EXP_UNARIA,
	EXP_VAR,
	EXP_CHAMADA,
	EXP_BASE,
	EXP_CMP,
	EXP_NEW,
	EXP_VARVETOR /* SEI LA*/,
	EXP_AS
} EXP_TAG;

//TODO sera que precisa de parentesis? '(' exp ')'
typedef union Exp{
	EXP_TAG tag;
  struct {
    EXP_TAG tag;
    Var *v;
  } expvar;
  struct {
    EXP_TAG tag;
    Constante *c;
  } expcte;
  struct {
    EXP_TAG tag;
    union Exp *e;
    Tipo* tipo;
  } expnewas;
  struct {
    EXP_TAG tag;
    union Exp *exp;
    enum {opnot,opmenos} opun;
  } expunaria;
  struct {
    EXP_TAG tag;
    union Exp *expesq;
    union Exp *expdir;
    enum {soma,sub,mult,divisao,equal,notequal,less,lessequal,greater,greaterequal,opand,opor} opbin;
  } expbin;
  struct {
    EXP_TAG tag;
    struct ExpL *params;
    const char *idFunc;
    //DefFunc *decl; pra costurar depois(?)
  } expchamada;
}Exp;

typedef struct ExpL
{
	Exp* e;
	struct ExpLt* prox;
} ExpL ;

/*
* Comando
*/
typedef enum CMD_TAG {
  CMD_ATR, /* atribuição */
  CMD_WHILE,
  CMD_IF,
  CMD_SKIP,
  CMD_RETURN,
  CMD_CHAMADA,
	CMD_BLOCK,
	CMD_PRINT,
  CMD_SEQ
} CMD_TAG;

typedef union CMD {
  CMD_TAG tag;
  struct {
    CMD_TAG tag;
    Exp *exp;
  } cmdexp; /* return, print, chamada */
  struct {
    CMD_TAG tag;
    Bloco *bloco;
  } block;
  struct {
    CMD_TAG tag;
		Exp* 	exp;
    Bloco *bloco  ;
	} cmdif;
  struct {
    CMD_TAG tag;
		Exp* 	exp;
    Bloco *blocoif;
    Bloco *blocoelse;
	} cmdifelse;
  struct {
    CMD_TAG tag;
    Exp *exp;
    Bloco *bloco;
  } cmdwhile;
  struct {
    CMD_TAG tag;
    Var *var;
    Exp *exp;
  } atr;
} CMD;

typedef struct CMDL
{
	CMD* c;
	struct CMDL* prox;
} CMDL ;
