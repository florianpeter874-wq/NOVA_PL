#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/lexer.h"

#define ADD_TOKEN(token_type, token_value)                         \
    do {                                                           \
        Token *new_tokens = realloc(                              \
            tokens,                                                \
            sizeof(Token) * (token_count_local + 1)              \
        );                                                         \
        if (new_tokens == NULL) {                                 \
            lexer_free_tokens(tokens, token_count_local);         \
            return NULL;                                          \
        }                                                          \
        tokens = new_tokens;                                      \
        tokens[token_count_local].type = token_type;              \
        tokens[token_count_local].value = strdup(token_value);    \
        tokens[token_count_local].line = line;                    \
        tokens[token_count_local].column = column;                \
        if (tokens[token_count_local].value == NULL) {            \
            lexer_free_tokens(tokens, token_count_local);         \
            return NULL;                                          \
        }                                                          \
        token_count_local++;                                      \
    } while (0)


Token *lexer_tokenize(
    const char *source,
    int *token_count
)
{
    Token *tokens = NULL;

    int token_count_local = 0;

    int i = 0;
    int line = 1;
    int column = 1;

    while (source[i] != '\0') {

        if (isspace((unsigned char)source[i])) {

            if (source[i] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }

            i++;
            continue;
        }

        if (isalpha((unsigned char)source[i]) ||
            source[i] == '_') {

            int start = i;
            int start_column = column;

            while (
                isalnum((unsigned char)source[i]) ||
                source[i] == '_'
            ) {
                i++;
                column++;
            }

            int length = i - start;

            char *word = malloc(length + 1);

            if (word == NULL) {
                lexer_free_tokens(
                    tokens,
                    token_count_local
                );

                return NULL;
            }

            memcpy(
                word,
                source + start,
                length
            );

            word[length] = '\0';

            if (strcmp(word, "program") == 0) {
                ADD_TOKEN(
                    TOKEN_PROGRAM,
                    word
                );
            }
            else if (strcmp(word, "function") == 0) {
                ADD_TOKEN(
                    TOKEN_FUNCTION,
                    word
                );
            }
            else if (strcmp(word, "if") == 0) {
                ADD_TOKEN(
                    TOKEN_IF,
                    word
                );
            }
            else if (strcmp(word, "else") == 0) {
                ADD_TOKEN(
                    TOKEN_ELSE,
                    word
                );
            }
            else if (strcmp(word, "entryp") == 0) {
                ADD_TOKEN(
                    TOKEN_ENTRYP,
                    word
                );
            }
            else if (strcmp(word, "while") == 0) {
                ADD_TOKEN(
                    TOKEN_WHILE,
                    word
                );
            }
            else if (strcmp(word, "set") == 0) {
                ADD_TOKEN(
                    TOKEN_SET,
                    word
                );
            }
            else if (strcmp(word, "int") == 0) {
                ADD_TOKEN(
                    TOKEN_INT,
                    word
                );
            }
            else if (strcmp(word, "str") == 0) {
                ADD_TOKEN(
                    TOKEN_STR,
                    word
                );
            }  
            else if (strcmp(word, "file") == 0) {
                ADD_TOKEN(
                    TOKEN_FILE,
                    word
                );
            }
            else if (strcmp(word, "intret") == 0) {
                ADD_TOKEN(
                    TOKEN_INTRET,
                    word
                );
            }
            else if (strcmp(word, "strret") == 0) {
                ADD_TOKEN(
                    TOKEN_STRRET,
                    word
                );
            }
            else if (strcmp(word, "ret") == 0) {
                ADD_TOKEN(
                    TOKEN_RET,
                    word
                );
            }
            else if (strcmp(word, "input") == 0) {
                ADD_TOKEN(
                    TOKEN_INPUT,
                    word
                );
            } else if (strcmp(word, "tostr") == 0) {
                ADD_TOKEN(
                    TOKEN_TOSTR,
                    word
                );
            }
            else {
                ADD_TOKEN(
                    TOKEN_IDENTIFIER,
                    word
                );
            }

            free(word);

            continue;
        }

        if (isdigit((unsigned char)source[i])) {

            int start = i;
            int start_column = column;

            while (
                isdigit((unsigned char)source[i])
            ) {
                i++;
                column++;
            }

            int length = i - start;

            char *number = malloc(
                length + 1
            );

            if (number == NULL) {
                lexer_free_tokens(
                    tokens,
                    token_count_local
                );

                return NULL;
            }

            memcpy(
                number,
                source + start,
                length
            );

            number[length] = '\0';

            ADD_TOKEN(
                TOKEN_NUMBER,
                number
            );

            free(number);

            continue;
        }

        if (source[i] == '"') {

            int start_column = column;

            i++;
            column++;

            int start = i;

            char *string_value = malloc(1);

            if (string_value == NULL) {
                lexer_free_tokens(
                    tokens,
                    token_count_local
                );

                return NULL;
            }

            int string_length = 0;

            while (
                source[i] != '\0' &&
                source[i] != '"'
            ) {

                char character;

                if (source[i] == '\\') {

                    i++;
                    column++;

                    if (source[i] == 'n') {
                        character = '\n';
                    }
                    else if (source[i] == 't') {
                        character = '\t';
                    }
                    else if (source[i] == 'r') {
                        character = '\r';
                    }
                    else if (source[i] == '\\') {
                        character = '\\';
                    }
                    else if (source[i] == '"') {
                        character = '"';
                    }
                    else {
                        character = source[i];
                    }

                    i++;
                    column++;
                }
                else {
                    character = source[i];

                    i++;
                    column++;

                    if (character == '\n') {
                        line++;
                        column = 1;
                    }
                }

                char *new_string = realloc(
                    string_value,
                    string_length + 2
                );

                if (new_string == NULL) {
                    free(string_value);

                    lexer_free_tokens(
                        tokens,
                        token_count_local
                    );

                    return NULL;
                }

                string_value = new_string;

                string_value[string_length] =
                    character;

                string_length++;
            }

            if (source[i] != '"') {

                fprintf(
                    stderr,
                    "NOVA LEXER ERROR: unterminated string at line %d, column %d\n",
                    line,
                    start_column
                );

                free(string_value);

                lexer_free_tokens(
                    tokens,
                    token_count_local
                );

                return NULL;
            }

            string_value[string_length] = '\0';

            i++;
            column++;

            ADD_TOKEN(
                TOKEN_STRING,
                string_value
            );

            free(string_value);

            continue;
        }

        if (source[i] == '(') {
            ADD_TOKEN(
                TOKEN_LPAREN,
                "("
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == ')') {
            ADD_TOKEN(
                TOKEN_RPAREN,
                ")"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == '{') {
            ADD_TOKEN(
                TOKEN_LBRACE,
                "{"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == '}') {
            ADD_TOKEN(
                TOKEN_RBRACE,
                "}"
            );

            i++;
            column++;
            continue;
        }

        /*
         * ARRAY INDEX:
         *
         * numbers[0]
         *
         * becomes:
         *
         * IDENTIFIER
         * LBRACKET
         * NUMBER
         * RBRACKET
         */

        if (source[i] == '[') {
            ADD_TOKEN(
                TOKEN_LBRACKET,
                "["
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == ']') {
            ADD_TOKEN(
                TOKEN_RBRACKET,
                "]"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == ',') {

            ADD_TOKEN(
                TOKEN_COMMA,
                ","
            );

            i++;
            column++;

            continue;
        
        
        }

        if (source[i] == '.') {

            ADD_TOKEN(
                TOKEN_DOT,
                "."
            );

            i++;
            column++;

            continue;
        }

        if (source[i] == ';') {
            ADD_TOKEN(
                TOKEN_SEMICOLON,
                ";"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == '+') {
            ADD_TOKEN(
                TOKEN_PLUS,
                "+"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == '-') {
            ADD_TOKEN(
                TOKEN_MINUS,
                "-"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == '*') {
            ADD_TOKEN(
                TOKEN_STAR,
                "*"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == '/') {
            ADD_TOKEN(
                TOKEN_SLASH,
                "/"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == '%') {
            ADD_TOKEN(
                TOKEN_PERCENT,
                "%"
            );

            i++;
            column++;
            continue;
        }

        if (source[i] == '=') {

            if (source[i + 1] == '=') {

                ADD_TOKEN(
                    TOKEN_EQUAL_EQUAL,
                    "=="
                );

                i += 2;
                column += 2;

            } else {

                ADD_TOKEN(
                    TOKEN_EQUAL,
                    "="
                );

                i++;
                column++;
            }

            continue;
        }

        if (source[i] == '!') {

            if (source[i + 1] == '=') {

                ADD_TOKEN(
                    TOKEN_NOT_EQUAL,
                    "!="
                );

                i += 2;
                column += 2;

            } else {

                ADD_TOKEN(
                    TOKEN_NOT,
                    "!"
                );

                i++;
                column++;
            }

            continue;
        }

        if (source[i] == '<') {

            if (source[i + 1] == '=') {

                ADD_TOKEN(
                    TOKEN_LESS_EQUAL,
                    "<="
                );

                i += 2;
                column += 2;

            } else {

                ADD_TOKEN(
                    TOKEN_LESS,
                    "<"
                );

                i++;
                column++;
            }

            continue;
        }

        if (source[i] == '>') {

            if (source[i + 1] == '=') {

                ADD_TOKEN(
                    TOKEN_GREATER_EQUAL,
                    ">="
                );

                i += 2;
                column += 2;

            } else {

                ADD_TOKEN(
                    TOKEN_GREATER,
                    ">"
                );

                i++;
                column++;
            }

            continue;
        }

        if (source[i] == '&') {

            if (source[i + 1] == '&') {

                ADD_TOKEN(
                    TOKEN_AND_AND,
                    "&&"
                );

                i += 2;
                column += 2;

            } else {

                fprintf(
                    stderr,
                    "NOVA LEXER ERROR: single '&' is not supported at line %d, column %d\n",
                    line,
                    column
                );

                lexer_free_tokens(
                    tokens,
                    token_count_local
                );

                return NULL;
            }

            continue;
        }

        if (source[i] == '|') {

            if (source[i + 1] == '|') {

                ADD_TOKEN(
                    TOKEN_OR_OR,
                    "||"
                );

                i += 2;
                column += 2;

            } else {

                fprintf(
                    stderr,
                    "NOVA LEXER ERROR: single '|' is not supported at line %d, column %d\n",
                    line,
                    column
                );

                lexer_free_tokens(
                    tokens,
                    token_count_local
                );

                return NULL;
            }

            continue;
        }

        fprintf(
            stderr,
            "NOVA LEXER ERROR: unexpected character '%c' at line %d, column %d\n",
            source[i],
            line,
            column
        );

        lexer_free_tokens(
            tokens,
            token_count_local
        );

        return NULL;
    }

    ADD_TOKEN(
        TOKEN_EOF,
        ""
    );

    *token_count = token_count_local;

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
