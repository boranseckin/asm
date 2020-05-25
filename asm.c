/**
 * Basic Assembly Interpreter
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
bool setLabel(char *name);
bool addCallback(int line, fpos_t offset);
void removeCallback(void);
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

// Callback structure to save return pointers
typedef struct callback
{
    int line;
    fpos_t offset;
    struct callback *next;
}
callback;
// Linked list head for callbacks
callback *callbackHead = NULL;

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
        printf("Unable to open the file %s\n", argv[optind]);
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
            printf("Instructions are too long. File cannot exceed the maximum line limit of %i\n", MAX_LINES);
            exit(1);
        }

        // If there is a colon in the line, set it as a label
        char *colonptr = strchr(line, ':');
        if (colonptr != NULL)
        {
            *colonptr = '\0'; // Remove the colon
            if (!setLabel(line))
            {
                printf("Cannot set a duplicate label at line %i\n", lineIndex + 1);
                exit(1);
            }
        }
    }

    // Rewind back to top of the file
    rewind(file);
    lineIndex = -1;
}

// Saves a label into the array
bool setLabel(char *name)
{
    // Make sure a label with the same name doesn't exist
    for (int i = 0; i < MAX_LINES; i++)
    {
        if (strcmp(labels[i].name, name) == 0) return false;
    }

    // Save the label in the array using the line index
    strcpy(labels[lineIndex].name, name);
    fgetpos(file, &labels[lineIndex].offset);

    if (verbose) printf(" - Label %s set for line %i with offset %lli\n", name, lineIndex + 1, labels[lineIndex].offset);

    return true;
}

// Adds a new callback node to the linked list
bool addCallback(int line, fpos_t offset)
{
    // Allocate memory for the new node
    callback *node = malloc(sizeof(callback));
    if (node == NULL) return false;

    // Set line and offset
    node->line = line;
    node->offset = offset;

    if (callbackHead == NULL)
    {
        // If there are no other nodes, point head to the current node
        node->next = NULL;
        callbackHead = node;
        return true;
    }
    else
    {
        // Else, point current node to where head is pointing
        // Then, point head to the current node
        node->next = callbackHead;
        callbackHead = node;
        return true;
    }

    return false;
}

// Removes the last node from the linked list
void removeCallback(void)
{
    // Make sure the list is not empty
    if (callbackHead == NULL) return;
    // Save a reference for the rest of the list
    callback *trav = callbackHead->next;
    // Free the last item
    free(callbackHead);
    // Re-point the head
    callbackHead = trav;
}

// Reads the file line by line and sends the line to the parser
void readline(void)
{
    if (verbose) printf("\nReading instructions...\n");

    char line[MAX_LENGTH]; // Line cannot exceed the MAX_LENGTH
    // Read the file line by line into the line variable if end is not triggered
    while (fgets(line, MAX_LENGTH, file) != NULL && !end)
    {
        lineIndex++;
        // Parse the read line
        parse(line);
    }
}

// Parses a line and executes it accordingly
void parse(char *line)
{
    unsigned int len = strlen(line);
    // If length of the line is <= than 1, ignore
    if (len <= 1) return;

    char str[len + 1]; // Length of line + \0
    memcpy(str, line, len); // Copy line into temporary variable
    str[len] = '\0'; // Add \0 at the end

    // Ignore labels
    if (strchr(str, ':') != NULL) return;

    char *item;
    unsigned int itemIndex = 0; // Index of items in the line

    // First item must be a instruction and followed by a space or new line
    item = strtok(str, " \n");
    while (item != NULL && itemIndex <= 3)
    {
        // Allocate memory for the item and put it into the array
        char *charptr = malloc(strlen(item) + 1);
        strcpy(charptr, item);
        instructions[lineIndex][itemIndex] = charptr;

        itemIndex++;

        // Next item could be seperated by space, comma or semicolon
        // Passing NULL makes strtok use its previous value
        item = strtok(NULL, ", ;\n");
    }

    // If verbose, print the read line
    if (verbose)
    {
        printf(" - %i ->", lineIndex + 1);
        for (int i = 0; i < MAX_ITEMS; i++)
        {
            if (instructions[lineIndex][i] != NULL)
            {
                printf(" %s", instructions[lineIndex][i]);
            }
        }
        printf("\n");
    }

    // Handle jumps
    // Only jump instructions starts with j
    if (instructions[lineIndex][0][0] == 'j')
    {
        if (!handleJump(instructions[lineIndex]))
        {
            printf("Cannot jump to label %s at line %i\n", instructions[lineIndex][1], lineIndex + 1);
            exit(1);
        }
        return;
    }

    // Handle call
    if (strcasecmp(instructions[lineIndex][0], "call") == 0)
    {
        if (!handleCall(instructions[lineIndex]))
        {
            printf("Cannot call the subroutine %s at line %i\n", instructions[lineIndex][1], lineIndex + 1);
            exit(1);
        }
        return;
    }

    // Handle return
    if (strcasecmp(instructions[lineIndex][0], "ret") == 0)
    {
        if (!handleReturn())
        {
            printf("Cannot return from the subroutine at line %i\n", lineIndex + 1);
            exit(1);
        }
        return;
    }

    // Handle end
    if (strcasecmp(instructions[lineIndex][0], "end") == 0)
    {
        end = true; // Setting this true will break the read loop
        return;
    }

    // Handle instructions
    if (!execute(instructions[lineIndex], lineIndex, &compare))
    {
        exit(1);
    }
}

// Jumps to a label with or without a condition
bool handleJump(char *instruction[MAX_ITEMS])
{
    /**
     * jmp - Jump without condition
     * je  - Jump if equal
     * jne - Jump if not eqaul
     * jg  - Jump if greater
     * jge - Jump if greater or equal
     * jl  - Jump if less
     * jle - Jump if less or equal
     */

    char condition[4];
    memcpy(condition, instruction[0], 3);
    condition[3] = '\0';

    // If jump includes a condition, first check compare result
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

    // If condition passed or didn't exist, find the label and jump
    for (int i = 0; i <= MAX_LENGTH; i++)
    {
        if (strcmp(labels[i].name, instruction[1]) == 0)
        {
            if (verbose) printf(" - Jumped to the label %s at line %i\n", labels[i].name, i + 1);

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
            fpos_t offset;
            if (fgetpos(file, &offset) != 0) return false;
            // Add the return point to the linked list
            if (!addCallback(lineIndex, offset)) return false;
            // Jumpt to the subroutine
            if (fsetpos(file, &(labels[i].offset)) == 0)
            {
                if (verbose) printf(" - Called the subroutine %s at line %i\n", labels[i].name, i + 1);
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
    // Make sure that the return point exists
    if (callbackHead == NULL) return false;

    // Jump back to the return point
    if (fsetpos(file, &(callbackHead->offset)) == 0)
    {
        lineIndex = callbackHead->line;

        if (verbose) printf(" - Returned from the subroutine to line %i\n", callbackHead->line + 2);

        // Remove the return point from the list
        removeCallback();
        return true;
    }

    return false;
}

// Destroys a linked list
bool destroy(callback *trav)
{
    if (trav == NULL) return true;

    if (destroy(trav->next))
    {
        free(trav);
        return true;
    }

    return false;
}

// Frees instructions and remaining callbacks. Closes the file
void unload(void)
{
    if (verbose) printf("\nUnloading...\n");

    // If there are unused callback points, clear them
    if (callbackHead != NULL)
    {
        if (verbose) printf(" - Removing uncalled return points\n");

        callback *trav = callbackHead;
        if (!destroy(trav))
        {
            printf("Unable to free all callback points\n");
            exit(1);
        }
    }

    // Free all instructions
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

    // Close the file
    fclose(file);
}
