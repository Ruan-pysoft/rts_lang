Because I'm having difficulty with the code atm,
I'm going to write a description of what I'm working towards.

# The basic idea

The idea is to implement type checking on a stack-based program,
the syntax of which should be much easier to parse than a procedural one.

The idea is that each command would modify the stack in some fixed way,
from a typing perspective.
Eg. an input function might return "hi" or "bye" or "asdf",
but it will always return a string,
and so is deterministic from a typing perspective.

This means that the type checking can be implemented
as a stack-based language of sorts itself.
I believe that it should always be guaranteed to terminate
(since it is entirely deterministic and doesn't contain loops(?)).

Some operations (such as drop or dup) would require generics,
and the output cannot contain a generic not present in the input.

Some operations (such as addition) would perhaps need rust-like traits
(ie. generics with restrictions on the input types).

Note: I have seen Lobster's approach to things,
and perhaps that sort of type specialisation
could work better than traits?

When "applying" an operation to the stack of types,
the generics can be then resolved based on the current contents of the stack.

Theoretically, having a fully typed language will make optimisations easier,
by having separate stacks for each type,
allowing a stack of mixed types without keeping track of types at runtime
(ie. a mix of different sized values on the stack),
or by compiling it (whether to assembly, c, ir, or direct machine code).

# For now

For now I'm only going to implement basic types needed for functionality:
an int and a bool type, "transform" types (block/function signatures),
and catch-all generics.

Structured types, arrays, restricted generics, tuples, other numeric types,
strings, etc. can all be implemented later.

# Implementation

## Parsing

When tokenising the language,
the tokens consist of whitespace-separated strings.

There will be five single-character tokens:
`[` `]` `(` `)` `$`

The ast will then consist of symbols, literals, and generics.

A literal could be numeric (0, 1, -5),
it could be a string ("hello, world"),
a list (`(1 2 "this is a list")`),
or a block (`[1 2 "this is a block"]`).
There can also be *symbol literals*,
which is any symbol preceded by `$`.
This will then push a symbol to the stack,
rather than fetching the symbol's associated value from the environment
and executing that.

Generics would be any string starting with a single quote,
like `'a`, `'b`, `'0`, `'`, etc.

And symbols would be any other string.

Note that unbalanced parentheses, brackets, or quotes
should result in a parse error.

Eventually, the ast could be represented
by a list of values of the language itself.
This would allow similar levels of manipulation as in lisps.

## Type checking and execution

In the type checking phase,
the type interpreter will operate on
the ast, a type stack, and a type environment.

The type stack will mirror the eventual runtime stack,
but storing the types of the eventual values
rather than the values themselves.

The type environment will function similarly,
associating names (symbols) with types,
rather than values.

One consequence of this,
is that at any given time during execution,
the types of the stack and the types of values in the environment are fixed.

Each type has some effect on the actual stack when ran,
as well as a corresponding effect on the type stack.
The type checking phase then just runs the type effects
and verifies that no errors occur.

Furthermore, type values should retain their value
not just on the value (runtime) stack,
but also on the type (compiletime) stack.
Similarly, symbols are also compiletime-determined,
otherwise the environment and fetches cannot be type checked.

As such, the following program:

`1 true swp 2 + (bool int) :`

Would have the following effects on the type stack when executed:

`int` -> `int bool` -> `bool int` -> `bool int int` -> `bool int`
-> `bool int (type#bool type#int)` -> `bool int`

And the following effects on the value stack:

`1` -> `1 true` -> `true 1` -> `true 1 2` -> `true 3` -> `true 3 (bool int)`
-> `true 3`

As an optimisation,
later one there could be checks implemented
that types that are added to and then immediately popped from the stack
are not present during runtime.

### The syntax of type checking

By default, the environment contains some type names:
`bool`, `int`, etc.

The environment also contains some type operations:
`->`, `:`.

The `->` operation will work on two types or lists of types,
creating a "transform" type,
which is the type of blocks
(when a block is executed,
it takes a certain top-of-stack,
and *transforms* it to another).

The `:` type will type check:
It takes a single type or a list of types from the top of stack,
and then matches it to the top of the stack,
resulting in an error when there is no match.

### Generics

Generics should be matched from the top of the stack downwards,
such that matching `('a 'a)` to a stack top of `int bool`
would error on the `int`, rather than the `bool`.

Later, when variadic generics are introduced,
this will allow in-code specification of types such as `if-else`:
`'A ('A -> 'B bool) ('B -> 'C) ('B -> 'C) -> 'C`

Some similar operations:
 - `if`: `'A ('A -> 'B bool) ('B -> 'A) -> 'A`
 - `while`: `'A ('A -> 'B bool) ('B -> 'A) -> 'A`

(Of course, the actual syntax of specifying, say, the type of `if-else`,
would be something like `('A 'A ('B bool) -> 'B 'C -> 'B 'C ->) 'C ->`)
