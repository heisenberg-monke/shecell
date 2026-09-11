#include "parser.h"
#include "shecell.h"

#include <stddef.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

void showHistory()
{
    HIST_ENTRY **history = history_list();

    if(!history)
        return;

    for(size_t i = 0; history[i]; ++i)
        LOG_OUT("%zu  %s\n", i+1, history[i]->line);
}

void showHelp() {
    LOG_OUT("Builtins: history, exit, help\n");
}

void run()
{
    bool isRunning = true;

    while(isRunning)
    {
        char *input = readline("> ");

        if(!input)
        {
            printf("\n");
            break;
        }

        if(*input != '\0')
            add_history(input);

        Lexer lexer = {0};
        Tokens *tokens = &lexer.tokens;
        Parser parser = {tokens, 0};

        Lexer_tokenize(&lexer, input);
        Ast *root = Parser_parse(&parser);

        if(!strcmp(input, "exit"))
            isRunning = false;

        else if(!strcmp(input, "history"))
            showHistory();

        else if(!strcmp(input, "help"))
            showHelp();
        
        else {
            for(size_t i = 0; i < tokens->size; ++i)
            {
                Token *token = &tokens->data[i];
                LOG_OUT("Value: " SV_FMT " | type: %s\n", SV_ARG(&token->value), TokenType_toStr(token->type));
            }

            LOG_OUT("\n");
            Ast_print(root, 0);
        }
        
        LOG_OUT("\n");

        Lexer_destroy(&lexer); 
        free(input);
    }
}

int main(int argc, char **argv)
{
    bool debug = false;
    bool help = false;

    for(int i = 1; i < argc; ++i)
    {
        if(!strcmp(argv[i], "-d"))
            debug = true;

        else if(!strcmp(argv[i], "-h"))
            help = true;
    }

    Logger_setDebug(debug);

    if(help)
        showHelp();

    else
        run();

    return 0;
}