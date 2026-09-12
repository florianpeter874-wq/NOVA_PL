#ifndef NOVA_AST_H
#define NOVA_AST_H

typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_BLOCK,
    AST_PRINT,
    AST_STRING,

    AST_IF,
    AST_ELSE,
    AST_BINARY,
    AST_NUMBER,

    AST_ENTRY_POINT,
    AST_CALL,

    AST_VARIABLE_DECL,
    AST_VARIABLE_REF
} ASTNodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    ASTNodeType type;

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