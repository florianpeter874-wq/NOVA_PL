#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/parser.h"

static Parser parser;

static Token *current_token(void)
{
    if (parser.current >= parser.token_count) {
        return NULL;
    }

    return &parser.tokens[parser.current];
}

static int check(TokenType type)
{
    Token *token;

    token = current_token();

    if (token == NULL) {
        return 0;
    }

    return token->type == type;
}

static Token *advance_token(void)
{
    if (parser.current >= parser.token_count) {
        return NULL;
    }

    parser.current++;

    return &parser.tokens[parser.current - 1];
}

static int consume(TokenType type)
{
    if (!check(type)) {
        return 0;
    }

    advance_token();

    return 1;
}

static ASTNode *parse_statement(void);
static ASTNode *parse_block(void);
static ASTNode *parse_if(void);
static ASTNode *parse_expression(void);
static ASTNode *parse_primary(void);
static ASTNode *parse_function(void);
static ASTNode *parse_entry_point(void);
static ASTNode *parse_program(void);
static ASTNode *parse_call(void);
static ASTNode *parse_variable_decl(void);
static ASTNode *parse_print(void);

static ASTNode *parse_primary(void)
{
    ASTNode *node;
    Token *token;

    token = current_token();

    if (token == NULL) {
        return NULL;
    }

    if (check(TOKEN_NUMBER)) {
        node = ast_create(
            AST_NUMBER,
            token->value
        );

        if (node == NULL) {
            return NULL;
        }

        advance_token();

        return node;
    }

    if (check(TOKEN_STRING)) {
        node = ast_create(
            AST_STRING,
            token->value
        );

        if (node == NULL) {
            return NULL;
        }

        advance_token();

        return node;
    }

    if (check(TOKEN_IDENTIFIER)) {
        node = ast_create(
            AST_VARIABLE_REF,
            token->value
        );

        if (node == NULL) {
            return NULL;
        }

        advance_token();

        return node;
    }

    if (check(TOKEN_LPAREN)) {
        advance_token();

        node = parse_expression();

        if (node == NULL) {
            return NULL;
        }

        if (!consume(TOKEN_RPAREN)) {
            ast_free(node);
            return NULL;
        }

        return node;
    }

    return NULL;
}

static ASTNode *parse_expression(void)
{
    ASTNode *left;
    ASTNode *right;
    ASTNode *binary;

    Token *token;
    TokenType operator_type;

    left = parse_primary();

    if (left == NULL) {
        return NULL;
    }

    token = current_token();

    if (token == NULL) {
        return left;
    }

    operator_type = token->type;

    if (operator_type == TOKEN_PLUS) {
        advance_token();

        right = parse_primary();

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        binary = ast_create(
            AST_BINARY,
            "+"
        );

        if (binary == NULL) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        ast_add_child(
            binary,
            left
        );

        ast_add_child(
            binary,
            right
        );

        return binary;
    }

    if (operator_type == TOKEN_EQUAL_EQUAL ||
        operator_type == TOKEN_NOT_EQUAL ||
        operator_type == TOKEN_LESS ||
        operator_type == TOKEN_LESS_EQUAL ||
        operator_type == TOKEN_GREATER ||
        operator_type == TOKEN_GREATER_EQUAL) {

        advance_token();

        right = parse_primary();

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        switch (operator_type) {
            case TOKEN_EQUAL_EQUAL:
                binary = ast_create(
                    AST_BINARY,
                    "=="
                );
                break;

            case TOKEN_NOT_EQUAL:
                binary = ast_create(
                    AST_BINARY,
                    "!="
                );
                break;

            case TOKEN_LESS:
                binary = ast_create(
                    AST_BINARY,
                    "<"
                );
                break;

            case TOKEN_LESS_EQUAL:
                binary = ast_create(
                    AST_BINARY,
                    "<="
                );
                break;

            case TOKEN_GREATER:
                binary = ast_create(
                    AST_BINARY,
                    ">"
                );
                break;

            case TOKEN_GREATER_EQUAL:
                binary = ast_create(
                    AST_BINARY,
                    ">="
                );
                break;

            default:
                ast_free(left);
                ast_free(right);
                return NULL;
        }

        if (binary == NULL) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        ast_add_child(
            binary,
            left
        );

        ast_add_child(
            binary,
            right
        );

        return binary;
    }

    return left;
}

