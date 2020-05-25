/**
 * Custom Assembly Language Interpreter
 * Author: Boran Seckin <me@boranseckin.com>
 * Inspired by the Kata at https://www.codewars.com/kata/58e61f3d8ff24f774400002c/c
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "instructions.h"

void scan(void);
void readline(void);
void parse(char *line);
void setLabel(char *name);
bool handleJump(char *instruction[MAX_ITEMS]);
bool handleCall(char *instruction[MAX_ITEMS]);
bool handleReturn(void);
void unload(void);

// Prints debug values if true
bool verbose = false;

// Instructions file pointer
FILE *file = NULL;

// Instructions by line
char *instructions[MAX_LINES][MAX_ITEMS];

// Labels
typedef struct
{
    char name[MAX_LENGTH];
    fpos_t offset;
}
label;
// Labels by line
label labels[MAX_LINES];

// Line Index
int lineIndex = -1;

// Offset to return from a subroutine
int callbackLine = -1;
fpos_t callbackOfffset = -1;

// End the program if true
bool end = false;

// Result of the compare
// 0 if equal / 1 if greater / 2 if less
int compare = -1;

int main(int argc, char *argv[])
{
    // Print header
    printf("=========== asm v1.0 ===========\n");

    // Can be called with v flag or without flag
    char flag = getopt(argc, argv, "v");
    if (flag == '?')
    {
        exit(1);
    }
    else if (flag == 'v')
    {
        // If v flag is present make it verbose
        verbose = true;
    }

    // Must be called with one argument
    if (argc != optind + 1)
    {
        printf("Usage: asm [flag] <instructions>\n");
        exit(1);
    }

    // Try to open the file
    file = fopen(argv[optind], "r");
    if (file == NULL)
    {
        printf("Unable to open the file %s\n", argv[1]);
        exit(1);
    }

    // Scan the file for labels
    scan();

    // Read the file line by line
    readline();

    // Unload any allocated memory
    unload();

    // Print the final result of registers
    printf("A: %i, B: %i, C: %i, D: %i, COMP: %i\n",
        *readReg('a'),
        *readReg('b'),
        *readReg('c'),
        *readReg('d'),
        compare
    );
    printf("================================\n");

    // If end is triggered, program ends without error
    if (end) return 0;
    // Else ends with 1
    return 1;
}

// Scans the file only for labels and sets the labels
void scan(void)
{
    if (verbose) printf("\nScanning labels...\n");

    char line[MAX_LENGTH]; // Line cannot exceed the MAX_LENGTH
    // Read the file line by line into the line variable
    while (fgets(line, MAX_LENGTH, file) != NULL)
    {
        lineIndex++;
        // File cannot contain more lines than MAX_LINES
        if (lineIndex > MAX_LINES - 1)
        {
            printf("Instructions are too long. Max accepted lines are %i.\n", MAX_LINES);
            exit(1);
        }

        // If there is a colon in the line, set it as label
        char *colonptr = strchr(line, ':');
        if (colonptr != NULL)
        {
            *colonptr = '\0';
            setLabel(line);
        }
    }

    // Rewind back to top of the file
    rewind(file);
    lineIndex = -1;
}

// Saves a label into the array
void setLabel(char *name)
{
    strcpy(labels[lineIndex].name, name);
    fgetpos(file, &labels[lineIndex].offset);
    if (verbose) printf(" - Label %s set for line %i with offset %lli\n", name, lineIndex + 1, labels[lineIndex].offset);
}

// Read the file line by line and send the line to the parser
void readline(void)
{
    if (verbose) printf("\nReading instructions...\n");

    char line[MAX_LENGTH]; // Line cannot exceed the MAX_LENGTH
    // Read the file line by line into the line variable if end is not triggered
    while (fgets(line, MAX_LENGTH, file) != NULL && !end)
    {
        lineIndex++;
        // File cannot contain more lines than MAX_LINES
        if (lineIndex > MAX_LINES - 1)
        {
            printf("Instructions are too long. Max amount of lines is %i.\n", MAX_LINES);
            exit(1);
        }

        // Parse the read line
        parse(line);
    }
}

// Parse a line
void parse(char *line)
{
    unsigned int len = strlen(line);

    // If length of the line is <= than 1, ignore
    if (len <= 1) return;

    char str[len + 1]; // Length of line + \0
    memcpy(str, line, len); // Copy line into string
    str[len] = '\0'; // Add \0

    // Ignore labels
    if (strchr(str, ':') != NULL) return;

    unsigned int itemIndex = 0; // Index of the item in line

    char *item;
    // First item must be a instruction and followed by a space
    item = strtok(str, " ");
    while (item != NULL && itemIndex <= 3)
    {
        char *charptr = malloc(strlen(item) + 1);
        strcpy(charptr, item);
        instructions[lineIndex][itemIndex] = charptr;
        itemIndex++;
        // Next items could be seperated by space or comma
        // It might be indented by a tab
        // Semicolon indicates a comment
        // Passing NULL makes strtok use its previous value
        item = strtok(NULL, "\t, ;\n");
    }

    // If verbose, print the read line
    if (verbose) {
        printf(" - %i ->", lineIndex);
        for (int i = 0; i < MAX_ITEMS; i++)
        {
            if (instructions[lineIndex][i] != NULL)
            {
                printf(" %s", instructions[lineIndex][i]);
            }
        }
        printf("\n");
    }

    // If there is end, stop
    if (instructions[lineIndex][0][0] == 'e')
    {
        end = true;
        return;
    }

    // Handle jumps
    if (instructions[lineIndex][0][0] == 'j')
    {
        if (!handleJump(instructions[lineIndex]))
        {
            printf("Cannot jump to label %s @line %i\n", instructions[lineIndex][1], lineIndex);
            exit(1);
        }
        return;
    }

    // Handle call
    if (strcasecmp(instructions[lineIndex][0], "call") == 0)
    {
        if (!handleCall(instructions[lineIndex]))
        {
            printf("Cannot call the subroutine %s @line %i\n", instructions[lineIndex][1], lineIndex);
            exit(1);
        }
        return;
    }

    // Handle return
    if (strcasecmp(instructions[lineIndex][0], "ret") == 0)
    {
        if (!handleReturn())
        {
            printf("Cannot return from the subroutine @line %i\n", lineIndex);
            exit(1);
        }

        callbackLine = -1;
        callbackOfffset = -1;
        return;
    }

    // Handle instructions
    if (!execute(instructions[lineIndex], lineIndex, &compare))
    {
        exit(1);
    }
}

// Jumps to a label
bool handleJump(char *instruction[MAX_ITEMS])
{
    char condition[4];
    memcpy(condition, instruction[0], 3);
    condition[3] = '\0';
    
    // Decide to jump using condition

    // Equal
    if (condition[1] == 'e')
    {
        // Return if not equal
        if (compare != 0) return true;
    }
    // Not equal
    else if (condition[1] == 'n' && condition[2] == 'e')
    {
        // Return if equal
        if (compare == 0) return true;
    }
    // Greater
    else if (condition[1] == 'g')
    {
        // Greater or equal
        if (condition[2] == 'e')
        {
            // Return if less
            if (compare == 2) return true;
        }
        // Return if not greater
        else if (compare != 1) return true;
    }
    // Less
    else if (condition[1] == 'l')
    {
        // Less or equal
        if (condition[2] == 'e')
        {
            // Return if greater
            if (compare == 1) return true;
        }
        // Return if not less
        else if (compare != 2) return true;
    }

    // Find the label and jump
    for (int i = 0; i <= MAX_LENGTH; i++)
    {
        if (strcmp(labels[i].name, instruction[1]) == 0)
        {
            if (verbose) printf(" - Jumped to %s @line %i\n", labels[i].name, i);

            if (fsetpos(file, &(labels[i].offset)) == 0)
            {
                lineIndex = i;
                return true;
            }
        }
    }
    
    return false;
}

// Handles call to a subroutine
bool handleCall(char *instruction[MAX_ITEMS])
{
    for (int i = 0; i <= MAX_LENGTH; i++)
    {
        if (strcmp(labels[i].name, instruction[1]) == 0)
        {
            // Save the curren position
            callbackLine = lineIndex;
            if (fgetpos(file, &callbackOfffset) != 0)
            {
                return false;
            }

            if (verbose) printf(" - Called the subroutine %s @line %i\n", labels[i].name, i);

            if (fsetpos(file, &(labels[i].offset)) == 0)
            {
                lineIndex = i;
                return true;
            }
        }
    }

    return false;
}

// Handles returns from a subroutine
bool handleReturn(void)
{
    if (callbackOfffset == -1 || callbackLine == -1)
    {
        return false;
    }

    if (verbose) printf(" - Returned from the subroutine to line %i\n", callbackLine);

    if (fsetpos(file, &(callbackOfffset)) == 0)
    {
        lineIndex = callbackLine;
        return true;
    }

    return false;
}

// Frees instructions and close the file
void unload(void)
{
    if (verbose) printf("\nUnloading...\n");

    for (int i = 0; i <= MAX_LENGTH; i++)
    {
        for (int j = 0; j < MAX_ITEMS; j++)
        {
            if (instructions[i][j] != NULL)
            {
                if (verbose) printf(" - %s", instructions[i][j]);
                free(instructions[i][j]);
            }
        }
        if (instructions[i][0] != NULL && verbose)
        {
            printf("\n");
        }
    }

    if (verbose) printf("\n");

    fclose(file);
}
