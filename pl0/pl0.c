// pl0 compiler source code

#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"
Array *createlist(void)
{
	// 使用 malloc 分配内存来创建链表节�?
	Array *head = (Array *)malloc(sizeof(Array));

	if (head == NULL)
	{
		printf("Memory allocation failed!\n");
		return NULL;
	}

	// 初始化结构体的各个成�?
	head->id[0] = '\0'; // 字符�? id 初始化为空字符串
	head->dim = 0;		// 数组维度初始化为 0
	for (int i = 0; i < MAXARRAYDIM; i++)
	{
		head->dim_number[i] = 0; // 每个维度的大小初始化�? 0
	}
	head->next = NULL; // 链表中的下一个节点初始化�? NULL

	return head;
}
Array *findtail(Array *head)
{
	Array *p = head;
	while (p->next != NULL)
	{
		p = p->next;
	}
	return p;
}
int compare(char *a, char *b)
{
	int i = 0;
	while (*(a + i) != 0)
	{
		if (*(a + i) != *(b + i))
		{
			return 0;
		}
		i++;
	}
	if (*(b + i) != 0)
		return 0;
	return 1;
}
int searcharray(Array *head, char *id)
{
	Array *p = head;
	while (p != NULL)
	{
		if (!strcmp(p->id, id))
		{
			return 1;
		}
		p = p->next;
	}
	return 0;
}
Array *findarray(Array *head, char *id)
{
	Array *p = head;
	while (p->next != NULL)
	{
		if (strcmp(p->id, id) == 0)
		{
			break;
		}
		p = p->next;
	}
	return p;
}
//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			   && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];
	while (ch == ' ' || ch == '\t' || ch == '\n'|| ch == '\r')
	{
		getch();
	}
	while (ch == '/')
	{
		getch();
		if (ch == '/')
		{
			while (ch != '\n')
			{
				getch();
			}
			getch();
		}
		else if (ch == '*')
		{
			getch();
			while (ch != '*')
			{
				while (ch == '/')
				{
					getch();
					if (ch == '*')
					{
						error(33);
					}
				}
				getch();
				if (ch == '*')
				{
					getch();
					if (ch == '/')
					{
						getch();
						break;
					}
					else
						continue;
				}
			}
		}
		while (ch == ' ' || ch == '\t' || ch == '\n')
		{
			getch();
		}
		if (ch == '*')
		{
			error(33);
		}
	}

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]))
			;
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
		{
			length = 0;	 // 清零上次的数组长�?
			int dim = 0; // 用于记录当前是第几个维度，最前面为第一维度
			int dim_number[MAXARRAYDIM];
			if (ch == '[')
			{
				while (ch == '[')
				{
					getch();
					char len[MAXARRAYLEN];
					for (int i = 0; i < MAXARRAYLEN; i++)
					{
						len[i] = '\0';
					}
					int help_cacu_len = 0; // 用来计算数组长度的字符串数组的辅助变�?
					while (ch != ']')
					{
						if (help_cacu_len <= MAXARRAYLEN)
						{
							len[help_cacu_len] = ch;
						}
						else
							error(34);
						help_cacu_len++;
						getch();
					}
					dim_number[dim] = atoi(len);
					dim++;
					getch();
				}
				if (!searcharray(arraylist, id))
				{
					Array *newnode;
					newnode = (Array *)malloc(sizeof(Array));
					strcpy(newnode->id, id);
					newnode->next = NULL;
					newnode->dim = dim;
					for (int i = 0; i < dim; i++)
					{
						newnode->dim_number[i] = dim_number[i]; // 每个维度的大小初始化�? 0
					}
					Array*p=findtail(arraylist);
					p->next = newnode;
					int multiple = 1; // 计算乘积
					for (int num = 0; num < dim; num++)
					{
						multiple = multiple * dim_number[num];
					}
					length = multiple;
				}
				else
				{
					Array *current_array;
					current_array = findarray(arraylist, id);
					int current_dim = current_array->dim;
					for (int i = 0; i < dim; i++)
					{
						int multiple = dim_number[i];
						for (int j = i + 1; j < dim; j++)
						{
							multiple = multiple * (current_array->dim_number[j]);
						}
						length = length + multiple;
					}
				}
				sym = SYM_ARRAY;
			}
			else
				sym = SYM_IDENTIFIER;
		} // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25); // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL; // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ; // >=
			getch();
		}
		else
		{
			sym = SYM_GTR; // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ; // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ; // <>
			getch();
		}
		else
		{
			sym = SYM_LES; // <
		}
	}
	else if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND;
			getch();
		}
		else
		{
			sym = SYM_NULL; // illegal?
		}
		// TODO
	}
	else if (ch == '|')
	{
		getch();
		if (ch == '|')
		{
			sym = SYM_OR;
			getch();
		}
		// TODO
	}
	else if (ch == '!')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_NEQ;
			getch();
		}
		else
		{
			sym = SYM_NOT;
		}
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch)
			;
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
	printf("%\n");
	printf("%5d %s\t%d\t%d\n", cx - 1, mnemonic[code[cx - 1].f], code[cx - 1].l, code[cx - 1].a);
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (!inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while (!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx; // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask *mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	if (length == 0)
	{
		switch (kind)
		{
		case ID_CONSTANT:
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			table[tx].value = num;
			break;
		case ID_VARIABLE:
			mk = (mask *)&table[tx];
			mk->level = level;
			mk->address = dx++;
			break;
		case ID_PROCEDURE:
			mk = (mask *)&table[tx];
			mk->level = level;
			break;
		} // switch
	}
	else
	{
		if (kind == ID_ARRAY)
		{
			mk = (mask *)&table[tx];
			mk->level = level;
			mk->address = dx;
			dx = dx + length;
		}
	}
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char *id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0)
		;
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else
		error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
	}
	else if (sym == SYM_ARRAY)
	{
		enter(ID_ARRAY);
		getsym();
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	int i;
	symset set;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask *mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask *)&table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_ARRAY)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask *mk;
				case ID_ARRAY:
					mk = (mask *)&table[i];
					gen(LOD, level - mk->level, mk->address+length-1);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if (sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NEG);
		}
		else if (sym == SYM_NOT) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NOT);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop; // multiplication operator
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_AND, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH || sym == SYM_AND)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else if (mulop == SYM_AND)
		{
			gen(OPR, 0, OPR_AND);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_OR, SYM_NULL));

	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS || sym == SYM_OR || sym == SYM_EQU || sym == SYM_NEQ || sym == SYM_GEQ || sym == SYM_GTR || sym == SYM_LES || sym == SYM_LEQ)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else if (addop == SYM_OR)
		{
			gen(OPR, 0, OPR_OR);
		}
		else if (addop == SYM_EQU)
		{
			gen(OPR, 0, OPR_EQU);
		}
		else if (addop == SYM_NEQ)
		{
			gen(OPR, 0, OPR_NEQ);
		}
		else if (addop == SYM_GEQ)
		{
			gen(OPR, 0, OPR_GEQ);
		}
		else if (addop == SYM_GTR)
		{
			gen(OPR, 0, OPR_GTR);
		}
		else if (addop == SYM_LES)
		{
			gen(OPR, 0, OPR_LES);
		}
		else if (addop == SYM_LEQ)
		{
			gen(OPR, 0, OPR_LEQ);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while
	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(createset(SYM_NULL), fsys);
		expression(set);
		destroyset(set);
	} // else
} // condition

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		mask *mk;
		if (!(i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);
		mk = (mask *)&table[i];
		if (i)
		{
			gen(STO, level - mk->level, mk->address);
		}
	}
	else if (sym == SYM_ARRAY)
	{
		// variable assignment
		mask *mk;
		if (!(i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_ARRAY)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);
		mk = (mask *)&table[i];
		if (i)
		{
			gen(STO, level - mk->level, mk->address + length - 1);
		}
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (!(i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask *mk;
				mk = (mask *)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called.
			}
			getsym();
		}
	}
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	test(fsys, phi, 19);
} // statement

//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask *mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask *)&table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; // save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}

			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; // restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8);	  // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc; // program counter
	int stack[STACKSIZE];
	int top;	   // top of stack
	int b;		   // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			case OPR_AND: // ! AND
				top--;
				if (stack[top])
				{
					if (stack[top + 1])
					{
						stack[top] = 1;
					}
					else
					{
						stack[top] = 0;
					}
				}
				else
				{
					stack[top] = 0;
				}
				break;
			case OPR_OR: // ! OR
				top--;
				if (stack[top])
				{
					stack[top] = 1;
				}
				else if (stack[top + 1])
				{
					stack[top] = 1;
				}
				else
				{
					stack[top] = 0;
				}

				break;
			case OPR_NOT: // ! NOT
				if (stack[top])
					stack[top] = 0;
				else
					stack[top] = 1;
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		} // switch
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main()
{
	FILE *hbin;
	char s[80];
	int i;
	symset set, set1, set2;
	arraylist = createlist();
	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NOT, SYM_ARRAY, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err <= 100)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
