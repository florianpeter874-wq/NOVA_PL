# NOVA

**Nova 1.1**
*the children of the Nova project.*
**Built by Netfloor Software Corporation**

NOVA is a standalone programming language designed to be simple, readable, and easy to compile.

The NOVA compiler is written in **C** and currently generates C code, which is then compiled using **GCC**.

NOVA is an independent language project. It is not designed specifically for an operating system or for OOSB.

---

## Features

NOVA currently supports:

* Programs
* Functions
* Entry points
* Variables
* Integer values
* Strings
* `print()`
* Function calls
* `if / else`
* Comparisons
* Arithmetic with `+`
* Parenthesized expressions
* String and integer output
* C code generation
* GCC compilation
* Benchmarking

---

## Example

```nova
entryp Main main

program Main {
    function hello() {
        print("Hello from NOVA!");
    }

    function main() {
        set int x = 42;

        hello();

        print(x);
        print("The number is:" + x);

        if (x == 42) {
            print("YES!");
        } else {
            print("NO!");
        }
    }
}
```

---

## Compilation

The NOVA compiler is built as:

```text
nova.exe
```

Compile a NOVA program with:

```powershell
.\nova.exe examples\hello.nova -o hello.exe
```

Then run the generated program:

```powershell
.\hello.exe
```

The compilation pipeline is:

```text
NOVA source
    ↓
   Lexer
    ↓
  Parser
    ↓
    AST
    ↓
 Compiler
    ↓
generated C
    ↓
   GCC
    ↓
  .exe
```

---

## Project Structure

```text
NOVA/
├── src/
│   ├── main.c
│   ├── lexer.c
│   ├── parser.c
│   ├── ast.c
│   ├── compiler.c
│   ├── error.c
│   └── installgcc.c
│
├── include/
│   ├── lexer.h
│   ├── parser.h
│   ├── ast.h
│   ├── compiler.h
│   └── error.h
│
├── examples/
│   └── hello.nova
│
├── benchmark/
│   ├── run_benchmark.py
│   └── run_benchmark.exe
│
└── README.md
```

---

## Benchmark

NOVA includes a benchmark runner for testing the language and compiler.

The benchmark runner can be used directly as an executable:

```powershell
.\benchmark\run_benchmark.exe
```

The original Python version is also included:

```powershell
python benchmark\run_benchmark.py
```

The benchmark is intended to test things such as:

* Numeric operations
* Loops
* Function calls
* Recursion
* Lists
* Bitwise operations
* Strings
* File operations
* Compilation time
* Runtime
* Memory usage
* Compatibility

The benchmark is useful for tracking NOVA's development over time.

---

## Language Syntax

### Entry Point

```nova
entryp Main main
```

This tells NOVA which function should be used as the program entry point.

---

### Program

```nova
program Main {
    ...
}
```

A NOVA source file contains a program declaration.

---

### Functions

```nova
function hello() {
    print("Hello!");
}
```

Functions can be called from other functions:

```nova
hello();
```

---

### Variables

```nova
set int x = 42;
```

Variables can then be used:

```nova
print(x);
```

---

### Conditions

```nova
if (x == 42) {
    print("Correct!");
} else {
    print("Wrong!");
}
```

Supported comparison operators include:

```text
==
!=
<
<=
>
>=
```

---

### Arithmetic

```nova
print(5 + 10);
```

Variables can also be used:

```nova
set int x = 5;
print(x + 10);
```

---

### Strings

```nova
print("Hello, NOVA!");
```

Escape sequences are supported:

```text
\n
\t
\r
\\
\"
```

---

### String + Integer

NOVA can combine a string and an integer in `print()`:

```nova
set int x = 42;

print("The answer is:" + x);
```

Output:

```text
The answer is:42
```

---

## Compiler Architecture

NOVA is divided into several stages.

### Lexer

The lexer converts source code into tokens.

```text
Source
  ↓
Lexer
  ↓
Tokens
```

For example:

```nova
set int x = 42;
```

becomes approximately:

```text
SET
INT
IDENTIFIER
EQUAL
NUMBER
SEMICOLON
```

---

### Parser

The parser converts tokens into an Abstract Syntax Tree.

```text
Tokens
  ↓
Parser
  ↓
AST
```

---

### AST

The AST represents the structure of the NOVA program.

For example:

```nova
set int x = 42;
```

becomes approximately:

```text
VARIABLE_DECL
├── x
└── NUMBER
    └── 42
```

---

### Compiler

The compiler converts the AST into C code.

For example:

```nova
print(x);
```

can become:

```c
printf("%d\n", x);
```

The generated C file is then compiled using GCC.

---

## Requirements

To build NOVA from source, you need:

* A C compiler
* GCC
* Windows or another platform capable of running the compiler tools

The current NOVA compiler uses GCC for the final executable generation.

---

## Building NOVA

Compile the compiler sources with GCC.

The resulting executable should be:

```text
nova.exe
```

Then use:

```powershell
.\nova.exe examples\hello.nova -o hello.exe
```

---

## Version

Current version:

```text
Nova 1.1
```

---

## Project

**NOVA**

**Built by Netfloor Software Corporation**

> Simple language. Native compiler pipeline. Built from scratch.
