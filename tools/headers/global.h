#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <err.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//-----------------------------LEXER FUNCTIONS----------------
struct lexer
{
    const char *input;
    size_t pos;
    struct token *current_tok;
};
char *word(struct token *tok);

struct lexer *lexer_new(const char *input);

void lexer_free(struct lexer *lexer);

struct token *lexer_peek(struct lexer *lexer);

struct token *lexer_pop(struct lexer *lexer);

//-----------------------------AST FUNCTIONS----------------

enum ast_type
{
    NODE_NOYAU = 0,
    NODE_COMMAND,
    NODE_IF,
    NODE_ELIF,
    NODE_ELSE,
    NODE_FI,
    NODE_PIPE,
    NODE_RED,
    NODE_NEG,
    NODE_AND,
    NODE_OR,
    NODE_WHILE,
    NODE_UNTIL,
    NODE_FOR,
    NODE_FUNCTION,
    NODE_CASE,
};

struct ast_variable
{
    char *name;
    char *value_str;
    struct ast_command *value_node;
    struct ast_variable *next;
};

struct ast_negation
{
    struct ast *node_NY;
};

struct case_item
{
    int nb_variables;
    char **tab_variables;
    struct ast *function_NY;
};

struct ast_case
{
    char *variable;
    int nb_case_item;
    struct case_item **tab_case_item;
};

struct ast_pipe
{
    struct ast *left_NY;
    struct ast *right_NY;
};

struct ast_red
{
    char *val;
    struct ast *left_NY;
    char *right_name;
    int io_number;
};

struct ast_else
{
    struct ast *_then;
};

struct ast_if
{
    struct ast *left_condition;
    struct ast *right_then;
};

struct ast_function
{
    char *name;
    struct ast *func_NY;
    struct ast_function *next;
};

struct ast_and_or
{
    struct ast *left_NY;
    struct ast *right_NY;
};

struct ast_while
{
    struct ast *left_condition;
    struct ast *right_do;
};

struct ast_for
{
    struct ast *left_condition;
    struct ast *list;
    struct ast *right_do;
};

struct ast_command
{
    char *command;
    char **args;
    int nb_args;
};

union ast_data
{
    struct ast_variable *variable;
    struct ast_function *function;
    struct ast_and_or *and_or;
    struct ast_red *redirection;
    struct ast_negation *negation;
    struct ast_command *command;
    struct ast_if *_if;
    struct ast_else *_else;
    struct ast_if *elif;
    struct ast_pipe *pipe;
    struct ast_while *_while;
    struct ast_for *_for;
    struct ast_case *_case;
};

struct ast
{
    enum ast_type type; // type de noeud
    union ast_data data; // la data va varie en fonction du type du noeud

    int nb_childrens;
    struct ast **childrens;

    int status;
};

void ast_free_variables(struct ast_variable *list_variables);
void free_command(struct ast *node);
void free_if(struct ast *node);
void free_else(struct ast *node);
void ast_free(struct ast *ast);
void free_red(struct ast *node);
void free_while(struct ast *node);
void free_for(struct ast *node);
void free_pipe(struct ast *node);
void free_function(struct ast *node);
void free_and_or(struct ast *node);
void free_negation(struct ast *node);
void free_pipe(struct ast *node);
void free_case(struct ast *node);
void ast_free_function(struct ast_function *node);

struct ast *ast_init(enum ast_type type);

//------------------------- TOKEN_H--------------------

