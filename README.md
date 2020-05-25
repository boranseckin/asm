# Basic Assembly Interpreter

This is my final project for course CS50x.

## Syntax

Interpreter respects most of the regular assembly rules with some exceptions to make the code simpler and more user friendly.
- Each line can only contain one instruction, its arguments (if any) and a comment.
- Depending on the instruction, there could be zero, one or two arguments for an instruction.
- After an instructions, the first argument must be seperated by a space.
- Multiple arguments can be seperated by a space or a comma or both.
- Labels must be unique from instructions and other labels.
- Labels must end with a colon.
- The code must end with `end` instruction, otherwise the interpreter will exit 1.
- Everything is case-insensitive except the label names.
- Indentations, multiple spaces and line breaks are ignored by the interpreter.

## Instructions

In the table below, you can find all the supported instructions by the intrepreter. An asterisks indicates that, a variable can be both a register or an integer.

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

## Author
- Boran Seckin

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
