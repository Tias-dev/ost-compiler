# ost-compiler
Compiler of OST high-level programing language of Turing's machine algorithmic model

## Events Storm
## Token

Token:
- id
- begin
- end
- type

TokenTypes:
- name
- keyword
- operator

Token Invariants:
1. all tokens separated by spaces(space, newline, tab and etc) and operators
2. keyword can be part of name
3. operators can't be part of name or keyword

## AST
Ast Component:
- id
- have more than zero tokens
- have zero or more other components

AST invariants:
1. When creating component consume all tokens it needed if success, consume nothing on failure
2. On failure trowed exceptions with description the place if failed, expected and given tokens

Compiling invariants
1. Each MT begin with 0 state
2. Max used state is exit state i.e. there are mustn't be situations where max state placed as begin state to any commands in MT
