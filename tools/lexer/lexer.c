#include "../headers/global.h"

struct lexer *lexer_new(const char *input)
{
    struct lexer *new = malloc(sizeof(struct lexer));
    new->input = input;
    new->pos = 0;
    new->current_tok = NULL;
    return new;
}

void lexer_free(struct lexer *lexer)
{
    free(lexer);
}

struct token *lexer_peek(struct lexer *lexer)
{
    if (lexer)
    {
        while (lexer->input[lexer->pos] == ' '
               || lexer->input[lexer->pos] == '\t')
            lexer->pos += 1;
        char *inter = calloc(strlen(lexer->input) + 20, sizeof(char));
        size_t inter_lex = lexer->pos;
        int index = 0;
        if (lexer->input[lexer->pos] == '\'') // CAS SINGLE QUOTE
        {
            inter[index++] = lexer->input[inter_lex++];
            while (lexer->input[inter_lex] != '\''
                   && lexer->input[inter_lex] != '\0')
            {
                inter[index++] = lexer->input[inter_lex++];
            }
            if (lexer->input[inter_lex] == '\0')
            {
                lexer->current_tok = token_new(TOKEN_ERROR);
            }
            else
            {
                inter[index++] = lexer->input[inter_lex++];
                lexer->current_tok = token_new(TOKEN_WORD);
                lexer->current_tok->value = strdup(inter);
            }
        }
        else if (lexer->input[inter_lex] == '$')
        {
            int m1 = 0;
            int m2 = 0;
            inter[index++] = lexer->input[inter_lex++];
            if (lexer->input[inter_lex] == '{')
            {
                m1 = 1;
            }
            else if (lexer->input[inter_lex] == '(')
            {
                m2 = 1;
            }
            inter[index++] = lexer->input[inter_lex++];
            while (inter_lex < strlen(lexer->input)
                   && lexer->input[inter_lex] != '\0'
                   && lexer->input[inter_lex] != ' '
                   && lexer->input[inter_lex] != '\t'
                   && lexer->input[inter_lex] != ';'
                   && lexer->input[inter_lex] != '\n'
                   && lexer->input[inter_lex] != '<'
                   && lexer->input[inter_lex] != '>'
                   && lexer->input[inter_lex] != '|')
            {
                if (lexer->input[inter_lex] == '{')
                {
                    m1 = 1;
                }
                if (lexer->input[inter_lex] == '}')
                {
                    m1 = 0;
                }
                if (lexer->input[inter_lex] == '(')
                {
                    m2 = 1;
                }
                if (lexer->input[inter_lex] == ')')
                {
                    m2 = 0;
                }
                inter[index++] = lexer->input[inter_lex++];
            }
            if (m2 == 1 || m1 == 1)
            {
                lexer->current_tok = token_new(TOKEN_ERROR);
            }
            else
            {
                lexer->current_tok = token_new(TOKEN_ASSWORD);
                lexer->current_tok->value = strdup(inter);
            }
        }
        else if (lexer->input[lexer->pos] == '\"') // CAS DOUBLE QUOTE
        {
            inter[index++] = lexer->input[inter_lex++];
            while (lexer->input[inter_lex] != '\"'
                   && lexer->input[inter_lex] != '\0')
            {
                inter[index++] = lexer->input[inter_lex++];
            }
            if (lexer->input[inter_lex] == '\0')
            {
                lexer->current_tok = token_new(TOKEN_ERROR);
            }
            else
            {
                inter[index++] = lexer->input[inter_lex++];
                lexer->current_tok = token_new(TOKEN_WORD);
                lexer->current_tok->value = strdup(inter);
            }
        }
        else if (lexer->input[inter_lex] == '\0')
        {
            lexer->current_tok = token_new(TOKEN_EOF);
        }
        else // RESTE
        {
            int m = 0;
            int quit = 0;
            while (lexer->input[inter_lex] != '\0'
                   && lexer->input[inter_lex] != ' '
                   && lexer->input[inter_lex] != '\t'
                   && lexer->input[inter_lex] != ';'
                   && lexer->input[inter_lex] != '\n'
                   && lexer->input[inter_lex] != '<'
                   && lexer->input[inter_lex] != '}'
                   && lexer->input[inter_lex] != '>'
                   && lexer->input[inter_lex] != '{'
                   && lexer->input[inter_lex] != ')'
                   && lexer->input[inter_lex] != '('
                   && lexer->input[inter_lex] != '|'
                   && lexer->input[inter_lex] != '&')
            {
                if (lexer->input[inter_lex] == '\"')
                {
                    inter[index++] = lexer->input[inter_lex++];
                    while (lexer->input[inter_lex] != '\"'
                           && lexer->input[inter_lex] != '\0')
                    {
                        inter[index++] = lexer->input[inter_lex++];
                    }
                    if (lexer->input[inter_lex] == '\0')
                    {
                        lexer->current_tok = token_new(TOKEN_ERROR);
                        quit = 1;
                        m = 1;
                        break;
                    }
                    else
                    {
                        inter[index++] = lexer->input[inter_lex++];
                        quit = 1;
                        break;
                    }
                }
                else if (lexer->input[inter_lex] == '\'')
                {
                    inter[index++] = lexer->input[inter_lex++];
                    while (lexer->input[inter_lex] != '\''
                           && lexer->input[inter_lex] != '\0')
                    {
                        inter[index++] = lexer->input[inter_lex++];
                    }
                    if (lexer->input[inter_lex] == '\0')
                    {
                        lexer->current_tok = token_new(TOKEN_ERROR);
                        quit = 1;
                        m = 1;
                        break;
                    }
                    else
                    {
                        inter[index++] = lexer->input[inter_lex++];
                        quit = 1;
                        break;
                    }
                }
                else
                {
                    inter[index++] = lexer->input[inter_lex++];
                }
                if (quit == 1)
                {
                    break;
                }
            }
            if (m != 1)
            {
                if (strcmp(inter, "if") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_IF);
                }
                else if (strcmp(inter, "else") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_ELSE);
                }
                else if (strcmp(inter, "!") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_NEG);
                }
                else if (strcmp(inter, "elif") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_ELIF);
                }
                else if (strcmp(inter, "esac") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_ESAC);
                }
                else if (strcmp(inter, "case") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_CASE);
                }
                else if (strcmp(inter, "then") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_THEN);
                }
                else if (strcmp(inter, "fi") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_FI);
                }
                else if (strcmp(inter, "while") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_WHILE);
                }
                else if (strcmp(inter, "until") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_UNTIL);
                }
                else if (strcmp(inter, "in") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_IN);
                }
                else if (strcmp(inter, "do") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_DO);
                }
                else if (strcmp(inter, "done") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_DONE);
                }
                else if (strcmp(inter, "for") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_FOR);
                }
                else if (inter[0] == '\0' && lexer->input[inter_lex] == '\n')
                {
                    lexer->current_tok = token_new(TOKEN_NEWLINE);
                }
                else if (inter[0] == '\0' && lexer->input[inter_lex] == ';')
                {
                    lexer->current_tok = token_new(TOKEN_EOC);
                }
                else if (inter[0] == '\0' && lexer->input[inter_lex] == '&')
                {
                    if (lexer->input[inter_lex + 1] == '&')
                    {
                        lexer->current_tok = token_new(TOKEN_AND);
                    }
                    else
                    {
                        lexer->current_tok = token_new(TOKEN_ESPER);
                    }
                }
                else if (inter[0] == '\0' && lexer->input[inter_lex] == '|')
                {
                    if (lexer->input[inter_lex + 1] == '|')
                    {
                        lexer->current_tok = token_new(TOKEN_OR);
                    }
                    else
                    {
                        lexer->current_tok = token_new(TOKEN_PIPE);
                    }
                }
                else if (inter[0] == '\0' && lexer->input[inter_lex] == '}')
                {
                    lexer->current_tok = token_new(TOKEN_ACCOD);
                }
                else if (inter[0] == '\0' && lexer->input[inter_lex] == '{')
                {
                    lexer->current_tok = token_new(TOKEN_ACCOG);
                }
                else if (inter[0] == '\0' && lexer->input[inter_lex] == '(')
                {
                    lexer->current_tok = token_new(TOKEN_PARG);
                }
                else if (inter[0] == '\0' && lexer->input[inter_lex] == ')')
                {
                    lexer->current_tok = token_new(TOKEN_PARD);
                }
                else if (inter[0] == '\0'
                         && (lexer->input[inter_lex] == '>'
                             || lexer->input[inter_lex] == '<'))
                {
                    lexer->current_tok = token_new(TOKEN_RED);
                    char *red_tok = calloc(4, sizeof(char));
                    if (lexer->input[inter_lex] == '>') //>
                    {
                        if (inter_lex + 1 < strlen(lexer->input))
                        {
                            if (lexer->input[inter_lex + 1] == '>')
                            {
                                red_tok[0] = lexer->input[inter_lex];
                                red_tok[1] = lexer->input[inter_lex + 1];
                                inter = strcat(inter, red_tok);
                            }
                            else if (lexer->input[inter_lex + 1] == '|')
                            {
                                red_tok[0] = lexer->input[inter_lex];
                                red_tok[1] = lexer->input[inter_lex + 1];
                                inter = strcat(inter, red_tok);
                            }
                            else if (lexer->input[inter_lex + 1] == '&')
                            {
                                red_tok[0] = lexer->input[inter_lex];
                                red_tok[1] = lexer->input[inter_lex + 1];
                                inter = strcat(inter, red_tok);
                            }
                            else
                            {
                                red_tok[0] = lexer->input[inter_lex];
                                inter = strcat(inter, red_tok);
                            }
                        }
                    }
                    else // <
                    {
                        if (inter_lex + 1 < strlen(lexer->input))
                        {
                            if (lexer->input[inter_lex + 1] == '>')
                            {
                                red_tok[0] = lexer->input[inter_lex];
                                red_tok[1] = lexer->input[inter_lex + 1];
                                inter = strcat(inter, red_tok);
                            }
                            else if (lexer->input[inter_lex + 1] == '&')
                            {
                                red_tok[0] = lexer->input[inter_lex];
                                red_tok[1] = lexer->input[inter_lex + 1];
                                inter = strcat(inter, red_tok);
                            }
                            else
                            {
                                red_tok[0] = lexer->input[inter_lex];
                                inter = strcat(inter, red_tok);
                            }
                        }
                    }
                    free(red_tok);
                    lexer->current_tok->value = strdup(inter);
                }
                else if (strcmp(inter, "|") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_PIPE);
                }
                else if (strcmp(inter, "(") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_PARG);
                }
                else if (strcmp(inter, ")") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_PARD);
                }
                else if (strcmp(inter, "||") == 0)
                {
                    lexer->current_tok = token_new(TOKEN_OR);
                }
                else
                {
                    lexer->current_tok = token_new(TOKEN_WORD);
                    lexer->current_tok->value = strdup(inter);
                }
            }
        }
        free(inter);
        return lexer->current_tok;
    }
    lexer->current_tok = token_new(TOKEN_ERROR);
    return lexer->current_tok;
}

