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
static ASTNode *parse_while(void);

static ASTNode *parse_expression(void);
static ASTNode *parse_logical_or(void);
static ASTNode *parse_logical_and(void);
static ASTNode *parse_comparison(void);
static ASTNode *parse_addition(void);
static ASTNode *parse_multiplication(void);
static ASTNode *parse_unary(void);
static ASTNode *parse_primary(void);

static ASTNode *parse_function(void);
static ASTNode *parse_entry_point(void);
static ASTNode *parse_program(void);
static ASTNode *parse_call(void);
static ASTNode *parse_variable_decl(void);
static ASTNode *parse_assignment(void);
static ASTNode *parse_print(void);
static ASTNode *parse_return(void);

static ASTNode *create_binary(
    const char *operator,
    ASTNode *left,
    ASTNode *right
)
{
    ASTNode *binary;

    binary = ast_create(
        AST_BINARY,
        operator
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

static ASTNode *parse_call(void)
{
    ASTNode *call;
    ASTNode *argument;

    Token *token;
    Token *member_token;

    char call_name[512];

    /*
     * Normal function:
     *
     * hello(...)
     *
     * Qualified function:
     *
     * file.open(...)
     *
     * Built-in:
     *
     * tostr(...)
     */

    if (
        !check(TOKEN_IDENTIFIER) &&
        !check(TOKEN_FILE) &&
        !check(TOKEN_TOSTR)
    ) {
        return NULL;
    }

    token = current_token();

    /*
     * Built-in tostr(...)
     */

    if (check(TOKEN_TOSTR)) {

        call = ast_create(
            AST_CALL,
            "tostr"
        );

        if (call == NULL) {
            return NULL;
        }

        call->data_type =
            NOVA_TYPE_STR;

        advance_token();
    }

    /*
     * Qualified function:
     *
     * file.open(...)
     * file.read(...)
     * file.write(...)
     * file.close(...)
     */

    else if (
        parser.current + 2 < parser.token_count &&
        parser.tokens[
            parser.current + 1
        ].type == TOKEN_DOT &&
        parser.tokens[
            parser.current + 2
        ].type == TOKEN_IDENTIFIER
    ) {
        member_token = &parser.tokens[
            parser.current + 2
        ];

        if (
            token->value == NULL ||
            member_token->value == NULL
        ) {
            return NULL;
        }

        snprintf(
            call_name,
            sizeof(call_name),
            "%s.%s",
            token->value,
            member_token->value
        );

        call = ast_create(
            AST_CALL,
            call_name
        );

        if (call == NULL) {
            return NULL;
        }

        /*
         * Return types of built-in file functions.
         */

        if (
            strcmp(
                call_name,
                "file.open"
            ) == 0
        ) {
            call->data_type =
                NOVA_TYPE_FILE;
        }
        else if (
            strcmp(
                call_name,
                "file.read"
            ) == 0
        ) {
            call->data_type =
                NOVA_TYPE_STR;
        }
        else if (
            strcmp(
                call_name,
                "file.write"
            ) == 0
        ) {
            call->data_type =
                NOVA_TYPE_VOID;
        }
        else if (
            strcmp(
                call_name,
                "file.close"
            ) == 0
        ) {
            call->data_type =
                NOVA_TYPE_VOID;
        }

        advance_token();
        advance_token();
        advance_token();
    }

    /*
     * Normal function call.
     */

    else {

        if (!check(TOKEN_IDENTIFIER)) {
            return NULL;
        }

        call = ast_create(
            AST_CALL,
            token->value
        );

        if (call == NULL) {
            return NULL;
        }

        advance_token();
    }

    if (!consume(TOKEN_LPAREN)) {
        ast_free(call);
        return NULL;
    }

    /*
     * Arguments:
     *
     * hello("NOVA")
     * add(10, 20)
     * file.write(f, "Hello")
     * tostr(x)
     */

    if (!check(TOKEN_RPAREN)) {

        while (1) {

            argument = parse_expression();

            if (argument == NULL) {
                ast_free(call);
                return NULL;
            }

            ast_add_child(
                call,
                argument
            );

            if (consume(TOKEN_COMMA)) {
                continue;
            }

            break;
        }
    }

    if (!consume(TOKEN_RPAREN)) {
        ast_free(call);
        return NULL;
    }

    /*
     * parse_call() does NOT consume ';'.
     */

    return call;
}

static ASTNode *parse_primary(void)
{
    ASTNode *node;
    ASTNode *index_node;

    Token *token;

    token = current_token();

    if (token == NULL) {
        return NULL;
    }

    /*
     * Number.
     */

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

    /*
     * String.
     */

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

    /*
     * input()
     */

    if (check(TOKEN_INPUT)) {

        advance_token();

        if (!consume(TOKEN_LPAREN)) {
            return NULL;
        }

        if (!consume(TOKEN_RPAREN)) {
            return NULL;
        }

        node = ast_create(
            AST_INPUT,
            NULL
        );

        if (node == NULL) {
            return NULL;
        }

        return node;
    }

    /*
     * Identifier, TOKEN_FILE, or TOKEN_TOSTR.
     */

    if (
        check(TOKEN_IDENTIFIER) ||
        check(TOKEN_FILE) ||
        check(TOKEN_TOSTR)
    ) {

        /*
         * tostr(...)
         */

        if (check(TOKEN_TOSTR)) {
            return parse_call();
        }

        /*
         * Normal function:
         *
         * hello(...)
         */

        if (
            check(TOKEN_IDENTIFIER) &&
            parser.current + 1 < parser.token_count &&
            parser.tokens[
                parser.current + 1
            ].type == TOKEN_LPAREN
        ) {
            return parse_call();
        }

        /*
         * Qualified function:
         *
         * file.open(...)
         * file.read(...)
         * file.write(...)
         * file.close(...)
         */

        if (
            parser.current + 2 < parser.token_count &&
            parser.tokens[
                parser.current + 1
            ].type == TOKEN_DOT &&
            parser.tokens[
                parser.current + 2
            ].type == TOKEN_IDENTIFIER
        ) {
            return parse_call();
        }

        /*
         * TOKEN_FILE by itself cannot
         * be a variable reference.
         */

        if (check(TOKEN_FILE)) {
            return NULL;
        }

        /*
         * Variable reference.
         */

        node = ast_create(
            AST_VARIABLE_REF,
            token->value
        );

        if (node == NULL) {
            return NULL;
        }

        advance_token();

        /*
         * Array access:
         *
         * numbers[0]
         */

        if (consume(TOKEN_LBRACKET)) {

            index_node = parse_expression();

            if (index_node == NULL) {
                ast_free(node);
                return NULL;
            }

            if (!consume(TOKEN_RBRACKET)) {
                ast_free(index_node);
                ast_free(node);
                return NULL;
            }

            ast_add_child(
                node,
                index_node
            );
        }

        return node;
    }

    /*
     * Parenthesized expression.
     */

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

static ASTNode *parse_unary(void)
{
    ASTNode *operand;
    ASTNode *unary;

    if (check(TOKEN_NOT)) {

        advance_token();

        operand = parse_unary();

        if (operand == NULL) {
            return NULL;
        }

        unary = ast_create(
            AST_UNARY,
            "!"
        );

        if (unary == NULL) {
            ast_free(operand);
            return NULL;
        }

        ast_add_child(
            unary,
            operand
        );

        return unary;
    }

    return parse_primary();
}

static ASTNode *parse_multiplication(void)
{
    ASTNode *left;
    ASTNode *right;
    ASTNode *binary;

    const char *operator;

    left = parse_unary();

    if (left == NULL) {
        return NULL;
    }

    while (
        check(TOKEN_STAR) ||
        check(TOKEN_SLASH) ||
        check(TOKEN_PERCENT)
    ) {

        if (check(TOKEN_STAR)) {
            operator = "*";
        }
        else if (check(TOKEN_SLASH)) {
            operator = "/";
        }
        else {
            operator = "%";
        }

        advance_token();

        right = parse_unary();

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        binary = create_binary(
            operator,
            left,
            right
        );

        if (binary == NULL) {
            return NULL;
        }

        left = binary;
    }

    return left;
}

static ASTNode *parse_addition(void)
{
    ASTNode *left;
    ASTNode *right;
    ASTNode *binary;

    const char *operator;

    left = parse_multiplication();

    if (left == NULL) {
        return NULL;
    }

    while (
        check(TOKEN_PLUS) ||
        check(TOKEN_MINUS)
    ) {

        if (check(TOKEN_PLUS)) {
            operator = "+";
        }
        else {
            operator = "-";
        }

        advance_token();

        right = parse_multiplication();

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        binary = create_binary(
            operator,
            left,
            right
        );

        if (binary == NULL) {
            return NULL;
        }

        left = binary;
    }

    return left;
}

static ASTNode *parse_comparison(void)
{
    ASTNode *left;
    ASTNode *right;
    ASTNode *binary;

    const char *operator;

    left = parse_addition();

    if (left == NULL) {
        return NULL;
    }

    while (
        check(TOKEN_EQUAL_EQUAL) ||
        check(TOKEN_NOT_EQUAL) ||
        check(TOKEN_LESS) ||
        check(TOKEN_LESS_EQUAL) ||
        check(TOKEN_GREATER) ||
        check(TOKEN_GREATER_EQUAL)
    ) {

        if (check(TOKEN_EQUAL_EQUAL)) {
            operator = "==";
        }
        else if (check(TOKEN_NOT_EQUAL)) {
            operator = "!=";
        }
        else if (check(TOKEN_LESS)) {
            operator = "<";
        }
        else if (check(TOKEN_LESS_EQUAL)) {
            operator = "<=";
        }
        else if (check(TOKEN_GREATER)) {
            operator = ">";
        }
        else {
            operator = ">=";
        }

        advance_token();

        right = parse_addition();

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        binary = create_binary(
            operator,
            left,
            right
        );

        if (binary == NULL) {
            return NULL;
        }

        left = binary;
    }

    return left;
}

static ASTNode *parse_logical_and(void)
{
    ASTNode *left;
    ASTNode *right;
    ASTNode *binary;

    left = parse_comparison();

    if (left == NULL) {
        return NULL;
    }

    while (check(TOKEN_AND_AND)) {

        advance_token();

        right = parse_comparison();

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        binary = create_binary(
            "&&",
            left,
            right
        );

        if (binary == NULL) {
            return NULL;
        }

        left = binary;
    }

    return left;
}

static ASTNode *parse_logical_or(void)
{
    ASTNode *left;
    ASTNode *right;
    ASTNode *binary;

    left = parse_logical_and();

    if (left == NULL) {
        return NULL;
    }

    while (check(TOKEN_OR_OR)) {

        advance_token();

        right = parse_logical_and();

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        binary = create_binary(
            "||",
            left,
            right
        );

        if (binary == NULL) {
            return NULL;
        }

        left = binary;
    }

    return left;
}

static ASTNode *parse_expression(void)
{
    return parse_logical_or();
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

    if (
        token->value == NULL ||
        strcmp(
            token->value,
            "print"
        ) != 0
    ) {
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

static ASTNode *parse_return(void)
{
    ASTNode *return_node;
    ASTNode *expression;

    if (!consume(TOKEN_RET)) {
        return NULL;
    }

    expression = parse_expression();

    if (expression == NULL) {
        return NULL;
    }

    if (!consume(TOKEN_SEMICOLON)) {
        ast_free(expression);
        return NULL;
    }

    return_node = ast_create(
        AST_RETURN,
        NULL
    );

    if (return_node == NULL) {
        ast_free(expression);
        return NULL;
    }

    ast_add_child(
        return_node,
        expression
    );

    return return_node;
}

static ASTNode *parse_variable_decl(void)
{
    ASTNode *node;
    ASTNode *size_node;
    ASTNode *expression;

    Token *name_token;
    Token *type_token;

    if (!check(TOKEN_SET)) {
        return NULL;
    }

    advance_token();

    type_token = current_token();

    if (
        type_token == NULL ||
        (
            type_token->type != TOKEN_INT &&
            type_token->type != TOKEN_STR &&
            type_token->type != TOKEN_FILE
        )
    ) {
        return NULL;
    }

    advance_token();

    name_token = current_token();

    if (
        name_token == NULL ||
        name_token->type != TOKEN_IDENTIFIER
    ) {
        return NULL;
    }

    advance_token();

    node = ast_create(
        AST_VARIABLE_DECL,
        name_token->value
    );

    if (node == NULL) {
        return NULL;
    }

    if (
        type_token->type ==
        TOKEN_STR
    ) {
        node->data_type =
            NOVA_TYPE_STR;
    }
    else if (
        type_token->type ==
        TOKEN_FILE
    ) {
        node->data_type =
            NOVA_TYPE_FILE;
    }
    else {
        node->data_type =
            NOVA_TYPE_INT;
    }

    /*
     * Array declaration:
     *
     * set int numbers[5];
     */

    if (consume(TOKEN_LBRACKET)) {

        size_node = parse_expression();

        if (size_node == NULL) {
            ast_free(node);
            return NULL;
        }

        if (!consume(TOKEN_RBRACKET)) {
            ast_free(size_node);
            ast_free(node);
            return NULL;
        }

        node->is_array = 1;

        ast_add_child(
            node,
            size_node
        );

        if (!consume(TOKEN_SEMICOLON)) {
            ast_free(node);
            return NULL;
        }

        return node;
    }

    /*
     * Normal variable:
     *
     * set int x = 5;
     */

    if (!consume(TOKEN_EQUAL)) {
        ast_free(node);
        return NULL;
    }

    expression = parse_expression();

    if (expression == NULL) {
        ast_free(node);
        return NULL;
    }

    ast_add_child(
        node,
        expression
    );

    if (!consume(TOKEN_SEMICOLON)) {
        ast_free(node);
        return NULL;
    }

    return node;
}

static ASTNode *parse_assignment(void)
{
    ASTNode *assignment;
    ASTNode *index_node;
    ASTNode *value_node;

    Token *name_token;

    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    name_token = current_token();

    advance_token();

    assignment = ast_create(
        AST_ASSIGNMENT,
        name_token->value
    );

    if (assignment == NULL) {
        return NULL;
    }

    /*
     * Array assignment:
     *
     * numbers[0] = 10;
     */

    if (consume(TOKEN_LBRACKET)) {

        index_node = parse_expression();

        if (index_node == NULL) {
            ast_free(assignment);
            return NULL;
        }

        if (!consume(TOKEN_RBRACKET)) {
            ast_free(index_node);
            ast_free(assignment);
            return NULL;
        }

        ast_add_child(
            assignment,
            index_node
        );
    }

    if (!consume(TOKEN_EQUAL)) {
        ast_free(assignment);
        return NULL;
    }

    value_node = parse_expression();

    if (value_node == NULL) {
        ast_free(assignment);
        return NULL;
    }

    ast_add_child(
        assignment,
        value_node
    );

    if (!consume(TOKEN_SEMICOLON)) {
        ast_free(assignment);
        return NULL;
    }

    return assignment;
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

static ASTNode *parse_while(void)
{
    ASTNode *while_node;
    ASTNode *condition;
    ASTNode *body;

    if (!consume(TOKEN_WHILE)) {
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

    body = parse_block();

    if (body == NULL) {
        ast_free(condition);
        return NULL;
    }

    while_node = ast_create(
        AST_WHILE,
        NULL
    );

    if (while_node == NULL) {
        ast_free(condition);
        ast_free(body);
        return NULL;
    }

    ast_add_child(
        while_node,
        condition
    );

    ast_add_child(
        while_node,
        body
    );

    return while_node;
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

    if (check(TOKEN_RET)) {
        return parse_return();
    }

    if (check(TOKEN_IF)) {
        return parse_if();
    }

    if (check(TOKEN_WHILE)) {
        return parse_while();
    }

    /*
     * Identifier, TOKEN_FILE, or TOKEN_TOSTR.
     *
     * print(...)
     * greet(...)
     * file.write(...)
     * file.close(...)
     * tostr(...)
     */

    if (
        check(TOKEN_IDENTIFIER) ||
        check(TOKEN_FILE) ||
        check(TOKEN_TOSTR)
    ) {

        /*
         * print(...)
         */

        if (
            check(TOKEN_IDENTIFIER) &&
            token->value != NULL &&
            strcmp(
                token->value,
                "print"
            ) == 0
        ) {
            return parse_print();
        }

        /*
         * Assignment.
         *
         * Only identifiers can be variables.
         */

        if (
            check(TOKEN_IDENTIFIER) &&
            parser.current + 1 < parser.token_count &&
            (
                parser.tokens[
                    parser.current + 1
                ].type == TOKEN_EQUAL ||

                parser.tokens[
                    parser.current + 1
                ].type == TOKEN_LBRACKET
            )
        ) {
            return parse_assignment();
        }

        /*
         * Function call.
         */

        {
            ASTNode *call;

            call = parse_call();

            if (call == NULL) {
                return NULL;
            }

            if (!consume(TOKEN_SEMICOLON)) {
                ast_free(call);
                return NULL;
            }

            return call;
        }
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

    while (
        !check(TOKEN_RBRACE) &&
        !check(TOKEN_EOF)
    ) {

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
    ASTNode *parameter;

    Token *name_token;

    if (!consume(TOKEN_FUNCTION)) {
        return NULL;
    }

    /*
     * NOVA function return types:
     *
     * function main()
     * function intret add()
     * function strret hello()
     */

    if (
        check(TOKEN_INTRET) ||
        check(TOKEN_STRRET)
    ) {
        TokenType return_type;

        return_type =
            current_token()->type;

        advance_token();

        if (!check(TOKEN_IDENTIFIER)) {
            return NULL;
        }

        name_token =
            current_token();

        function = ast_create(
            AST_FUNCTION,
            name_token->value
        );

        if (function == NULL) {
            return NULL;
        }

        if (
            return_type ==
            TOKEN_INTRET
        ) {
            function->data_type =
                NOVA_TYPE_INT;
        }
        else {
            function->data_type =
                NOVA_TYPE_STR;
        }

        advance_token();
    }
    else {

        if (!check(TOKEN_IDENTIFIER)) {
            return NULL;
        }

        name_token =
            current_token();

        function = ast_create(
            AST_FUNCTION,
            name_token->value
        );

        if (function == NULL) {
            return NULL;
        }

        function->data_type =
            NOVA_TYPE_VOID;

        advance_token();
    }

    if (!consume(TOKEN_LPAREN)) {
        ast_free(function);
        return NULL;
    }

    /*
     * Function parameters.
     */

    if (!check(TOKEN_RPAREN)) {

        while (1) {

            Token *type_token;
            Token *parameter_name;

            type_token =
                current_token();

            if (
                type_token == NULL ||
                (
                    type_token->type != TOKEN_INT &&
                    type_token->type != TOKEN_STR &&
                    type_token->type != TOKEN_FILE
                )
            ) {
                ast_free(function);
                return NULL;
            }

            advance_token();

            parameter_name =
                current_token();

            if (
                parameter_name == NULL ||
                parameter_name->type != TOKEN_IDENTIFIER
            ) {
                ast_free(function);
                return NULL;
            }

            parameter = ast_create(
                AST_VARIABLE_DECL,
                parameter_name->value
            );

            if (parameter == NULL) {
                ast_free(function);
                return NULL;
            }

            if (
                type_token->type ==
                TOKEN_INT
            ) {
                parameter->data_type =
                    NOVA_TYPE_INT;
            }
            else if (
                type_token->type ==
                TOKEN_FILE
            ) {
                parameter->data_type =
                    NOVA_TYPE_FILE;
            }
            else {
                parameter->data_type =
                    NOVA_TYPE_STR;
            }

            advance_token();

            ast_add_child(
                function,
                parameter
            );

            if (consume(TOKEN_COMMA)) {
                continue;
            }

            break;
        }
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

    program_name =
        current_token();

    advance_token();

    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    function_name =
        current_token();

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

    token =
        current_token();

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

    while (
        !check(TOKEN_RBRACE) &&
        !check(TOKEN_EOF)
    ) {

        if (!check(TOKEN_FUNCTION)) {
            ast_free(program);
            return NULL;
        }

        function =
            parse_function();

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

    if (
        tokens == NULL ||
        token_count <= 0
    ) {
        return NULL;
    }

    parser.tokens =
        tokens;

    parser.token_count =
        token_count;

    parser.current =
        0;

    root =
        parse_program();

    return root;
}