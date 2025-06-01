%require "3.2"

%{
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>

#include "ast.hh"

int yylex();
int yyparse();

void yyerror(const char *s);

mlang::expr_list *prog;

%}

// token type definition
%union {
	int64_t int_val;
	char *str_val;
    std::vector<const mlang::expression *> *expr_list_t;
    const mlang::expression *expr_t;
    const mlang::statement *stmt_t;
    const mlang::type *value_t;
    mlang::builtin_binop_kind binop_t;
}

// constant tokens
%token ASSIGNMENT
%token BGN END COMMA IF ELSE UNTIL
%token LET
%token EQUAL NOTEQUAL GREATER GREATEREQUAL LESS LESSEQUAL PLUS MINUS MULTIPLY DIVIDE
%token UNIT
%token PRINT TOSTR TOINT ARG_PASS

// terminal symbols
%token <int_val> INT
%token <str_val> STRING
%token <str_val> IDENT

// non-terminal symbols
%type <stmt_t> stmt
%type <expr_list_t> exprs args
%type <expr_t> expr function_call
%type <value_t> value
%type <binop_t> builtin_binop

%start program

%%

program
	: exprs                           { prog = new mlang::expr_list($1); }
;

exprs
    : /* empty */                     { $$ = new std::vector<const mlang::expression *>(0); }
	| exprs expr                      { $1->push_back($2); }
;

expr
    : value                           { $$ = $1; }
    | function_call 
    | IDENT                           { $$ = new mlang::variable($1); free($1); }
    | BGN exprs END                   { $$ = new mlang::expr_list($2); }
    | LET IDENT ASSIGNMENT expr       { $$ = new mlang::var_decl($2, $4); free($2); }
    | IDENT ASSIGNMENT expr           { $$ = new mlang::essignment_expression($1, $3); free($1); }
    | IF expr expr ELSE expr          { $$ = new mlang::if_expression($2, $3, $5); }
    | IF expr expr                    { $$ = new mlang::if_expression($2, $3, nullptr); }                  
    | stmt
;

function_call
    : PRINT ARG_PASS args                      { $$ = new mlang::print_function($3); }
    | TOSTR ARG_PASS expr                      { $$ = new mlang::tostr_function($3); }
    | TOINT ARG_PASS expr                      { $$ = new mlang::toint_function($3); }
    | IDENT ARG_PASS args                      { $$ = new mlang::function_call($1, $3); }
    | builtin_binop ARG_PASS expr COMMA expr   { $$ = new mlang::builtin_binop_function($1, $3, $5); }
;

stmt
    : UNTIL expr expr                 { $$ = new mlang::until_statement($2, $3); }
;

args
    : expr                            { $$ = new std::vector<const mlang::expression *>(1, $1); }
    | args COMMA expr                 { $1->push_back($3); }
;

value
    : INT                     { $$ = new mlang::integer_type($1); }
    | STRING                  { $$ = new mlang::string_type($1); free($1); }
    | UNIT                    { $$ = &mlang::UNIT__; }
;

builtin_binop
    : EQUAL                   { $$ = mlang::builtin_binop_kind::EQUAL; }
    | NOTEQUAL                { $$ = mlang::builtin_binop_kind::NOTEQUAL; }
    | GREATER                 { $$ = mlang::builtin_binop_kind::GREATER; }
    | GREATEREQUAL            { $$ = mlang::builtin_binop_kind::GREATEREQUAL; }
    | LESS                    { $$ = mlang::builtin_binop_kind::LESS; }
    | LESSEQUAL               { $$ = mlang::builtin_binop_kind::LESSEQUAL; }
    | PLUS                    { $$ = mlang::builtin_binop_kind::PLUS; }
    | MINUS                   { $$ = mlang::builtin_binop_kind::MINUS; }
    | MULTIPLY                { $$ = mlang::builtin_binop_kind::MULTIPLY; }
    | DIVIDE                  { $$ = mlang::builtin_binop_kind::DIVIDE; }
;

%%

int main(int argc, char **argv){
    yyparse();

    mlang::context ctx{};

    auto *ret = prog->value(ctx);

    int status = ret->to_integer_type().data__();

    delete prog;
	return status;
}

void yyerror(const char *s) {
	std::cout << "Error: " << s << std::endl;
}
