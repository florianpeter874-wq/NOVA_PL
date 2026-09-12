#include <stdlib.h>
#include <string.h>

#include "../include/ast.h"

static char *copy_string(const char *text)
{
    if (text == NULL)
        return NULL;

    size_t length = strlen(text);

    char *result = malloc(length + 1);

    if (result == NULL)
        return NULL;

    memcpy(result, text, length + 1);

    return result;
}

ASTNode *ast_create(ASTNodeType type, const char *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
        return NULL;

    node->type = type;
    node->value = copy_string(value);

    node->children = NULL;
    node->child_count = 0;

    return node;
}

void ast_add_child(ASTNode *parent, ASTNode *child)
{
    if (parent == NULL || child == NULL)
        return;

    ASTNode **new_children =
        realloc(
            parent->children,
            sizeof(ASTNode *) * (parent->child_count + 1)
        );

    if (new_children == NULL)
        return;

    parent->children = new_children;

    parent->children[parent->child_count] = child;
    parent->child_count++;
}

void ast_free(ASTNode *node)
{
    if (node == NULL)
        return;

    for (int i = 0; i < node->child_count; i++)
        ast_free(node->children[i]);

    free(node->children);
    free(node->value);
    free(node);
}
