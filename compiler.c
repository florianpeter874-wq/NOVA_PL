#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/compiler.h"

static FILE *output;

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

static int expression_is_string(
    ASTNode *node
)
{
    if (node == NULL) {
        return 0;
    }

    if (node->type == AST_STRING) {
        return 1;
    }

    if (node->type == AST_VARIABLE_REF) {
        return 0;
    }

    if (node->type == AST_BINARY &&
        node->value != NULL &&
        strcmp(node->value, "+") == 0 &&
        node->child_count >= 1) {

        return expression_is_string(
            node->children[0]
        );
    }

    return 0;
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
        if (*p == '"' &&
            p[1] == '\0') {

            break;
        }

        if (*p == '\\') {
            p++;

            if (*p == 'n') {
                fputs("\\n", output);
            }
            else if (*p == 't') {
                fputs("\\t", output);
            }
            else if (*p == 'r') {
                fputs("\\r", output);
            }
            else if (*p == '\\') {
                fputs("\\\\", output);
            }
            else if (*p == '"') {
                fputs("\\\"", output);
            }
            else {
                fputc('\\', output);
                fputc(*p, output);
            }

            p++;

            continue;
        }

        if (*p == '"') {
            fputs("\\\"", output);
        }
        else {
            fputc(*p, output);
        }

        p++;
    }
}

static int compile_expression(
    ASTNode *node
)
{
    if (node == NULL) {
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

            fputc('"', output);

            write_c_string(
                node->value
            );

            fputc('"', output);

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

            return 0;

        case AST_BINARY:
            if (node->child_count != 2 ||
                node->value == NULL) {

                return 1;
            }

            fputc('(', output);

            if (compile_expression(
                    node->children[0]
                ) != 0) {

                return 1;
            }

            fprintf(
                output,
                " %s ",
                node->value
            );

            if (compile_expression(
                    node->children[1]
                ) != 0) {

                return 1;
            }

            fputc(')', output);

            return 0;

        default:
            return 1;
    }
}

