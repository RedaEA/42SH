#include "execution.h"

#include <err.h>
#include <sys/wait.h>

#include "../headers/global.h"

//-------------------------ECHO FUNCTION-------------------------------
int options(char *commande, int *e, int *newline) // ECHO -e tools QUESTION ACU
{
    int j = 0;
    if (commande[0] == '-')
    {
        for (int i = 0;
             ((commande[i] == '-'
               && (commande[i + 1] == 'e' || commande[i + 1] == 'n')))
             || commande[i] == ' ';
             i++)
        {
            if (commande[i] == '-')
            {
                if (commande[i + 1] == 'n')
                {
                    *newline = 1;
                    j += 1;
                    i += 1;
                    if (commande[i + 1] == 'e')
                    {
                        *e = 1;
                        j += 1;
                        i += 1;
                    }
                }
                if (commande[i + 1] == 'e')
                {
                    *e = 1;
                    i += 1;
                    j += 1;
                    if (commande[i + 1] == 'n')
                    {
                        *newline = 1;
                        j += 1;
                        i += 1;
                    }
                }
            }
            j += 1;
        }
    }
    while (commande[j] == ' ')
        j++;
    return j;
}

char *variable_treatement(char *command, struct ast_variable **variable)
{
    if (command)
    {
        int i = 0;
        int position = 0;
        char *new_com = calloc(strlen(command) + 10, sizeof(char));
        int guillemets = 0;
        while (command[i])
        {
            if ((command[i] == '"' || command[i] == '\'')
                && (i - 1 < 0 || command[i - 1] != '\\'))
            {
                guillemets = 1;
                i++;
            }
            else if (command[i] == '$'
                     && (!command[i + 1] || command[i + 1] == ' '
                         || command[i + 1] == '\t' || command[i + 1] == '\0'))
            {
                new_com[position++] = command[i++];
            }
            else if (command[i] == '$' && command[i + 1] == '$')
            {
                struct ast_variable *parc = variable[0];
                while (parc)
                {
                    if (!strcmp(parc->name, "$"))
                    {
                        char *tmp =
                            calloc(strlen(new_com) + strlen(parc->value_str)
                                       + strlen(command) + 10,
                                   sizeof(char));
                        strcat(tmp, new_com);
                        strcat(tmp, parc->value_str);
                        free(new_com);
                        new_com = tmp;
                        position = strlen(new_com);
                        break;
                    }
                    parc = parc->next;
                }
                i += 2;
            }
            else if (command[i] == '$' && command[i + 1] != '{'
                     && command[i + 1] != '=')
            {
                int index = 0;
                char *variable_name =
                    calloc(strlen(command) + 10, sizeof(char));
                i++;
                while (command[i] && command[i] != ' ' && command[i] != '"'
                       && command[i] != '$' && command[i] != '\'')
                {
                    variable_name[index++] = command[i++];
                }
                if (command[i] == '"' || command[i] == '\'')
                {
                    if (guillemets)
                    {
                        guillemets = 0;
                    }
                    else
                    {
                        guillemets = 1;
                    }
                    i++;
                }
                if (!strcmp(variable_name, "RANDOM"))
                {
                    int rd = rand() % 32767;
                    char *rdm = my_itoa(rd);
                    char *tmp =
                        calloc(strlen(new_com) + 9 + strlen(command) + 10,
                               sizeof(char));
                    strcat(tmp, new_com);
                    strcat(tmp, rdm);
                    free(new_com);
                    free(rdm);
                    new_com = tmp;
                    position = strlen(new_com);
                }
                else
                {
                    struct ast_variable *parc = variable[0];
                    while (parc)
                    {
                        if (!strcmp(parc->name, variable_name))
                        {
                            char *tmp =
                                calloc(strlen(new_com) + strlen(parc->value_str)
                                           + strlen(command) + 10,
                                       sizeof(char));
                            strcat(tmp, new_com);
                            strcat(tmp, parc->value_str);
                            free(new_com);
                            new_com = tmp;
                            position = strlen(new_com);
                            break;
                        }
                        parc = parc->next;
                    }

                    if (!parc && guillemets == 0)
                    {
                        while (command[i] == ' ')
                        {
                            i++;
                        }
                    }
                }
                free(variable_name);
            }
            else if (command && (command[i] == '$' && command[i + 1] == '{'))
            {
                int index = 0;
                char *variable_name =
                    calloc(strlen(command) + 10, sizeof(char));
                i += 2;
                while (command[i] != '}')
                {
                    variable_name[index++] = command[i++];
                }
                if (!strcmp(variable_name, "RANDOM"))
                {
                    int rd = rand() % 32767;
                    char *rdm = my_itoa(rd);
                    char *tmp =
                        calloc(strlen(new_com) + 9 + strlen(command) + 10,
                               sizeof(char));
                    strcat(tmp, new_com);
                    strcat(tmp, rdm);
                    free(new_com);
                    free(rdm);
                    new_com = tmp;
                    position = strlen(new_com);
                }
                else
                {
                    struct ast_variable *parc = variable[0];
                    while (parc)
                    {
                        if (!strcmp(parc->name, variable_name))
                        {
                            char *tmp =
                                calloc(strlen(new_com) + strlen(parc->value_str)
                                           + strlen(command) + 10,
                                       sizeof(char));
                            strcat(tmp, new_com);
                            strcat(tmp, parc->value_str);
                            free(new_com);
                            new_com = tmp;
                            position = strlen(new_com);
                            break;
                        }
                        parc = parc->next;
                    }
                    i++;
                }
                free(variable_name);
            }
            else
            {
                new_com[position++] = command[i++];
            }
        }
        return new_com;
    }
    else
    {
        char *new_com = strdup("");
        return new_com;
    }
}

