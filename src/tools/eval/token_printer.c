#include "../headers/global.h"

enum parser_status main1_handler(char *commande)
{
    struct lexer *lexer = lexer_new(commande); // Vec cstring
    struct token *token = lexer_pop(lexer);
    while (token->type != TOKEN_EOF && token->type != TOKEN_ERROR)
    {
        if (token->type == TOKEN_WORD || token->type == TOKEN_ASSWORD)
        {
            printf("Word = %s\n", token->value);
        }
        else if (token->type == TOKEN_EOC)
        {
            printf(";\n");
        }
        else if (token->type == TOKEN_RED)
        {
            printf("red = %s\n", token->value);
        }
        else if (token->type == TOKEN_IF)
        {
            printf("if\n");
        }
        else if (token->type == TOKEN_ELSE)
        {
            printf("else\n");
        }
        else if (token->type == TOKEN_ELIF)
        {
            printf("else if \n");
        }
        else if (token->type == TOKEN_UNTIL)
        {
            printf("until\n");
        }
        else if (token->type == TOKEN_THEN)
        {
            printf("then\n");
        }
        else if (token->type == TOKEN_FI)
        {
            printf("fi\n");
        }
        else if (token->type == TOKEN_NEWLINE)
        {
            printf("\n");
        }
        else if (token->type == TOKEN_AND)
        {
            printf("&&\n");
        }
        else if (token->type == TOKEN_OR)
        {
            printf("||\n");
        }
        else if (token->type == TOKEN_DO)
        {
            printf("do \n");
        }
        else if (token->type == TOKEN_DONE)
        {
            printf("done \n");
        }
        else if (token->type == TOKEN_PIPE)
        {
            printf("| \n");
        }
        else if (token->type == TOKEN_ESPER)
        {
            printf("&\n");
        }
        else if (token->type == TOKEN_ACCOG)
        {
            printf("{\n");
        }
        else if (token->type == TOKEN_ACCOD)
        {
            printf("}\n");
        }
        else
        {
            printf("Type = %d\n", token->type);
        }
        token_free(token);
        token = lexer_pop(lexer);
    }
    if (token->type == TOKEN_ERROR)
        printf("Syntax error\n");
    if (token->type == TOKEN_EOF)
    {
        printf("EOF\n");
    }
    token_free(token);
    lexer_free(lexer);
    return PARSER_OK;
}