enum token_type
{
    TOKEN_IF = 0, // 0
    TOKEN_THEN, // 1
    TOKEN_ELIF, // 3
    TOKEN_ELSE, // 4
    TOKEN_FI, // 5
    TOKEN_EOC, // 6
    TOKEN_EOF, // 7
    TOKEN_NEWLINE, // 8
    TOKEN_WORD, // 9
    TOKEN_ERROR, // 10
    TOKEN_PIPE, // 11
    TOKEN_PARG, // 12
    TOKEN_PARD, // 13
    TOKEN_ACCOG, // 14
    TOKEN_ACCOD, // 15
    TOKEN_OR, // 16
    TOKEN_AND, // 17
    TOKEN_WHILE, // 18
    TOKEN_FOR, // 19
    TOKEN_UNTIL, // 20
    TOKEN_NEG, // 21
    TOKEN_DO, // 22
    TOKEN_DONE, // 23
    TOKEN_ESPER, // 24
    TOKEN_RED, // 25
    TOKEN_IN, // 26
    TOKEN_ESAC, // 27
    TOKEN_CASE, // 28
    TOKEN_ASSWORD,
};

struct token
{
    enum token_type type; ///< The kind of token
    char *value; ///< If the token is a word
};

/**
 * \brief Allocate a new token
 */
struct token *token_new(enum token_type type);

/**
 * \brief Frees a token
 */
void token_free(struct token *token);

//-------------parser.h----------------------
enum parser_status
{
    PARSER_OK = 0,
    PARSER_UNEXPECTED_TOKEN,
};

//----------------BNF FUNCTIONS-------------
enum parser_status handle_parse_error(enum parser_status status,
                                      struct ast **ast);
enum parser_status rule_for(struct ast **ast, struct lexer *lexer);
enum parser_status rule_while(struct ast **ast, struct lexer *lexer);
enum parser_status rule_until(struct ast **ast, struct lexer *lexer);
enum parser_status rule_case(struct ast **ast, struct lexer *lexer);
enum parser_status rule_if(struct ast **ast, struct lexer *lexer);
enum parser_status input(struct ast **ast, struct lexer *lexer);
enum parser_status list(struct ast **ast, struct lexer *lexer);
enum parser_status and_or(struct ast **ast, struct lexer *lexer);
enum parser_status pipeline(struct ast **ast, struct lexer *lexer);
enum parser_status command(struct ast **ast, struct lexer *lexer);
enum parser_status simple_command(struct ast **ast, struct lexer *lexer);
enum parser_status funcdec(struct ast **ast, struct lexer *lexer);
enum parser_status shell_command(struct ast **ast, struct lexer *lexer);
enum parser_status compound_list(struct ast **ast, struct lexer *lexer);
enum parser_status else_clause(struct ast **ast, struct lexer *lexer);
enum parser_status redirection(struct ast **ast, struct lexer *lexer);
enum parser_status element(struct ast **ast, struct ast_command **node_sc,
                           struct lexer *lexer);
enum parser_status prefix(struct ast **ast, struct ast_command **node_sc,
                          struct lexer *lexer);
enum parser_status do_group(struct ast **ast, struct lexer *lexer);
enum parser_status rule_while(struct ast **ast, struct lexer *lexer);
enum parser_status rule_until(struct ast **ast, struct lexer *lexer);
enum parser_status rule_case(struct ast **ast, struct lexer *lexer);
enum parser_status case_clause(struct ast_case **node_case,
                               struct lexer *lexer);
enum parser_status case_item(struct ast_case **node_case, struct lexer *lexer);
//----------------------TEST_FUNCTION----------------
enum parser_status main_handler(char *command, struct ast_function **list,
                                struct ast_variable **variables);
//-------------------EXECUTION-FUNCTIONS-------------

int options(char *commande, int *e, int *newline);

int echo_42(char *commande, struct ast_variable **variable);

int pipe_function(struct ast *ast, struct ast_function **list,
                  struct ast_variable **list_variables, int type_loop);

int match_builtins(char *tab_command);

int match_node(struct ast *ast, struct ast_function **list,
               struct ast_variable **list_variables, int type_loop);

int ast_execution(struct ast *tree, struct ast_function **list,
                  struct ast_variable **variables, int type_loop);

int my_atoi(const char *str);

char *my_itoa(int value);

#endif
