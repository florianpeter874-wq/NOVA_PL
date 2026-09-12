#ifndef NOVA_PARSER_H
#define NOVA_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Token *tokens;
    int token_count;
    int current;
} Parser;

ASTNode *parser_parse(Token *tokens, int token_count);

#endif