static ASTNode *parse_print(void)
{
    ASTNode *print_node;
    ASTNode *expression;

    Token *token;

    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    token = current_token();

    if (token->value == NULL ||
        strcmp(token->value, "print") != 0) {

        return NULL;
    }

    advance_token();

    if (!consume(TOKEN_LPAREN)) {
        return NULL;
    }

    expression = parse_expression();

    if (expression == NULL) {
        return NULL;
    }

    if (!consume(TOKEN_RPAREN)) {
        ast_free(expression);
        return NULL;
    }

    if (!consume(TOKEN_SEMICOLON)) {
        ast_free(expression);
        return NULL;
    }

    print_node = ast_create(
        AST_PRINT,
        NULL
    );

    if (print_node == NULL) {
        ast_free(expression);
        return NULL;
    }

    ast_add_child(
        print_node,
        expression
    );

    return print_node;
}

static ASTNode *parse_variable_decl(void)
{
    ASTNode *variable;
    ASTNode *value_node;

    Token *name_token;

    if (!consume(TOKEN_SET)) {
        return NULL;
    }

    if (!consume(TOKEN_INT)) {
        return NULL;
    }

    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    name_token = current_token();

    advance_token();

    if (!consume(TOKEN_EQUAL)) {
        return NULL;
    }

    value_node = parse_expression();

    if (value_node == NULL) {
        return NULL;
    }

    if (!consume(TOKEN_SEMICOLON)) {
        ast_free(value_node);
        return NULL;
    }

    variable = ast_create(
        AST_VARIABLE_DECL,
        name_token->value
    );

    if (variable == NULL) {
        ast_free(value_node);
        return NULL;
    }

    ast_add_child(
        variable,
        value_node
    );

    return variable;
}

static ASTNode *parse_if(void)
{
    ASTNode *if_node;
    ASTNode *condition;
    ASTNode *then_block;
    ASTNode *else_block;

    if (!consume(TOKEN_IF)) {
        return NULL;
    }

    if (!consume(TOKEN_LPAREN)) {
        return NULL;
    }

    condition = parse_expression();

    if (condition == NULL) {
        return NULL;
    }

    if (!consume(TOKEN_RPAREN)) {
        ast_free(condition);
        return NULL;
    }

    then_block = parse_block();

    if (then_block == NULL) {
        ast_free(condition);
        return NULL;
    }

    if_node = ast_create(
        AST_IF,
        NULL
    );

    if (if_node == NULL) {
        ast_free(condition);
        ast_free(then_block);
        return NULL;
    }

    ast_add_child(
        if_node,
        condition
    );

    ast_add_child(
        if_node,
        then_block
    );

    if (check(TOKEN_ELSE)) {
        advance_token();

        else_block = parse_block();

        if (else_block == NULL) {
            ast_free(if_node);
            return NULL;
        }

        ast_add_child(
            if_node,
            else_block
        );
    }

    return if_node;
}

static ASTNode *parse_call(void)
{
    ASTNode *call;

    Token *token;

    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    token = current_token();

    call = ast_create(
        AST_CALL,
        token->value
    );

    if (call == NULL) {
        return NULL;
    }

    advance_token();

    if (!consume(TOKEN_LPAREN)) {
        ast_free(call);
        return NULL;
    }

    if (!consume(TOKEN_RPAREN)) {
        ast_free(call);
        return NULL;
    }

    if (!consume(TOKEN_SEMICOLON)) {
        ast_free(call);
        return NULL;
    }

    return call;
}

static ASTNode *parse_statement(void)
{
    Token *token;

    token = current_token();

    if (token == NULL) {
        return NULL;
    }

    if (check(TOKEN_SET)) {
        return parse_variable_decl();
    }

    if (check(TOKEN_IF)) {
        return parse_if();
    }

    if (check(TOKEN_IDENTIFIER)) {
        if (token->value != NULL &&
            strcmp(token->value, "print") == 0) {

            return parse_print();
        }

        return parse_call();
    }

    return NULL;
}

