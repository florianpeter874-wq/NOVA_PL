# NOVA

## Nova 1.1

### the children of the Nova project.

### Built by Netfloor Software Corporation

NOVA is a standalone programming language designed to provide a simple, readable syntax while compiling programs to native C code.

NOVA is **not tied to an operating system** and is designed as an independent programming language and compiler project.

---

## Features

NOVA currently supports:

* `program`
* `function`
* `entryp`
* `set`
* `int`
* `str`
* `file`
* `if / else`
* `while`
* `ret`
* `input()`
* Function parameters
* Function calls
* Integer expressions
* String expressions
* String + integer expressions
* Comparisons
* Logical operators
* Arrays
* File operations
* Automatic C code generation
* Native compilation through GCC

---

# Example

A basic NOVA program:

```nova
entryp Main main

program Main {

    function main() {
        print("Hello, NOVA!");
    }

}
```

Compile it:

```powershell
nova.exe examples\hello.nova -o hello.exe
```

Run it:

```powershell
.\hello.exe
```

Output:

```text
Hello, NOVA!
```

---

# Variables

NOVA supports integer and string variables.

## Integer

```nova
set int x = 42;

print(x);
```

## String

```nova
set str name = "NOVA";

print(name);
```

---

# Input

Input can be read using `input()`.

```nova
function main() {

    set int number = input();

    print(number);

}
```

---

# Arithmetic

NOVA supports arithmetic operators:

```text
+
-
*
/
%
```

Example:

```nova
set int x = 10;
set int y = 5;

print(x + y);
print(x - y);
print(x * y);
print(x / y);
print(x % y);
```

---

# Comparisons

NOVA supports:

```text
==
!=
<
<=
>
>=
```

Example:

```nova
if (x == 42) {
    print("The answer is 42!");
}
```

---

# Logical Operators

NOVA supports:

```text
&&
||
!
```

Example:

```nova
if (x > 10 && x < 100) {
    print("x is between 10 and 100");
}
```

---

# If / Else

```nova
if (x == 42) {

    print("Correct!");

} else {

    print("Wrong!");

}
```

---

# While

```nova
set int x = 0;

while (x < 10) {

    print(x);

    x = x + 1;

}
```

---

# Functions

Functions can be declared using `function`.

```nova
function hello() {

    print("Hello!");

}
```

They can then be called:

```nova
function main() {

    hello();

}
```

---

# Function Parameters

Functions can accept parameters.

```nova
function greet(str name) {

    print(name);

}
```

Call the function:

```nova
greet("NOVA");
```

Multiple parameters are supported:

```nova
function intret add(int a, int b) {

    ret a + b;

}
```

---

# Return Values

NOVA supports return values using `ret`.

## Integer return

```nova
function intret add(int a, int b) {

    ret a + b;

}
```

## String return

```nova
function strret hello() {

    ret "Hello from NOVA!";

}
```

A returned value can be used in an expression:

```nova
print(add(5, 7));
```

---

# Arrays

Arrays can be declared using:

```nova
set int numbers[5];
```

Array elements can be accessed using indexes:

```nova
numbers[0] = 10;
numbers[1] = 20;

print(numbers[0]);
```

---

# File Operations

NOVA provides file operations through the `file` type.

Example:

```nova
function main() {

    set file f = file.open("test.txt");

    file.write(f, "Hello from NOVA!");

    set str content = file.read(f);

    print(content);

    file.close(f);

}
```

The available operations are:

```text
file.open()
file.read()
file.write()
file.close()
```

### `file.open`

Opens or creates a file.

```nova
set file f = file.open("test.txt");
```

### `file.write`

Writes text to a file.

```nova
file.write(f, "Hello from NOVA!");
```

### `file.read`

Reads the contents of a file.

```nova
set str content = file.read(f);
```

### `file.close`

Closes the file.

```nova
file.close(f);
```

---

# Program Structure

A NOVA program uses an entry point and a program declaration.

