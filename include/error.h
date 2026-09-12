#ifndef NOVA_ERROR_H
#define NOVA_ERROR_H

typedef enum {
    NOVA_ERROR_NONE,

    NOVA_ERROR_UNKNOWN_CHARACTER,
    NOVA_ERROR_UNTERMINATED_STRING,
    NOVA_ERROR_UNEXPECTED_TOKEN,
    NOVA_ERROR_UNEXPECTED_EOF,
    NOVA_ERROR_INVALID_SYNTAX,

    NOVA_ERROR_MEMORY,
    NOVA_ERROR_FILE
} NovaErrorType;

typedef struct {
    NovaErrorType type;
    const char *message;
    int line;
    int column;
} NovaError;

void error_report(
    NovaErrorType type,
    const char *message,
    int line,
    int column
);

const char *error_type_name(NovaErrorType type);

#endif