int echo_42(char *commande, struct ast_variable **variable)
{
    commande = variable_treatement(commande, variable);
    int *newline = calloc(1, sizeof(int));
    int *e = calloc(1, sizeof(int));
    char *tmp = commande;
    int dec = options(commande, e, newline);
    if (*newline > 0 || *e > 0)
        commande += dec;
    for (int i = 0; commande[i]; i++)
    {
        if (commande[i] == '"' || commande[i] == '\'')
            continue;
        if (commande[i] == '\\' && commande[i + 1] == 'n')
        {
            if (!*e)
            {
                printf("\\");
                printf("n");
                fflush(stdout);
            }
            else
            {
                printf("\n");
                fflush(stdout);
            }
            i += 1;
        }
        else if (commande[i] == '\\' && commande[i + 1] == 't')
        {
            if (!*e)
            {
                printf("\\");
                printf("t");
                fflush(stdout);
            }
            else
            {
                printf("\t");
                fflush(stdout);
            }
            i += 1;
        }
        else
        {
            printf("%c", commande[i]);
            fflush(stdout);
        }
    }
    if (!*newline)
    {
        printf("\n");
        fflush(stdout);
    }
    free(tmp);
    free(newline);
    free(e);
    return 0;
}

//------------------------THE CD FUNCTION ------------------------------------

int cd(char *path, struct ast_variable **list_variables)
{
    char pwd[9560];
    struct ast_variable *new = calloc(1, sizeof(struct ast_variable));
    new->name = strdup("OLDPWD");
    new->value_node = NULL;
    if (!strcmp("", path) || !strcmp("~", pwd))
    {
        getcwd(pwd, 9256);
        setenv("OLDPWD", pwd, 1);
        new->value_str = strdup(pwd);
        if (chdir(getenv("HOME")) == -1)
        {
            free(new->name);
            free(new->value_str);
            free(new);
            fprintf(stderr, "error\n");
            fflush(stderr);
            return 2;
        }
        new->next = (*list_variables);
        (*list_variables) = new;
        return 0;
    }
    else if (!strcmp("pwd", path))
    {
        free(new->name);
        free(new);
        getcwd(pwd, 9256);
        printf("%s\n", pwd);
        fflush(stdout);
        return 0;
    }
    else if (!strcmp("-", path)) //
    {
        char *old = getenv("OLDPWD");
        getcwd(pwd, 9256);
        setenv("OLDPWD", pwd, 1);
        new->value_str = strdup(pwd);
        if (chdir(old) == -1)
        {
            free(new->name);
            free(new->value_str);
            free(new);
            return 2;
        }
        getcwd(pwd, 9256);
        printf("%s\n", old);
        new->next = (*list_variables);
        (*list_variables) = new;
        return 0;
    }
    else if (getcwd(pwd, 9256) == NULL)
    {
        free(new->name);
        free(new);
        return 1;
    }
    else
    {
        setenv("OLDPWD", pwd, 1);
        new->value_str = strdup(pwd);
        if (chdir(path) == -1)
        {
            free(new->name);
            free(new->value_str);
            free(new);
            fprintf(stderr, "error\n");
            fflush(stderr);
            return 2;
        }
        getcwd(pwd, 9256);
        setenv("PWD", pwd, 1);
        new->next = (*list_variables);
        (*list_variables) = new;
    }
    return 0;
}

//------------------PIPE FUNCTION ----------------------------------------------

int pipe_function(struct ast *ast, struct ast_function **list,
                  struct ast_variable **list_variables, int type_loop)
{
    struct ast *argv_left = ast->data.pipe->left_NY;
    struct ast *argv_right = ast->data.pipe->right_NY;
    int tube[2];
    if (pipe(tube) == -1)
    {
        ast->status = 1;
        return 1;
    }
    int _stdout;
    if ((_stdout = dup(STDOUT_FILENO)) == -1)
    {
        ast->status = 1;
        return 1;
    }
    if ((dup2(tube[1], STDOUT_FILENO)) == -1)
    {
        ast->status = 1;
        return 1;
    }
    int status = ast_execution(argv_left, list, list_variables, type_loop);
    if ((dup2(_stdout, STDOUT_FILENO)) == -1)
    {
        ast->status = status;
        return status;
    }
    if ((close(_stdout) == -1))
    {
        ast->status = status;
        return status;
    }
    if ((close(tube[1]) == -1))
    {
        ast->status = status;
        return status;
    }
    int _stdin;
    if ((_stdin = dup(STDIN_FILENO)) == -1)
    {
        ast->status = status;
        return status;
    }
    if ((dup2(tube[0], STDIN_FILENO)) == -1)
    {
        ast->status = status;
        return status;
    }
    status = ast_execution(argv_right, list, list_variables, type_loop);
    if ((dup2(_stdin, STDIN_FILENO)) == -1)
    {
        ast->status = status;
        return status;
    }
    if (close(_stdin) == -1)
    {
        ast->status = status;
        return status;
    }
    if (close(tube[0]) == -1)
    {
        ast->status = status;
        return status;
    }
    ast->status = status;
    return status;
}