```nova
entryp Main main

program Main {

    function main() {

        print("Hello, NOVA!");

    }

}
```

The `entryp` declaration specifies the program entry point.

---

# Compiler Architecture

NOVA follows a multi-stage compilation pipeline:

```text
NOVA source
    |
    v
Lexer
    |
    v
Tokens
    |
    v
Parser
    |
    v
AST
    |
    v
Compiler
    |
    v
Generated C
    |
    v
GCC
    |
    v
Native executable
```

In simplified form:

```text
hello.nova
     |
     v
  lexer.c
     |
     v
 parser.c
     |
     v
   ast.c
     |
     v
compiler.c
     |
     v
generated.c
     |
     v
    GCC
     |
     v
 hello.exe
```

---

# Project Structure

```text
NOVA/
│
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
│   ├── build/
│   └── tests/
│
└── README.md
```

---

# Building NOVA

NOVA is written in C.

A C compiler such as GCC is required to build the NOVA compiler.

On Windows with MinGW/MSYS2:

```powershell
gcc src\main.c src\lexer.c src\parser.c src\ast.c src\compiler.c src\error.c -Iinclude -o nova.exe
```

After compilation:

```powershell
.\nova.exe
```

---

# Compiling a NOVA Program

Once `nova.exe` has been built:

```powershell
.\nova.exe examples\hello.nova -o hello.exe
```

Then run:

```powershell
.\hello.exe
```

---

# Windows

The NOVA project is developed and tested on Windows.

The project can be located anywhere, for example:

```text
W:\NOVA
```

A typical setup can look like:

```text
W:\NOVA\
    nova.exe
    README.md
    examples\
    benchmark\
```

The benchmark can use the NOVA compiler from the project directory.

---

# Benchmark

NOVA includes a benchmark system.

The benchmark tests different parts of the compiler and generated programs.

Example benchmark categories include:

* Hello world
* Repeated printing
* Long strings
* Variables
* Integer expressions
* String + integer expressions
* Function calls
* `if / else`
* Comparisons
* Nested logic
* Large programs

The benchmark performs multiple runs and measures:

```text
Compile
Runtime
Total
```

The benchmark identifies itself as:

```text
========================================================================
NOVA 1.1 BENCHMARK
the children of the Nova project.
Built by Netfloor Software Corporation
========================================================================
```

---

# Error Handling

NOVA contains a dedicated error module.

Compiler errors are reported when invalid NOVA syntax or unsupported operations are encountered.

The compiler pipeline is designed to detect errors during lexical analysis, parsing, AST processing, and C compilation.

---

# Design Goals

The main goals of NOVA are:

1. Keep the language readable.
2. Keep the syntax simple.
3. Compile to native code.
4. Make the compiler understandable.
5. Support normal programming features.
6. Remain independent from any specific operating system.
7. Provide a foundation for future versions of NOVA.

---

# NOVA Is Not an OS

NOVA is a **programming language and compiler project**.

It is separate from operating-system projects and is not designed specifically for a particular OS.

The compiler generates C code, which can then be compiled using a native C compiler such as GCC.

---

# Version

Current version:

```text
Nova 1.1
```

Project slogan:

```text
the children of the Nova project.
```

Built by:

```text
Netfloor Software Corporation
```

---

# Roadmap

Possible future NOVA features include:

* More built-in types
* More standard library functionality
* Better error messages
* More advanced arrays
* Additional file operations
* More compiler optimizations
* More benchmark tests
* Improved generated C code
* Cross-platform support
* More control-flow features
* Additional language features

---

# Contributing

NOVA is currently developed as the **Netfloor Software Corporation** project.

The compiler architecture is intentionally divided into separate components so that the language can continue to grow:

```text
Lexer
Parser
AST
Compiler
Error system
```

---

# License

License information will be added in a future release.

---

# Credits

## Netfloor Software Corporation

**Nova 1.1**

*the children of the Nova project.*

**Built by Netfloor Software Corporation.**
