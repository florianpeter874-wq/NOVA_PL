#include <stdio.h>

#include "../include/error.h"

const char *error_type_name(NovaErrorType type)
{
    switch (type)
    {
        case NOVA_ERROR_NONE:
            return "None";

        case NOVA_ERROR_UNKNOWN_CHARACTER:
            return "Unknown Character";

        case NOVA_ERROR_UNTERMINATED_STRING:
            return "Unterminated String";

        case NOVA_ERROR_UNEXPECTED_TOKEN:
            return "Unexpected Token";

        case NOVA_ERROR_UNEXPECTED_EOF:
            return "Unexpected End Of File";

        case NOVA_ERROR_INVALID_SYNTAX:
            return "Invalid Syntax";

        case NOVA_ERROR_MEMORY:
            return "Memory Error";

        case NOVA_ERROR_FILE:
            return "File Error";

        default:
            return "Unknown Error";
    }
}

void error_report(
    NovaErrorType type,
    const char *message,
    int line,
    int column
)
{
    fprintf(
        stderr,
        "NOVA ERROR [%s]\n",
        error_type_name(type)
    );

    if (line > 0)
    {
        fprintf(
            stderr,
            "Line %d, Column %d\n",
            line,
            column
        );
    }

    if (message != NULL)
        fprintf(stderr, "%s\n", message);
}
