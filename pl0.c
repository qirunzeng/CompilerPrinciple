// pl0 compiler source code

// #pragma warning(disable:4996)

#include "pl0.h"

const char* err_msg[] = {
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "",
/* 27 */    "",
/* 28 */    "",
/* 29 */    "",
/* 30 */    "",
/* 31 */    "",
/* 32 */    "There are too many levels.",
/* 33 */    "Big ARRAY ERROR",
/* 34 */    "Parameter count mismatch.",
/* 35 */    "Parameter type mismatch."
};

int length ;		   // 用于存数组长度
Array *arraylist;
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  char_cnt;         // character count
int  line_length;         // line length
int  id_index;                // index of identifier in table
int  err;
int  curr_ins;         // index of current instruction to be generated.
int  level;
int  table_index; // table index
int  data_alloc_index;  // data allocation index
char line[80];
instruction code[CXMAX];

const char *word[NRW+1] = {
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while"
};

const int wsym[NRW+1] = {
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE
};

const int ssym[NSYM+1] = {
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON, SYM_AND, SYM_OR, SYM_NOT
};

char csym[NSYM+1] = {
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';', '&', '|', '!'
};

const char* mnemonic[MAXINS] = { // 指令助记符
	"LIT", // Load constant value to stack top
    "OPR", // Arithmetic operation
    "LOD", // Load value to stack top from stack
    "STO", // Store value to stack top from stack
    "CAL", // Call procedure
    "INT", // Increment t-register
    "JMP", // Jump
    "JPC"  // Jump conditional
};

comtab table[TABLE_INDEX_MAX];    // symbol table
FILE* infile;
//List相关操作用于实现数组
Array *createlist(void)
{
	// 使用 malloc 分配内存来创建链表节点
	Array *head = (Array *)malloc(sizeof(Array));

	if (head == NULL)
	{
		printf("Memory allocation failed!\n");
		return NULL;
	}

	// 初始化结构体的各个成员
	head->id[0] = '\0'; // 字符串 id 初始化为空字符串
	head->dim = 0;		// 数组维度初始化为 0
	for (int i = 0; i < MAXARRAYDIM; i++)
	{
		head->dim_number[i] = 0; // 每个维度的大小初始化为 0
	}
	head->next = NULL; // 链表中的下一个节点初始化为 NULL

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
// print error message.
void error(const int n) {
	int i;
	printf("      ");
	for (i = 1; i+1 <= char_cnt; i++) {
		printf(" ");
    }
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

void getch() {
    // 读取一个字符，并存入 ch 中
	if (char_cnt == line_length) {
		if (feof(infile)) {
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		line_length = char_cnt = 0;
		printf("%5d  ", curr_ins);
		while ( (!feof(infile)) && ((ch = getc(infile)) != '\n')) {
			printf("%c", ch);
			line[++line_length] = ch;
		}
		printf("\n");
		line[++line_length] = ' ';
	}
	ch = line[++char_cnt];
} // getch

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
	    length = 0;	 // 清零上次的数组长度
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
			length = 0;	 // 清零上次的数组长度
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
					int help_cacu_len = 0; // 用来计算数组长度的字符串数组的辅助变量
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
						newnode->dim_number[i] = dim_number[i]; // 每个维度的大小初始化为 0
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
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (curr_ins > CXMAX) {
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[curr_ins].func_code = x;
	code[curr_ins].level = y;
	code[curr_ins].addr = z;
    curr_ins++; // 下一条指令
} // gen

// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;
	if (!inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test


// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask *mk;

	table_index++;
	strcpy(table[table_index].name, id);
	table[table_index].kind = kind;
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
			table[table_index].value = num;
			break;
		case ID_VARIABLE:
			if (table[table_index].param_count == 0)
			{
				mk = (mask *)&table[table_index];
				mk->level = level;
				mk->address = data_alloc_index++;
				break;
			}
			else
			{
				mk = (mask*)&table[table_index];
				mk->level = level;
				mk->address = data_alloc_index++;
				// mk->address = 3 + table[table_index].param_count;
				break;
			}
			
		case ID_PROCEDURE:
			mk = (mask *)&table[table_index];
			mk->level = level;
			// table[table_index].param_count = 0;
			break;
		} // switch
	}
	else
	{
		if (kind == ID_ARRAY)
		{
			mk = (mask *)&table[table_index];
			mk->level = level;
			mk->address = data_alloc_index;
			data_alloc_index = data_alloc_index + length;
		}
	}
} // enter


// locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = table_index + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position


/**
 * @brief 用于处理常量声明
 */
void constdeclaration() {
	if (sym == SYM_IDENTIFIER) {
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES) {
			if (sym == SYM_BECOMES) {
				error(1); // Found ':=' when expecting '='.
            }
			getsym();
			if (sym == SYM_NUMBER) {
				enter(ID_CONSTANT);
				getsym();
			}
			else {
				error(2); // There must be a number to follow '='.
			}
		}
		else {
			error(3); // There must be an '=' to follow the identifier.
		}
	} 
    else {
        error(4);
    }
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
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].func_code], code[i].level, code[i].addr);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
/**
 * @brief 处理因子
 */
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
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH,SYM_AND, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH||sym==SYM_AND)
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


