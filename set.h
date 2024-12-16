#ifndef SET_H
#define SET_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

typedef struct snode {
	int elem;
	struct snode* next;
} snode, *symset;


/**
 * phi: 空集
 * declbegsys: 声明开始符号集
 * statbegsys: 语句开始符号集
 * facbegsys: 因子开始符号集
 * relset: 关系运算符集
 */
symset phi, declbegsys, statbegsys, facbegsys, relset;

symset createset(int data, .../* SYM_NULL */); // 创建集合
void destroyset(symset s); // 销毁集合
symset uniteset(symset s1, symset s2); // 合并集合
int inset(int elem, symset s); // 判断elem是否在集合s中

#endif
// EOF set.h
