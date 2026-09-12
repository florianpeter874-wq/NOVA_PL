#ifndef NOVA_COMPILER_H
#define NOVA_COMPILER_H

#include "ast.h"

int compiler_compile(ASTNode *root, const char *output_file);

#endif