void expression(symset fsys) {
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_OR, SYM_NULL));
	
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS||sym==SYM_OR||sym == SYM_EQU || sym == SYM_NEQ||sym == SYM_GEQ||sym == SYM_GTR||sym == SYM_LES||sym == SYM_LEQ) {
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS) {
			gen(OPR, 0, OPR_ADD);
		}
		else if(addop == SYM_OR){
			gen(OPR, 0, OPR_OR);
		}
		else if(addop == SYM_EQU){
			gen(OPR, 0, OPR_EQU);
		}
		else if(addop == SYM_NEQ){
			gen(OPR, 0, OPR_NEQ);
		}
		else if(addop == SYM_GEQ){
			gen(OPR, 0, OPR_GEQ);
		}
		else if(addop == SYM_GTR){
			gen(OPR, 0, OPR_GTR);
		}
		else if(addop == SYM_LES){
			gen(OPR, 0, OPR_LES);
		}
		else if(addop == SYM_LEQ){
			gen(OPR, 0, OPR_LEQ);
		}
		else {
			gen(OPR, 0, OPR_MIN);
		}
    } // while
	destroyset(set);
} // expression

void condition(symset fsys)
{
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
		mask* mk;
		if (! (i = position(id)))
		{
			printf("bug in statement");
			printf("the bug is %s", id);
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
		mk = (mask*) &table[i];
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
			int proc_index = position(id);
			printf("position_id %d", proc_index);
			if (proc_index == 0)
			{
				printf("bug in call");
				error(11); // Undeclared identifier.
			}
			else if (table[proc_index].kind == ID_PROCEDURE)
			{
				int expected_params = table[proc_index].param_count;
				printf("expected_params %d", expected_params);
				int actual_params = 0;
				int actual_param_types[MAXIDLEN];

				getsym();
				if (sym == SYM_LPAREN)
				{
					getsym();
					while (sym != SYM_RPAREN && sym != SYM_NULL)
					{
						expression(fsys);
						if (actual_params < expected_params)
						{
							int param_type = table[proc_index].param_types[actual_params];
							if (param_type == PARAM_CONSTANT)
							{
								// 假设表达式已经将常量值推入栈中，直接使用
								// 不需要额外指令，因为LIT已被生成
								// 如果需要区分，可以在表达式解析时标记类型
							}
							else if (param_type == PARAM_VARIABLE)
							{
								// 需要加载变量值
								// 假设变量的地址已知，通过LOD加载
								// 这里假设实参是标识符，则可以使用LOD
								// 具体实现视`expression()`函数如何处理
								// 如果`expression()`已将值推入栈中，则无需额外操作
							}
							// 如果有更多类型，按需处理
						}
						if (table[proc_index].param_types[actual_params] == PARAM_CONSTANT && sym == SYM_IDENTIFIER) {
							error(35); // 常量参数不能传递变量
						}
						// expression(fsys); // 将实参表达式的值压入栈中
						actual_params++;
						if (sym == SYM_COMMA)
						{
							getsym();
						}
						else
						{
							break;
						}
					}
					if (sym == SYM_RPAREN || sym == SYM_SEMICOLON)
					{
						getsym();
					}
					else
					{
						printf("bug3, %d", sym);
						error(5); // 缺少')'
					}
				}

				// 参数个数检查
				if (actual_params > expected_params)
				{
					printf("actual_params %d", actual_params);
					printf("expected_params %d", expected_params);
					error(34); // 参数个数不匹配
				}
				for (int i = 0; i < actual_params; i++)
				{
					int param_type = table[proc_index].param_types[i];
					if (param_type == PARAM_CONSTANT)
					{
						// 假设常量已通过LIT指令推入栈中，无需额外操作
					}
					else if (param_type == PARAM_VARIABLE)
					{
						// 传递变量值，生成LOD指令加载变量值
						// 假设变量的层次差和地址已记录
						mask* mk = (mask*)&table[proc_index];
						gen(LOD, level - mk->level, mk->address + i); // 根据具体实现调整地址
					}
					// 根据需要添加更多类型的处理
				}
				mask* mk;
				mk = (mask*) &table[proc_index];
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
		cx1 = curr_ins;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].addr = curr_ins;
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
		cx1 = curr_ins;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = curr_ins;
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
		code[cx2].addr = curr_ins;
	}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
/**
 * @brief 处理块，包括常量声明、变量声明、过程声明、语句
 */
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_data_alloc_index;
	int saved_table_index;
	symset set1, set;

	data_alloc_index = 3;
	block_data_alloc_index = data_alloc_index;
	mk = (mask*) &table[table_index];
	mk->address = curr_ins;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL) {
		error(32); // There are too many levels.
	}
	do { // while (inset(sym, declbegsys))
		printf("\n\nsym_type\n, %d", sym);
		if (sym == SYM_CONST) { 
            // constant declarations
			getsym();
			do { // while (sym == SYM_IDENTIFIER)
				constdeclaration();
				while (sym == SYM_COMMA) { // ,
					getsym();
					constdeclaration();
				}
                if (sym == SYM_SEMICOLON) { // ;
					getsym();
                    break;
				}
				else {
					error(5); 
                    // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if
		if (sym == SYM_VAR) { 
            // variable declarations
			getsym();
			printf("SVM_VAR\n");
			do {
				vardeclaration();
				while (sym == SYM_COMMA) {
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON) { 
                    // 表示变量声明结束，应该进行下一步
					getsym(); 
                    break;
				}
				else {
					error(5); 
                    // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		}
		block_data_alloc_index = data_alloc_index; 
        // save data_alloc_index before handling procedure call!
		while (sym == SYM_PROCEDURE) { 
			int procedure_index = -1;
            // procedure declarations
			getsym();
			printf("find procedure");
			if (sym == SYM_IDENTIFIER) {
				enter(ID_PROCEDURE);
				procedure_index = table_index;
				printf("procedure_index %d\n", procedure_index);
				table[procedure_index].param_count = 0;
				getsym();
			}
			else {
				error(4); 
                // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}
			printf("find identifier");
			if (sym == SYM_LPAREN)
			{
				printf("find (");
				getsym();
				int param_num = 0;
				while (sym == SYM_CONST || sym == SYM_VAR)
				{
					int param_type;

					// 确定参数类型
					if (sym == SYM_CONST)
					{
						param_type = PARAM_CONSTANT;
						printf("is constant");
					}
					else if (sym == SYM_VAR)
					{
						param_type = PARAM_VARIABLE;
						printf("is var");
					}

					// char param_id[MAXIDLEN + 1];
					// strcpy(param_id, id);
					getsym();

					// 假设参数类型默认为变量
					if (sym == SYM_IDENTIFIER)
					{
						printf("table_index %d ", table_index);
						table[procedure_index].param_types[param_num] = param_type;
						// table[table_index].address = 3 + param_num;
						enter(param_type == PARAM_CONSTANT ? ID_CONSTANT : ID_VARIABLE);
						printf("table_index %d ", table_index);
						param_num++;
						table[procedure_index].param_count++;
						getsym();
					}
					else
					{
						error(4);
					}
					// table[table_index].param_types[param_num++] = PARAM_VARIABLE;


					if (sym == SYM_COMMA)
					{
						getsym();
					}
					else
					{
						break;
					}
				}
				if (sym == SYM_RPAREN)
				{
					getsym();
				}
				else
				{
					printf("bug5");
					error(5); // 缺少','或')'
				}
				// printf("sore param_count %d", param_num);
				// printf("table_index %d ", table_index);	
				// table[table_index].param_count = param_num;
			}		
			if (sym == SYM_SEMICOLON) {
				getsym();
			}
			else {
				printf("bug1");
				error(5); // Missing ',' or ';'.
			}
			level++;
			saved_table_index = table_index	;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			printf("start recurson");
			block(set);
			destroyset(set1);
			destroyset(set);
			table_index = saved_table_index;
			level--;
			if (sym == SYM_SEMICOLON) {
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else {
				printf("bug2");
				error(5); // Missing ',' or ';'.
			}
		}
		data_alloc_index = block_data_alloc_index; 
        // restore data_alloc_index after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));
	printf("\n\nout of block loop\n\n");
	code[mk->address].addr = curr_ins;
	mk->address = curr_ins;
	cx0 = curr_ins;
	gen(INT, 0, block_data_alloc_index);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, curr_ins);
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
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.func_code)
		{
		case LIT:
			stack[++top] = i.addr;
			break;
		case OPR:
			switch (i.addr) // operator
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
					else{
					stack[top] = 0;
				    }
				}
				else{
					stack[top] = 0;
				}
				break;
			case OPR_OR: // ! OR
				top--;
				if(stack[top]){
					stack[top]=1;
				}
				else if(stack[top+1]){
					stack[top]=1;
				}
				else{
					stack[top]=0;
				}

				break;
			case OPR_NOT: // ! NOT
			    if(stack[top])
				stack[top] = 0;
				else
				stack[top] = 1;
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.level) + i.addr];
			break;
		case STO:
			stack[base(stack, b, i.level) + i.addr] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.level);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.addr;
			break;
		case INT:
			top += i.addr;
			break;
		case JMP:
			pc = i.addr;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.addr;
			top--;
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

// eof pl0.c

