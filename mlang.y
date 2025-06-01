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
	uint8_t int_val;
	char *str_val;
    std::vector<mlang::expression *> *expr_list_t;
    std::vector<std::string> *ident_list_t;
    mlang::expression *expr_t;
    mlang::statement *stmt_t;
    mlang::type *value_t;
    mlang::comp_kind comp_t;
}

// constant tokens
%token ASSIGNMENT
%token BGN END COMMA IF ELSE UNTIL
%token LET
%token EQUAL NOTEQUAL GREATER GREATEREQUAL LESS LESSEQUAL
%token UNIT
%token PRINT TOSTR TOINT LARROW SEMICOL

// terminal symbols
%token <int_val> INT
%token <str_val> STRING
%token <str_val> IDENT

// non-terminal symbols
%type <stmt_t> stmt
%type <expr_list_t> exprs args
%type <ident_list_t> args_decl
%type <expr_t> expr function_call
%type <value_t> value

%start program

%%

program
	: exprs                           { prog = new mlang::expr_list($1); }
;

exprs
    : /* empty */                     { $$ = new std::vector<mlang::expression *>(0); }
	| exprs expr                      { $1->push_back($2); }
;

expr
    : value                           { $$ = $1; }
    | function_call
    | IDENT                           { $$ = new mlang::function_call($1, new std::vector<mlang::expression *>(0)); free($1); }
    | BGN exprs END                   { $$ = new mlang::expr_list($2); }
    | LET IDENT ASSIGNMENT expr       { $$ = new mlang::function_decl($2, $4); free($2); }
    | IF expr expr ELSE expr          { $$ = new mlang::if_expression($2, $3, $5); }
    | IF expr expr                    { $$ = new mlang::if_expression($2, $3, nullptr); }                   
    | stmt                            { $$ = $1; }
    | SEMICOL args_decl SEMICOL expr  { $4->set_args_names($2); $$ = $4; std::cout << "expr : " << $4 << " args : " << $2 << std::endl; }
;

function_call
    : PRINT LARROW args                      { $$ = new mlang::print_function($3); }
    | TOSTR LARROW expr                      { $$ = new mlang::tostr_function($3); }
    | TOINT LARROW expr                      { $$ = new mlang::toint_function($3); }
    | IDENT LARROW args                      { $$ = new mlang::function_call($1, $3); free($1); }
    | expr  LARROW args                      { $$ = new mlang::function_call($1, $3); free($1); }
;

stmt
    : UNTIL expr expr                        { $$ = new mlang::until_statement($2, $3); }
;

args_decl
    : IDENT                            { $$ = new std::vector<std::string>(1, $1); }
    | args_decl COMMA IDENT            { $1->push_back($3); }
;

args
    : expr                            { $$ = new std::vector<mlang::expression *>(1, $1); }
    | args COMMA expr                 { $1->push_back($3); }
;

value
    : INT                             { $$ = new mlang::integer_type($1); }
    | STRING                          { $$ = new mlang::string_type($1); free($1); }
    | UNIT                            { $$ = (mlang::type *)&mlang::UNIT__; }
;

// builtin_binop
//     : EQUAL                   { $$ = mlang::comp_kind::EQUAL; }
//     | NOTEQUAL                { $$ = mlang::comp_kind::NOTEQUAL; }
//     | GREATER                 { $$ = mlang::comp_kind::GREATER; }
//     | GREATEREQUAL            { $$ = mlang::comp_kind::GREATEREQUAL; }
//     | LESS                    { $$ = mlang::comp_kind::LESS; }
//     | LESSEQUAL               { $$ = mlang::comp_kind::LESSEQUAL; }
// ;

%%

int main(int argc, char **argv){
    yyparse();

    mlang::context ctx{};

    auto *ret = prog->value(ctx);

    int status = ret->as_type()->to_integer_type().data__();

    delete prog;
	return status;
}

void yyerror(const char *s) {
	std::cout << "Error: " << s << std::endl;
}
