#include "../headers/global.h"
char *word(struct token *tok);
enum parser_status compound_list(struct ast **ast, struct lexer *lexer);
enum parser_status command(struct ast **ast, struct lexer *lexer);
enum parser_status simple_command(struct ast **ast, struct lexer *lexer);
enum parser_status rule_if(struct ast **ast, struct lexer *lexer);
enum parser_status else_clause(struct ast **ast, struct lexer *lexer);
enum parser_status shell_command(struct ast **ast, struct lexer *lexer);
enum parser_status prefix(struct ast **ast, struct ast_command **node_sc,
                          struct lexer *lexer);
enum parser_status redirection(struct ast **ast, struct lexer *lexer);
enum parser_status element(struct ast **ast, struct ast_command **node_sc,
                           struct lexer *lexer);
enum parser_status do_group(struct ast **ast, struct lexer *lexer);
enum parser_status rule_while(struct ast **ast, struct lexer *lexer);
enum parser_status rule_until(struct ast **ast, struct lexer *lexer);
enum parser_status handle_parse_error(enum parser_status status,
                                      struct ast **res)
{
    *res = NULL;
    warnx("unexpected token");
    return status;
}
/*
input: | list \n
| list EOF
| EOF
| \n
*/
enum parser_status input(struct ast **ast, struct lexer *lexer)
{
    struct token *tok = lexer_peek(lexer);
    if (tok->type == TOKEN_EOF || tok->type == TOKEN_NEWLINE)
    {
        token_free(tok);
        tok = NULL;
        return PARSER_OK;
    }
    token_free(tok);
    tok = NULL;
    *ast = ast_init(NODE_NOYAU);
    enum parser_status status = list(ast, lexer);
    if (status != PARSER_OK)
    {
        ast_free(*ast);
        return status;
    }
    if ((tok = lexer_peek(lexer))->type == TOKEN_EOF
        || tok->type == TOKEN_NEWLINE)
    {
        token_free(tok);
        return PARSER_OK;
    }
    ast_free(*ast);
    token_free(tok);
    return PARSER_UNEXPECTED_TOKEN;
}

/*
  list: and_or ((';'|'&') and_or)* [';'|'&']
*/
enum parser_status list(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    enum parser_status status = and_or(ast, lexer);
    if (status != PARSER_OK)
    {
        return status;
    }
    while (true)
    {
        tok = lexer_peek(lexer);
        if (tok->type == TOKEN_EOC || tok->type == TOKEN_ESPER)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            status = and_or(ast, lexer);
            if (status != PARSER_OK)
            {
                return PARSER_OK;
            }
        }
        else
        {
            token_free(tok);
            return PARSER_OK;
        }
    }
}
/*
  and_or: pipeline (('&&'|'||') ('\n')* pipeline)*
*/

enum parser_status and_or(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    enum parser_status status = pipeline(ast, lexer);
    if (status != PARSER_OK)
    {
        return status;
    }
    while (true)
    {
        tok = lexer_peek(lexer);
        if (tok->type == TOKEN_AND || tok->type == TOKEN_OR)
        {
            // INITIALISATION DES NOEUDS
            struct ast *new_node_and_or;
            if (tok->type == TOKEN_OR)
                new_node_and_or = ast_init(NODE_OR);
            else
                new_node_and_or = ast_init(NODE_AND);
            new_node_and_or->data.and_or = calloc(1, sizeof(struct ast_and_or));
            new_node_and_or->data.and_or->left_NY = ast_init(NODE_NOYAU);
            new_node_and_or->data.and_or->right_NY = ast_init(NODE_NOYAU);
            // FIN INITIALISATION
            token_free(tok);
            token_free(lexer_pop(lexer));
            int pos = (*ast)->nb_childrens - 1;
            if ((*ast)->childrens[pos]->type == NODE_FI)
            {
                while ((*ast)->childrens[pos]->type != NODE_IF)
                {
                    pos--;
                }
                while ((*ast)->childrens[pos]->type != NODE_FI)
                {
                    new_node_and_or->data.and_or->left_NY->childrens
                        [new_node_and_or->data.and_or->left_NY->nb_childrens] =
                        (*ast)->childrens[pos];
                    new_node_and_or->data.and_or->left_NY->nb_childrens += 1;
                    pos++;
                    (*ast)->nb_childrens -= 1;
                }
                new_node_and_or->data.and_or->left_NY
                    ->childrens[new_node_and_or->data.and_or->left_NY
                                    ->nb_childrens] = (*ast)->childrens[pos];
                new_node_and_or->data.and_or->left_NY->nb_childrens += 1;
                pos++;
                (*ast)->nb_childrens -= 1;
            } // rajouter le meme comportement pour while et for, ..
            else
            {
                new_node_and_or->data.and_or->left_NY->childrens[0] =
                    (*ast)->childrens[pos];
                (*ast)->nb_childrens -= 1;
                new_node_and_or->data.and_or->left_NY->nb_childrens += 1;
            }
            tok = lexer_peek(lexer);

            while (tok->type == TOKEN_NEWLINE)
            {
                token_free(tok);
                token_free(lexer_pop(lexer));
                tok = lexer_peek(lexer);
            }
            token_free(tok);

            status = pipeline(&new_node_and_or->data.and_or->right_NY, lexer);

            if (status != PARSER_OK)
            {
                free_and_or(new_node_and_or);
                free(new_node_and_or);
                return PARSER_UNEXPECTED_TOKEN;
            }
            (*ast)->childrens[(*ast)->nb_childrens] = new_node_and_or;
            (*ast)->nb_childrens += 1;
        }
        else
        {
            token_free(tok);
            return PARSER_OK;
        }
    }
}

