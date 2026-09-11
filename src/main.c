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
        LOG_OUT("%zu  %s\n", i+1, history[i]->line);
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

        Lexer lexer = {0};

        Lexer_tokenize(&lexer, input);

        Tokens *tokens = &lexer.tokens;
        for(size_t i = 0; i < tokens->size; ++i)
        {
            Token *token = &tokens->data[i];
            LOG_OUT("Value: " SV_FMT " | type: %s\n", SV_ARG(&token->value), TokenType_toStr(token->type));
        }

        LOG_OUT("\n");

        Lexer_destroy(&lexer); 
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