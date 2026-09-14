#ifndef NOVA_AST_H
#define NOVA_AST_H

typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_BLOCK,
    AST_PRINT,
    AST_RETURN,
    AST_STRING,

    AST_IF,
    AST_ELSE,
    AST_WHILE,
    AST_BINARY,
    AST_UNARY,
    AST_NUMBER,

    AST_ENTRY_POINT,
    AST_CALL,
    AST_INPUT,

    AST_VARIABLE_DECL,
    AST_VARIABLE_REF,
    AST_ASSIGNMENT
} ASTNodeType;

typedef enum {
    NOVA_TYPE_VOID,
    NOVA_TYPE_INT,
    NOVA_TYPE_STR,
    NOVA_TYPE_FILE
} NovaType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    ASTNodeType type;

    NovaType data_type;

    int is_array;

    char *value;

    ASTNode **children;
    int child_count;
};

ASTNode *ast_create(
    ASTNodeType type,
    const char *value
);

void ast_add_child(
    ASTNode *parent,
    ASTNode *child
);

void ast_free(
    ASTNode *node
);

#endif