/*
  pipeline :['!'] command ('|' ('\n')* command)*
*/
enum parser_status pipeline(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    tok = lexer_peek(lexer);
    enum parser_status status;
    if (tok->type == TOKEN_NEG)
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
        // INITIALISATION DES NOEUDS
        struct ast *new_neg_node = ast_init(NODE_NEG);
        new_neg_node->data.negation = calloc(1, sizeof(struct ast_negation));
        new_neg_node->data.negation->node_NY = ast_init(NODE_NOYAU);
        status = command(&new_neg_node->data.negation->node_NY, lexer);
        if (status != PARSER_OK)
        {
            free_negation(new_neg_node);
            free(new_neg_node);
            return status;
        }
        (*ast)->childrens[(*ast)->nb_childrens] = new_neg_node;
        (*ast)->nb_childrens += 1;
    }
    else
    {
        token_free(tok);
        status = command(ast, lexer);
        if (status != PARSER_OK)
        {
            return status;
        }
    }
    while (true)
    {
        if ((tok = lexer_peek(lexer))->type != TOKEN_PIPE)
        {
            token_free(tok);
            return PARSER_OK;
        }
        token_free(tok);
        token_free(lexer_pop(lexer));
        if ((*ast)->nb_childrens < 1)
        {
            return PARSER_UNEXPECTED_TOKEN;
        }
        struct ast *new_node_pipe = ast_init(NODE_PIPE);
        new_node_pipe->data.pipe = calloc(1, sizeof(struct ast_pipe));
        new_node_pipe->data.pipe->left_NY = ast_init(NODE_NOYAU);
        new_node_pipe->data.pipe->right_NY = ast_init(NODE_NOYAU);
        int pos = (*ast)->nb_childrens - 1;
        if ((*ast)->childrens[pos]->type == NODE_FI)
        {
            while ((*ast)->childrens[pos]->type != NODE_IF)
            {
                pos--;
            }
            while ((*ast)->childrens[pos]->type != NODE_FI)
            {
                new_node_pipe->data.pipe->left_NY
                    ->childrens[new_node_pipe->data.pipe->left_NY
                                    ->nb_childrens] = (*ast)->childrens[pos];
                new_node_pipe->data.pipe->left_NY->nb_childrens += 1;
                pos++;
                (*ast)->nb_childrens -= 1;
            }
            new_node_pipe->data.pipe->left_NY
                ->childrens[new_node_pipe->data.pipe->left_NY->nb_childrens] =
                (*ast)->childrens[pos];
            new_node_pipe->data.pipe->left_NY->nb_childrens += 1;
            pos++;
            (*ast)->nb_childrens -= 1;
        }
        else
        {
            new_node_pipe->data.pipe->left_NY->childrens[0] =
                (*ast)->childrens[pos];
            (*ast)->nb_childrens -= 1;
            new_node_pipe->data.pipe->left_NY->nb_childrens += 1;
        }
        tok = lexer_peek(lexer);
        while (tok->type == TOKEN_NEWLINE)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
        }
        token_free(tok);
        status = command(&new_node_pipe->data.pipe->right_NY, lexer);
        if (status != PARSER_OK)
        {
            free_pipe(new_node_pipe);
            free(new_node_pipe);
            return PARSER_UNEXPECTED_TOKEN;
        }
        (*ast)->childrens[(*ast)->nb_childrens] = new_node_pipe;
        (*ast)->nb_childrens += 1;
    }
}
/*
command: simple_command
| shell_command (redirection)*
| funcdec (redirection)*
*/

enum parser_status command(struct ast **ast, struct lexer *lexer)
{
    int inter_pos = lexer->pos;
    struct token *inter_tok = lexer->current_tok;
    struct ast *noyau_red = ast_init(NODE_NOYAU);
    enum parser_status status = shell_command(ast, lexer);
    if (status != PARSER_OK)
    {
        lexer->pos = inter_pos;
        lexer->current_tok = inter_tok;
        status = funcdec(ast, lexer);
        if (status != PARSER_OK)
        {
            lexer->pos = inter_pos;
            status = simple_command(ast, lexer);
            if (status != PARSER_OK)
            {
                ast_free(noyau_red);
                return status;
            }
            else
            {
                ast_free(noyau_red);
                return PARSER_OK;
            }
        }
        while (true)
        {
            status = redirection(&noyau_red, lexer);
            if (status != PARSER_OK)
            {
                goto end_redirection;
            }
        }
    }
    else
    {
        while (true)
        {
            status = redirection(&noyau_red, lexer);
            if (status != PARSER_OK)
            {
                goto end_redirection;
            }
        }
    }
end_redirection:
    if (noyau_red->nb_childrens > 0)
    {
        int pos = (*ast)->nb_childrens - 1;
        if ((*ast)->childrens[pos]->type == NODE_FI)
        {
            while ((*ast)->childrens[pos]->type != NODE_IF)
            {
                pos--;
            }
            while ((*ast)->childrens[pos]->type != NODE_FI)
            {
                noyau_red->childrens[0]
                    ->data.redirection->left_NY
                    ->childrens[noyau_red->childrens[0]
                                    ->data.redirection->left_NY->nb_childrens] =
                    (*ast)->childrens[pos];
                noyau_red->childrens[0]
                    ->data.redirection->left_NY->nb_childrens += 1;
                pos++;
                (*ast)->nb_childrens -= 1;
            }
            noyau_red->childrens[0]
                ->data.redirection->left_NY
                ->childrens[noyau_red->childrens[0]
                                ->data.redirection->left_NY->nb_childrens] =
                (*ast)->childrens[pos];
            noyau_red->childrens[0]->data.redirection->left_NY->nb_childrens +=
                1;
            pos++;
            (*ast)->nb_childrens -= 1;
        }
        else
        {
            noyau_red->childrens[0]->data.redirection->left_NY->childrens[0] =
                (*ast)->childrens[pos];
            (*ast)->nb_childrens -= 1;
            noyau_red->childrens[0]->data.redirection->left_NY->nb_childrens +=
                1;
        }
        int i1 = 0;
        int inter = noyau_red->nb_childrens;
        for (int i = 1; i < inter; i++)
        {
            noyau_red->childrens[i]->data.redirection->left_NY->childrens[0] =
                noyau_red->childrens[i1];
            noyau_red->childrens[i]->data.redirection->left_NY->nb_childrens +=
                1;
            i1++;
        }
        (*ast)->childrens[(*ast)->nb_childrens] =
            noyau_red->childrens[noyau_red->nb_childrens - 1];
        (*ast)->nb_childrens += 1;
        free(noyau_red->childrens);
        free(noyau_red);
    }
    else
    {
        ast_free(noyau_red);
    }
    return PARSER_OK;
}

