#include "../headers/global.h"
#ifndef EXECUTION_H
#    define EXECUTION_H

int ast_execution(struct ast *tree, struct ast_function **list,
                  struct ast_variable **variables, int type_loop);

#endif
