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
		float f;
	} val;
} Constante;

/*
* Tipo
*/

typedef enum Tipo_TAG { base, array} Tipo_TAG;
typedef enum Base_TAG {bVoid, bChar, bInt, bFloat} Base_TAG;
typedef struct Tipo
{
	Tipo_TAG tag;
	Base_TAG tipo_base;
	struct Tipo* de;
} Tipo;

/*
* Definicao Variavel
*/
typedef enum EscopoTag { EscopoGlobal,EscopoLocal, EscopoFunc } EscopoTag;

typedef struct DefVar
{
	char* id;
  Tipo* tipo;
	int idTemp;
	EscopoTag escopo;
} DefVar;

/*
* Bloco
*/
typedef struct DefVarL
{
	DefVar* v;
	struct DefVarL * prox;
	struct DefVarL * prim;
} DefVarL;


typedef struct Bloco {
  DefVarL *declVars;
  struct CMDL *cmds;
}Bloco;

/*
* Declaracao Funcao
*/
typedef struct ParametroL
{
	Tipo *tipo;
	char *id;
	int idtemp;
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
typedef enum Def_TAG { DFunc, DVar} Def_TAG;
typedef struct definicao {
		Def_TAG tag;
	 	union {
			DefVar *v;
			DefFunc *f;
		}u;
    struct definicao *prox;
}Definicao;

typedef struct programa {
	Definicao* defs;
} Programa;

/*
* Variavel
*/
typedef enum Var_TAG { vVar, vAcesso} Var_TAG;
typedef struct Var
{
	Var_TAG tag;
  Tipo *tipo;
	union {
		struct {
			const char *id;
			EscopoTag escopo;
			union {
		  	DefVar* def;
				ParametroL *defp;
			}d;
		}vvar;
		struct {
			struct Exp *expvar, *expindex;
		}vacesso;
}u;
} Var;



/*
* Expressao
*/
typedef enum EXP_TAG {
	EXP_ARITH,
	EXP_AND,
	EXP_OR,
	EXP_CMP,
	EXP_UNARIA,
	EXP_CTE,
	EXP_VAR,
	EXP_CHAMADA,
	EXP_NEW,
	EXP_AS
} EXP_TAG;

typedef struct Exp{
	EXP_TAG tag;
	Tipo *tipo;
	union {
	    Var *expvar;
			struct Exp * exp;
	    Constante *expcte;
	  struct {
	    struct Exp *exp;
	    Tipo* tipo;
	  } expnewas;
	  struct {
	    struct Exp *exp;
	    enum {opnot,opmenos} opun;
	  } expunaria;
	  struct {
	    struct Exp *expesq;
	    struct Exp *expdir;
			enum {opadd,opsub,opmult,opdiv,equal,notequal,less,lessequal,greater,greaterequal,opand,opor} opbin;
	  } expbin;
	  struct {
	    struct ExpL *params;
	    const char *idFunc;
	    DefFunc *def;
	  } expchamada;
	}u;
}Exp;

typedef struct ExpL
{
	Exp* e;
	struct ExpL* prox;
} ExpL ;

/*
* Comando
*/
typedef enum CMD_TAG {
  CMD_ATR, /* atribuição */
  CMD_WHILE,
  CMD_IF,
	CMD_IFELSE,
  CMD_SKIP,
  CMD_RETURN,
	CMD_RETURNVOID,
  CMD_CHAMADA,
	CMD_BLOCK,
	CMD_PRINT,
} CMD_TAG;

typedef struct CMD {
  CMD_TAG tag;
	union {
	    Exp *e; /* return, print, chamada */
	    Bloco *bloco;
	  struct {
			Exp* 	exp;
	    Bloco *bloco  ;
		} cmdif;
	  struct {
			Exp* 	exp;
	    Bloco *blocoif;
	    Bloco *blocoelse;
		} cmdifelse;
	  struct {
	    Exp *exp;
	    Bloco *bloco;
	  } cmdwhile;
	  struct {
	    Var *var;
	    Exp *exp;
	  } atr;
	}u;
} CMD;

typedef struct CMDL
{
	CMD* c;
	struct CMDL* prox;
} CMDL ;

char*  getStringTipo (Tipo *t);
void print_tree (Programa *p);
