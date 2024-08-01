grammar expression;

options {
    language = C;
    output = AST;
}

tokens {
    TOK_INEQ;
    TOK_EXPR;
    TOK_TERM;
    TOK_VAR;
    TOK_ARRAY;
    TOK_FUNC;
    TOK_AGG;
    TOK_FILT;
}

@lexer::postinclude {
    #define _empty NULL
}

start
    : (inequality|expression) EOF!
    ;

inequality
    : left_bound? '[' expression ']' right_bound?
    -> ^(TOK_INEQ left_bound? right_bound? expression)
    ;

expression
    : add_expression
    -> ^(TOK_EXPR add_expression)
    ;

add_expression
    : mul_expression ((TOK_PLUS | TOK_MINUS)^ mul_expression)*
    ;

mul_expression
    : unr_expression ((TOK_MUL | TOK_DIV)^ unr_expression)*
    ;

unr_expression
    : (TOK_PLUS|TOK_MINUS)? term
    -> ^(TOK_TERM TOK_PLUS? TOK_MINUS? term)
    ;

term
    : literal
    | variable
    | frag
    | detail
    | function_call
    | aggregate_call
    | filter_call
    | '(' expression ')' -> expression
    ;

left_bound
    : numeric_literal TOK_LEQ -> ^(TOK_GEQ numeric_literal)
    | numeric_literal TOK_GEQ -> ^(TOK_LEQ numeric_literal)
    | numeric_literal TOK_EQ -> ^(TOK_EQ numeric_literal)
    ;

right_bound
    : TOK_LEQ numeric_literal -> ^(TOK_LEQ numeric_literal)
    | TOK_GEQ numeric_literal -> ^(TOK_GEQ numeric_literal)
    | TOK_EQ numeric_literal -> ^(TOK_EQ numeric_literal)
    ;

identifier
    : TOK_ID
    ;

literal
    : integer_literal
    | float_literal
    | string_literal
    ;

numeric_literal
    : integer_literal
    | float_literal
    ;

integer_literal
    : TOK_INTEGER
    ;

float_literal
    : TOK_FLOATING
    ;

string_literal
    : TOK_STRING
    ;

variable
    : identifier array_element?
    -> ^(TOK_VAR identifier array_element?)
    ;

frag
    : TOK_FRAG (identifier|'(' string_literal ')')
    -> ^(TOK_FRAG identifier? string_literal?)
    ;

detail
    : TOK_DETAIL (identifier|'(' string_literal ')') array_element?
    -> ^(TOK_DETAIL identifier? string_literal? array_element?)
    ;

array_element
    : '[' (identifier|integer_literal|string_literal) ']'
    -> ^(TOK_ARRAY identifier? integer_literal? string_literal?)
    ;

function_call
    : identifier '(' expression (',' expression)* ')'
    -> ^(TOK_FUNC identifier expression+)
    ;

aggregate_call
    : identifier '[' identifier 'in' (identifier|detail) ']' '{' expression '}'
    -> ^(TOK_AGG identifier identifier identifier? detail? expression)
    ;

filter_call
    : 'filter' '[' detail ']' '{' expression '}'
    -> ^(TOK_FILT detail expression)
    ;


TOK_PLUS : '+';
TOK_MINUS : '-';
TOK_MUL : '*';
TOK_DIV : '/';
TOK_LEQ : '<=';
TOK_GEQ : '>=';
TOK_EQ : '=';
TOK_FRAG : '@';
TOK_DETAIL : '$';

TOK_ID : ('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*;

TOK_INTEGER : '-'? '0'..'9'+;
TOK_FLOATING : '-'? ('0'..'9')+ '.' ('0'..'9')* FRAG_EXPONENT? | '-'? ('0'..'9')+ FRAG_EXPONENT;
TOK_STRING : '"' (~'"')* '"' | '\'' (~'\'')* '\'';

TOK_WS : ( ' ' | '\t' | '\r' | '\n' ) {$channel=HIDDEN;};

fragment FRAG_EXPONENT : ('e'|'E') ('+'|'-')? ('0'..'9')+;
