%{

#include <cstdio>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cstring>

#include "ast.hh"

#include "parser.tab.hh"

%}

ALPHA    [A-Za-z]
DIGIT    [0-9]

%%
"//".*          ; /* ignore single-line comment */
[ \t\n]	        ;
{DIGIT}+		{ yylval.int_val = atoi(yytext); return INT; }
\"[^\"\n]*\"	{	// remove the quote marks from either end
					yylval.str_val = (char *)malloc(sizeof(char)*strlen(yytext)-1);
					strncpy(yylval.str_val, yytext+1, strlen(yytext)-2);
					yylval.str_val[strlen(yytext)-2] = '\0';	// terminate the string!
					return STRING;
				}
\<- 			{ return LARROW; }
\|	 			{ return SEMICOL; }
print           { return PRINT; }
toint           { return TOINT; }
tostr           { return TOSTR; }
let				{ return LET; }
\( 				{ return BGN; }
begin 			{ return BGN; }
\)				{ return END; }
end 			{ return END; }
if				{ return IF; }
else			{ return ELSE; }
until 			{ return UNTIL; }

\<\?			{ return LESS; }
\>\?			{ return GREATER; }
\<\=\?			{ return LESSEQUAL; }
\>\=\?			{ return GREATEREQUAL; }
\!\=\?			{ return NOTEQUAL; }
\=\?			{ return EQUAL; }

\=				{ return ASSIGNMENT; }
T               { return UNIT; }
, 				{ return COMMA; }
{ALPHA}({ALPHA}|{DIGIT})*   { yylval.str_val = strdup(yytext); return IDENT; }
.	            { return 0; }
%%

int yywrap() {
    return 1;
}
