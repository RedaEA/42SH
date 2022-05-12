#include "../headers/global.h"

enum parser_status main_handler(char *line, struct ast_function **list,
                                struct ast_variable **list_variables)
{
    struct lexer *lexer = lexer_new(line);
    struct ast *final_ast = NULL;
    enum parser_status status = input(&final_ast, lexer);

    if (status == PARSER_OK)
    {
        status = 0;
        if (final_ast)
        {
            status = ast_execution(final_ast, list, list_variables, 0);
        }
        ast_free(final_ast);
        lexer_free(lexer);
        if (status == 200 || status == 300)
        {
            return 0;
        }
        return status;
    }
    else
    {
        fprintf(stderr, "unexpected token\n");
        fflush(stderr);
        status = 2;
    }
    lexer_free(lexer);
    return status;
}
