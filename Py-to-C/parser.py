"""
Scanner and Parser for Python-Subset Language
Using Lark

Supports:
- variables and assignments
- arithmetic expressions
- comparison
- booleans
- strings
- control flow
- functions
- comments
"""


import os
import sys
import json

from lark import Lark, Transformer, v_args
from lark.indenter import Indenter
from dataclasses import asdict, dataclass


# ==============================================================================
# INDENTATION HANDLING
# ==============================================================================

# Need to update OPEN_PAREN_types and CLOSE_PAREN_types to handle multi-line
# expressions later
class PythonIndenter(Indenter):
    NL_type = '_NL'
    OPEN_PAREN_types = []
    CLOSE_PAREN_types = []
    INDENT_type = '_INDENT'
    DEDENT_type = '_DEDENT'
    tab_len = 4


# ==============================================================================
# EBNF GRAMMAR DEFINITION
# ==============================================================================

grammar = r"""
?start: _NL* statement+

?statement: simple_statement _NL? | compound_statement

simple_statement: assignment | return_statement | expr

compound_statement: if_statement | while_statement | for_statement | func_def

assignment: NAME "=" expr

return_statement: "return" expr

if_statement: "if" expr ":" suite
            | "if" expr ":" suite "else" ":" suite

while_statement: "while" expr ":" suite

for_statement: "for" NAME "in" "range" "(" expr ")" ":" suite

func_def: "def" NAME "(" params? ")" ":" suite

suite: _NL _INDENT statement+ _DEDENT

params: NAME ("," NAME)*

?expr: or_expr
             
?or_expr: or_expr "or" and_expr   -> or_
        | and_expr

?and_expr: and_expr "and" not_expr -> and_
         | not_expr

?not_expr: "not" not_expr         -> not_
         | cmp_expr

?cmp_expr: add_expr COMP_OP add_expr -> compare
         | add_expr

?add_expr: add_expr "+" mul_expr -> add
         | add_expr "-" mul_expr -> sub
         | mul_expr

?mul_expr: mul_expr "*" unary_expr -> mul
         | mul_expr "/" unary_expr -> div
         | unary_expr

?unary_expr: "-" unary_expr       -> neg
         | factor

?factor: NUMBER -> number
       | STRING -> string
       | BOOL -> boolean
       | NAME -> var
       | func_call
       | "(" expr ")"

func_call: NAME "(" args? ")"

args: expr ("," expr)*
         
BOOL.2: "True" | "False"    # To give BOOL higher precedence than NAME

COMP_OP: "==" | "!=" | "<" | ">" | "<=" | ">="

%import common.INT
%import common.FLOAT
NUMBER: FLOAT | INT

STRING: /"([^"\\\n]|\\.)*"/

NAME: /[a-zA-Z_][a-zA-Z0-9_]*/

COMMENT: /#[^\n]*/

%import common.WS_INLINE
%ignore WS_INLINE

%declare _INDENT _DEDENT
_NL: /(\r?\n[\t ]*)+/

%ignore COMMENT
"""


# ==============================================================================
# AST NODES
# ==============================================================================

@dataclass
class Program:
    type: str
    statements: list

@dataclass
class If:
    type: str
    cond: object
    then_body: list
    else_body: list | None

@dataclass
class While:
    type: str
    cond: object
    body: list

@dataclass
class For:
    type: str
    var: str
    stop: object
    body: list

@dataclass
class Assignment:
    type: str
    name: str
    expr: object

@dataclass
class Return:
    type: str
    expr: object

@dataclass
class BinOp:
    type: str
    op: str
    left: object
    right: object

@dataclass
class UnaryOp:
    type: str
    op: str
    operand: object

@dataclass
class Compare:
    type: str
    op: str
    left: object
    right: object

@dataclass
class Var:
    type: str
    name: str

@dataclass
class Literal:
    type: str
    value: object

@dataclass
class FunctionDef:
    type: str
    name: str
    params: list
    body: list

@dataclass
class FuncCall:
    type: str
    name: str
    args: list


# ==============================================================================
# TRANSFORMER
# ==============================================================================

@v_args(inline=True)
class ASTBuilder(Transformer):
    def start(self, *statements):
        return Program("Program", list(statements))

    def statement(self, stmt):
        return stmt

    def simple_statement(self, stmt):
        return stmt

    def compound_statement(self, stmt):
        return stmt

    def suite(self, *statements):
        return list(statements)

    def if_statement(self, cond, then_suite, else_suite=None):
        return If("If", cond, then_suite, else_suite)

    def while_statement(self, cond, body):
        return While("While", cond, body)

    def for_statement(self, name, stop, body):
        return For("For", str(name), stop, body)

    def assignment(self, name, expr):
        return Assignment("Assignment", str(name), expr)

    def return_statement(self, expr):
        return Return("Return", expr)

    def var(self, name):
        return Var("Var", str(name))

    def number(self, token):
        s = str(token)
        if "." in s:
            return Literal("Literal", float(s))
        else:
            return Literal("Literal", int(s))

    def string(self, token):
        return Literal("Literal", str(token)[1:-1])  # Remove quotes

    def boolean(self, token):
        return Literal("Literal", token == "True")

    def add(self, a, b):
        return BinOp("BinOp", "+", a, b)

    def sub(self, a, b):
        return BinOp("BinOp", "-", a, b)

    def mul(self, a, b):
        return BinOp("BinOp", "*", a, b)

    def div(self, a, b):
        return BinOp("BinOp", "/", a, b)

    def compare(self, a, op, b):
        return Compare("Compare", str(op), a, b)

    def or_(self, a, b):
        return BinOp("BinOp", "or", a, b)

    def and_(self, a, b):
        return BinOp("BinOp", "and", a, b)
    
    def not_(self, value):
        return UnaryOp("UnaryOp", "not", value)

    def neg(self, value):
        return UnaryOp("UnaryOp", "-", value)

    def func_def(self, name, *args):
        if len(args) == 1:
            # No params, just body
            return FunctionDef("FunctionDef", str(name), [], args[0])
        else:
            # Has params and body
            return FunctionDef("FunctionDef", str(name), args[0], args[1])

    def params(self, *names):
        return [str(n) for n in names]

    def func_call(self, name, args=None):
        return FuncCall("FuncCall", str(name), args or [])

    def args(self, *args):
        return list(args)


# ==============================================================================
# PARSER
# ==============================================================================

parser = Lark(grammar, parser="lalr", postlex=PythonIndenter())

def parse(file_path, show_tree=False):
    with open(file_path, "r") as f:
        code = f.read()

    tree = parser.parse(code)

    if show_tree:
        print("\n" + "="*70)
        print("PARSE TREE")
        print("="*70)
        print(tree.pretty())
        print("="*70 + "\n")

    ast = ASTBuilder().transform(tree)
    return ast

def main():

    if len(sys.argv) < 2:
        print("Usage: python parser.py <sourcefile> [--tree]")
        print("  --tree: Show the parse tree before transformation")
        sys.exit(1)

    src = sys.argv[1]
    show_tree = "--tree" in sys.argv or "-t" in sys.argv

    ast = parse(src, show_tree=show_tree)

    os.makedirs("asts", exist_ok=True)
    out_file = os.path.join("asts", os.path.basename(src) + ".ast.json")
    
    with open(out_file, "w") as f:
        json.dump(asdict(ast), f, indent=2)
    
    print(f"AST written to {out_file}")


if __name__ == "__main__":
    main()