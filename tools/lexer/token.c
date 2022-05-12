#include "../headers/global.h"

struct token *token_new(enum token_type type)
{
    struct token *new = calloc(1, sizeof(struct token));
    new->type = type;
    return new;
}

void token_free(struct token *token)
{
    if (token->type == TOKEN_WORD || token->type == TOKEN_RED
        || token->type == TOKEN_ASSWORD)
    {
        free(token->value);
    }
    free(token);
}