static int compile_variable_decl(
    ASTNode *node
)
{
    if (node == NULL ||
        node->value == NULL ||
        node->child_count != 1) {

        return 1;
    }

    fprintf(
        output,
        "    int %s = ",
        node->value
    );

    if (compile_expression(
            node->children[0]
        ) != 0) {

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

    if (node == NULL ||
        node->child_count != 1) {

        return 1;
    }

    expression = node->children[0];

    if (expression == NULL) {
        return 1;
    }

    /*
     * print("Hello");
     */
    if (expression->type == AST_STRING) {
        fputs(
            "    printf(",
            output
        );

        fputc('"', output);

        write_c_string(
            expression->value
        );

        fputc('"', output);

        fputs(
            ");\n",
            output
        );

        return 0;
    }

    /*
     * print(x);
     */
    if (expression->type == AST_VARIABLE_REF) {
        fputs(
            "    printf(\"%d\\n\", ",
            output
        );

        if (compile_expression(
                expression
            ) != 0) {

            return 1;
        }

        fputs(
            ");\n",
            output
        );

        return 0;
    }

    /*
     * print(123);
     */
    if (expression->type == AST_NUMBER) {
        fputs(
            "    printf(\"%d\\n\", ",
            output
        );

        if (compile_expression(
                expression
            ) != 0) {

            return 1;
        }

        fputs(
            ");\n",
            output
        );

        return 0;
    }

    /*
     * print("a szam:" + x);
     */
    if (expression->type == AST_BINARY &&
        expression->value != NULL &&
        strcmp(
            expression->value,
            "+"
        ) == 0 &&
        expression->child_count == 2) {

        ASTNode *left;
        ASTNode *right;

        left = expression->children[0];
        right = expression->children[1];

        /*
         * String + int
         */
        if (left != NULL &&
            left->type == AST_STRING) {

            fputs(
                "    printf(\"",
                output
            );

            write_c_string(
                left->value
            );

            fputs(
                "%d\\n\", ",
                output
            );

            if (compile_expression(
                    right
                ) != 0) {

                return 1;
            }

            fputs(
                ");\n",
                output
            );

            return 0;
        }
    }

    /*
     * Numeric expression
     */
    fputs(
        "    printf(\"%d\\n\", ",
        output
    );

    if (compile_expression(
            expression
        ) != 0) {

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
    if (node == NULL ||
        node->child_count < 2) {

        return 1;
    }

    fputs(
        "    if (",
        output
    );

    if (compile_expression(
            node->children[0]
        ) != 0) {

        return 1;
    }

    fputs(
        ") {\n",
        output
    );

    if (compile_block(
            node->children[1]
        ) != 0) {

        return 1;
    }

    fputs(
        "    }",
        output
    );

    if (node->child_count >= 3 &&
        node->children[2] != NULL) {

        fputs(
            " else {\n",
            output
        );

        if (compile_block(
                node->children[2]
            ) != 0) {

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

static int compile_call(
    ASTNode *node
)
{
    if (node == NULL ||
        node->value == NULL) {

        return 1;
    }

    fprintf(
        output,
        "    %s();\n",
        node->value
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
            for (i = 0; i < node->child_count; i++) {
                if (compile_node(
                        node->children[i]
                    ) != 0) {

                    return 1;
                }
            }

            return 0;

        case AST_FUNCTION:
            return compile_function(node);

        case AST_BLOCK:
            return compile_block(node);

        case AST_PRINT:
            return compile_print(node);

        case AST_VARIABLE_DECL:
            return compile_variable_decl(node);

        case AST_IF:
            return compile_if(node);

        case AST_CALL:
            return compile_call(node);

        case AST_ENTRY_POINT:
            return 0;

        case AST_STRING:
        case AST_NUMBER:
        case AST_VARIABLE_REF:
        case AST_BINARY:
            return compile_expression(node);

        default:
            return 1;
    }
}

static int compile_block(
    ASTNode *node
)
{
    int i;

    if (node == NULL ||
        node->type != AST_BLOCK) {

        return 1;
    }

    for (i = 0; i < node->child_count; i++) {
        if (compile_node(
                node->children[i]
            ) != 0) {

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

    if (node == NULL ||
        node->value == NULL ||
        node->child_count != 1) {

        return 1;
    }

    name = node->value;

    if (strcmp(
            name,
            "main"
        ) == 0) {

        fputs(
            "static void nova_main(void)\n",
            output
        );
    }
    else {
        fprintf(
            output,
            "static void %s(void)\n",
            name
        );
    }

    fputs(
        "{\n",
        output
    );

    if (compile_block(
            node->children[0]
        ) != 0) {

        return 1;
    }

    fputs(
        "}\n\n",
        output
    );

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

    for (i = 0; i < root->child_count; i++) {
        if (root->children[i] != NULL &&
            root->children[i]->type ==
                AST_ENTRY_POINT) {

            return root->children[i];
        }
    }

    return NULL;
}

int compiler_compile(
    ASTNode *root,
    const char *output_file
)
{
    char generated_file[1024];

    ASTNode *entry;

    const char *entry_function;

    int i;
    int result;

    if (root == NULL ||
        output_file == NULL) {

        return 1;
    }

    if (root->type != AST_PROGRAM) {
        return 1;
    }

    entry = find_entry_point(root);

    if (entry == NULL ||
        entry->child_count < 1 ||
        entry->children[0] == NULL ||
        entry->children[0]->value == NULL) {

        return 1;
    }

    entry_function =
        entry->children[0]->value;

    snprintf(
        generated_file,
        sizeof(generated_file),
        "%s.generated.c",
        output_file
    );

    output = fopen(
        generated_file,
        "w"
    );

    if (output == NULL) {
        return 1;
    }

    fputs(
        "#include <stdio.h>\n\n",
        output
    );

    for (i = 0; i < root->child_count; i++) {
        if (root->children[i] != NULL &&
            root->children[i]->type ==
                AST_FUNCTION) {

            if (compile_function(
                    root->children[i]
                ) != 0) {

                fclose(output);
                remove(generated_file);

                return 1;
            }
        }
    }

    fputs(
        "int main(void)\n"
        "{\n",
        output
    );

    if (strcmp(
            entry_function,
            "main"
        ) == 0) {

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

    fclose(output);

    {
        char command[2048];

        snprintf(
            command,
            sizeof(command),
            "gcc \"%s\" -o \"%s\"",
            generated_file,
            output_file
        );

        result = system(command);
    }

    remove(generated_file);

    return result;
}