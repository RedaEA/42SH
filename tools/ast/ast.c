#include "../headers/global.h"

struct ast *ast_init(enum ast_type type)
{
    struct ast *new = calloc(1, sizeof(struct ast));
    new->type = type;
    if (type == NODE_NOYAU)
    {
        new->childrens = calloc(640000, sizeof(struct ast));
    }
    new->nb_childrens = 0;
    new->status = 0;
    return new;
}
void free_command(struct ast *node);
void free_if(struct ast *node);
void free_else(struct ast *node);
void free_red(struct ast *node);
void free_while(struct ast *node);
void free_pipe(struct ast *node);
void free_for(struct ast *node);
void free_function(struct ast *node);
void free_and_or(struct ast *node);
void free_negation(struct ast *node);
void free_case(struct ast *node);
void ast_free_variables(struct ast_variable *list)
{
    if (list)
    {
        free(list->value_str);
        if (list->value_node)
        {
            struct ast *new = ast_init(NODE_COMMAND);
            new->data.command = list->value_node;
            free_command(new);
            free(new);
            list->value_node = NULL;
        }
        free(list->name);

        ast_free_variables(list->next);
        free(list);
    }
}

void ast_free_function(struct ast_function *ast)
{
    if (ast)
    {
        free(ast->name);
        ast_free(ast->func_NY);
        ast_free_function(ast->next);
        free(ast);
    }
}

