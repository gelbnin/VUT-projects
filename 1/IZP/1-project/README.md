# T9 Contact Search (Projekt 1)

## Description

This project involves creating a C program (`t9search.c`) that emulates a simplified T9-like contact search algorithm found in mobile phones. The program reads a contact list (name and phone number pairs) from standard input and filters it based on a sequence of digits provided as a command-line argument. Each digit (2-9, 0) represents a set of possible characters (e.g., '2' maps to 'a', 'b', 'c'). The program searches for contacts where either the name or the phone number contains an **unbroken sequence** of characters corresponding to the input digit sequence. The search is case-insensitive.

## Prerequisites

* GCC compiler.

## Compilation

```bash
gcc -std=c99 -Wall -Wextra -Werror t9search.c -o t9search
```

## Usage

```bash
$ ./t9search <input.txt
petr dvorak, 603123456
jana novotna, 777987654
bedrich smetana ml., 541141120
$ ./t9search 12 <input.txt
petr dvorak, 603123456
bedrich smetana ml., 541141120
$ ./t9search 686 <input.txt
jana nOVOtna, 777987654
$ ./t9search 38 <input.txt
pETr DVorak, 603123456
bedrich smETana ml., 541141120
$ ./t9search 111 <input.txt
Not found
```

## Evaluation
9/10