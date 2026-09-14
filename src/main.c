
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/compiler.h"

static char *read_file(
    const char *filename
)
{
    FILE *file;
    long size;
    char *buffer;

    file = fopen(
        filename,
        "rb"
    );

    if (file == NULL) {
        fprintf(
            stderr,
            "NOVA error: cannot open '%s'.\n",
            filename
        );

        return NULL;
    }

    fseek(
        file,
        0,
        SEEK_END
    );

    size = ftell(file);

    if (size < 0) {
        fclose(file);
        return NULL;
    }

    fseek(
        file,
        0,
        SEEK_SET
    );

    buffer = malloc(
        (size_t)size + 1
    );

    if (buffer == NULL) {
        fclose(file);

        fprintf(
            stderr,
            "NOVA error: out of memory.\n"
        );

        return NULL;
    }

    if (fread(
            buffer,
            1,
            (size_t)size,
            file
        ) != (size_t)size) {

        free(buffer);
        fclose(file);

        fprintf(
            stderr,
            "NOVA error: failed to read '%s'.\n",
            filename
        );

        return NULL;
    }

    buffer[size] = '\0';

    fclose(file);

    return buffer;
}

static void print_usage(
    const char *program
)
{
    printf(
        "NOVA compiler\n\n"
        "Usage:\n"
        "  %s <input.nova> -o <output.exe>\n"
        "  %s <input.nova> -o <output.exe> --debug\n"
        "  %s -v\n"
        "  %s --version\n",
        program,
        program,
        program,
        program
    );
}

static void print_version(void)
{
    printf(
        "Nova 1.1\n"
        "the children of the Nova project.\n"
        "Built by Netfloor Software Corporation\n"
    );
}

int main(
    int argc,
    char **argv
)
{
    const char *input_file;
    const char *output_file;

    char *source;

    Token *tokens;
    int token_count;

    ASTNode *root;

    int result;
    int debug;

    int i;

    input_file = NULL;
    output_file = NULL;
    debug = 0;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(
                argv[i],
                "-v"
            ) == 0 ||
            strcmp(
                argv[i],
                "--version"
            ) == 0) {

            print_version();
            return 0;
        }

        if (strcmp(
                argv[i],
                "--debug"
            ) == 0) {

            debug = 1;
            continue;
        }

        if (input_file == NULL) {
            input_file = argv[i];
            continue;
        }

        if (strcmp(
                argv[i],
                "-o"
            ) == 0) {

            if (i + 1 >= argc) {
                fprintf(
                    stderr,
                    "NOVA error: output file is required after -o.\n"
                );

                return 1;
            }

            output_file = argv[i + 1];
            i++;

            continue;
        }

        fprintf(
            stderr,
            "NOVA error: unknown argument '%s'.\n",
            argv[i]
        );

        return 1;
    }

    if (input_file == NULL) {
        print_usage(argv[0]);
        return 1;
    }

    if (output_file == NULL) {
        fprintf(
            stderr,
            "NOVA error: output file is required.\n\n"
        );

        print_usage(argv[0]);

        return 1;
    }

    source = read_file(
        input_file
    );

    if (source == NULL) {
        return 1;
    }

    printf(
        "NOVA: compiling %s\n",
        input_file
    );

    tokens = lexer_tokenize(
        source,
        &token_count
    );

    if (tokens == NULL) {
        free(source);

        fprintf(
            stderr,
            "NOVA error: lexical analysis failed.\n"
        );

        return 1;
    }

    root = parser_parse(
        tokens,
        token_count
    );

    if (root == NULL) {
        lexer_free_tokens(
            tokens,
            token_count
        );

        free(source);

        fprintf(
            stderr,
            "NOVA error: parsing failed.\n"
        );

        return 1;
    }

    result = compiler_compile(
        root,
        output_file,
        debug
    );

    ast_free(root);

    lexer_free_tokens(
        tokens,
        token_count
    );

    free(source);

    if (result != 0) {
        fprintf(
            stderr,
            "NOVA error: compilation failed.\n"
        );

        return 1;
    }

    printf(
        "NOVA: output -> %s\n",
        output_file
    );

    return 0;
}

