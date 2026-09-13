#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/compiler.h"

static FILE *output;

static int debug_enabled;

typedef struct {
    char name[256];
    NovaType type;
    int is_array;
} VariableInfo;

static VariableInfo variables[1024];
static int variable_count;

typedef struct {
    char name[256];
    NovaType return_type;
    NovaType parameter_types[256];
    int parameter_count;
} FunctionInfo;

static FunctionInfo functions[1024];
static int function_count;

static int compile_node(
    ASTNode *node
);

static int compile_expression(
    ASTNode *node
);

static int compile_block(
    ASTNode *node
);

static int compile_function(
    ASTNode *node
);

static int compile_call(
    ASTNode *node
);

static int compile_while(
    ASTNode *node
);

static int compile_assignment(
    ASTNode *node
);

static int compile_variable_decl(
    ASTNode *node
);

static int compile_print(
    ASTNode *node
);

static int compile_if(
    ASTNode *node
);

static int get_variable_info(
    const char *name,
    NovaType *type,
    int *is_array
)
{
    int i;

    if (
        name == NULL ||
        type == NULL ||
        is_array == NULL
    ) {
        return 0;
    }

    for (
        i = variable_count - 1;
        i >= 0;
        i--
    ) {
        if (
            strcmp(
                variables[i].name,
                name
            ) == 0
        ) {
            *type =
                variables[i].type;

            *is_array =
                variables[i].is_array;

            return 1;
        }
    }

    return 0;
}

static int get_variable_type(
    const char *name,
    NovaType *type
)
{
    int is_array;

    return get_variable_info(
        name,
        type,
        &is_array
    );
}

static int register_variable(
    const char *name,
    NovaType type,
    int is_array
)
{
    int i;

    if (name == NULL) {
        return 0;
    }

    if (variable_count >= 1024) {
        return 0;
    }

    for (
        i = 0;
        i < variable_count;
        i++
    ) {
        if (
            strcmp(
                variables[i].name,
                name
            ) == 0
        ) {
            return 0;
        }
    }

    strncpy(
        variables[variable_count].name,
        name,
        sizeof(
            variables[variable_count].name
        ) - 1
    );

    variables[variable_count].name[
        sizeof(
            variables[variable_count].name
        ) - 1
    ] = '\0';

    variables[variable_count].type =
        type;

    variables[variable_count].is_array =
        is_array;

    variable_count++;

    return 1;
}

static int register_function(
    ASTNode *function
)
{
    int i;
    int parameter_count;
    int index;

    if (
        function == NULL ||
        function->value == NULL
    ) {
        return 0;
    }

    if (function_count >= 1024) {
        return 0;
    }

    for (
        i = 0;
        i < function_count;
        i++
    ) {
        if (
            strcmp(
                functions[i].name,
                function->value
            ) == 0
        ) {
            functions[i].return_type =
                function->data_type;

            functions[i].parameter_count = 0;

            parameter_count =
                function->child_count - 1;

            if (parameter_count < 0) {
                parameter_count = 0;
            }

            for (
                index = 0;
                index < parameter_count &&
                index < 256;
                index++
            ) {
                if (
                    function->children[index] == NULL ||
                    function->children[index]->type !=
                        AST_VARIABLE_DECL
                ) {
                    return 0;
                }

                functions[i].parameter_types[index] =
                    function->children[index]->data_type;

                functions[i].parameter_count++;
            }

            return 1;
        }
    }

    strncpy(
        functions[function_count].name,
        function->value,
        sizeof(
            functions[function_count].name
        ) - 1
    );

    functions[function_count].name[
        sizeof(
            functions[function_count].name
        ) - 1
    ] = '\0';

    functions[function_count].return_type =
        function->data_type;

    functions[function_count].parameter_count = 0;

    parameter_count =
        function->child_count - 1;

    if (parameter_count < 0) {
        parameter_count = 0;
    }

    for (
        index = 0;
        index < parameter_count &&
        index < 256;
        index++
    ) {
        if (
            function->children[index] == NULL ||
            function->children[index]->type !=
                AST_VARIABLE_DECL
        ) {
            return 0;
        }

        functions[function_count].parameter_types[index] =
            function->children[index]->data_type;

        functions[function_count].parameter_count++;
    }

    function_count++;

    return 1;
}

static int get_function_type(
    const char *name,
    NovaType *type
)
{
    int i;

    if (
        name == NULL ||
        type == NULL
    ) {
        return 0;
    }

    for (
        i = 0;
        i < function_count;
        i++
    ) {
        if (
            strcmp(
                functions[i].name,
                name
            ) == 0
        ) {
            *type =
                functions[i].return_type;

            return 1;
        }
    }

    return 0;
}

static const char *type_name(
    NovaType type
)
{
    if (
        type == NOVA_TYPE_STR
    ) {
        return "str";
    }

    if (
        type == NOVA_TYPE_VOID
    ) {
        return "void";
    }

    if (
        type == NOVA_TYPE_FILE
    ) {
        return "file";
    }

    return "int";
}