//----------------------NEG FUNCTION-----------------------------------------
int negation_function(struct ast *ast, struct ast_function **list,
                      struct ast_variable **list_variables, int type_loop)
{
    int status = ast_execution(ast->data.negation->node_NY, list,
                               list_variables, type_loop);
    if (status == 0)
    {
        ast->status = 1;
        return status;
    }
    ast->status = 0;
    return status;
}

//----------------------WHILE FUNCTION--------------------------------------
//

// verifier la valeur de retour
int while_function(struct ast *ast, struct ast_function **list,
                   struct ast_variable **list_variables)
{
    while (
        ast_execution(ast->data._while->left_condition, list, list_variables, 1)
        == 0)
    {
        int status =
            ast_execution(ast->data._while->right_do, list, list_variables, 1);
        if (status == 200)
        {
            break;
        }
        else if (status == 300)
        {
            continue;
        }
    }
    ast->status = 0;
    return 0;
}

//------------------------UNTIL FUNCTION-------------------------------------

int until_function(struct ast *ast, struct ast_function **list,
                   struct ast_variable **list_variables)
{
    while (
        ast_execution(ast->data._while->left_condition, list, list_variables, 0)
        != 0)
    {
        int status =
            ast_execution(ast->data._while->right_do, list, list_variables, 1);
        if (status == 200)
        {
            break;
        }
        else if (status == 300)
        {
            continue;
        }
    }
    ast->status = 0;
    return 0;
}
/*-------------------CASE FUNCTIOn----------------------------------*/

int case_function(struct ast *ast, struct ast_function **list,
                  struct ast_variable **list_variables, int type_loop)
{
    struct ast_case *current_case = ast->data._case;
    char *var_cond =
        variable_treatement(current_case->variable, list_variables);
    ast->status = 0;
    for (int i = 0; i < current_case->nb_case_item; i++)
    {
        struct case_item *cursor = current_case->tab_case_item[i];
        for (int y = 0; y < cursor->nb_variables; y++)
        {
            char *tmp =
                variable_treatement(cursor->tab_variables[y], list_variables);
            if (!strcmp(tmp, var_cond))
            {
                free(tmp);
                free(var_cond);
                ast->status = ast_execution(cursor->function_NY, list,
                                            list_variables, type_loop);
                return ast->status;
            }
            free(tmp);
        }
    }
    free(var_cond);
    return ast->status;
}

//---------------------FOR FUNCTION------------------------------------
//
int for_function(struct ast *ast, struct ast_function **list,
                 struct ast_variable **list_variables)
{
    struct ast_for *my_for = ast->data._for;
    if (my_for && my_for->list->nb_childrens > 0)
    {
        for (int i = 0; i < my_for->list->nb_childrens; i++)
        {
            struct ast_variable *new_var =
                calloc(1, sizeof(struct ast_variable));
            new_var->name = strdup(
                my_for->left_condition->childrens[0]->data.command->command);
            if (my_for->list->childrens[i]->type == NODE_COMMAND)
            {
                new_var->value_str = variable_treatement(
                    my_for->list->childrens[i]->data.command->command,
                    list_variables);
            }
            else
            {
                free(new_var->name);
                free(new_var);
                ast->status = 1;
                return 1;
            }
            if (!strcmp(new_var->value_str, ""))
            {
                free(new_var->name);
                free(new_var->value_str);
                free(new_var);
                ast->status = 0;
                return 0;
            }
            struct ast_command *value_node =
                calloc(1, sizeof(struct ast_command));
            struct lexer *lexer = lexer_new(new_var->value_str);
            struct token *tok = lexer_peek(lexer);
            value_node->command = word(tok);
            value_node->args =
                calloc(strlen(new_var->value_str) + 10, sizeof(char *));
            token_free(tok);
            token_free(lexer_pop(lexer));
            tok = lexer_peek(lexer);
            while (tok->type != TOKEN_EOF)
            {
                value_node->args[value_node->nb_args] = word(tok);
                value_node->nb_args += 1;
                token_free(tok);
                token_free(lexer_pop(lexer));
                tok = lexer_peek(lexer);
            }
            token_free(tok);
            lexer_free(lexer);
            new_var->value_node = value_node;
            new_var->next = (*list_variables);
            (*list_variables) = new_var;
            int status =
                ast_execution(my_for->right_do, list, list_variables, 1);
            if (status == 200)
            {
                ast->status = 0;
                return 0;
                ;
            }
            else if (status == 300)
            {
                ast->status = 0;
                continue;
            }
            else
            {
                ast->status = status;
            }
        }
        return ast->status;
    }
    else
    {
        ast->status = 0;
        return 0;
    }
}

//----------------------REDIRECTION FUNCTION------------------------------------

