# Basic Assembly Interpreter

This is my final project for [CS50](https://cs50.harvard.edu/x/2020/project/) 🎉.

This assembly interpreter is focused to run a simple version of the [assembly language](https://wikipedia.org/wiki/Assembly_language) but at the same time, it supports the fundamental features like subroutines and conditional jumps to run complex alogrithms. It has 4 registers (A, B, C, D) and can process mathematical and logical operations.

```
Usage: ./asm [-v] instructions.asm
```
On top of that, it has the option to be really verbose. If the `-v` tag is passed, it will print out every single step while it is executing the program. This allows the user to see debuging information like which line is being executed, what is the instruction on that line and where the program jumped.

## Design
This interpreter is made to be as dynamic as possible. It can read inifinite lines of code, iterate over infinite loops and can handle recursion thanks to its call stacking feature. Altough there are predefined limits like maximum amount of [lines](instructions.h#L7) and maximum [characters](instructions.h#L8) in one line to limit memory usage, they can be adjusted or even removed.

Interpreter has 5 main components:
#### Scanner
When the file is successfully opened, a scanner function reads the entire file line by line and looks for the labels. If it finds a label, it stores the name and the position off the label (relative to the start of the file) into an array.

#### Reader
Once the scan is finished, a reading loop is activated. The loop keeps reading line by line until either the file stream reaches the EOF or the instruction `end` is executed. This loop makes it very easy to handle jumps and allows the interpreter to be dynamic. Once a line is read, it is sent to the parser.

#### Parser
The main purpose of the parser is to isolate the instruction and its arguments (if any). It mostly benefits from the `strtok` function to seperate a line into pieces. Then, it decides what instruction should be executed and calls the corresponding executer.

#### Executer
The main executer in the [instructions](instructions.c#L17) file handles operations with the registers like `mov` and `cmp`. 

Rest of the functions are handled inside the [main](asm.c) file.

Jumps (if conditional) uses a global variable to see the last comparison result and executes accordingly. When a jump happens, the position of the label is fetched from the array (thanks to the scanner the label can be after the jump instruction too) and the file stream pointer is positioned to where the label is located.

Calls to subroutines fundamently uses the same mechanic as the jumps, however, when a return instruction is called the program needs to know where to return. For that reason, when a `call` is happend, in addition to the jumping position, the current position is also saved. To save the position, the interpreter uses a linked list, therefore, it stacks the returning pointer and if another subroutine call happens inside a subroutine, the program can safely return to the most recent call.

#### Unloader
Once the execution is done, the program frees all the dynamic memory it allocates and closes the file stream. Thus, there are no memory leaks possible.

## Syntax

Interpreter respects most of the regular assembly rules with some exceptions to make the code simpler and more user friendly.
- Each line can only contain one instruction, its arguments (if any) and a comment (starts with semicolon).
- Depending on the instruction, there could be zero, one or two arguments.
- After an instructions, the first argument must be seperated by a space.
- Multiple arguments can be seperated by a space, a comma or both.
- Labels must be unique from instructions and other labels.
- Labels must end with a colon.
- The code must end with `end` instruction, otherwise the interpreter will exit with 1.
- Everything is case-insensitive except the label names.
- Indentations, multiple spaces and line breaks are ignored by the interpreter.

## Instructions

In the table below, you can find all the supported instructions by the intrepreter. An asterisks indicates that, a variable can be a register or an integer.

| Instruction | Description |
| ----------- | ----------- |
| `mov x, y*` | Copies the value of `y` into `x` |
| `inc x`     | Increments the value of `x` |
| `dec x`     | Decrements the value of `x` |
| `add x, y*` | Adds the value of `y` to `x` and stores it in `x` |
| `sub x, y*` | Substracts the value of `y` from `x`and stores it in `x` |
| `mul x, y*` | Multiplies `x` by the value of `y` and stores it in `x` |
| `div x, y*` | Divides `x` by the value of `y` and stores it in `x` |
| `cmp x*, y*`| Compares `x` and `y`, see [conditions](#conditions) for more details |
| `prnt x`    | Prints the value of `x` during runtime |
| `jmp lbl`   | Jumps to label `lbl` without a condition |
| `je lbl`    | Jumps to label `lbl`, if compare was equal |
| `jne lbl`   | Jumps to label `lbl`, if compare was not equal |
| `jg lbl`    | Jumps to label `lbl`, if compare was greater |
| `jge lbl`   | Jumps to label `lbl`, if compare was equal or greater |
| `jl lbl`    | Jumps to label `lbl`, if compare was less |
| `jle lbl`   | Jumps to label `lbl`, if compare was equal or less |
| `call lbl`  | Calls the subroutine `lbl`, see [subroutines](#subroutines) for more details |
| `ret`       | Returns from the most recent subroutine |
| `end`       | Ends the program |

## Conditions
`cmp` instruction will compare the value of first argument with the value of second argument. The result of the comparison is stored in a global variable and overriten with every successful `cmp` call. The call does not affect any register, instead, it is used as a reference for an imminent conditional jump.

For instance, the code below will not jump to `foo` at line 4 but will jump to `bar` at line 5 because `a` is greater than `b`.
```
mov a, 1
mov b, 0
cmp a, b
jl  foo
jg  bar
```

The global variable is initialized as -1 and changed as 0 if equal, 1 if greater and 2 if less.

## Subroutines
In addition to jumps, labels can be used to signify subroutines. A subroutine, when called, acts like a synchronous function. The frame is added on top of the call stack and executed until it returns by the `ret` instruction.

When `ret` is found in a subroutine the program is returned to the next line where that subroutine is called.

A subroutine can call itself or other subroutines. In case of multiple subroutine calls, `ret` will only returns the last subroutine call. In theory, with this method, infinite amount of subroutine calls can be called (until the system runs out of memory) and recursion can be easily achieved.

`ret` should only be used in a subroutine call. Using it outside a subroutine will cause your program to terminate with an error. Other the other hand, if a subroutine is not returned by `ret` (for example, ended or jumped), it will not cause any problem and the unused pointers will be freed.

## Code Examples
#### Fibonacci Sequence
```
mov a, 1
mov b, 0

fib:
    mov c, 0
    add c, a
    add c, b
    prnt c
    mov a, b
    mov b, c

cmp c 100
jl fib

end
```
Output:
```
=========== asm v1.0 ===========
 > 1
 > 1
 > 2
 > 3
 > 5
 > 8
 > 13
 > 21
 > 34
 > 55
 > 89
 > 144
A: 89, B: 144, C: 144, D: 0, COMP: 1
================================
```

## Author
- Boran Seckin

## Reference
I got the idea of building a basic assembly interpreter from the Kata - [Assembler interpreter (part II)](https://www.codewars.com/kata/58e61f3d8ff24f774400002c/c) at [Code Wars](https://www.codewars.com/).

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