static int expression_type(
    ASTNode *node,
    NovaType *type
)
{
    NovaType left_type;
    NovaType right_type;
    NovaType variable_type;
    NovaType function_type;

    int is_array;

    if (
        node == NULL ||
        type == NULL
    ) {
        return 0;
    }

    switch (node->type) {

        case AST_NUMBER:

            *type =
                NOVA_TYPE_INT;

            return 1;

        case AST_STRING:

            *type =
                NOVA_TYPE_STR;

            return 1;

        case AST_INPUT:

            *type =
                NOVA_TYPE_INT;

            return 1;

        case AST_VARIABLE_REF:

            if (
                node->value == NULL
            ) {
                return 0;
            }

            if (
                !get_variable_info(
                    node->value,
                    &variable_type,
                    &is_array
                )
            ) {
                printf(
                    "NOVA TYPE ERROR: undefined variable '%s'\n",
                    node->value
                );

                return 0;
            }

            if (
                node->child_count > 0
            ) {
                if (!is_array) {
                    printf(
                        "NOVA TYPE ERROR: variable '%s' is not an array\n",
                        node->value
                    );

                    return 0;
                }

                if (
                    node->child_count != 1
                ) {
                    return 0;
                }

                if (
                    !expression_type(
                        node->children[0],
                        &left_type
                    )
                ) {
                    return 0;
                }

                if (
                    left_type !=
                    NOVA_TYPE_INT
                ) {
                    printf(
                        "NOVA TYPE ERROR: array index must be int\n"
                    );

                    return 0;
                }

                *type =
                    variable_type;

                return 1;
            }

            if (is_array) {
                printf(
                    "NOVA TYPE ERROR: array '%s' requires an index\n",
                    node->value
                );

                return 0;
            }

            *type =
                variable_type;

            return 1;

        case AST_CALL:

            if (
                node->value == NULL
            ) {
                return 0;
            }

            /*
             * tostr(int)
             *
             * Returns:
             *
             * str
             */

            if (
                strcmp(
                    node->value,
                    "tostr"
                ) == 0
            ) {
                if (
                    node->child_count != 1
                ) {
                    printf(
                        "NOVA TYPE ERROR: tostr requires 1 argument\n"
                    );

                    return 0;
                }

                if (
                    !expression_type(
                        node->children[0],
                        &left_type
                    )
                ) {
                    return 0;
                }

                if (
                    left_type !=
                    NOVA_TYPE_INT
                ) {
                    printf(
                        "NOVA TYPE ERROR: tostr requires int argument, got %s\n",
                        type_name(left_type)
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_STR;

                return 1;
            }

            /*
             * file.open(path)
             */

            if (
                strcmp(
                    node->value,
                    "file.open"
                ) == 0
            ) {
                if (
                    node->child_count != 1
                ) {
                    printf(
                        "NOVA TYPE ERROR: file.open requires 1 argument\n"
                    );

                    return 0;
                }

                if (
                    !expression_type(
                        node->children[0],
                        &left_type
                    )
                ) {
                    return 0;
                }

                if (
                    left_type !=
                    NOVA_TYPE_STR
                ) {
                    printf(
                        "NOVA TYPE ERROR: file.open requires str path\n"
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_FILE;

                return 1;
            }

            /*
             * file.read(file)
             */

            if (
                strcmp(
                    node->value,
                    "file.read"
                ) == 0
            ) {
                if (
                    node->child_count != 1
                ) {
                    printf(
                        "NOVA TYPE ERROR: file.read requires 1 argument\n"
                    );

                    return 0;
                }

                if (
                    !expression_type(
                        node->children[0],
                        &left_type
                    )
                ) {
                    return 0;
                }

                if (
                    left_type !=
                    NOVA_TYPE_FILE
                ) {
                    printf(
                        "NOVA TYPE ERROR: file.read requires file argument\n"
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_STR;

                return 1;
            }

            /*
             * file.write(file, str)
             */

            if (
                strcmp(
                    node->value,
                    "file.write"
                ) == 0
            ) {
                if (
                    node->child_count != 2
                ) {
                    printf(
                        "NOVA TYPE ERROR: file.write requires 2 arguments\n"
                    );

                    return 0;
                }

                if (
                    !expression_type(
                        node->children[0],
                        &left_type
                    )
                ) {
                    return 0;
                }

                if (
                    !expression_type(
                        node->children[1],
                        &right_type
                    )
                ) {
                    return 0;
                }

                if (
                    left_type != NOVA_TYPE_FILE ||
                    right_type != NOVA_TYPE_STR
                ) {
                    printf(
                        "NOVA TYPE ERROR: file.write requires file and str arguments\n"
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_VOID;

                return 1;
            }

            /*
             * file.close(file)
             */

            if (
                strcmp(
                    node->value,
                    "file.close"
                ) == 0
            ) {
                if (
                    node->child_count != 1
                ) {
                    printf(
                        "NOVA TYPE ERROR: file.close requires 1 argument\n"
                    );

                    return 0;
                }

                if (
                    !expression_type(
                        node->children[0],
                        &left_type
                    )
                ) {
                    return 0;
                }

                if (
                    left_type !=
                    NOVA_TYPE_FILE
                ) {
                    printf(
                        "NOVA TYPE ERROR: file.close requires file argument\n"
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_VOID;

                return 1;
            }

            /*
             * Normal NOVA function.
             */

            if (
                !get_function_type(
                    node->value,
                    &function_type
                )
            ) {
                printf(
                    "NOVA TYPE ERROR: undefined function '%s'\n",
                    node->value
                );

                return 0;
            }

            if (
                function_type ==
                NOVA_TYPE_VOID
            ) {
                printf(
                    "NOVA TYPE ERROR: void function '%s' cannot be used as an expression\n",
                    node->value
                );

                return 0;
            }

            *type =
                function_type;

            return 1;

        case AST_UNARY:

            if (
                node->child_count != 1 ||
                node->value == NULL
            ) {
                return 0;
            }

            if (
                !expression_type(
                    node->children[0],
                    &left_type
                )
            ) {
                return 0;
            }

            if (
                strcmp(
                    node->value,
                    "!"
                ) == 0
            ) {
                if (
                    left_type !=
                    NOVA_TYPE_INT
                ) {
                    printf(
                        "NOVA TYPE ERROR: operator ! requires int, got %s\n",
                        type_name(left_type)
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_INT;

                return 1;
            }

            printf(
                "NOVA TYPE ERROR: unknown unary operator '%s'\n",
                node->value
            );

            return 0;

        case AST_BINARY:

            if (
                node->child_count != 2 ||
                node->value == NULL
            ) {
                return 0;
            }

            if (
                !expression_type(
                    node->children[0],
                    &left_type
                )
            ) {
                return 0;
            }

            if (
                !expression_type(
                    node->children[1],
                    &right_type
                )
            ) {
                return 0;
            }

            if (
                strcmp(
                    node->value,
                    "+"
                ) == 0
            ) {
                if (
                    left_type == NOVA_TYPE_STR &&
                    right_type == NOVA_TYPE_STR
                ) {
                    *type =
                        NOVA_TYPE_STR;

                    return 1;
                }

                if (
                    left_type == NOVA_TYPE_INT &&
                    right_type == NOVA_TYPE_INT
                ) {
                    *type =
                        NOVA_TYPE_INT;

                    return 1;
                }

                printf(
                    "NOVA TYPE ERROR: cannot add %s and %s\n",
                    type_name(left_type),
                    type_name(right_type)
                );

                return 0;
            }

            if (
                strcmp(
                    node->value,
                    "-"
                ) == 0 ||
                strcmp(
                    node->value,
                    "*"
                ) == 0 ||
                strcmp(
                    node->value,
                    "/"
                ) == 0 ||
                strcmp(
                    node->value,
                    "%"
                ) == 0 ||
                strcmp(
                    node->value,
                    "*%*"
                ) == 0 ||
                strcmp(
                    node->value,
                    "\**%**"
                ) == 0
            ) {
                if (
                    left_type != NOVA_TYPE_INT ||
                    right_type != NOVA_TYPE_INT
                ) {
                    printf(
                        "NOVA TYPE ERROR: operator %s requires int operands, got %s and %s\n",
                        node->value,
                        type_name(left_type),
                        type_name(right_type)
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_INT;

                return 1;
            }

            if (
                strcmp(
                    node->value,
                    "=="
                ) == 0 ||
                strcmp(
                    node->value,
                    "!="
                ) == 0
            ) {
                if (
                    left_type != right_type
                ) {
                    printf(
                        "NOVA TYPE ERROR: cannot compare %s and %s\n",
                        type_name(left_type),
                        type_name(right_type)
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_INT;

                return 1;
            }

            if (
                strcmp(
                    node->value,
                    "<"
                ) == 0 ||
                strcmp(
                    node->value,
                    "<="
                ) == 0 ||
                strcmp(
                    node->value,
                    ">"
                ) == 0 ||
                strcmp(
                    node->value,
                    ">="
                ) == 0
            ) {
                if (
                    left_type != NOVA_TYPE_INT ||
                    right_type != NOVA_TYPE_INT
                ) {
                    printf(
                        "NOVA TYPE ERROR: operator %s requires int operands, got %s and %s\n",
                        node->value,
                        type_name(left_type),
                        type_name(right_type)
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_INT;

                return 1;
            }

            if (
                strcmp(
                    node->value,
                    "&&"
                ) == 0 ||
                strcmp(
                    node->value,
                    "||"
                ) == 0
            ) {
                if (
                    left_type != NOVA_TYPE_INT ||
                    right_type != NOVA_TYPE_INT
                ) {
                    printf(
                        "NOVA TYPE ERROR: operator %s requires int operands, got %s and %s\n",
                        node->value,
                        type_name(left_type),
                        type_name(right_type)
                    );

                    return 0;
                }

                *type =
                    NOVA_TYPE_INT;

                return 1;
            }

            printf(
                "NOVA TYPE ERROR: unknown operator '%s'\n",
                node->value
            );

            return 0;

        default:
            return 0;
    }
}

static void write_c_string(
    const char *text
)
{
    const char *p;

    if (text == NULL) {
        return;
    }

    p = text;

    if (*p == '"') {
        p++;
    }

    while (*p != '\0') {

        if (
            *p == '"' &&
            p[1] == '\0'
        ) {
            break;
        }

        switch (*p) {

            case '\n':
                fputs(
                    "\\n",
                    output
                );
                break;

            case '\t':
                fputs(
                    "\\t",
                    output
                );
                break;

            case '\r':
                fputs(
                    "\\r",
                    output
                );
                break;

            case '\\':
                fputs(
                    "\\\\",
                    output
                );
                break;

            case '"':
                fputs(
                    "\\\"",
                    output
                );
                break;

            default:
                fputc(
                    *p,
                    output
                );
                break;
        }

        p++;
    }
}

static int compile_expression(
    ASTNode *node
)
{
    NovaType type;

    if (node == NULL) {
        return 1;
    }

    if (
        !expression_type(
            node,
            &type
        )
    ) {
        return 1;
    }

    switch (node->type) {

        case AST_NUMBER:

            if (node->value == NULL) {
                return 1;
            }

            fprintf(
                output,
                "%s",
                node->value
            );

            return 0;

        case AST_STRING:

            if (node->value == NULL) {
                return 1;
            }

            fputc(
                '"',
                output
            );

            write_c_string(
                node->value
            );

            fputc(
                '"',
                output
            );

            return 0;

        case AST_VARIABLE_REF:

            if (node->value == NULL) {
                return 1;
            }

            fprintf(
                output,
                "%s",
                node->value
            );

            if (
                node->child_count == 1
            ) {
                fputc(
                    '[',
                    output
                );

                if (
                    compile_expression(
                        node->children[0]
                    ) != 0
                ) {
                    return 1;
                }

                fputc(
                    ']',
                    output
                );
            }

            return 0;

        case AST_CALL:

            if (
                node->value == NULL
            ) {
                return 1;
            }

            /*
             * tostr(int)
             */

            if (
                strcmp(
                    node->value,
                    "tostr"
                ) == 0
            ) {
                if (
                    node->child_count != 1
                ) {
                    return 1;
                }

                fputs(
                    "nova_tostr(",
                    output
                );

                if (
                    compile_expression(
                        node->children[0]
                    ) != 0
                ) {
                    return 1;
                }

                fputc(
                    ')',
                    output
                );

                return 0;
            }

            /*
             * file.open(path)
             */

            if (
                strcmp(
                    node->value,
                    "file.open"
                ) == 0
            ) {
                if (
                    node->child_count != 1
                ) {
                    return 1;
                }

                fputs(
                    "nova_file_open(",
                    output
                );

                if (
                    compile_expression(
                        node->children[0]
                    ) != 0
                ) {
                    return 1;
                }

                fputc(
                    ')',
                    output
                );

                return 0;
            }

            /*
             * file.read(file)
             */

            if (
                strcmp(
                    node->value,
                    "file.read"
                ) == 0
            ) {
                if (
                    node->child_count != 1
                ) {
                    return 1;
                }

                fputs(
                    "nova_file_read(",
                    output
                );

                if (
                    compile_expression(
                        node->children[0]
                    ) != 0
                ) {
                    return 1;
                }

                fputc(
                    ')',
                    output
                );

                return 0;
            }

            /*
             * Statement-only file functions.
             */

            if (
                strcmp(
                    node->value,
                    "file.write"
                ) == 0 ||
                strcmp(
                    node->value,
                    "file.close"
                ) == 0
            ) {
                return 1;
            }

            /*
             * Normal function expression.
             */

            fprintf(
                output,
                "%s(",
                node->value
            );

            if (
                node->child_count > 0
            ) {
                int i;

                for (
                    i = 0;
                    i < node->child_count;
                    i++
                ) {
                    if (i > 0) {
                        fputs(
                            ", ",
                            output
                        );
                    }

                    if (
                        compile_expression(
                            node->children[i]
                        ) != 0
                    ) {
                        return 1;
                    }
                }
            }

            fputc(
                ')',
                output
            );

            return 0;

        case AST_INPUT:

            fputs(
                "nova_input()",
                output
            );

            return 0;

        case AST_UNARY:

            if (
                node->child_count != 1 ||
                node->value == NULL
            ) {
                return 1;
            }

            fputc(
                '(',
                output
            );

            fprintf(
                output,
                "%s",
                node->value
            );

            if (
                compile_expression(
                    node->children[0]
                ) != 0
            ) {
                return 1;
            }

            fputc(
                ')',
                output
            );

            return 0;

        case AST_BINARY:

            if (
                node->child_count != 2 ||
                node->value == NULL
            ) {
                return 1;
            }

            if (
                strcmp(
                    node->value,
                    "+"
                ) == 0 &&
                type == NOVA_TYPE_STR
            ) {
                fputs(
                    "nova_concat(",
                    output
                );

                if (
                    compile_expression(
                        node->children[0]
                    ) != 0
                ) {
                    return 1;
                }

                fputs(
                    ", ",
                    output
                );

                if (
                    compile_expression(
                        node->children[1]
                    ) != 0
                ) {
                    return 1;
                }

                fputc(
                    ')',
                    output
                );

                return 0;
            }

            fputc(
                '(',
                output
            );

            if (
                compile_expression(
                    node->children[0]
                ) != 0
            ) {
                return 1;
            }

            fprintf(
                output,
                " %s ",
                node->value
            );

            if (
                compile_expression(
                    node->children[1]
                ) != 0
            ) {
                return 1;
            }

            fputc(
                ')',
                output
            );

            return 0;

        default:
            return 1;
    }
}

static int compile_variable_decl(
    ASTNode *node
)
{
    NovaType expression_type_value;

    if (
        node == NULL ||
        node->value == NULL
    ) {
        return 1;
    }

    /*
     * Array declaration.
     */

    if (node->is_array) {

        if (
            node->data_type ==
            NOVA_TYPE_FILE
        ) {
            printf(
                "NOVA TYPE ERROR: file arrays are not supported\n"
            );

            return 1;
        }

        if (
            node->child_count != 1 ||
            node->children[0] == NULL
        ) {
            return 1;
        }

        if (
            !expression_type(
                node->children[0],
                &expression_type_value
            )
        ) {
            return 1;
        }

        if (
            expression_type_value !=
            NOVA_TYPE_INT
        ) {
            printf(
                "NOVA TYPE ERROR: array size must be int\n"
            );

            return 1;
        }

        if (
            !register_variable(
                node->value,
                node->data_type,
                1
            )
        ) {
            printf(
                "NOVA TYPE ERROR: variable '%s' already exists\n",
                node->value
            );

            return 1;
        }

        if (
            node->data_type ==
            NOVA_TYPE_STR
        ) {
            fprintf(
                output,
                "    char *%s[",
                node->value
            );
        }
        else {
            fprintf(
                output,
                "    int %s[",
                node->value
            );
        }

        if (
            compile_expression(
                node->children[0]
            ) != 0
        ) {
            return 1;
        }

        fputs(
            "];\n",
            output
        );

        return 0;
    }

    /*
     * Normal variable.
     */

    if (
        node->child_count != 1
    ) {
        return 1;
    }

    if (
        !expression_type(
            node->children[0],
            &expression_type_value
        )
    ) {
        return 1;
    }

    if (
        node->data_type !=
        expression_type_value
    ) {
        printf(
            "NOVA TYPE ERROR: cannot assign %s to %s variable '%s'\n",
            type_name(expression_type_value),
            type_name(node->data_type),
            node->value
        );

        return 1;
    }

    if (
        !register_variable(
            node->value,
            node->data_type,
            0
        )
    ) {
        printf(
            "NOVA TYPE ERROR: variable '%s' already exists\n",
            node->value
        );

        return 1;
    }

    if (
        node->data_type ==
        NOVA_TYPE_STR
    ) {
        fprintf(
            output,
            "    char *%s = ",
            node->value
        );
    }
    else if (
        node->data_type ==
        NOVA_TYPE_FILE
    ) {
        fprintf(
            output,
            "    FILE *%s = ",
            node->value
        );
    }
    else {
        fprintf(
            output,
            "    int %s = ",
            node->value
        );
    }

    if (
        compile_expression(
            node->children[0]
        ) != 0
    ) {
        return 1;
    }

    fputs(
        ";\n",
        output
    );

    return 0;
}

static int compile_assignment(
    ASTNode *node
)
{
    NovaType variable_type_value;
    NovaType expression_type_value;

    int is_array;

    if (
        node == NULL ||
        node->value == NULL
    ) {
        return 1;
    }

    if (
        !get_variable_info(
            node->value,
            &variable_type_value,
            &is_array
        )
    ) {
        printf(
            "NOVA TYPE ERROR: undefined variable '%s'\n",
            node->value
        );

        return 1;
    }

    /*
     * Array assignment.
     */

    if (
        node->child_count == 2
    ) {
        if (!is_array) {
            printf(
                "NOVA TYPE ERROR: variable '%s' is not an array\n",
                node->value
            );

            return 1;
        }

        if (
            !expression_type(
                node->children[0],
                &expression_type_value
            )
        ) {
            return 1;
        }

        if (
            expression_type_value !=
            NOVA_TYPE_INT
        ) {
            printf(
                "NOVA TYPE ERROR: array index must be int\n"
            );

            return 1;
        }

        if (
            !expression_type(
                node->children[1],
                &expression_type_value
            )
        ) {
            return 1;
        }

        if (
            variable_type_value !=
            expression_type_value
        ) {
            printf(
                "NOVA TYPE ERROR: cannot assign %s to %s array '%s'\n",
                type_name(expression_type_value),
                type_name(variable_type_value),
                node->value
            );

            return 1;
        }

        fprintf(
            output,
            "    %s[",
            node->value
        );

        if (
            compile_expression(
                node->children[0]
            ) != 0
        ) {
            return 1;
        }

        fputs(
            "] = ",
            output
        );

        if (
            compile_expression(
                node->children[1]
            ) != 0
        ) {
            return 1;
        }

        fputs(
            ";\n",
            output
        );

        return 0;
    }

    if (
        node->child_count != 1
    ) {
        return 1;
    }

    if (is_array) {
        printf(
            "NOVA TYPE ERROR: array '%s' requires an index\n",
            node->value
        );

        return 1;
    }

    if (
        !expression_type(
            node->children[0],
            &expression_type_value
        )
    ) {
        return 1;
    }

    if (
        variable_type_value !=
        expression_type_value
    ) {
        printf(
            "NOVA TYPE ERROR: cannot assign %s to %s variable '%s'\n",
            type_name(expression_type_value),
            type_name(variable_type_value),
            node->value
        );

        return 1;
    }

    fprintf(
        output,
        "    %s = ",
        node->value
    );

    if (
        compile_expression(
            node->children[0]
        ) != 0
    ) {
        return 1;
    }

    fputs(
        ";\n",
        output
    );

    return 0;
}

static int compile_print(
    ASTNode *node
)
{
    ASTNode *expression;
    NovaType type;

    if (
        node == NULL ||
        node->child_count != 1
    ) {
        return 1;
    }

    expression =
        node->children[0];

    if (expression == NULL) {
        return 1;
    }

    if (
        !expression_type(
            expression,
            &type
        )
    ) {
        return 1;
    }

    /*
     * String literal:
     *
     * print("Hello");
     *
     * NOVA print() does not automatically
     * add a newline.
     */

    if (
        expression->type ==
        AST_STRING
    ) {
        fputs(
            "    printf(",
            output
        );

        fputc(
            '"',
            output
        );

        write_c_string(
            expression->value
        );

        fputc(
            '"',
            output
        );

        fputs(
            ");\n",
            output
        );

        return 0;
    }

    /*
     * String expression.
     */

    if (
        type == NOVA_TYPE_STR
    ) {
        fputs(
            "    printf(\"%s\", ",
            output
        );

        if (
            compile_expression(
                expression
            ) != 0
        ) {
            return 1;
        }

        fputs(
            ");\n",
            output
        );

        return 0;
    }

    /*
     * Integer expression.
     */

    fputs(
        "    printf(\"%d\", ",
        output
    );

    if (
        compile_expression(
            expression
        ) != 0
    ) {
        return 1;
    }

    fputs(
        ");\n",
        output
    );

    return 0;
}

static int compile_if(
    ASTNode *node
)
{
    NovaType condition_type;

    if (
        node == NULL ||
        node->child_count < 2
    ) {
        return 1;
    }

    if (
        !expression_type(
            node->children[0],
            &condition_type
        )
    ) {
        return 1;
    }

    if (
        condition_type !=
        NOVA_TYPE_INT
    ) {
        printf(
            "NOVA TYPE ERROR: if condition must be int\n"
        );

        return 1;
    }

    fputs(
        "    if (",
        output
    );

    if (
        compile_expression(
            node->children[0]
        ) != 0
    ) {
        return 1;
    }

    fputs(
        ") {\n",
        output
    );

    if (
        compile_block(
            node->children[1]
        ) != 0
    ) {
        return 1;
    }

    fputs(
        "    }",
        output
    );

    if (
        node->child_count >= 3 &&
        node->children[2] != NULL
    ) {
        fputs(
            " else {\n",
            output
        );

        if (
            compile_block(
                node->children[2]
            ) != 0
        ) {
            return 1;
        }

        fputs(
            "    }",
            output
        );
    }

    fputc(
        '\n',
        output
    );

    return 0;
}

static int compile_while(
    ASTNode *node
)
{
    NovaType condition_type;

    if (
        node == NULL ||
        node->child_count != 2 ||
        node->children[0] == NULL ||
        node->children[1] == NULL
    ) {
        return 1;
    }

    if (
        !expression_type(
            node->children[0],
            &condition_type
        )
    ) {
        return 1;
    }

    if (
        condition_type !=
        NOVA_TYPE_INT
    ) {
        printf(
            "NOVA TYPE ERROR: while condition must be int\n"
        );

        return 1;
    }

    fputs(
        "    while (",
        output
    );

    if (
        compile_expression(
            node->children[0]
        ) != 0
    ) {
        return 1;
    }

    fputs(
        ") {\n",
        output
    );

    if (
        compile_block(
            node->children[1]
        ) != 0
    ) {
        return 1;
    }

    fputs(
        "    }\n",
        output
    );

    return 0;
}

static int compile_call(
    ASTNode *node
)
{
    int i;

    if (
        node == NULL ||
        node->value == NULL
    ) {
        return 1;
    }

    /*
     * file.write(file, str)
     */

    if (
        strcmp(
            node->value,
            "file.write"
        ) == 0
    ) {
        if (
            node->child_count != 2
        ) {
            return 1;
        }

        fputs(
            "    nova_file_write(",
            output
        );

        for (
            i = 0;
            i < node->child_count;
            i++
        ) {
            if (i > 0) {
                fputs(
                    ", ",
                    output
                );
            }

            if (
                compile_expression(
                    node->children[i]
                ) != 0
            ) {
                return 1;
            }
        }

        fputs(
            ");\n",
            output
        );

        return 0;
    }

    /*
     * file.close(file)
     */

    if (
        strcmp(
            node->value,
            "file.close"
        ) == 0
    ) {
        if (
            node->child_count != 1
        ) {
            return 1;
        }

        fputs(
            "    nova_file_close(",
            output
        );

        if (
            compile_expression(
                node->children[0]
            ) != 0
        ) {
            return 1;
        }

        fputs(
            ");\n",
            output
        );

        return 0;
    }

    /*
     * tostr() is expression-only.
     */

    if (
        strcmp(
            node->value,
            "tostr"
        ) == 0
    ) {
        printf(
            "NOVA TYPE ERROR: tostr() must be used as an expression\n"
        );

        return 1;
    }

    /*
     * Normal function call.
     */

    fprintf(
        output,
        "    %s(",
        node->value
    );

    for (
        i = 0;
        i < node->child_count;
        i++
    ) {
        if (i > 0) {
            fputs(
                ", ",
                output
            );
        }

        if (
            compile_expression(
                node->children[i]
            ) != 0
        ) {
            return 1;
        }
    }

    fputs(
        ");\n",
        output
    );

    return 0;
}

static int compile_node(
    ASTNode *node
)
{
    int i;

    if (node == NULL) {
        return 1;
    }

    switch (node->type) {

        case AST_PROGRAM:

            for (
                i = 0;
                i < node->child_count;
                i++
            ) {
                if (
                    compile_node(
                        node->children[i]
                    ) != 0
                ) {
                    return 1;
                }
            }

            return 0;

        case AST_FUNCTION:

            return compile_function(
                node
            );

        case AST_BLOCK:

            return compile_block(
                node
            );

        case AST_PRINT:

            return compile_print(
                node
            );

        case AST_RETURN:

            if (
                node->child_count != 1
            ) {
                return 1;
            }

            fputs(
                "    return ",
                output
            );

            if (
                compile_expression(
                    node->children[0]
                ) != 0
            ) {
                return 1;
            }

            fputs(
                ";\n",
                output
            );

            return 0;

        case AST_VARIABLE_DECL:

            return compile_variable_decl(
                node
            );

        case AST_ASSIGNMENT:

            return compile_assignment(
                node
            );

        case AST_IF:

            return compile_if(
                node
            );

        case AST_WHILE:

            return compile_while(
                node
            );

        case AST_CALL:

            return compile_call(
                node
            );

        case AST_ENTRY_POINT:

            return 0;

        case AST_STRING:
        case AST_NUMBER:
        case AST_VARIABLE_REF:
        case AST_BINARY:
        case AST_UNARY:
        case AST_INPUT:

            return compile_expression(
                node
            );

        default:
            return 1;
    }
}

static int compile_block(
    ASTNode *node
)
{
    int i;

    if (
        node == NULL ||
        node->type != AST_BLOCK
    ) {
        return 1;
    }

    for (
        i = 0;
        i < node->child_count;
        i++
    ) {
        if (
            compile_node(
                node->children[i]
            ) != 0
        ) {
            return 1;
        }
    }

    return 0;
}

static int compile_function(
    ASTNode *node
)
{
    const char *name;

    int old_variable_count;
    int parameter_count;
    int i;

    ASTNode *parameter;
    ASTNode *block;

    if (
        node == NULL ||
        node->value == NULL ||
        node->child_count < 1
    ) {
        return 1;
    }

    name =
        node->value;

    block =
        node->children[
            node->child_count - 1
        ];

    if (
        block == NULL ||
        block->type != AST_BLOCK
    ) {
        return 1;
    }

    parameter_count =
        node->child_count - 1;

    old_variable_count =
        variable_count;

    variable_count = 0;

    /*
     * Function header.
     */

    if (
        strcmp(
            name,
            "main"
        ) == 0
    ) {
        fputs(
            "static void nova_main(void)\n",
            output
        );
    }
    else if (
        node->data_type ==
        NOVA_TYPE_INT
    ) {
        fprintf(
            output,
            "static int %s(",
            name
        );
    }
    else if (
        node->data_type ==
        NOVA_TYPE_STR
    ) {
        fprintf(
            output,
            "static char *%s(",
            name
        );
    }
    else if (
        node->data_type ==
        NOVA_TYPE_FILE
    ) {
        fprintf(
            output,
            "static FILE *%s(",
            name
        );
    }
    else {
        fprintf(
            output,
            "static void %s(",
            name
        );
    }

    /*
     * Parameters.
     */

    if (
        strcmp(
            name,
            "main"
        ) != 0
    ) {
        if (
            parameter_count == 0
        ) {
            fputs(
                "void",
                output
            );
        }
        else {

            for (
                i = 0;
                i < parameter_count;
                i++
            ) {
                parameter =
                    node->children[i];

                if (
                    parameter == NULL ||
                    parameter->type !=
                        AST_VARIABLE_DECL ||
                    parameter->value == NULL
                ) {
                    variable_count =
                        old_variable_count;

                    return 1;
                }

                if (i > 0) {
                    fputs(
                        ", ",
                        output
                    );
                }

                if (
                    parameter->data_type ==
                    NOVA_TYPE_STR
                ) {
                    fprintf(
                        output,
                        "char *%s",
                        parameter->value
                    );
                }
                else if (
                    parameter->data_type ==
                    NOVA_TYPE_FILE
                ) {
                    fprintf(
                        output,
                        "FILE *%s",
                        parameter->value
                    );
                }
                else {
                    fprintf(
                        output,
                        "int %s",
                        parameter->value
                    );
                }

                if (
                    !register_variable(
                        parameter->value,
                        parameter->data_type,
                        0
                    )
                ) {
                    printf(
                        "NOVA TYPE ERROR: parameter '%s' already exists\n",
                        parameter->value
                    );

                    variable_count =
                        old_variable_count;

                    return 1;
                }
            }
        }

        fputs(
            ")\n",
            output
        );
    }

    fputs(
        "{\n",
        output
    );

    /*
     * Compile function body.
     */

    if (
        compile_block(
            block
        ) != 0
    ) {
        variable_count =
            old_variable_count;

        return 1;
    }

    if (
        node->data_type ==
        NOVA_TYPE_VOID
    ) {
        fputs(
            "    return;\n",
            output
        );
    }

    fputs(
        "}\n\n",
        output
    );

    variable_count =
        old_variable_count;

    return 0;
}

static ASTNode *find_entry_point(
    ASTNode *root
)
{
    int i;

    if (root == NULL) {
        return NULL;
    }

    for (
        i = 0;
        i < root->child_count;
        i++
    ) {
        if (
            root->children[i] != NULL &&
            root->children[i]->type ==
                AST_ENTRY_POINT
        ) {
            return root->children[i];
        }
    }

    return NULL;
}

int compiler_compile(
    ASTNode *root,
    const char *output_file,
    int debug
)
{
    char generated_file[1024];

    ASTNode *entry;

    const char *entry_function;

    int i;
    int result;

    debug_enabled =
        debug;

    variable_count =
        0;

    function_count =
        0;

    output =
        NULL;

    if (
        root == NULL ||
        output_file == NULL
    ) {
        return 1;
    }

    if (
        root->type != AST_PROGRAM
    ) {
        return 1;
    }

    entry =
        find_entry_point(
            root
        );

    if (
        entry == NULL ||
        entry->child_count < 1 ||
        entry->children[0] == NULL ||
        entry->children[0]->value == NULL
    ) {
        return 1;
    }

    entry_function =
        entry->children[0]->value;

    /*
     * First pass:
     *
     * Register every function.
     */

    for (
        i = 0;
        i < root->child_count;
        i++
    ) {
        if (
            root->children[i] != NULL &&
            root->children[i]->type ==
                AST_FUNCTION
        ) {
            if (
                !register_function(
                    root->children[i]
                )
            ) {
                printf(
                    "NOVA COMPILER ERROR: failed to register function\n"
                );

                return 1;
            }
        }
    }

    /*
     * Create generated C file.
     */

    snprintf(
        generated_file,
        sizeof(generated_file),
        "%s.generated.c",
        output_file
    );

    output =
        fopen(
            generated_file,
            "w"
        );

    if (output == NULL) {
        return 1;
    }

    /*
     * Generated C headers.
     */

    fputs(
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "#include <string.h>\n\n",
        output
    );

    /*
     * NOVA input runtime.
     */

    fputs(
        "static int nova_input(void)\n"
        "{\n"
        "    int value;\n"
        "    scanf(\"%d\", &value);\n"
        "    return value;\n"
        "}\n\n",
        output
    );

    /*
     * NOVA integer -> string runtime.
     *
     * Used by:
     *
     * tostr(42)
     */

    fputs(
        "static char *nova_tostr(int value)\n"
        "{\n"
        "    char buffer[64];\n"
        "    char *result;\n"
        "    int length;\n"
        "\n"
        "    length = snprintf(\n"
        "        buffer,\n"
        "        sizeof(buffer),\n"
        "        \"%d\",\n"
        "        value\n"
        "    );\n"
        "\n"
        "    if (length < 0) {\n"
        "        return NULL;\n"
        "    }\n"
        "\n"
        "    result = malloc(\n"
        "        (size_t)length + 1\n"
        "    );\n"
        "\n"
        "    if (result == NULL) {\n"
        "        return NULL;\n"
        "    }\n"
        "\n"
        "    memcpy(\n"
        "        result,\n"
        "        buffer,\n"
        "        (size_t)length + 1\n"
        "    );\n"
        "\n"
        "    return result;\n"
        "}\n\n",
        output
    );

    /*
     * NOVA string concatenation runtime.
     */

    fputs(
        "static char *nova_concat(\n"
        "    const char *a,\n"
        "    const char *b\n"
        ")\n"
        "{\n"
        "    size_t a_len;\n"
        "    size_t b_len;\n"
        "    char *result;\n"
        "\n"
        "    if (a == NULL) {\n"
        "        a = \"\";\n"
        "    }\n"
        "\n"
        "    if (b == NULL) {\n"
        "        b = \"\";\n"
        "    }\n"
        "\n"
        "    a_len = strlen(a);\n"
        "    b_len = strlen(b);\n"
        "\n"
        "    result = malloc(\n"
        "        a_len + b_len + 1\n"
        "    );\n"
        "\n"
        "    if (result == NULL) {\n"
        "        fprintf(\n"
        "            stderr,\n"
        "            \"NOVA RUNTIME ERROR: string allocation failed\\n\"\n"
        "        );\n"
        "        exit(1);\n"
        "    }\n"
        "\n"
        "    memcpy(\n"
        "        result,\n"
        "        a,\n"
        "        a_len\n"
        "    );\n"
        "\n"
        "    memcpy(\n"
        "        result + a_len,\n"
        "        b,\n"
        "        b_len\n"
        "    );\n"
        "\n"
        "    result[a_len + b_len] = '\\0';\n"
        "\n"
        "    return result;\n"
        "}\n\n",
        output
    );

    /*
     * NOVA file.open runtime.
     */

    fputs(
        "static FILE *nova_file_open(const char *path)\n"
        "{\n"
        "    FILE *file;\n"
        "\n"
        "    if (path == NULL) {\n"
        "        return NULL;\n"
        "    }\n"
        "\n"
        "    file = fopen(path, \"r+\");\n"
        "\n"
        "    if (file == NULL) {\n"
        "        file = fopen(path, \"w+\");\n"
        "    }\n"
        "\n"
        "    return file;\n"
        "}\n\n",
        output
    );

    /*
     * NOVA file.read runtime.
     */

    fputs(
        "static char *nova_file_read(FILE *file)\n"
        "{\n"
        "    long size;\n"
        "    char *buffer;\n"
        "    size_t read_size;\n"
        "\n"
        "    if (file == NULL) {\n"
        "        return NULL;\n"
        "    }\n"
        "\n"
        "    fseek(file, 0, SEEK_END);\n"
        "\n"
        "    size = ftell(file);\n"
        "\n"
        "    if (size < 0) {\n"
        "        return NULL;\n"
        "    }\n"
        "\n"
        "    fseek(file, 0, SEEK_SET);\n"
        "\n"
        "    buffer = malloc(\n"
        "        (size_t)size + 1\n"
        "    );\n"
        "\n"
        "    if (buffer == NULL) {\n"
        "        return NULL;\n"
        "    }\n"
        "\n"
        "    read_size = fread(\n"
        "        buffer,\n"
        "        1,\n"
        "        (size_t)size,\n"
        "        file\n"
        "    );\n"
        "\n"
        "    buffer[read_size] = '\\0';\n"
        "\n"
        "    return buffer;\n"
        "}\n\n",
        output
    );

    /*
     * NOVA file.write runtime.
     */

    fputs(
        "static void nova_file_write(\n"
        "    FILE *file,\n"
        "    const char *text\n"
        ")\n"
        "{\n"
        "    if (file == NULL || text == NULL) {\n"
        "        return;\n"
        "    }\n"
        "\n"
        "    fputs(\n"
        "        text,\n"
        "        file\n"
        "    );\n"
        "\n"
        "    fflush(file);\n"
        "}\n\n",
        output
    );

    /*
     * NOVA file.close runtime.
     */

    fputs(
        "static void nova_file_close(FILE *file)\n"
        "{\n"
        "    if (file == NULL) {\n"
        "        return;\n"
        "    }\n"
        "\n"
        "    fclose(file);\n"
        "}\n\n",
        output
    );

    /*
     * Compile every NOVA function.
     */

    for (
        i = 0;
        i < root->child_count;
        i++
    ) {
        if (
            root->children[i] != NULL &&
            root->children[i]->type ==
                AST_FUNCTION
        ) {
            if (
                compile_function(
                    root->children[i]
                ) != 0
            ) {
                fclose(
                    output
                );

                output =
                    NULL;

                remove(
                    generated_file
                );

                return 1;
            }
        }
    }

    /*
     * Generated C main().
     */

    fputs(
        "int main(void)\n"
        "{\n",
        output
    );

    if (
        strcmp(
            entry_function,
            "main"
        ) == 0
    ) {
        fputs(
            "    nova_main();\n",
            output
        );
    }
    else {
        fprintf(
            output,
            "    %s();\n",
            entry_function
        );
    }

    fputs(
        "    return 0;\n"
        "}\n",
        output
    );

    fclose(
        output
    );

    output =
        NULL;

    /*
     * Compile generated C with GCC.
     */

    {
        char command[2048];

        snprintf(
            command,
            sizeof(command),
            "gcc \"%s\" -o \"%s\"",
            generated_file,
            output_file
        );

        if (debug_enabled) {
            printf(
                "NOVA DEBUG: %s\n",
                command
            );
        }

        result =
            system(
                command
            );

        if (debug_enabled) {
            printf(
                "NOVA DEBUG: GCC result = %d\n",
                result
            );
        }
    }

    /*
     * Delete temporary generated file.
     */

    remove(
        generated_file
    );

    return result;
}