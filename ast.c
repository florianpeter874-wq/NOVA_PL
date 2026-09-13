#include <stdlib.h>
#include <string.h>

#include "../include/ast.h"

ASTNode *ast_create(
    ASTNodeType type,
    const char *value
)
{
    ASTNode *node;

    node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        return NULL;
    }

    node->type = type;

    node->data_type = NOVA_TYPE_INT;

    node->is_array = 0;

    node->value = NULL;

    if (value != NULL) {
        node->value = malloc(
            strlen(value) + 1
        );

        if (node->value == NULL) {
            free(node);
            return NULL;
        }

        strcpy(
            node->value,
            value
        );
    }

    node->children = NULL;
    node->child_count = 0;

    return node;
}

void ast_add_child(
    ASTNode *parent,
    ASTNode *child
)
{
    ASTNode **new_children;

    if (
        parent == NULL ||
        child == NULL
    ) {
        return;
    }

    new_children = realloc(
        parent->children,
        sizeof(ASTNode *) *
        (parent->child_count + 1)
    );

    if (new_children == NULL) {
        return;
    }

    parent->children = new_children;

    parent->children[
        parent->child_count
    ] = child;

    parent->child_count++;
}

void ast_free(
    ASTNode *node
)
{
    int i;

    if (node == NULL) {
        return;
    }

    for (
        i = 0;
        i < node->child_count;
        i++
    ) {
        ast_free(
            node->children[i]
        );
    }

    free(node->children);
    free(node->value);
    free(node);
}