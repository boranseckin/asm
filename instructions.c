/**
 * Instructions for Basic Assembly Interpreter
 * Author: Boran Seckin <me@boranseckin.com>
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "instructions.h"

// Registers
int regA, regB, regC, regD, value = 0;

bool execute(char *line[4], int lineIndex, int *compare)
{
    // Set 1st argument - must be a registery
    int *firstArg = NULL;
    if (line[1] != NULL)
    {
        if (strcasecmp(line[1], "a") == 0)
        {
            firstArg = &regA;
        }
        else if (strcasecmp(line[1], "b") == 0)
        {
            firstArg = &regB;
        }
        else if (strcasecmp(line[1], "c") == 0)
        {
            firstArg = &regC;
        }
        else if (strcasecmp(line[1], "d") == 0)
        {
            firstArg = &regD;
        }
        else
        {
            printf("Unknown registery %s at line %i\n", line[1], lineIndex + 1);
            return false;
        }
    }
    
    // Set 2nd argument (optional for some instructions) - a registery or an int
    int *secondArg = NULL;
    if (line[2] != NULL)
    {
        // Try to parse register name if not try to parse int
        if (strcasecmp(line[2], "a") == 0)
        {
            secondArg = &regA;
        }
        else if (strcasecmp(line[2], "b") == 0)
        {
            secondArg = &regB;
        }
        else if (strcasecmp(line[2], "c") == 0)
        {
            secondArg = &regC;
        }
        else if (strcasecmp(line[2], "d") == 0)
        {
            secondArg = &regD;
        }
        else
        {
            // Parse int
            value = atoi(line[2]);
            secondArg = &value;
        }
    }

    // MOV
    if (strcasecmp(line[0], "mov") == 0)
    {
        if (secondArg == NULL)
        {
            printf("Cannot call mov without a second argument at line %i\n", lineIndex + 1);
            return false;
        }
        mov(firstArg, secondArg);
        return true;
    }
    // INC
    else if (strcasecmp(line[0], "inc") == 0)
    {
        inc(firstArg);
        return true;
    }
    // DEC
    else if (strcasecmp(line[0], "dec") == 0)
    {
        dec(firstArg);
        return true;
    }
    // ADD
    else if (strcasecmp(line[0], "add") == 0)
    {
        if (secondArg == NULL)
        {
            printf("Cannot call add without a second argument at line %i\n", lineIndex + 1);
            return false;
        }
        add(firstArg, secondArg);
        return true;
    }
    // SUB
    else if (strcasecmp(line[0], "sub") == 0)
    {
        if (secondArg == NULL)
        {
            printf("Cannot call sub without a second argument at line %i\n", lineIndex + 1);
            return false;
        }
        sub(firstArg, secondArg);
        return true;
    }
    // MUL
    else if (strcasecmp(line[0], "mul") == 0)
    {
        if (secondArg == NULL)
        {
            printf("Cannot call mul without a second argument at line %i\n", lineIndex + 1);
            return false;
        }
        mul(firstArg, secondArg);
        return true;
    }
    // DIV
    else if (strcasecmp(line[0], "div") == 0)
    {
        if (secondArg == NULL)
        {
            printf("Cannot call div without a second argument at line %i\n", lineIndex + 1);
            return false;
        }
        div1(firstArg, secondArg);
        return true;
    }
    // CMP
    else if (strcasecmp(line[0], "cmp") == 0)
    {
        if (secondArg == NULL)
        {
            printf("Cannot call cmp without a second argument at line %i\n", lineIndex + 1);
            return false;
        }
        cmp(firstArg, secondArg, compare);
        return true;
    }
    // PRNT
    else if (strcasecmp(line[0], "prnt") == 0)
    {
        prnt(firstArg);
        return true;
    }
    // UNKNOWN
    else
    {
        printf("Unknown command %s at line %i\n", line[0], lineIndex + 1);
        return false;
    }

    return false;
}

// Returns a pointer to the asked register
int * readReg(char reg)
{
    if (tolower(reg) == 'a')
    {
        return &regA;
    }
    else if (tolower(reg) == 'b')
    {
        return &regB;
    }
    else if (tolower(reg) == 'c')
    {
        return &regC;
    }
    else if (tolower(reg) == 'd')
    {
        return &regD;
    }
    
    return NULL;
}

// Moves y into x
void mov(int *x, int *y)
{
    *x = *y;
    return;
}

// Increments x
void inc(int *x)
{
    (*x)++;
    return;
}

// Decrements x
void dec(int *x)
{
    (*x)--;
    return;
}

// Adds y to x
void add(int *x, int *y)
{
    *x += *y;
    return;
}

// Substracts y from x
void sub(int *x, int *y)
{
    *x -= *y;
    return;
}

// Multiply x by y
void mul(int *x, int *y)
{
    *x *= *y;
}

// Divides x by y
void div1(int *x, int *y)
{
    *x /= *y;
}

// Compares x and y, updates compare variable according to the result
// Returns 0, if equal
// Returns 1, if x is more than y
// Returns 2, if x is less than y
void cmp(int *x, int *y, int *compare)
{
    if (*x == *y)
    {
        *compare = 0;
    }
    else if (*x > *y)
    {
        *compare = 1;
    }
    else if (*x < *y)
    {
        *compare = 2;
    }
    else
    {
        printf("Compare error between %i and %i\n", *x, *y);
        exit(1);
    }
}

// Prints the value of the asked register
void prnt(int *x)
{
    printf(" > %i\n", *x);
}