int red_function(struct ast *ast, struct ast_function **list,
                 struct ast_variable **list_variables, int type_loop,
                 int *fd_env_in, int *fd_env_out)
{
    struct ast_red *red = ast->data.redirection;
    char *redir = red->val;
    int status = 0;
    if (strcmp(redir, ">") == 0) // succe
    {
        int fd = open(red->right_name, O_WRONLY | O_CREAT, 0644);
        if (fd == -1)
        {
            ast->status = 2;
            return 2;
        }
        int old_stdout = dup(STDOUT_FILENO);
        if (red->io_number != -1)
        {
            if ((dup2(fd, red->io_number)) == -1)
            {
                ast->status = 1;
                return 1;
            }
        }
        else
        {
            if (*fd_env_out == STDOUT_FILENO)
            {
                if (dup2(fd, *fd_env_out) == -1)
                {
                    ast->status = 1;
                    return 1;
                }
                *fd_env_out = fd;
            }
        }
        if (red->left_NY->childrens[0] != NULL
            && red->left_NY->childrens[0]->type != NODE_RED)
        {
            status =
                ast_execution(red->left_NY, list, list_variables, type_loop);
        }
        else
        {
            status =
                red_function(red->left_NY->childrens[0], list, list_variables,
                             type_loop, fd_env_in, fd_env_out);
        }
        if (close(fd) == -1)
        {
            ast->status = status;
            return status;
        }
        dup2(old_stdout, 1);
        ast->status = status;
        return status;
    }
    else if (strcmp(redir, "<") == 0) // succe
    {
        int fd = open(red->right_name, O_RDONLY, 0644);
        if (fd == -1)
        {
            fprintf(stderr, "error\n");
            fflush(stderr);
            ast->status = 2;
            return 2;
        }
        int old_stdin = dup(STDIN_FILENO);
        if (red->io_number != -1)
        {
            if ((dup2(fd, red->io_number)) == -1)
            {
                ast->status = 1;
                return 1;
            }
        }
        else
        {
            if (*fd_env_in == STDIN_FILENO)
            {
                if (dup2(fd, STDIN_FILENO) == -1)
                {
                    ast->status = 1;
                    return 1;
                }
                *fd_env_in = fd;
            }
        }
        if (red->left_NY->childrens[0] != NULL
            && red->left_NY->childrens[0]->type != NODE_RED)
        {
            status =
                ast_execution(red->left_NY, list, list_variables, type_loop);
        }
        else
        {
            status =
                red_function(red->left_NY->childrens[0], list, list_variables,
                             type_loop, fd_env_in, fd_env_out);
        }
        if (close(fd) == -1)
        {
            ast->status = status;
            return status;
        }
        dup2(old_stdin, 0);
        ast->status = status;
        return status;
    }
    else if (strcmp(redir, ">>") == 0) // succe
    {
        int fd = open(red->right_name, O_APPEND | O_WRONLY | O_CREAT, 0644);
        if (fd == -1)
        {
            ast->status = 2;
            return 2;
        }
        int old_stdout = dup(STDOUT_FILENO);
        if (red->io_number != -1)
        {
            if ((dup2(fd, red->io_number)) == -1)
            {
                ast->status = 1;
                return 1;
            }
        }
        else
        {
            if (*fd_env_out == STDOUT_FILENO)
            {
                if (dup2(fd, *fd_env_out) == -1)
                {
                    ast->status = 1;
                    return 1;
                }
                *fd_env_out = fd;
            }
        }
        if (red->left_NY->childrens[0] != NULL
            && red->left_NY->childrens[0]->type != NODE_RED)
        {
            status =
                ast_execution(red->left_NY, list, list_variables, type_loop);
        }
        else
        {
            status =
                red_function(red->left_NY->childrens[0], list, list_variables,
                             type_loop, fd_env_in, fd_env_out);
        }
        if (close(fd) == -1)
        {
            ast->status = status;
            return status;
        }
        dup2(old_stdout, 1);
        ast->status = status;
        return status;
    }
    else if (strcmp(redir, "<<") == 0)
    {}
    else if (strcmp(redir, "<<-") == 0)
    {}
    else if (strcmp(redir, ">&") == 0) // SUCCES
    {
        int fd = my_atoi(red->right_name);
        if (fd == -1)
        {
            fd = open(red->right_name, O_WRONLY | O_CREAT, 0644);
        }
        if (fd == -1)
        {
            ast->status = 2;
            return 2;
        }
        int old_stdout = dup(STDOUT_FILENO);
        if (red->io_number != -1)
        {
            if ((dup2(fd, red->io_number)) == -1)
            {
                ast->status = 1;
                return 1;
            }
        }
        else
        {
            if (*fd_env_out == STDOUT_FILENO)
            {
                if (dup2(fd, *fd_env_out) == -1)
                {
                    ast->status = 1;
                    return 1;
                }
                *fd_env_out = fd;
            }
        }
        if (red->left_NY->childrens[0] != NULL
            && red->left_NY->childrens[0]->type != NODE_RED)
        {
            status =
                ast_execution(red->left_NY, list, list_variables, type_loop);
        }
        else
        {
            status =
                red_function(red->left_NY->childrens[0], list, list_variables,
                             type_loop, fd_env_in, fd_env_out);
        }
        if (close(fd) == -1)
        {
            ast->status = status;
            return status;
        }
        dup2(old_stdout, 1);
        ast->status = status;
        return status;
    }
    else if (strcmp(redir, "<&") == 0)
    {
        int fd = my_atoi(red->right_name);
        if (fd == -1)
        {
            fd = open(red->right_name, O_RDONLY, 0644);
        }
        if (fd == -1)
        {
            fprintf(stderr, "error");
            ast->status = 2;
            return 2;
        }
        int old_stdin = dup(STDIN_FILENO);
        if (red->io_number != -1)
        {
            if ((dup2(fd, red->io_number)) == -1)
            {
                ast->status = 1;
                return 1;
            }
        }
        else
        {
            if (*fd_env_in == STDIN_FILENO)
            {
                if (dup2(fd, STDIN_FILENO) == -1)
                {
                    ast->status = 1;
                    return 1;
                }
            }
        }
        *fd_env_in = fd;
        if (red->left_NY->childrens[0] != NULL
            && red->left_NY->childrens[0]->type != NODE_RED)
        {
            status =
                ast_execution(red->left_NY, list, list_variables, type_loop);
        }
        else
        {
            status =
                red_function(red->left_NY->childrens[0], list, list_variables,
                             type_loop, fd_env_in, fd_env_out);
        }
        if (close(fd) == -1)
        {
            ast->status = status;
            return status;
        }
        dup2(old_stdin, 0);
        ast->status = status;
        return status;
    }
    else if (strcmp(redir, ">|") == 0)
    {}
    else if (strcmp(redir, "<>") == 0)
    {}
    ast->status = 1;
    return 1;
}