static ASTNode *parse_block(void)
{
    ASTNode *block;
    ASTNode *statement;

    if (!consume(TOKEN_LBRACE)) {
        return NULL;
    }

    block = ast_create(
        AST_BLOCK,
        NULL
    );

    if (block == NULL) {
        return NULL;
    }

    while (!check(TOKEN_RBRACE) &&
           !check(TOKEN_EOF)) {

        statement = parse_statement();

        if (statement == NULL) {
            ast_free(block);
            return NULL;
        }

        ast_add_child(
            block,
            statement
        );
    }

    if (!consume(TOKEN_RBRACE)) {
        ast_free(block);
        return NULL;
    }

    return block;
}

static ASTNode *parse_function(void)
{
    ASTNode *function;
    ASTNode *block;

    Token *token;

    if (!consume(TOKEN_FUNCTION)) {
        return NULL;
    }

    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    token = current_token();

    function = ast_create(
        AST_FUNCTION,
        token->value
    );

    if (function == NULL) {
        return NULL;
    }

    advance_token();

    if (!consume(TOKEN_LPAREN)) {
        ast_free(function);
        return NULL;
    }

    if (!consume(TOKEN_RPAREN)) {
        ast_free(function);
        return NULL;
    }

    block = parse_block();

    if (block == NULL) {
        ast_free(function);
        return NULL;
    }

    ast_add_child(
        function,
        block
    );

    return function;
}

static ASTNode *parse_entry_point(void)
{
    ASTNode *entry;
    ASTNode *function_name_node;

    Token *program_name;
    Token *function_name;

    if (!consume(TOKEN_ENTRYP)) {
        return NULL;
    }

    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    program_name = current_token();

    advance_token();

    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    function_name = current_token();

    advance_token();

    entry = ast_create(
        AST_ENTRY_POINT,
        program_name->value
    );

    if (entry == NULL) {
        return NULL;
    }

    function_name_node = ast_create(
        AST_STRING,
        function_name->value
    );

    if (function_name_node == NULL) {
        ast_free(entry);
        return NULL;
    }

    ast_add_child(
        entry,
        function_name_node
    );

    return entry;
}

static ASTNode *parse_program(void)
{
    ASTNode *program;
    ASTNode *entry;
    ASTNode *function;

    Token *token;

    entry = parse_entry_point();

    if (entry == NULL) {
        return NULL;
    }

    if (!consume(TOKEN_PROGRAM)) {
        ast_free(entry);
        return NULL;
    }

    if (!check(TOKEN_IDENTIFIER)) {
        ast_free(entry);
        return NULL;
    }

    token = current_token();

    program = ast_create(
        AST_PROGRAM,
        token->value
    );

    if (program == NULL) {
        ast_free(entry);
        return NULL;
    }

    advance_token();

    ast_add_child(
        program,
        entry
    );

    if (!consume(TOKEN_LBRACE)) {
        ast_free(program);
        return NULL;
    }

    while (!check(TOKEN_RBRACE) &&
           !check(TOKEN_EOF)) {

        if (!check(TOKEN_FUNCTION)) {
            ast_free(program);
            return NULL;
        }

        function = parse_function();

        if (function == NULL) {
            ast_free(program);
            return NULL;
        }

        ast_add_child(
            program,
            function
        );
    }

    if (!consume(TOKEN_RBRACE)) {
        ast_free(program);
        return NULL;
    }

    if (!check(TOKEN_EOF)) {
        ast_free(program);
        return NULL;
    }

    return program;
}

ASTNode *parser_parse(
    Token *tokens,
    int token_count
)
{
    ASTNode *root;

    if (tokens == NULL ||
        token_count <= 0) {

        return NULL;
    }

    parser.tokens = tokens;
    parser.token_count = token_count;
    parser.current = 0;

    root = parse_program();

    return root;
}
