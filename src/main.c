#include "shecell.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

void show_history()
{
    HIST_ENTRY **history = history_list();

    if(!history)
        return;

    for(size_t i = 0; history[i]; ++i)
        printf("%zu  %s\n", i+1, history[i]->line);
}

void run()
{
    while(true)
    {
        char *input = readline("> ");

        if(!input)
        {
            printf("\n");
            break;
        }

        if(*input != '\0')
            add_history(input);

        if(!strcmp(input, "exit"))
        {
            free(input);
            break;
        }

        if(!strcmp(input, "history"))
            show_history();

        printf("You entered: %s\n", input);
        free(input);
    }
}

int main(int argc, char **argv)
{
    bool debug = false;

    for(int i = 1; i < argc; ++i)
    {
        if(!strcmp(argv[i], "-d"))
            debug = true;
    }

    Logger_setDebug(debug);
    run();

    return 0;
}