#include "ast.h"
#include "logger.h"

static const char * Ast_redirect_name(RedirectType type)
{
    switch(type)
    {
        case REDIRECT_IN:       return "<";
        case REDIRECT_OUT:      return ">";
        case REDIRECT_APPEND:   return ">>";
        case REDIRECT_HEREDOC:  return "<<";
    }

    return "?";
}

static void Ast_print_indent(size_t depth)
{
    for(size_t i = 0; i < depth; i++)
        LOG_OUT("    ");
}


static void Ast_print_redirects(const AstRedirects *redirects, size_t depth)
{
    for(size_t i = 0; i < redirects->size; i++)
    {
        const AstRedirect *redirect = &redirects->data[i];

        Ast_print_indent(depth);
        LOG_OUT("redirect %s ", Ast_redirect_name(redirect->type));
        LOG_OUT(SV_FMT "\n", SV_ARG(&redirect->target->value));
    }
}

static void Ast_print_tokens(const Tokens *tokens, size_t depth)
{
    for(size_t i = 0; i < tokens->size; i++)
    {
        const Token *token = &tokens->data[i];

        Ast_print_indent(depth);
        LOG_OUT("word ");
        LOG_OUT(SV_FMT "\n", SV_ARG(&token->value));
    }
}

void Ast_print(const Ast *ast, size_t depth)
{
    if(ast == NULL)
    {
        Ast_print_indent(depth);
        LOG_OUT("(null)\n");
        return;
    }

    switch(ast->type)
    {
        case AST_LIST:
        {
            Ast_print_indent(depth);
            LOG_OUT("LIST\n");

            for(size_t i = 0; i < ast->list.size; i++)
                Ast_print(ast->list.data[i], depth + 1);

            break;
        }

        case AST_AND:
        {
            Ast_print_indent(depth);
            LOG_OUT("AND\n");

            Ast_print(ast->and.left, depth + 1);
            Ast_print(ast->and.right, depth + 1);

            break;
        }

        case AST_OR:
        {
            Ast_print_indent(depth);
            LOG_OUT("OR\n");

            Ast_print(ast->or.left, depth + 1);
            Ast_print(ast->or.right, depth + 1);

            break;
        }

        case AST_ASYNC:
        {
            Ast_print_indent(depth);
            LOG_OUT("ASYNC\n");

            Ast_print(ast->async.body, depth + 1);

            break;
        }

        case AST_PIPELINE:
        {
            Ast_print_indent(depth);
            LOG_OUT("PIPELINE\n");

            for(size_t i = 0; i < ast->pipeline.size; i++)
                Ast_print(ast->pipeline.data[i], depth + 1);

            break;
        }

        case AST_SIMPLE_COMMAND:
        {
            const ASTSimpleCommand *command = &ast->commands;

            Ast_print_indent(depth);
            LOG_OUT("SIMPLE_COMMAND\n");

            if(command->tokens.size > 0)
            {
                Ast_print_indent(depth + 1);
                LOG_OUT("WORDS\n");

                Ast_print_tokens(&command->tokens, depth + 2);
            }

            if(command->redirects.size > 0)
            {
                Ast_print_indent(depth + 1);
                LOG_OUT("REDIRECTS\n");

                Ast_print_redirects(&command->redirects, depth + 2);
            }

            break;
        }

        case AST_SUBSHELL:
        {
            Ast_print_indent(depth);
            LOG_OUT("SUBSHELL\n");

            if(ast->subshell.body != NULL)
            {
                Ast_print_indent(depth + 1);
                LOG_OUT("BODY\n");

                Ast_print(ast->subshell.body, depth + 2);
            }

            if(ast->subshell.redirects.size > 0)
            {
                Ast_print_indent(depth + 1);
                LOG_OUT("REDIRECTS\n");

                Ast_print_redirects(&ast->subshell.redirects, depth + 2);
            }

            break;
        }

        case AST_BRACE_GROUP:
        {
            Ast_print_indent(depth);
            LOG_OUT("BRACE_GROUP\n");

            if(ast->groups.body != NULL)
            {
                Ast_print_indent(depth + 1);
                LOG_OUT("BODY\n");

                Ast_print(ast->groups.body, depth + 2);
            }

            if(ast->groups.redirects.size > 0)
            {
                Ast_print_indent(depth + 1);
                LOG_OUT("REDIRECTS\n");

                Ast_print_redirects(&ast->groups.redirects, depth + 2);
            }

            break;
        }
    }
}