//----------------------------------EXIT---------------------------------------
int exit_42(char *in, struct ast_variable **list_variables)
{
    if (list_variables)
    {
        if (!strcmp(in, ""))
        {
            exit(0);
        }
        int exit_status = my_atoi(in);
        if (exit_status == -1)
        {
            fprintf(stderr, "unexpected token\n");
            fflush(stderr);
            return 2;
        }
        else
        {
            exit(exit_status);
        }
        return exit_status;
    }

    if (!strcmp(in, ""))
    {
        exit(0);
    }
    int exit_status = my_atoi(in);
    if (exit_status == -1)
    {
        fprintf(stderr, "unexpected token\n");
        fflush(stderr);
        return 2;
    }
    else
    {
        exit(exit_status);
    }
    return exit_status;
}

int unset_42(char *variable, struct ast_variable **variables_list)
{
    struct ast_variable *parc = variables_list[0];
    while (parc)
    {
        if (!strcmp(variable, parc->name))
        {
            free(parc->name);
            parc->name = strdup("");
        }
        parc = parc->next;
    }
    return 0;
}

//------------------------------------- EXECUTION PART--------------------------
int match_builtins(char *command)
{
    if (command)
    {
        char *builtins[2] = { "cd", "exit" };
        for (int i = 0; i < 2; i++)
        {
            if (!strcmp(builtins[i], command))
            {
                return i;
            }
        }
    }
    return -1;
}

int match_builtins_with_var(char *command)
{
    if (command)
    {
        char *builtins[2] = { "echo", "unset" };
        for (int i = 0; i < 2; i++)
        {
            if (!strcmp(builtins[i], command))
            {
                return i;
            }
        }
    }
    return -1;
}

void array_free(char **array)
{
    for (int i = 0; array[i]; i++)
        free(array[i]);
    free(array);
}

void ast_variables_function(char *data, struct ast_variable **list_variables)
{
    // rajoute une fonction dans la liste de fonctions et fait l expansion si
    // besoin
    struct ast_variable *new = calloc(1, sizeof(struct ast_variable));
    char *name = calloc(strlen(data) + 1, sizeof(char));
    // char *value_str = calloc(strlen(data) + 1, sizeof(char));
    struct ast_command *value_node = calloc(1, sizeof(struct ast_command));
    value_node->args = calloc(strlen(data) + 10, sizeof(char *));
    size_t i = 0;
    while (data[i] != '=')
    {
        name[i] = data[i];
        i++;
    }
    i++;
    int pos = 0;
    char *tmp = calloc(strlen(data) + 10, sizeof(char));
    while (i < strlen(data))
    {
        tmp[pos++] = data[i++];
    }
    char *var = variable_treatement(tmp, list_variables);
    free(tmp);

    struct lexer *lexer = lexer_new(var);
    struct token *tok = lexer_peek(lexer);
    value_node->command = word(tok);
    token_free(tok);
    token_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
    while (tok->type != TOKEN_EOF)
    {
        char *concat = calloc(10000, sizeof(char));
        char *tmp = word(tok);
        size_t y = 0;
        int pos = 0;
        while (y < strlen(tmp))
        {
            if (tmp[y] == '$')
            {
                char *var = calloc(strlen(tmp) + 10, sizeof(char));
                int pos_var = 0;
                var[pos_var++] = tmp[y++];
                while (y < strlen(tmp) && tmp[y] != '$')
                {
                    var[pos_var++] = tmp[y++];
                }
                char *tmp2 = variable_treatement(var, list_variables);
                strcat(concat, tmp2);
                free(tmp2);
                free(var);
                pos = strlen(concat);
            }
            else
            {
                concat[pos++] = tmp[y++];
            }
        }
        free(tmp);
        value_node->args[value_node->nb_args] = concat;
        value_node->nb_args += 1;
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
    }
    token_free(tok);
    lexer_free(lexer);
    new->name = name;
    new->value_str = var;
    new->value_node = value_node;
    new->next = (*list_variables);
    (*list_variables) = new;
}

