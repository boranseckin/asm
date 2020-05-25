// Declares instructions' functionality
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdbool.h>

#define MAX_LINES 100 // Max amount of lines
#define MAX_LENGTH 60 // Max length of a line
#define MAX_ITEMS 3 // Max amount of args in one line

// Prototypes
void mov(int *x, int *y);
void inc(int *x);
void dec(int *x);
void add(int *x, int *y);
void sub(int *x, int *y);
void mul(int *x, int *y);
void div1(int *x, int *y);
void cmp(int *x, int *y, int *compare);
void prnt(int *x);

bool execute(char *line[4], int lineIndex, int *compare);
int * readReg(char reg);

#endif // INSTRUCTIONS_H