/*
simple_command : (prefix)+
| (prefix)* (element)+
*/
enum parser_status simple_command(struct ast **ast, struct lexer *lexer)
{
    int inter = lexer->pos;
    struct token *inter_tok = lexer->current_tok;
    struct ast *noyau_red = ast_init(NODE_NOYAU);
    struct ast *node_sc = ast_init(NODE_COMMAND);
    node_sc->data.command = calloc(1, sizeof(struct ast_command));
    node_sc->data.command->args = calloc(strlen(lexer->input), sizeof(char *));
    enum parser_status status =
        prefix(&noyau_red, &node_sc->data.command, lexer);
    if (status != PARSER_OK)
    {
        lexer->current_tok = inter_tok;
        lexer->pos = inter;
        status = element(&noyau_red, &node_sc->data.command, lexer);
        if (status != PARSER_OK)
        {
            ast_free(noyau_red);
            free_command(node_sc);
            free(node_sc);
            return PARSER_UNEXPECTED_TOKEN;
        }
        while (true)
        {
            status = element(&noyau_red, &node_sc->data.command, lexer);
            if (status != PARSER_OK)
            {
                goto end_simplecommand;
            }
        }
    }
    while (true)
    {
        status = prefix(&noyau_red, &node_sc->data.command, lexer);
        if (status != PARSER_OK)
        {
            status = element(&noyau_red, &node_sc->data.command, lexer);
            if (status != PARSER_OK)
            {
                goto end_simplecommand;
            }
            while (true)
            {
                status = element(&noyau_red, &node_sc->data.command, lexer);
                if (status != PARSER_OK)
                {
                    goto end_simplecommand;
                }
            }
        }
    }
end_simplecommand:
    if (noyau_red->nb_childrens > 0)
    {
        noyau_red->childrens[0]->data.redirection->left_NY->childrens[0] =
            node_sc;
        noyau_red->childrens[0]->data.redirection->left_NY->nb_childrens += 1;
        int i1 = 0;
        int inter = noyau_red->nb_childrens;
        for (int i = 1; i < inter; i++)
        {
            noyau_red->childrens[i]->data.redirection->left_NY->childrens[0] =
                noyau_red->childrens[i1];
            noyau_red->childrens[i]->data.redirection->left_NY->nb_childrens +=
                1;
            i1++;
        }
        (*ast)->childrens[(*ast)->nb_childrens] =
            noyau_red->childrens[noyau_red->nb_childrens - 1];
        (*ast)->nb_childrens += 1;
        free(noyau_red->childrens);
        free(noyau_red);
    }
    else
    {
        ast_free(noyau_red);
        (*ast)->childrens[(*ast)->nb_childrens] = node_sc;
        (*ast)->nb_childrens += 1;
    }
    return PARSER_OK;
}
/* shell_command : '{' compound_list '}'
   | '(' compound_list ')'
   | rule_for
   | rule_while
   | rule_until
   | rule_case
   | rule_if
*/

enum parser_status shell_command(struct ast **ast, struct lexer *lexer)
{
    int inter = lexer->pos;
    enum parser_status status = rule_if(ast, lexer);
    if (status != PARSER_OK)
    {
        lexer->pos = inter;
        status = rule_case(ast, lexer);
        if (status != PARSER_OK)
        {
            lexer->pos = inter;
            status = rule_while(ast, lexer);
            if (status != PARSER_OK)
            {
                lexer->pos = inter;
                status = rule_until(ast, lexer);
                if (status != PARSER_OK)
                {
                    lexer->pos = inter;
                    status = rule_for(ast, lexer);
                    if (status != PARSER_OK)
                    {
                        struct token *tok = lexer_peek(lexer);
                        if (tok->type == TOKEN_ACCOG)
                        {
                            token_free(tok);
                            token_free(lexer_pop(lexer));
                            status = compound_list(ast, lexer);
                            if (status != PARSER_OK)
                            {
                                return PARSER_UNEXPECTED_TOKEN;
                            }
                            tok = lexer_peek(lexer);
                            if (tok->type != TOKEN_ACCOD)
                            {
                                token_free(tok);
                                return PARSER_UNEXPECTED_TOKEN;
                            }
                            token_free(tok);
                            token_free(lexer_pop(lexer));
                            return PARSER_OK;
                        }
                        if (tok->type == TOKEN_PARG)
                        {
                            token_free(tok);
                            token_free(lexer_pop(lexer));
                            status = compound_list(ast, lexer);
                            if (status != PARSER_OK)
                            {
                                return PARSER_UNEXPECTED_TOKEN;
                            }
                            tok = lexer_peek(lexer);
                            if (tok->type != TOKEN_PARD)
                            {
                                token_free(tok);
                                return PARSER_UNEXPECTED_TOKEN;
                            }
                            token_free(tok);
                            token_free(lexer_pop(lexer));
                            return PARSER_OK;
                        }
                        token_free(tok);
                        return PARSER_UNEXPECTED_TOKEN;
                    }
                }
            }
        }
    }
    return PARSER_OK;
}

