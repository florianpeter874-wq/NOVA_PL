#ifndef NOVA_LEXER_H
#define NOVA_LEXER_H

typedef enum {
    TOKEN_EOF,

    TOKEN_PROGRAM,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_ENTRYP,
    TOKEN_WHILE,

    TOKEN_SET,
    TOKEN_INT,
    TOKEN_EQUAL,

    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    TOKEN_EQUAL_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    TOKEN_AND_AND,
    TOKEN_OR_OR,
    TOKEN_NOT,

    TOKEN_INPUT,
    TOKEN_STR,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_INTRET,
    TOKEN_STRRET,
    TOKEN_RET,
    TOKEN_COMMA, TOKEN_FILE,
    TOKEN_DOT,
    TOKEN_TOSTR,
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

Token *lexer_tokenize(
    const char *source,
    int *token_count
);

void lexer_free_tokens(
    Token *tokens,
    int token_count
);

#endif