int variable_execution(struct ast_command *ast, struct ast_function **list,
                       struct ast_variable **list_variables, int type_loop)
{
    char *val = variable_treatement(ast->command, list_variables);
    char *cmd = calloc(strlen(val) + 10, sizeof(char));
    strcat(cmd, val);
    free(val);

    char *cmd_args = calloc(1, sizeof(char));
    int i = 0;
    while (i < ast->nb_args)
    {
        char *tmp =
            calloc(strlen(cmd_args) + strlen(ast->args[i]) + 10, sizeof(char));
        strcat(tmp, cmd_args);
        strcat(tmp, " ");
        strcat(tmp, ast->args[i]);
        free(cmd_args);
        cmd_args = tmp;
        i++;
    }
    val = variable_treatement(cmd_args, list_variables);
    free(cmd_args);
    char *tmp = calloc(strlen(cmd) + strlen(val) + 10, sizeof(char));
    strcat(tmp, cmd);
    strcat(tmp, val);
    free(val);
    free(cmd);
    cmd = strdup(tmp);
    free(tmp);

    struct ast_command *value_node = calloc(1, sizeof(struct ast_command));
    struct lexer *lexer = lexer_new(cmd);
    struct token *tok = lexer_peek(lexer);
    value_node->command = word(tok);
    value_node->args = calloc(strlen(cmd) + 10, sizeof(char *));
    token_free(tok);
    token_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
    while (tok->type != TOKEN_EOF)
    {
        value_node->args[value_node->nb_args] = word(tok);
        value_node->nb_args += 1;
        token_free(tok);
        token_free(lexer_pop(lexer));
        tok = lexer_peek(lexer);
    }
    token_free(tok);
    lexer_free(lexer);
    struct ast *new_ast = ast_init(NODE_COMMAND);
    new_ast->data.command = value_node;
    int status = match_node(new_ast, list, list_variables, type_loop);
    free_command(new_ast);
    free(new_ast);
    free(cmd);
    return status;
}