/*
funcdec: WORD '(' ')' ('\n')* shell_command
*/

enum parser_status funcdec(struct ast **ast, struct lexer *lexer)
{
    struct token *tok = lexer_peek(lexer);
    if (tok->type != TOKEN_WORD)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    // INITIALSATION DES NOEUDS
    struct ast *function = ast_init(NODE_FUNCTION);
    function->data.function = calloc(1, sizeof(struct ast_function));
    function->data.function->name = strdup(tok->value);
    function->data.function->func_NY = ast_init(NODE_NOYAU);
    // FIN D'INITIALISATION
    token_free(tok);
    token_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_PARG)
    {
        token_free(tok);
        free_function(function);
        free(function);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_PARD)
    {
        token_free(tok);
        free_function(function);
        free(function);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
    while (tok->type == TOKEN_NEWLINE)
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
    }
    token_free(tok);
    enum parser_status status =
        shell_command(&function->data.function->func_NY, lexer);
    if (status != PARSER_OK)
    {
        free_function(function);
        free(function);
        return PARSER_UNEXPECTED_TOKEN;
    }
    (*ast)->childrens[(*ast)->nb_childrens] = function;
    (*ast)->nb_childrens += 1;
    return PARSER_OK;
}

/*
redirection: [IONUMBER] '>' WORD
| [IONUMBER] '<' WORD
| [IONUMBER] '>&' WORD
| [IONUMBER] '<&' WORD
| [IONUMBER] '>>' WORD
| [IONUMBER] '<>' WORD
| [IONUMBER] '>|' WORD
*/

enum parser_status redirection(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    // INITIALISATION DES NOEUDS PRINCIPAUX
    struct ast *red_node = ast_init(NODE_RED);
    red_node->data.redirection = calloc(1, sizeof(struct ast_red));
    red_node->data.redirection->io_number = -1;
    red_node->data.redirection->left_NY = ast_init(NODE_NOYAU);
    // FIN INITIALISATION DES NOEUDS
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_WORD && tok->type != TOKEN_RED)
    {
        token_free(tok);
        free_red(red_node);
        free(red_node);
        return PARSER_UNEXPECTED_TOKEN;
    }
    if (tok->type == TOKEN_WORD)
    {
        if (lexer->input[lexer->pos + strlen(tok->value)] != ' ')
        {
            int fd = my_atoi(tok->value);
            if (fd != -1)
            {
                red_node->data.redirection->io_number = fd;
                token_free(tok);
                token_free(lexer_pop(lexer));
                tok = NULL;
            }
            else
            {
                token_free(tok);
                free_red(red_node);
                free(red_node);
                return PARSER_UNEXPECTED_TOKEN;
            }
        }
        else
        {
            token_free(tok);
            free_red(red_node);
            free(red_node);
            return PARSER_UNEXPECTED_TOKEN;
        }
    }
    if (tok == NULL)
    {
        tok = lexer_peek(lexer);
    }
    else
    {
        token_free(tok);
        tok = lexer_peek(lexer);
    }
    if (tok->type != TOKEN_RED)
    {
        token_free(tok);
        free_red(red_node);
        free(red_node);
        return PARSER_UNEXPECTED_TOKEN;
    }
    red_node->data.redirection->val = strdup(tok->value);
    token_free(tok);
    token_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_WORD)
    {
        token_free(tok);
        free_red(red_node);
        free(red_node);
        return PARSER_UNEXPECTED_TOKEN;
    }
    red_node->data.redirection->right_name = strdup(tok->value);
    token_free(tok);
    token_free(lexer_pop(lexer));
    (*ast)->childrens[(*ast)->nb_childrens] = red_node;
    (*ast)->nb_childrens += 1;
    return PARSER_OK;
}

/*
  prefix: redirection
  | ASSIGNEMENT_WORD
*/
enum parser_status prefix(struct ast **tab, struct ast_command **cmd,
                          struct lexer *lexer)
{
    struct token *tok = lexer_peek(lexer);
    int inter = lexer->pos;
    if (tok->type == TOKEN_ASSWORD)
    {
        if ((*cmd)->command == NULL)
        {
            (*cmd)->command = strdup(tok->value);
        }
        else
        {
            (*cmd)->args[(*cmd)->nb_args] = strdup(tok->value);
            (*cmd)->nb_args += 1;
        }
        token_free(tok);
        token_free(lexer_pop(lexer));
    }
    else
    {
        lexer->pos = inter;
        token_free(tok);
        enum parser_status status = redirection(tab, lexer);
        if (status != PARSER_OK)
        {
            return PARSER_UNEXPECTED_TOKEN;
        }
    }
    return PARSER_OK;
}

/*
  element : WORD
  | redirection
*/
enum parser_status element(struct ast **tab, struct ast_command **cmd,
                           struct lexer *lexer)
{
    struct token *tok;
    int inter = lexer->pos;
    enum parser_status status = redirection(tab, lexer);
    if (status == PARSER_OK)
    {
        return PARSER_OK;
    }
    lexer->pos = inter;
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_EOC && tok->type != TOKEN_EOF
        && tok->type != TOKEN_NEWLINE && tok->type != TOKEN_ERROR
        && tok->type != TOKEN_PIPE && tok->type != TOKEN_AND
        && tok->type != TOKEN_OR && tok->type != TOKEN_ACCOG
        && tok->type != TOKEN_ACCOD && tok->type != TOKEN_PARD
        && tok->type != TOKEN_PARG && tok->type != TOKEN_RED
        && tok->type != TOKEN_ERROR && tok->type != TOKEN_ESPER)
    {
        if ((*cmd)->command == NULL)
        {
            if (tok->type == TOKEN_WORD)
            {
                (*cmd)->command = word(tok);
            }
            else
            {
                token_free(tok);
                return PARSER_UNEXPECTED_TOKEN;
            }
        }
        else
        {
            (*cmd)->args[(*cmd)->nb_args] = word(tok);
            (*cmd)->nb_args += 1;
        }
        token_free(tok);
        token_free(lexer_pop(lexer));
        return PARSER_OK;
    }
    token_free(tok);
    return PARSER_UNEXPECTED_TOKEN;
}

