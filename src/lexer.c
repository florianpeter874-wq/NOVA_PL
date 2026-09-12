#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/lexer.h"

static char *copy_string(
    const char *start,
    size_t length
)
{
    char *result;

    result = malloc(length + 1);

    if (result == NULL) {
        return NULL;
    }

    memcpy(
        result,
        start,
        length
    );

    result[length] = '\0';

    return result;
}

static TokenType keyword_type(
    const char *text
)
{
    if (strcmp(text, "program") == 0) {
        return TOKEN_PROGRAM;
    }

    if (strcmp(text, "function") == 0) {
        return TOKEN_FUNCTION;
    }

    if (strcmp(text, "if") == 0) {
        return TOKEN_IF;
    }

    if (strcmp(text, "else") == 0) {
        return TOKEN_ELSE;
    }

    if (strcmp(text, "entryp") == 0) {
        return TOKEN_ENTRYP;
    }

    if (strcmp(text, "set") == 0) {
        return TOKEN_SET;
    }

    if (strcmp(text, "int") == 0) {
        return TOKEN_INT;
    }

    return TOKEN_IDENTIFIER;
}

Token *lexer_tokenize(
    const char *source,
    int *token_count
)
{
    Token *tokens;

    int capacity;
    int count;

    int line;
    int column;

    size_t position;

    if (source == NULL ||
        token_count == NULL) {

        return NULL;
    }

    capacity = 64;
    count = 0;

    line = 1;
    column = 1;

    position = 0;

    tokens = malloc(
        sizeof(Token) * capacity
    );

    if (tokens == NULL) {
        return NULL;
    }

#define ADD_TOKEN(token_type, token_value, token_line, token_column) \
    do { \
        if (count >= capacity) { \
            Token *new_tokens; \
            capacity *= 2; \
            new_tokens = realloc( \
                tokens, \
                sizeof(Token) * capacity \
            ); \
            if (new_tokens == NULL) { \
                lexer_free_tokens(tokens, count); \
                return NULL; \
            } \
            tokens = new_tokens; \
        } \
        tokens[count].type = token_type; \
        tokens[count].value = token_value; \
        tokens[count].line = token_line; \
        tokens[count].column = token_column; \
        count++; \
    } while (0)

    while (source[position] != '\0') {
        char current;

        int token_line;
        int token_column;

        current = source[position];

        token_line = line;
        token_column = column;

        if (current == ' ' ||
            current == '\t' ||
            current == '\r') {

            position++;
            column++;

            continue;
        }

        if (current == '\n') {
            position++;
            line++;
            column = 1;

            continue;
        }

        if (isalpha((unsigned char)current) ||
            current == '_') {

            size_t start;
            char *value;
            TokenType type;

            start = position;

            while (
                isalnum(
                    (unsigned char)source[position]
                ) ||
                source[position] == '_'
            ) {
                position++;
                column++;
            }

            value = copy_string(
                source + start,
                position - start
            );

            if (value == NULL) {
                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            type = keyword_type(value);

            ADD_TOKEN(
                type,
                value,
                token_line,
                token_column
            );

            continue;
        }

        if (isdigit((unsigned char)current)) {
            size_t start;
            char *value;

            start = position;

            while (
                isdigit(
                    (unsigned char)source[position]
                )
            ) {
                position++;
                column++;
            }

            value = copy_string(
                source + start,
                position - start
            );

            if (value == NULL) {
                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            ADD_TOKEN(
                TOKEN_NUMBER,
                value,
                token_line,
                token_column
            );

            continue;
        }

        if (current == '"') {
            size_t start;
            char *value;

            start = position;

            position++;
            column++;

            while (
                source[position] != '\0' &&
                source[position] != '"'
            ) {
                if (source[position] == '\\') {
                    if (source[position + 1] == '\0') {
                        lexer_free_tokens(
                            tokens,
                            count
                        );

                        return NULL;
                    }

                    position += 2;
                    column += 2;

                    continue;
                }

                if (source[position] == '\n') {
                    line++;
                    column = 1;
                    position++;

                    continue;
                }

                position++;
                column++;
            }

            if (source[position] != '"') {
                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            position++;
            column++;

            value = copy_string(
                source + start,
                position - start
            );

            if (value == NULL) {
                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            ADD_TOKEN(
                TOKEN_STRING,
                value,
                token_line,
                token_column
            );

            continue;
        }

        switch (current) {
            case '(':
                ADD_TOKEN(
                    TOKEN_LPAREN,
                    copy_string("(", 1),
                    token_line,
                    token_column
                );

                position++;
                column++;

                break;

            case ')':
                ADD_TOKEN(
                    TOKEN_RPAREN,
                    copy_string(")", 1),
                    token_line,
                    token_column
                );

                position++;
                column++;

                break;

            case '{':
                ADD_TOKEN(
                    TOKEN_LBRACE,
                    copy_string("{", 1),
                    token_line,
                    token_column
                );

                position++;
                column++;

                break;

            case '}':
                ADD_TOKEN(
                    TOKEN_RBRACE,
                    copy_string("}", 1),
                    token_line,
                    token_column
                );

                position++;
                column++;

                break;

            case ';':
                ADD_TOKEN(
                    TOKEN_SEMICOLON,
                    copy_string(";", 1),
                    token_line,
                    token_column
                );

                position++;
                column++;

                break;

            case '=':
                if (source[position + 1] == '=') {
                    ADD_TOKEN(
                        TOKEN_EQUAL_EQUAL,
                        copy_string("==", 2),
                        token_line,
                        token_column
                    );

                    position += 2;
                    column += 2;
                } else {
                    ADD_TOKEN(
                        TOKEN_EQUAL,
                        copy_string("=", 1),
                        token_line,
                        token_column
                    );

                    position++;
                    column++;
                }

                break;

            case '!':
                if (source[position + 1] == '=') {
                    ADD_TOKEN(
                        TOKEN_NOT_EQUAL,
                        copy_string("!=", 2),
                        token_line,
                        token_column
                    );

                    position += 2;
                    column += 2;
                } else {
                    lexer_free_tokens(
                        tokens,
                        count
                    );

                    return NULL;
                }

                break;

            case '<':
                if (source[position + 1] == '=') {
                    ADD_TOKEN(
                        TOKEN_LESS_EQUAL,
                        copy_string("<=", 2),
                        token_line,
                        token_column
                    );

                    position += 2;
                    column += 2;
                } else {
                    ADD_TOKEN(
                        TOKEN_LESS,
                        copy_string("<", 1),
                        token_line,
                        token_column
                    );

                    position++;
                    column++;
                }

                break;

            case '>':
                if (source[position + 1] == '=') {
                    ADD_TOKEN(
                        TOKEN_GREATER_EQUAL,
                        copy_string(">=", 2),
                        token_line,
                        token_column
                    );

                    position += 2;
                    column += 2;
                } else {
                    ADD_TOKEN(
                        TOKEN_GREATER,
                        copy_string(">", 1),
                        token_line,
                        token_column
                    );

                    position++;
                    column++;
                }

                break;
            case '+':
                ADD_TOKEN(
                    TOKEN_PLUS,
                    copy_string("+", 1),
                    token_line,
                    token_column
                );

                position++;
                column++;

                break;

            default:
                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
        }
    }

    ADD_TOKEN(
        TOKEN_EOF,
        copy_string("", 0),
        line,
        column
    );

#undef ADD_TOKEN

    *token_count = count;

    return tokens;
}

void lexer_free_tokens(
    Token *tokens,
    int token_count
)
{
    int i;

    if (tokens == NULL) {
        return;
    }

    for (i = 0; i < token_count; i++) {
        free(tokens[i].value);
    }

    free(tokens);
}