struct token *lexer_pop(struct lexer *lexer)
{
    if (lexer)
    {
        struct token *stock = lexer_peek(lexer);
        char *inter = stock->value;
        if (stock->type == TOKEN_WORD || stock->type == TOKEN_RED
            || stock->type == TOKEN_ASSWORD)
        {
            lexer->pos += strlen(inter);
        }
        else if (stock->type == TOKEN_IF || stock->type == TOKEN_FI
                 || stock->type == TOKEN_DO || stock->type == TOKEN_AND
                 || stock->type == TOKEN_OR || stock->type == TOKEN_IN)
        {
            lexer->pos += 2;
        }
        else if (stock->type == TOKEN_ELSE || stock->type == TOKEN_ELIF
                 || stock->type == TOKEN_THEN || stock->type == TOKEN_DONE
                 || stock->type == TOKEN_ESAC || stock->type == TOKEN_CASE)
        {
            lexer->pos += 4;
        }
        else if (stock->type == TOKEN_WHILE || stock->type == TOKEN_UNTIL)
        {
            lexer->pos += 5;
        }
        else if (stock->type == TOKEN_FOR)
        {
            lexer->pos += 3;
        }
        else if (stock->type == TOKEN_ACCOG || stock->type == TOKEN_ACCOD
                 || stock->type == TOKEN_PARG || stock->type == TOKEN_PARD
                 || stock->type == TOKEN_PIPE || stock->type == TOKEN_NEG)
        {
            lexer->pos += 1;
        }
        else
        {
            if (stock->type != TOKEN_EOF)
            {
                lexer->pos += 1;
            }
        }
        lexer->current_tok = NULL;
        return stock;
    }
    lexer->current_tok = token_new(TOKEN_ERROR);
    return lexer->current_tok;
}