/*
  compound_list:
  ('\n')*  and_or((';'|'\n') ('\n')* and_or)* [(';'|'\n') ('\n')*]
*/
enum parser_status compound_list(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    while ((tok = lexer_peek(lexer))->type == TOKEN_NEWLINE)
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
    }
    token_free(tok);
    enum parser_status status = and_or(ast, lexer);
    if (status != PARSER_OK)
        return PARSER_UNEXPECTED_TOKEN;
    while (true)
    {
        tok = lexer_peek(lexer);
        if (tok->type == TOKEN_EOC || tok->type == TOKEN_NEWLINE
            || tok->type == TOKEN_ESPER)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            while ((tok = lexer_peek(lexer))->type == TOKEN_NEWLINE)
            {
                token_free(tok);
                token_free(lexer_pop(lexer));
            }
            token_free(tok);
            status = and_or(ast, lexer);
            if (status != PARSER_OK)
                return PARSER_OK;
        }
        else
        {
            token_free(tok);
            tok = NULL;
            return PARSER_OK;
        }
    }
    return PARSER_UNEXPECTED_TOKEN;
}
/*
rule for :For WORD ([';']|[('\n')* 'in' (WORD)* (';'|'\n')]) ('\n')* do_group
*/

enum parser_status rule_for(struct ast **ast, struct lexer *lexer)
{
    struct token *tok = lexer_peek(lexer);
    if (tok->type != TOKEN_FOR)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));

    // INITIALISATION DES NOEUDS
    struct ast *new_node_for = ast_init(NODE_FOR);
    new_node_for->data._for = calloc(1, sizeof(struct ast_for));
    new_node_for->data._for->left_condition = ast_init(NODE_NOYAU);
    new_node_for->data._for->list = ast_init(NODE_NOYAU);
    new_node_for->data._for->right_do = ast_init(NODE_NOYAU);
    // FIN INITIALISATION DES NOEUDS

    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_WORD)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    /*-------------------gestion du noeud gauche/ left_condition------------*/

    // creation noeud commande qui sera le fils du left_condition
    struct ast *cmd = ast_init(NODE_COMMAND);
    cmd->data.command = calloc(1, sizeof(struct ast_command));
    cmd->data.command->command = strdup(tok->value);
    // connexion du noeud cmd avec left_condition
    new_node_for->data._for->left_condition->childrens[0] = cmd;
    new_node_for->data._for->left_condition->nb_childrens += 1;
    /*------------------------------------------------------------------------*/
    token_free(tok);
    token_free(lexer_pop(lexer));
    int inter = lexer->pos;
    enum parser_status status =
        do_group(&new_node_for->data._for->right_do, lexer);
    if (status == PARSER_OK)
    {
        (*ast)->childrens[(*ast)->nb_childrens] = new_node_for;
        (*ast)->nb_childrens += 1;
        return PARSER_OK;
    }
    lexer->pos = inter;
    tok = lexer_peek(lexer);
    if (tok->type == TOKEN_EOC)
    {
        new_node_for->data._for->right_do->nb_childrens = 0;
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
        while (tok->type == TOKEN_NEWLINE)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
        }
        token_free(tok);
        status = do_group(&new_node_for->data._for->right_do, lexer);
        if (status != PARSER_OK)
        {
            free_for(new_node_for);
            free(new_node_for);
            return PARSER_UNEXPECTED_TOKEN;
        }
        (*ast)->childrens[(*ast)->nb_childrens] = new_node_for;
        (*ast)->nb_childrens += 1;
        return PARSER_OK;
    }
    else if (tok->type == TOKEN_NEWLINE)
    {
        new_node_for->data._for->right_do->nb_childrens = 0;
        while (tok->type == TOKEN_NEWLINE)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
        }
        token_free(tok);
        status = do_group(&new_node_for->data._for->right_do, lexer);
        if (status == PARSER_OK)
        {
            (*ast)->childrens[(*ast)->nb_childrens] = new_node_for;
            (*ast)->nb_childrens += 1;
            return PARSER_OK;
        }
        new_node_for->data._for->right_do->nb_childrens = 0;
        tok = lexer_peek(lexer);
        if (tok->type != TOKEN_IN)
        {
            token_free(tok);
            free_for(new_node_for);
            free(new_node_for);
            return PARSER_UNEXPECTED_TOKEN;
        }
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
        while (tok->type == TOKEN_WORD)
        {
            struct ast *new_in = ast_init(NODE_COMMAND);
            new_in->data.command = calloc(1, sizeof(struct ast_command));

            new_in->data.command->command = strdup(tok->value);
            new_node_for->data._for->list
                ->childrens[new_node_for->data._for->list->nb_childrens] =
                new_in;
            new_node_for->data._for->list->nb_childrens += 1;

            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
        }
        if (tok->type != TOKEN_EOC && tok->type != TOKEN_NEWLINE)
        {
            token_free(tok);
            free_for(new_node_for);
            free(new_node_for);
            return PARSER_UNEXPECTED_TOKEN;
        }
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
        while (tok->type == TOKEN_NEWLINE)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
        }
        token_free(tok);
        status = do_group(&new_node_for->data._for->right_do, lexer);
        if (status != PARSER_OK)
        {
            free_for(new_node_for);
            free(new_node_for);
            return PARSER_UNEXPECTED_TOKEN;
        }
        (*ast)->childrens[(*ast)->nb_childrens] = new_node_for;
        (*ast)->nb_childrens += 1;
        return PARSER_OK;
    }
    else if (tok->type == TOKEN_IN)
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
        while (tok->type == TOKEN_WORD)
        {
            struct ast *new_in = ast_init(NODE_COMMAND);
            new_in->data.command = calloc(1, sizeof(struct ast_command));

            new_in->data.command->command = strdup(tok->value);
            new_node_for->data._for->list
                ->childrens[new_node_for->data._for->list->nb_childrens] =
                new_in;
            new_node_for->data._for->list->nb_childrens += 1;

            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
        }
        if (tok->type != TOKEN_EOC && tok->type != TOKEN_NEWLINE)
        {
            token_free(tok);
            free_for(new_node_for);
            free(new_node_for);
            return PARSER_UNEXPECTED_TOKEN;
        }
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
        while (tok->type == TOKEN_NEWLINE)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
        }
        token_free(tok);
        status = do_group(&new_node_for->data._for->right_do, lexer);
        if (status != PARSER_OK)
        {
            free_for(new_node_for);
            free(new_node_for);
            return PARSER_UNEXPECTED_TOKEN;
        }
        (*ast)->childrens[(*ast)->nb_childrens] = new_node_for;
        (*ast)->nb_childrens += 1;
        return PARSER_OK;
    }
    else
    {
        token_free(tok);
        free_for(new_node_for);
        free(new_node_for);
        return PARSER_UNEXPECTED_TOKEN;
    }
}

