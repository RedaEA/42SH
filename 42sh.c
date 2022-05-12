#include <err.h>
#include <io/cstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/vec.h>

#include "tools/headers/global.h"

/**
 * \brief Parse the command line arguments
 * \return A character stream
 */
static struct cstream *parse_args(int argc, char *argv[])
{
    // If launched without argument, read the standard input
    if (argc == 1)
    {
        if (isatty(STDIN_FILENO))
            return cstream_readline_create();
        return cstream_file_create(stdin, /* fclose_on_free */ false);
    }

    // 42sh FILENAME
    if (argc == 2)
    {
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            warn("failed to open input file");
            return NULL;
        }

        return cstream_file_create(fp, /* fclose_on_free */ true);
    }

    if (argc == 3)
    {
        if (strcmp("-c", argv[1]) == 0)
        {
            char *tmp = "cmd.txt";
            FILE *fp = fopen(tmp, "w");
            if (fp == NULL)
            {
                warn("failed to open input file");
                return NULL;
            }
            fwrite(argv[2], sizeof(char), strlen(argv[2]), fp);
            fwrite("\n", sizeof(char), 1, fp);
            fclose(fp);
            FILE *rfp = fopen(tmp, "r");
            return cstream_file_create(rfp, /* fclose_on_free */ true);
        }

        else if (strcmp("<", argv[1]))
        {
            FILE *fp = fopen(argv[2], "r");
            if (fp == NULL)
            {
                warn("failed to open input file");
                return NULL;
            }

            return cstream_file_create(fp, /* fclose_on_free */ true);
        }
    }
    fprintf(stderr, "Usage: %s [COMMAND]\n", argv[0]);
    fflush(stderr);
    return NULL;
}

/**
 * \brief Read and print lines on newlines until EOF
 * \return An error code
 */
int read_print_loop(struct cstream *cs, struct vec *line)
{
    enum error err;
    /*----------------liste chainée de fonctions-----------------------*/
    struct ast_function *list = NULL;
    /*----------------liste chainée de variables-----------------------*/
    struct ast_variable *list_variables = NULL;
    /*-----------------------------------------------------------------*/
    int res = 0;

    struct ast_variable *new = calloc(1, sizeof(struct ast_variable));
    new->name = strdup("UID");
    new->value_node = NULL;
    int uid = geteuid();
    new->value_str = my_itoa(uid);
    new->next = list_variables;
    list_variables = new;

    struct ast_variable *new3 = calloc(1, sizeof(struct ast_variable));
    new3->name = strdup("?");
    new3->value_node = NULL;
    new3->value_str = strdup("0");
    new3->next = list_variables;
    list_variables = new3;

    struct ast_variable *new2 = calloc(1, sizeof(struct ast_variable));
    new2->name = strdup("$");
    new2->value_node = NULL;
    int pid = getpid();
    new2->value_str = my_itoa(pid);
    new2->next = list_variables;
    list_variables = new2;

    struct ast_variable *new4 = calloc(1, sizeof(struct ast_variable));
    new4->name = strdup("#");
    new4->value_node = NULL;
    new4->value_str = strdup("0");
    new4->next = list_variables;
    list_variables = new4;

    while (true)
    {
        // Read the next character
        int c;
        if ((err = cstream_pop(cs, &c)))
        {
            ast_free_function(list);
            ast_free_variables(list_variables);
            return res;
        }

        // If the end of file was reached, stop right there
        if (c == EOF)
        {
            ast_free_function(list);
            ast_free_variables(list_variables);

            break;
        }
        // If a newline was met, print the line
        if (c == '\n')
        {
            res = main_handler(vec_cstring(line), &list, &list_variables);
            vec_reset(line);

            struct ast_variable *inter = calloc(1, sizeof(struct ast_variable));
            inter->name = strdup("?");
            inter->value_node = NULL;
            if (res == 2)
            {
                inter->value_str = strdup("2");
            }
            else if (res == 1)
            {
                inter->value_str = strdup("1");
            }
            else if (res == 127)
            {
                inter->value_str = strdup("127");
            }
            else
            {
                inter->value_str = strdup("0");
            }
            inter->next = list_variables;
            list_variables = inter;

            continue;
        }

        // Otherwise, add the character to the line
        vec_push(line, c);
    }

    return res;
}

int main(int argc, char *argv[])
{
    // Parse command line arguments and get an input stream
    struct cstream *cs;
    if ((cs = parse_args(argc, argv)) == NULL)
    {
        cstream_free(cs);
        return 1;
    }

    // Create a vector to hold the current line
    struct vec line;
    vec_init(&line);

    // Run the test loop
    int res = read_print_loop(cs, &line);

    cstream_free(cs);
    vec_destroy(&line);
    return res;
}
