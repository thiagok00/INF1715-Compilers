/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_GRAMMAR_TAB_H_INCLUDED
# define YY_YY_GRAMMAR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 1 "grammar.y" /* yacc.c:1909  */

  #include <stdio.h>
  #include <string.h>
  #if !defined(tree_h)
  #include "tree.h"
  #define tree_h
  #endif

#line 53 "grammar.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TK_INT = 258,
    TK_FLOAT = 259,
    TK_CHAR = 260,
    TK_VOID = 261,
    TK_NEW = 262,
    TK_IF = 263,
    TK_ELSE = 264,
    TK_WHILE = 265,
    TK_AS = 266,
    TK_RETURN = 267,
    TK_DEC = 268,
    TK_REAL = 269,
    TK_ID = 270,
    TK_EQUAL = 271,
    TK_NOTEQUAL = 272,
    TK_LESSEQUAL = 273,
    TK_GREATEREQUAL = 274,
    TK_AND = 275,
    TK_OR = 276,
    TK_STRING = 277
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 22 "grammar.y" /* yacc.c:1909  */

  int int_val;
  char *str_val;
  float float_val;
  Programa *prog;
  Definicao* def;
  DefVar* defVar;
  DefFunc* defFunc;
  Tipo* t;
  ParametroL* paramsL;
  Bloco* bloco;
  DefVarL* defvars;
  CMD *cmd;
  CMDL *cmdL;
  Exp* exp;
  Constante* cons;
  Var *var;
  ExpL* expl;


#line 109 "grammar.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAMMAR_TAB_H_INCLUDED  */