/*
  rule while :While compound_list do_group
*/
enum parser_status rule_while(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    if ((tok = lexer_peek(lexer))->type != TOKEN_WHILE)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    // INITIALISATION DES NOEUDS
    struct ast *while_node = ast_init(NODE_WHILE);
    while_node->data._while = calloc(1, sizeof(struct ast_while));
    while_node->data._while->left_condition = ast_init(NODE_NOYAU);
    while_node->data._while->right_do = ast_init(NODE_NOYAU);
    // FIN INITIALISATION DES NOEUDS
    enum parser_status status =
        compound_list(&while_node->data._while->left_condition, lexer);
    if (status != PARSER_OK)
    {
        free_while(while_node);
        free(while_node);
        return status;
    }
    status = do_group(&while_node->data._while->right_do, lexer);
    if (status != PARSER_OK)
    {
        free_while(while_node);
        free(while_node);
        return status;
    }
    (*ast)->childrens[(*ast)->nb_childrens] = while_node;
    (*ast)->nb_childrens += 1;
    return PARSER_OK;
}
/*
  rule until :Until compound_list do_group
*/
enum parser_status rule_until(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    if ((tok = lexer_peek(lexer))->type != TOKEN_UNTIL)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    // INITIALISATION DES NOEUDS
    struct ast *until_node = ast_init(NODE_UNTIL);
    until_node->data._while = calloc(1, sizeof(struct ast_while));
    until_node->data._while->left_condition = ast_init(NODE_NOYAU);
    until_node->data._while->right_do = ast_init(NODE_NOYAU);
    // FIN INITIALISATION DES NOEUDS
    enum parser_status status =
        compound_list(&until_node->data._while->left_condition, lexer);
    if (status != PARSER_OK)
    {
        free_while(until_node);
        free(until_node);
        return status;
    }
    status = do_group(&until_node->data._while->right_do, lexer);
    if (status != PARSER_OK)
    {
        free_while(until_node);
        free(until_node);
        return status;
    }
    (*ast)->childrens[(*ast)->nb_childrens] = until_node;
    (*ast)->nb_childrens += 1;
    return PARSER_OK;
}

/* rule_if: If compound_list Then compound_list [else_clause] Fi
 */
