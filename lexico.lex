%{
#include <stdio.h>
#include "grammar.tab.h"
#include "tree.h"

/* malloc string */
static char* createstring(int length);

/* strdup */
static char * copystring(char *text);

/* copy string translating escapes */
static char* copyescapes(char *text);

/* line counter */
int yy_lines = 1;

%}

%%

#[^\n]*                                 {}
"\n"                                    {yy_lines++;}
" "                                     {}
"as"                                    { return TK_AS; }
"int"                                   { return TK_INT; }
"float"                                 { return TK_FLOAT; }
"char"                                  { return TK_CHAR; }
"void"                                  { return TK_VOID; }
"new"                                   { return TK_NEW; }
"if"                                    { return TK_IF; }
"else"                                  { return TK_ELSE; }
"while"                                 { return TK_WHILE; }
"return"                                { return TK_RETURN; }
"=="                                    { return TK_EQUAL; }
"˜="                                    { return TK_NOTEQUAL; }
"<="                                    { return TK_LESSEQUAL; }
">="                                    { return TK_GREATEREQUAL; }
"&&"                                    { return TK_AND; }
"||"                                    { return TK_OR; }
[a-zA-Z_][a-zA-Z0-9_]*                  { yylval.s = copystring(yytext); return TK_ID; }
\"(\\.|[^\\"])*\"                       { yylval.s = copyescapes(yytext); return TK_STRING; }

([1-9][0-9]*)|0                         { yylval.i = atoi(yytext); return TK_DEC; }
0[xX][0-9a-fA-F]+|o[0-7]*               { yylval.i = strtol(yytext,NULL,0); return TK_DEC; }

[0-9]+[Ee][+-]?[0-9]+(f|F)?		          { yylval.f = strtof(yytext,NULL); return TK_REAL;}
[0-9]*"."[0-9]+([Ee][+-]?[0-9]+)?(f|F)?	{ yylval.f = strtof(yytext,NULL); return TK_REAL;}
[0-9]+"."[0-9]*([Ee][+-]?[0-9]+)?(f|F)? { yylval.f = strtof(yytext,NULL); return TK_REAL;}

0[xX][0-9a-fA-F]+[Pp][+-]?[0-9]+(f|F)?		                 { yylval.f = strtof(yytext,NULL); return TK_REAL;}
0[xX][0-9a-fA-F]*"."[0-9a-fA-F]+([Pp][+-]?[0-9]+)?(f|F)?	 { yylval.f = strtof(yytext,NULL); return TK_REAL;}
0[xX][0-9a-fA-F]+"."[0-9a-fA-F]*([Pp][+-]?[0-9]+)?(f|F)?   { yylval.f = strtof(yytext,NULL); return TK_REAL;}

.                                       { return yytext[0]; }

%%

static char* createstring(int length) {

  char *str = (char *) malloc(length);
  if (str == NULL) {
      printf("Falha ao alocar memória.");
      exit(1);
  }
  return str;
}

static char * copystring(char *text) {

    int length = strlen(text);
    char *copy = createstring(length+1);
    strcpy(copy, text);
    return copy;
}

static char* copyescapes(char *text) {

    int length = strlen(text);
    char *copy = createstring(length);
    int i,j, flag = 0;
    for (j = 0, i = 1; i < length - 1; i++) {

      if (flag) {
          switch(text[i]) {
            case '\\':
              copy[j] = '\\';
              j++;
              break;
            case 'n':
              copy[j] = '\n';
              j++;
              break;
            case 't':
              copy[j] = '\t';
              j++;
              break;
            case '\"':
              copy[j] = '\"';
              j++;
              break;
            default:
              printf("Escape Invalido");
              exit(1);
          }
        flag = 0;
      }
      else {
        if(text[i] == '\\') {
          flag = 1;
        }
        else {
          copy[j] = text[i];
          j++;
        }
      }

    }
    copy[j] = '\0';
    return copy;
}