int match_node(struct ast *ast, struct ast_function **list,
               struct ast_variable **list_variables, int type_loop)
{
    // d'abord verifier si nous avons une variable
    char *var = ast->data.command->command;
    char *expansion = variable_treatement(var, list_variables);
    if (!strcmp(expansion, "break") && type_loop)
    {
        free(expansion);
        ast->status = 200;
        return 200;
    }
    else if (!strcmp(expansion, "continue") && type_loop)
    {
        free(expansion);
        ast->status = 300;
        return 300;
    }
    else if (!strcmp(expansion, "continue") || !strcmp(expansion, "break"))
    {
        fprintf(stderr,
                "%s: only meaningful in a `for', `while', or `until' loop \n",
                expansion);
        fflush(stderr);
        free(expansion);
        ast->status = 0;
        return 0;
    }
    free(expansion);
    if (var && var[0] == '$')
    {
        ast->status = variable_execution(ast->data.command, list,
                                         list_variables, type_loop);
        return ast->status;
    }
    else if (var && var[0] != '\"' && strstr(var, "=") && var[0] != '=')
    {
        // on a trouve un = et nous avons donc une variable
        ast_variables_function(ast->data.command->command, list_variables);
        ast->status = 0;
        // on a des arguments alors je cree un noeud commande et je relance
        // match node)
        if (ast->data.command->nb_args > 0)
        {
            struct ast_command *cmd = calloc(1, sizeof(struct ast_command));
            cmd->command = strdup(ast->data.command->args[0]);
            cmd->args = calloc(ast->data.command->nb_args + 3, sizeof(char *));
            cmd->nb_args = ast->data.command->nb_args - 1;
            int i = 1;
            while (i < ast->data.command->nb_args)
            {
                cmd->args[i - 1] = strdup(ast->data.command->args[i]);
                i++;
            }
            struct ast *rec = ast_init(NODE_COMMAND);
            rec->data.command = cmd;
            ast->status = match_node(rec, list, list_variables, type_loop);
            // ne pas oublier de free
            free(rec);
            free(cmd->command);
            i = 0;
            while (cmd->args[i])
            {
                free(cmd->args[i]);
                i++;
            }
            free(cmd->args);
            free(cmd);
        }
        return ast->status;
    }
    struct ast_function *cursor = (*list);
    while (cursor)
    {
        if (!strcmp(cursor->name, ast->data.command->command))
        {
            struct ast_command *my_cmd = ast->data.command;

            struct ast_variable *new = calloc(1, sizeof(struct ast_variable));
            new->name = strdup("@");
            new->value_node = NULL;
            char *value_str = calloc(1, sizeof(char));
            int j = 0;
            while (j < my_cmd->nb_args)
            {
                char *tmp =
                    calloc(strlen(value_str) + strlen(my_cmd->args[j]) + 10,
                           sizeof(char));
                strcat(tmp, value_str);
                if (strlen(value_str) > 0)
                {
                    strcat(tmp, " ");
                }
                strcat(tmp, my_cmd->args[j]);
                free(value_str);
                value_str = tmp;
                j++;
            }
            new->value_str = value_str;
            new->next = (*list_variables);
            (*list_variables) = new;

            struct ast_variable *new2 = calloc(1, sizeof(struct ast_variable));
            new2->name = strdup("*");
            new2->value_node = NULL;
            new2->value_str = strdup(value_str);
            new2->next = (*list_variables);
            (*list_variables) = new2;

            struct ast_variable *new3 = calloc(1, sizeof(struct ast_variable));
            new3->name = strdup("#");
            new3->value_node = NULL;
            new3->value_str = my_itoa(my_cmd->nb_args);
            new3->next = (*list_variables);
            (*list_variables) = new3;

            for (int x = 0; x < my_cmd->nb_args; x++)
            {
                struct ast_variable *newarg =
                    calloc(1, sizeof(struct ast_variable));
                newarg->name = my_itoa(x + 1);
                newarg->value_node = NULL;
                newarg->value_str = strdup(my_cmd->args[x]);
                newarg->next = (*list_variables);
                (*list_variables) = newarg;
            }

            int status =
                ast_execution(cursor->func_NY, list, list_variables, type_loop);
            unset_42("@", list_variables);
            unset_42("*", list_variables);

            struct ast_variable *new4 = calloc(1, sizeof(struct ast_variable));
            new4->name = strdup("#");
            new4->value_node = NULL;
            new4->value_str = strdup("0");
            new4->next = (*list_variables);
            (*list_variables) = new4;

            for (int x = 0; x < my_cmd->nb_args; x++)
            {
                char *tmpp = my_itoa(x + 1);
                unset_42(tmpp, list_variables);
                free(tmpp);
            }
            return status;
        }
        cursor = cursor->next;
    }
    int ret = 0;
    int (*functions[2])(char *commande,
                        struct ast_variable **list_variables) = { cd, exit_42 };
    int (*functions_with_var[2])(
        char *commande, struct ast_variable **as) = { echo_42, unset_42 };
    struct ast_command *all_commands = ast->data.command;
    char **args = all_commands->args;
    char *command = all_commands->command;
    int i = 0;
    int function_index = match_builtins(command);
    int changed = 0;
    if (function_index < 0)
    {
        function_index = match_builtins_with_var(command);
        if (function_index >= 0)
        {
            changed = 1;
        }
        else
        {
            ret = 1;
        }
    }
    int len = 0;
    for (int j = 0; j < all_commands->nb_args; j++)
    {
        len += strlen(args[j]);
    }
    char *concat = calloc(len + len * 2, sizeof(char));
    if (all_commands->nb_args > 0)
    {
        while (i < all_commands->nb_args) // Appeller notre fonction sur
                                          // l'ensemble des arguments
        {
            strcat(concat, args[i]);
            if (args[i + 1])
                strcat(concat, " ");
            i++;
        }
    }
    else
    {
        free(concat);
        concat = strdup("");
    }
    if (function_index >= 0)
    {
        if (changed == 0)
            ret = functions[function_index](concat, list_variables);
        else
            ret = functions_with_var[function_index](concat, list_variables);
    }
    free(concat);

    if (function_index == -1)
    {
        char **arg2 = calloc(20000, sizeof(char *));
        int fk = fork();
        if (fk == 0)
        {
            if (command)
            {
                arg2[0] = command;
                for (int i = 0; i < all_commands->nb_args; i++)
                {
                    arg2[i + 1] = variable_treatement(args[i], list_variables);
                }
                int UKC = execvp(arg2[0], arg2);

                exit(UKC);
            }
            else
            {
                exit(2);
            }
        }
        array_free(arg2);
        int cstatus = 0;
        waitpid(fk, &cstatus, 0);
        if (command
            && (!strcmp("continue", command) || !strcmp("break", command)))
        {
            fprintf(
                stderr,
                "%s: only meaningful in a `for', `while', or `until' loop \n",
                command);
            fflush(stderr);
            ast->status = 0;
            return 0;
        }
        else if (WEXITSTATUS(cstatus) == 2)
        {
            // fprintf(stderr, "%s: command not found\n", command);
            // fflush(stderr);
            ast->status = 2;
            return 2;
        }
        else if (WEXITSTATUS(cstatus) == 1)
        {
            ast->status = 1;
            return 1;
        }
        else if (WEXITSTATUS(cstatus) != 0)
        {
            if (command)
            {
                fprintf(stderr, "%s: command not found\n", command);
                fflush(stderr);
            }
            ast->status = 127;
            return 127;
        }
        ast->status = 0;
        return 0;
    }
    ast->status = ret;
    return ret;
}