enum parser_status rule_if(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    if ((tok = lexer_peek(lexer))->type != TOKEN_IF)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    // INITILIASTION DES NOEUDS
    struct ast *if_node = ast_init(NODE_IF);
    if_node->data._if = calloc(1, sizeof(struct ast_if));
    if_node->data._if->left_condition = ast_init(NODE_NOYAU);
    if_node->data._if->right_then = ast_init(NODE_NOYAU);
    // FIN INITILIASTION DES NOEUDS
    enum parser_status status =
        compound_list(&if_node->data._if->left_condition, lexer);
    if (status != PARSER_OK)
    {
        free_if(if_node);
        free(if_node);
        return status;
    }
    if ((tok = lexer_peek(lexer))->type != TOKEN_THEN)
    {
        token_free(tok);
        free_if(if_node);
        free(if_node);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    status = compound_list(&if_node->data._if->right_then, lexer);
    if (status != PARSER_OK)
    {
        free_if(if_node);
        free(if_node);
        return status;
    }
    (*ast)->childrens[(*ast)->nb_childrens] = if_node;
    (*ast)->nb_childrens += 1;
    tok = lexer_peek(lexer);
    if (tok->type == TOKEN_ELIF || tok->type == TOKEN_ELSE)
    {
        status = else_clause(ast, lexer);
        if (status != PARSER_OK)
        {
            token_free(tok);
            free_if((*ast)->childrens[(*ast)->nb_childrens - 1]);
            free((*ast)->childrens[(*ast)->nb_childrens - 1]);
            (*ast)->nb_childrens -= 1;
            return status;
        }
    }
    token_free(tok);
    if ((tok = lexer_peek(lexer))->type != TOKEN_FI)
    {
        token_free(tok);
        if ((*ast)->childrens[(*ast)->nb_childrens - 1]->type == NODE_ELIF
            || (*ast)->childrens[(*ast)->nb_childrens - 1]->type == NODE_IF)
        {
            free_if((*ast)->childrens[(*ast)->nb_childrens - 1]);
        }
        else
        {
            free_else((*ast)->childrens[(*ast)->nb_childrens - 1]);
        }
        free((*ast)->childrens[(*ast)->nb_childrens - 1]);
        (*ast)->nb_childrens -= 1;
        return PARSER_UNEXPECTED_TOKEN;
    }
    (*ast)->childrens[(*ast)->nb_childrens] = ast_init(NODE_FI);
    (*ast)->nb_childrens += 1;
    token_free(lexer_pop(lexer));
    token_free(tok);
    return PARSER_OK;
}

/*
else_clause: Else compound_list
| Elif compound_list Then compound_list [else_clause]
*/

enum parser_status else_clause(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    if ((tok = lexer_peek(lexer))->type != TOKEN_ELSE
        && tok->type != TOKEN_ELIF)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    if (tok->type == TOKEN_ELSE)
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
        // INITIALISATION DES NOEUDS
        struct ast *else_node = ast_init(NODE_ELSE);
        else_node->data._else = calloc(1, sizeof(struct ast_else));
        else_node->data._else->_then = ast_init(NODE_NOYAU);
        // FIN INITIALISATION DES NOEUDS
        enum parser_status status =
            compound_list(&else_node->data._else->_then, lexer);
        if (status != PARSER_OK)
        {
            free_else(else_node);
            free(else_node);
            return PARSER_UNEXPECTED_TOKEN;
        }
        (*ast)->childrens[(*ast)->nb_childrens] = else_node;
        (*ast)->nb_childrens += 1;
        return PARSER_OK;
    }
    else
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
        // INITIALISATION DES NOEUDS
        struct ast *elif_node = ast_init(NODE_ELIF);
        elif_node->data._if = calloc(1, sizeof(struct ast_if));
        elif_node->data._if->left_condition = ast_init(NODE_NOYAU);
        elif_node->data._if->right_then = ast_init(NODE_NOYAU);
        // FIN INITIALISATION DES NOEUDS
        enum parser_status status =
            compound_list(&elif_node->data._if->left_condition, lexer);
        if (status != PARSER_OK)
        {
            free_if(elif_node);
            free(elif_node);
            return PARSER_UNEXPECTED_TOKEN;
        }
        if ((tok = lexer_peek(lexer))->type != TOKEN_THEN)
        {
            free_if(elif_node);
            free(elif_node);
            token_free(tok);
            return PARSER_UNEXPECTED_TOKEN;
        }
        token_free(tok);
        token_free(lexer_pop(lexer));
        status = compound_list(&elif_node->data._if->right_then, lexer);
        if (status != PARSER_OK)
        {
            free_if(elif_node);
            free(elif_node);
            return PARSER_UNEXPECTED_TOKEN;
        }
        (*ast)->childrens[(*ast)->nb_childrens] = elif_node;
        (*ast)->nb_childrens += 1;

        tok = lexer_peek(lexer);
        if (tok->type == TOKEN_ELIF || tok->type == TOKEN_ELSE)
        {
            status = else_clause(ast, lexer);
            if (status != PARSER_OK)
            {
                token_free(tok);
                free_if((*ast)->childrens[(*ast)->nb_childrens - 1]);
                free((*ast)->childrens[(*ast)->nb_childrens - 1]);
                (*ast)->nb_childrens -= 1;
                return status;
            }
        }
        token_free(tok);

        return PARSER_OK;
    }
    token_free(tok);
    return PARSER_UNEXPECTED_TOKEN;
}

/*
  do_group : Do compound_list Done
*/
enum parser_status do_group(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    if ((tok = lexer_peek(lexer))->type != TOKEN_DO)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    enum parser_status status = compound_list(ast, lexer);
    if (status != PARSER_OK)
    {
        return status;
    }
    if ((tok = lexer_peek(lexer))->type != TOKEN_DONE)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    return PARSER_OK;
}

enum parser_status rule_case(struct ast **ast, struct lexer *lexer)
{
    struct token *tok;
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_CASE)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(lexer_pop(lexer));
    token_free(tok);
    // INITIALISATION DES NOEUDS
    struct ast *node_case = ast_init(NODE_CASE);
    node_case->data._case = calloc(1, sizeof(struct ast_case));
    node_case->data._case->tab_case_item =
        calloc(strlen(lexer->input), sizeof(struct case_item *));
    // FIN D'INITIALISATION DES NOEUDS
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_WORD)
    {
        token_free(tok);
        free_case(node_case);
        free(node_case);
        return PARSER_UNEXPECTED_TOKEN;
    }
    node_case->data._case->variable =
        strdup(tok->value); // Ajout de la variable du case
    token_free(lexer_pop(lexer));
    token_free(tok);
    tok = lexer_peek(lexer);
    while (tok->type == TOKEN_NEWLINE)
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
    }
    if (tok->type != TOKEN_IN)
    {
        token_free(tok);
        free_case(node_case);
        free(node_case);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(lexer_pop(lexer));
    token_free(tok);
    tok = lexer_peek(lexer);
    while (tok->type == TOKEN_NEWLINE)
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
    }
    token_free(tok);
    case_clause(&node_case->data._case, lexer);
    if ((tok = lexer_peek(lexer))->type != TOKEN_ESAC)
    {
        token_free(tok);
        free_case(node_case);
        free(node_case);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(lexer_pop(lexer));
    token_free(tok);
    (*ast)->childrens[(*ast)->nb_childrens] = node_case;
    (*ast)->nb_childrens += 1;
    return PARSER_OK;
}

