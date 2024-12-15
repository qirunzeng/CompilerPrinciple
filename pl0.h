#ifndef _PL0_H_
#define _PL0_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "set.h"

#define NRW        11     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       13     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

/**
 * 符号
 */
enum symtype {
	SYM_NULL,       // 
	SYM_IDENTIFIER, // 标识符
	SYM_NUMBER,     // 数字
	SYM_PLUS,       // +
	SYM_MINUS,      // -
	SYM_TIMES,      // *
	SYM_SLASH,      // /
	SYM_ODD,        // odd
	SYM_EQU,        // =
	SYM_NEQ,        // <>
	SYM_LES,        // <
	SYM_LEQ,        // <=
	SYM_GTR,        // >
	SYM_GEQ,        // >=
	SYM_LPAREN,     // (
	SYM_RPAREN,     // )
	SYM_COMMA,      // ,
	SYM_SEMICOLON,  // ;
	SYM_PERIOD,     // .
	SYM_BECOMES,    // :=
    SYM_BEGIN,      // begin
	SYM_END,        // end
	SYM_IF,         // if
	SYM_THEN,       // then
	SYM_WHILE,      // while
	SYM_DO,         // do
	SYM_CALL,       // call
	SYM_CONST,      // const
	SYM_VAR,        // var
	SYM_PROCEDURE,  // procedure
    SYM_AND,        // &&
    SYM_OR,         // ||
    SYM_NOT         // !
};

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_AND, OPR_OR, OPR_NOT
};


typedef struct
{
	int func_code; // function code
	int level; // level
	int addr; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
extern const char* err_msg[];

//////////////////////////////////////////////////////////////////////
extern char ch;         // last character read
extern int  sym;        // last symbol read
extern char id[MAXIDLEN+1]; // last identifier read
extern int  num;        // last number read
extern int  char_cnt;         // character count
extern int  line_length;         // line length
extern int  kk;
extern int  err;
extern int  curr_ins;         // index of current instruction to be generated.
extern int  level;
extern int  tx;
extern int  dx;  // data allocation index

extern char line[80];

extern instruction code[CXMAX];

/**
 * 关键字
 */
extern const char* word[NRW + 1];


/**
 * 关键字
 */
extern const int wsym[NRW + 1];

extern const int ssym[NSYM+1];

extern char csym[NSYM+1];
#define MAXINS   8
extern const char* mnemonic[MAXINS];

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
} comtab;

extern comtab table[TXMAX];

/**
 * 符号集
 */
typedef struct {
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
} mask;

extern FILE* infile;


void error(int);
void getch();
void getsym();
void gen(int, int, int);
void test(symset, symset, int);
void enter(int);
int position(char*);
void constdeclaration();
void vardeclaration();
void listcode(int, int);
void factor(symset);
void term(symset);
void expression(symset);
void condition(symset);
void statement(symset);
void block(symset);
int base(int [], int, int);
void interpret();

#endif /**pl0.h */
// EOF PL0.h
