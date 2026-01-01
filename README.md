# Monty

Monty is an ahead-of-time (AOT) compiler and experimental functional programming language built in modern C++ on top of LLVM. It focuses on expression-oriented semantics, user-defined operators, and interop with C/C++ via the C ABI.

## Table of Contents
1. [Highlights](#highlights)
2. [Language Features](#language-features)
3. [Roadmap](#roadmap)
4. [Code Examples](#code-examples)
5. [Building & Running](#building--running)
6. [Using Monty with C/C++](#using-monty-with-cc)
7. [Optimization Passes](#optimization-passes)
8. [Project Goals & Philosophy](#project-goals--philosophy)
9. [Acknowledgements](#acknowledgements)

## Highlights
- **Native AOT compilation** to standalone executables.
- **Object file emission** for seamless linking into C/C++ (C ABI).
- **Expression-only language** with first-class function definitions.
- **Custom operators** (binary/unary) with precedence control.
- **Interop via `using`** to call external (e.g., C) functions.
- **Built-in optimizations**, including tail-call optimization.

## Language Features
- Expressions only (no statements).
- `fn` function definitions.
- `if … then … else` expressions.
- `let … in …` expressions.
- Extern bindings with `using` to call out to C/C++ symbols.
- User-defined unary and binary operators with precedences.

## Roadmap
- Semantic analysis and a Hindley–Milner type system
- More base types (currently only `double`)
- Lambda expressions and first-class functions
- Lists and collection primitives
- Functional utilities: `map`, `fold`, `filter`, etc.

## Code Examples

### Logical unary not
```monty
fn unary!(v)
  if v then
    0
  else
    1;
```

### Unary negate
```monty
fn unary-(v)
  0 - v;
```

### Define `>` with the same precedence as `<`
```monty
fn binary> 10 (LHS RHS)
  RHS < LHS;
```

### Binary logical or (non–short-circuiting)
```monty
fn binary| 5 (LHS RHS)
  if LHS then
    1
  else if RHS then
    1
  else
    0;
```

### Binary logical and (non–short-circuiting)
```monty
fn binary& 6 (LHS RHS)
  if !LHS then
    0
  else
    !!RHS;
```

### Define `=` with slightly lower precedence than relationals
```monty
fn binary = 9 (LHS RHS)
  !(LHS < RHS | LHS > RHS);
```

### Define `:` for sequencing (returns RHS)
```monty
fn binary : 1 (x y) y;
```

### Simple function with `let … in …`
```monty
fn foo(x) let y = 2, z = 2 in x + y + z;
```

## Building & Running
> Prerequisites: A recent LLVM toolchain and a C++17 (or later) compiler.

Typical workflow (adjust paths/targets as needed):
```bash
# Configure
cmake -S . -B build -DLLVM_DIR=/path/to/llvm

# Build
cmake --build build --config Release

# Compile a Monty source file to a native executable
./build/monty examples/hello.my -o hello

# Run
./build/hello
```

## Using Monty with C/C++
Monty can emit object files that link cleanly with C/C++ via the C ABI:
```bash
# Emit object file
./build/montyc src/module.my -o module.o

# Link with a C++ application
c++ main.cpp module.o -o app $(llvm-config --ldflags --libs)
```
Inside Monty, declare external symbols with `using`:
```monty
using printd(x);

fn main()
  printd(42);
```

## Optimization Passes
- Tail-call optimization (TCO)
- Multiple LLVM-backed passes for code quality and performance

## Project Goals & Philosophy
Monty explores a compact, expression-oriented functional core with strong native-code generation. Interop with C/C++ keeps Monty practical for systems work while LLVM provides a mature backend for optimization and portability.

## Acknowledgements
The initial version of Monty and its compiler began as an extension of the well-known LLVM **Kaleidoscope** tutorial and has evolved from there.