void ast_free(struct ast *ast)
{
    if (ast)
    {
        for (int i = 0; i < ast->nb_childrens; i++)
        {
            if (ast->childrens[i] != NULL
                && ast->childrens[i]->type == NODE_COMMAND)
            {
                free_command(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_IF
                     || ast->childrens[i]->type == NODE_ELIF)
            {
                free_if(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_ELSE)
            {
                free_else(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_CASE)
            {
                free_case(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_WHILE)
            {
                free_while(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_AND)
            {
                free_pipe(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_OR)
            {
                free_pipe(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_NEG)
            {
                free_negation(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_PIPE)
            {
                free_pipe(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_UNTIL)
            {
                free_while(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_FOR)
            {
                free_for(ast->childrens[i]);
            }
            else if (ast->childrens[i]->type == NODE_RED)
            {
                free_red(ast->childrens[i]);
            }
            free(ast->childrens[i]);
            ast->childrens[i] = NULL;
        }
        free(ast->childrens);
        ast->childrens = NULL;
        free(ast);
        ast = NULL;
    }
}
void free_red(struct ast *node)
{
    if (node)
    {
        struct ast_red *red_node = node->data.redirection;
        if (red_node->left_NY)
        {
            ast_free(red_node->left_NY);
            red_node->left_NY = NULL;
        }
        if (red_node->right_name)
        {
            free(red_node->right_name);
            red_node->right_name = NULL;
        }
        free(node->data.redirection->val);
        node->data.redirection->val = NULL;
        free(red_node);
        red_node = NULL;
    }
}
void free_pipe(struct ast *node)
{
    if (node)
    {
        struct ast_pipe *pipe_node = node->data.pipe;
        if (pipe_node->left_NY)
        {
            ast_free(pipe_node->left_NY);
        }
        if (pipe_node->right_NY)
        {
            ast_free(pipe_node->right_NY);
        }
        free(pipe_node);
    }
}
void free_command(struct ast *node)
{
    if (node && node->data.command)
    {
        free(node->data.command->command);
        node->data.command->command = NULL;
        for (int i = 0; i < node->data.command->nb_args; i++)
        {
            free(node->data.command->args[i]);
            node->data.command->args[i] = NULL;
        }
        free(node->data.command->args);
        node->data.command->args = NULL;
        free(node->data.command);
        node->data.command = NULL;
    }
}
void free_if(struct ast *node)
{
    if (node)
    {
        struct ast_if *if_node = node->data._if;
        if (if_node->left_condition)
            ast_free(if_node->left_condition);
        if (if_node->right_then)
            ast_free(if_node->right_then);
        free(if_node);
        if_node = NULL;
    }
}

void free_else(struct ast *node)
{
    struct ast_else *else_node = node->data._else;
    ast_free(else_node->_then);
    free(else_node);
    else_node = NULL;
}

void free_function(struct ast *node)
{
    struct ast_function *function_node = node->data.function;
    free(function_node->name);
    ast_free(function_node->func_NY);
    free(function_node);
}
void free_while(struct ast *node)
{
    if (node)
    {
        struct ast_while *while_node = node->data._while;
        if (while_node->left_condition)
            ast_free(while_node->left_condition);
        if (while_node->right_do)
            ast_free(while_node->right_do);
        free(while_node);
        while_node = NULL;
    }
}

void free_for(struct ast *node)
{
    if (node)
    {
        struct ast_for *for_node = node->data._for;
        if (for_node->left_condition)
            ast_free(for_node->left_condition);
        if (for_node->right_do)
            ast_free(for_node->right_do);
        if (for_node->list)
            ast_free(for_node->list);
        free(for_node);
        for_node = NULL;
    }
}
void free_and_or(struct ast *node)
{
    if (node)
    {
        struct ast_and_or *and_or_node = node->data.and_or;
        if (and_or_node->left_NY)
            ast_free(and_or_node->left_NY);
        if (and_or_node->right_NY)
            ast_free(and_or_node->right_NY);
        free(and_or_node);
    }
}
void free_negation(struct ast *node)
{
    struct ast_negation *negation_node = node->data.negation;
    ast_free(negation_node->node_NY);
    free(negation_node);
}

void free_case(struct ast *node)
{
    if (node)
    {
        struct ast_case *node_case = node->data._case;
        free(node_case->variable);
        for (int i = 0; i < node_case->nb_case_item; i++)
        {
            struct case_item *current_item = node_case->tab_case_item[i];
            for (int j = 0; j < current_item->nb_variables; j++)
            {
                free(current_item->tab_variables[j]);
            }
            ast_free(current_item->function_NY);
            free(current_item->tab_variables);
            free(current_item);
        }
        free(node_case->tab_case_item);
        free(node_case);
    }
}

int my_atoi(const char *str)
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    int number = 0;
    int nb_sign = 0;
    int step = 0;
    for (int i = 0; i < len; i++)
    {
        if (str[i] == ' ' && step == 0)
        {
            continue;
        }
        if ((str[i] == '+') || (str[i] == '-'))
        {
            if (nb_sign != 0 || step > 1)
            {
                return -1;
            }
            if (str[i] == '-')
            {
                nb_sign = 2;
            }
            else
            {
                nb_sign = 1;
            }
            step = 1;
            continue;
        }
        if ((str[i] >= '0') && (str[i] <= '9'))
        {
            step = 2;
            number = number * 10 + (str[i] - '0');
        }
        else
        {
            return -1;
        }
    }
    if (nb_sign == 2)
    {
        return -number;
    }
    return number;
}

char *my_itoa(int value)
{
    if (value == 0)
    {
        return strdup("0");
    }

    int i = 0;
    int test = 1;
    while (value / test != 0)
    {
        if (test >= 1000000000)
        {
            i++;
            break;
        }
        i++;
        test = test * 10;
    }
    char *s = calloc(30, sizeof(char));
    if (value > 0)
    {
        for (int y = i - 1; y >= 0; y--)
        {
            s[y] = (value % 10) + '0';
            value = value / 10;
        }
        s[i] = '\0';
        return s;
    }
    else
    {
        s[0] = '-';
        value = -value;
        for (int y = i; y >= 1; y--)
        {
            s[y] = (value % 10) + '0';
            value = value / 10;
        }
        s[i + 1] = '\0';
        return s;
    }
}