int ast_execution(struct ast *ast, struct ast_function **list,
                  struct ast_variable **list_variables, int type_loop)
{
    for (int pos_child = 0; pos_child < ast->nb_childrens; pos_child++)
    {
        if (ast->childrens[pos_child]->type == NODE_PIPE)
        {
            pipe_function(ast->childrens[pos_child], list, list_variables,
                          type_loop);
        }
        else if (ast->childrens[pos_child]->type == NODE_RED)
        {
            int m = STDOUT_FILENO;
            int n = STDIN_FILENO;
            red_function(ast->childrens[pos_child], list, list_variables,
                         type_loop, &n, &m);
        }
        else if (ast->childrens[pos_child]->type == NODE_COMMAND)
        {
            int status = match_node(ast->childrens[pos_child], list,
                                    list_variables, type_loop);
            ast->childrens[pos_child]->status = status;
            if (status == 200 || status == 300)
            {
                return status;
            }
        }
        else if (ast->childrens[pos_child]->type == NODE_IF
                 || ast->childrens[pos_child]->type == NODE_ELIF)
        {
            int status = ast_execution(
                ast->childrens[pos_child]->data._if->left_condition, list,
                list_variables, type_loop);
            if (status == 0 || status == 200 || status == 300)
            {
                ast->childrens[pos_child]->status = status;
                ast->childrens[pos_child]->data._if->right_then->status =
                    ast_execution(
                        ast->childrens[pos_child]->data._if->right_then, list,
                        list_variables, type_loop);
                pos_child++;
                while (pos_child < ast->nb_childrens
                       && ast->childrens[pos_child]->type != NODE_FI)
                {
                    ast->childrens[pos_child]->status = 1;
                    pos_child++;
                }
                pos_child--;
            }
            else
            {
                ast->childrens[pos_child]->status = 1;
            }
        }
        else if (ast->childrens[pos_child]->type == NODE_ELSE)
        {
            ast->childrens[pos_child]->status =
                ast_execution(ast->childrens[pos_child]->data._else->_then,
                              list, list_variables, type_loop);
        }
        else if (ast->childrens[pos_child]->type == NODE_FI)
        {
            int status = 0;
            int i = pos_child - 1;
            int ok = 1;
            if (ast->childrens[i]->type == NODE_ELSE)
            {
                status = 127;
                if (ast->childrens[i]->status == 0
                    || ast->childrens[i]->status == 200
                    || ast->childrens[i]->status == 300)
                {
                    ok = 0;
                    status = ast->childrens[i]->status;
                }
                else
                {
                    i--;
                }
            }
            if (ok)
            {
                while (i >= 0 && ast->childrens[i]->type != NODE_IF)
                {
                    if (ast->childrens[i]->status == 0
                        || ast->childrens[i]->status == 200
                        || ast->childrens[i]->status == 300)
                    {
                        ok = 0;
                        status =
                            ast->childrens[i]->data._if->right_then->status;
                        if (status == 1)
                            status = 127;
                        if (status == 2)
                            status = 1;
                        break;
                    }
                    i--;
                }
                if (ok)
                {
                    if (ast->childrens[i]->data._if->left_condition->status == 0
                        || ast->childrens[i]->status == 200
                        || ast->childrens[i]->status == 300)
                    {
                        status =
                            ast->childrens[i]->data._if->right_then->status;
                        if (status == 1)
                            status = 127;
                        if (status == 2)
                            status = 1;
                    }
                }
            }
            ast->childrens[pos_child]->status = status;
        }
        else if (ast->childrens[pos_child]->type == NODE_NEG)
        {
            negation_function(ast->childrens[pos_child], list, list_variables,
                              type_loop);
        }
        else if (ast->childrens[pos_child]->type == NODE_WHILE)
        {
            while_function(ast->childrens[pos_child], list, list_variables);
        }
        else if (ast->childrens[pos_child]->type == NODE_UNTIL)
        {
            until_function(ast->childrens[pos_child], list, list_variables);
        }
        else if (ast->childrens[pos_child]->type == NODE_FUNCTION)
        {
            struct ast_function *new_func =
                ast->childrens[pos_child]->data.function;
            new_func->next = NULL;
            if (!(*list))
            {
                *list = new_func;
            }
            else
            {
                new_func->next = (*list);
                (*list) = new_func;
            }
        }
        else if (ast->childrens[pos_child]->type == NODE_FOR)
        {
            for_function(ast->childrens[pos_child], list, list_variables);
        }
        else if (ast->childrens[pos_child]->type == NODE_OR)
        {
            int status =
                ast_execution(ast->childrens[pos_child]->data.and_or->left_NY,
                              list, list_variables, type_loop);
            if (status != 0)
                status = ast_execution(
                    ast->childrens[pos_child]->data.and_or->right_NY, list,
                    list_variables, type_loop);
            ast->childrens[pos_child]->status = status;
        }
        else if (ast->childrens[pos_child]->type == NODE_AND)
        {
            int status =
                ast_execution(ast->childrens[pos_child]->data.and_or->left_NY,
                              list, list_variables, type_loop);
            if (status == 0)
                status = ast_execution(
                    ast->childrens[pos_child]->data.and_or->right_NY, list,
                    list_variables, type_loop);
            ast->childrens[pos_child]->status = status;
        }
        else if (ast->childrens[pos_child]->type == NODE_CASE)
        {
            int status = case_function(ast->childrens[pos_child], list,
                                       list_variables, type_loop);
            ast->childrens[pos_child]->status = status;
        }
        else
        {
            ast->status = 1;
            return 1;
        }
    }
    if (ast->nb_childrens > 0)
    {
        ast->status = ast->childrens[ast->nb_childrens - 1]->status;
    }
    else
    {
        ast->status = 0;
    }
    return ast->status;
}