enum parser_status case_clause(struct ast_case **node_case, struct lexer *lexer)
{
    struct token *tok;
    enum parser_status status = case_item(node_case, lexer);
    if (status != PARSER_OK)
    {
        return status;
    }
    tok = lexer_peek(lexer);
    if (tok->type == TOKEN_NEWLINE)
    {
        while (tok->type == TOKEN_NEWLINE)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
        }
        token_free(tok);
        return PARSER_OK;
    }
    token_free(tok);
    while (true)
    {
        tok = lexer_peek(lexer);
        if (tok->type == TOKEN_EOC)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
            if (tok->type == TOKEN_EOC)
            {
                token_free(tok);
                token_free(lexer_pop(lexer));
                tok = lexer_peek(lexer);
                while (tok->type == TOKEN_NEWLINE)
                {
                    token_free(tok);
                    token_free(lexer_pop(lexer));
                    tok = lexer_peek(lexer);
                }
                token_free(tok);
                status = case_item(node_case, lexer);
                if (status != PARSER_OK)
                {
                    return PARSER_OK;
                }
            }
            else
            {
                token_free(tok);
                return PARSER_UNEXPECTED_TOKEN;
            }
        }
        else
        {
            token_free(tok);
            return PARSER_OK;
        }
    }
}

enum parser_status case_item(struct ast_case **node_case, struct lexer *lexer)
{
    // INITIALISATION DE LA STRUCTURE CASE ITEM
    struct case_item *current_item = calloc(1, sizeof(struct case_item));
    current_item->tab_variables = calloc(strlen(lexer->input), sizeof(char *));
    current_item->function_NY = ast_init(NODE_NOYAU);
    // FIIN INITIALISATION DE LA STRUCTURE CASE ITEM
    struct token *tok = lexer_peek(lexer);
    if (tok->type == TOKEN_PARG)
    {
        token_free(lexer_pop(lexer));
    }
    token_free(tok);
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_WORD)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    current_item->tab_variables[current_item->nb_variables] =
        strdup(tok->value);
    current_item->nb_variables += 1;
    token_free(lexer_pop(lexer));
    token_free(tok);
    while (true)
    {
        tok = lexer_peek(lexer);
        // pipe represente un ou dans les cases
        if (tok->type == TOKEN_PIPE)
        {
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
            if (tok->type != TOKEN_WORD)
            {
                token_free(tok);
                return PARSER_UNEXPECTED_TOKEN;
            }
            current_item->tab_variables[current_item->nb_variables] =
                strdup(tok->value);
            current_item->nb_variables += 1;
            token_free(tok);
            token_free(lexer_pop(lexer));
        }
        else
        {
            token_free(tok);
            break;
        }
    }
    tok = lexer_peek(lexer);
    if (tok->type != TOKEN_PARD)
    {
        token_free(tok);
        return PARSER_UNEXPECTED_TOKEN;
    }
    token_free(tok);
    token_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
    while (tok->type == TOKEN_NEWLINE)
    {
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
    }
    token_free(tok);
    compound_list(&current_item->function_NY, lexer);
    (*node_case)->tab_case_item[(*node_case)->nb_case_item] = current_item;
    (*node_case)->nb_case_item += 1;
    return PARSER_OK;
}

char *word(struct token *tok)
{
    char *ret;
    if (tok->type == TOKEN_IF)
    {
        ret = strdup("if");
    }
    else if (tok->type == TOKEN_THEN)
    {
        ret = strdup("then");
    }
    else if (tok->type == TOKEN_ELIF)
    {
        ret = strdup("elif");
    }
    else if (tok->type == TOKEN_ELSE)
    {
        ret = strdup("else");
    }
    else if (tok->type == TOKEN_FI)
    {
        ret = strdup("fi");
    }
    else if (tok->type == TOKEN_EOC)
    {
        ret = strdup(";");
    }
    else if (tok->type == TOKEN_NEWLINE)
    {
        ret = strdup("\n");
    }
    else if (tok->type == TOKEN_PIPE)
    {
        ret = strdup("|");
    }
    else if (tok->type == TOKEN_PARG)
    {
        ret = strdup("(");
    }
    else if (tok->type == TOKEN_PARD)
    {
        ret = strdup(")");
    }
    else if (tok->type == TOKEN_ACCOG)
    {
        ret = strdup("{");
    }
    else if (tok->type == TOKEN_ACCOD)
    {
        ret = strdup("}");
    }
    else if (tok->type == TOKEN_OR)
    {
        ret = strdup("||");
    }
    else if (tok->type == TOKEN_AND)
    {
        ret = strdup("||");
    }
    else if (tok->type == TOKEN_WHILE)
    {
        ret = strdup("while");
    }
    else if (tok->type == TOKEN_FOR)
    {
        ret = strdup("for");
    }
    else if (tok->type == TOKEN_UNTIL)
    {
        ret = strdup("until");
    }
    else if (tok->type == TOKEN_NEG)
    {
        ret = strdup("!");
    }
    else if (tok->type == TOKEN_DO)
    {
        ret = strdup("do");
    }
    else if (tok->type == TOKEN_DONE)
    {
        ret = strdup("done");
    }
    else if (tok->type == TOKEN_ESPER)
    {
        ret = strdup("&");
    }
    else if (tok->type == TOKEN_RED || tok->type == TOKEN_WORD
             || tok->type == TOKEN_ASSWORD)
    {
        ret = strdup(tok->value);
    }
    else if (tok->type == TOKEN_IN)
    {
        ret = strdup("in");
    }
    else if (tok->type == TOKEN_ESAC)
    {
        ret = strdup("esac");
    }
    else if (tok->type == TOKEN_CASE)
    {
        ret = strdup("case");
    }
    else if (tok->type == TOKEN_EOC)
    {
        ret = strdup(";");
    }
    else
    {
        ret = NULL;
    }
    return ret;
}
