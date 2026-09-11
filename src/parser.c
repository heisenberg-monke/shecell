#include "parser.h"
#include "lexer.h"
#include "logger.h"
#include "utils.h"

static const Token *peek(Parser *this)
{
    if(this->pos >= this->tokens->size)
        return NULL;

    return &this->tokens->data[this->pos];
}

static const Token *advance(Parser *this)
{
    const Token *token = peek(this);

    if(token)
        ++this->pos;

    return token;
}

static bool check(Parser *this, TokenType type)
{
    const Token *token = peek(this);
    return token && token->type == type;
}

static bool match(Parser *this, TokenType type)
{
    if(!check(this, type))
        return false;

    ++this->pos;
    return true;
}

static bool isRedirect(const Token *token) {
    return token && token->type >= TOKEN_REDIRECT_IN && token->type <= TOKEN_HEREDOC;
}

static bool isCommandStart(const Token *token) {
    return token && token->type >= TOKEN_WORD && token->type <= TOKEN_LBRACE;
}

static bool isListTerminator(const Token *token) {
    return token && token->type >= TOKEN_RPAREN && token->type <= TOKEN_EOF;
}

static void consumeNewlines(Parser *this) {
    while(match(this, TOKEN_NEWLINE));
}

static Ast *createAst(AstType type)
{
    Ast *ast = calloc(1, sizeof(*ast));

    if(!ast)
    {
        LOG_ERR("Out of memory (buy more RAM bozo)\n");
        return NULL;
    }

    ast->type = type;

    return ast;
}

static Ast *parseList(Parser *this);
static Ast *parseListItem(Parser *this);
static Ast *parseAndOr(Parser *this);
static Ast *parsePipeline(Parser *this);
static Ast *parseCommand(Parser *this);
static Ast *parseSimpleCommand(Parser *this);
static Ast *parseSubshell(Parser *this);
static Ast *parseBraceGroup(Parser *this);

static bool parseRedirect(Parser *this, AstRedirect *out)
{
    const Token *operator = peek(this);

    if(!operator || !(isRedirect(operator)))
    {
        LOG_ERR("Syntax error: Unexpected redirection.\n");
        return false;
    }

    advance(this);

    if(!check(this, TOKEN_WORD))
    {
        LOG_ERR("Syntax error: Expected word after redirection.\n");
        return false;
    }

    const Token *target = advance(this);

    if(!isRedirect(operator))
    {
        LOG_ERR("Internal error: Invalid redirection.\n");
        return false;
    }

    out->type = operator->type - TOKEN_REDIRECT_IN;
    out->target = target;

    return true;
}

static bool parseRedirects(Parser *this, AstRedirects *redirects)
{
    while(isRedirect(peek(this)))
    {
        AstRedirect redirect;

        if(!parseRedirect(this, &redirect))
            return false;

        VEC_PUSH(redirects, redirect);
    }

    return true;
}

static Ast *parseList(Parser *this)
{
    AstList list = {0};

    if(isListTerminator(peek(this)))
        return NULL;

    Ast *item = parseListItem(this);

    if(!item)
        return NULL;

    VEC_PUSH(&list, item);

    while(true)
    {
        if(match(this, TOKEN_SEMICOLON) || check(this, TOKEN_NEWLINE))
        {
            consumeNewlines(this);

            if(isListTerminator(peek(this)))
                break;

            item = parseListItem(this);

            if(!item)
            {
                free(list.data);
                return NULL;
            }

            VEC_PUSH(&list, item);
            continue;
        }

        break;
    }

    if(list.size == 1)
    {
        Ast *result = list.data[0];
        free(list.data);

        return result;
    }

    Ast *ast = createAst(AST_LIST);

    if(!ast)
    {
        free(list.data);
        return NULL;
    }

    ast->list = list;
    return ast;
}

static Ast *parseListItem(Parser *this)
{
    Ast *body = parseAndOr(this);

    if(!body)
        return NULL;

    if(match(this, TOKEN_AMPERSAND))
    {
        Ast *ast = createAst(AST_ASYNC);

        if(!ast)
            return NULL;

        ast->async.body = body;

        return ast;
    }

    return body;
}

static Ast *parseAndOr(Parser *this)
{
    Ast *left = parsePipeline(this);

    if(!left)
        return NULL;

    while(check(this, TOKEN_AND) || check(this, TOKEN_OR))
    {
        const Token *operator = advance(this);
        Ast *right = parsePipeline(this);

        if(!right)
        {
            LOG_ERR("Syntax error: Expected command after '" SV_FMT "'\n", SV_ARG(&operator->value));
            return NULL;
        }

        Ast *ast;

        if(operator->type == TOKEN_AND)
        {
            ast = createAst(AST_AND);

            if(!ast)
                return NULL;

            ast->and.left = left;
            ast->and.right = right;
        }
            
        else
        {
            ast = createAst(AST_OR);

            if(!ast)
                return NULL;

            ast->or.left = left;
            ast->or.right = right;
        }
            
        left = ast;
    }

    return left;
}

static Ast *parsePipeline(Parser *this)
{
    AstPipeline pipeline = {0};

    Ast *command = parseCommand(this);

    if(!command)
        return NULL;

    VEC_PUSH(&pipeline, command);

    while(match(this, TOKEN_PIPE))
    {
        const Token *token = peek(this);

        if(!isCommandStart(token))
        {
            LOG_ERR("Syntax error: Expected command after '|'\n");
            return NULL;
        }

        command = parseCommand(this);

        if(!command)
        {
            free(pipeline.data);
            return NULL;
        }

        VEC_PUSH(&pipeline, command);
    }

    if(pipeline.size == 1)
    {
        Ast *result = pipeline.data[0];
        free(pipeline.data);

        return result;
    }

    Ast *ast = createAst(AST_PIPELINE);

    if(!ast)
    {
        free(pipeline.data);
        return NULL;
    }

    ast->pipeline = pipeline;

    return ast;
}

static Ast *parseCommand(Parser *this)
{
    const Token *token = peek(this);

    if(!token)
    {
        LOG_ERR("Syntax error: Unexpected end of input.\n");
        return NULL;
    }

    switch(token->type)
    {
        case TOKEN_LPAREN:
            return parseSubshell(this); 

        case TOKEN_LBRACE:
            return parseBraceGroup(this);

        default:
        {
            if(token->type >= TOKEN_WORD && token->type <= TOKEN_HEREDOC)
                return parseSimpleCommand(this);

            LOG_ERR("Syntax error: Expected command.\n");
            return NULL;
        }
    }
}

static Ast *parseSimpleCommand(Parser *this)
{
    ASTSimpleCommand command = {0};
    bool hasComponent = false;

    while(true)
    {
        const Token *token = peek(this);

        if(!token)
            break;

        if(token->type == TOKEN_WORD)
        {
            VEC_PUSH(&command.tokens, *advance(this));
            hasComponent = true;

            continue;
        }

        if(isRedirect(token))
        {
            AstRedirect redirect;

            if(!parseRedirect(this, &redirect))
            {
                free(command.tokens.data);
                free(command.redirects.data);

                return NULL;
            }

            VEC_PUSH(&command.redirects, redirect);
            hasComponent = true;

            continue;
        }

        break;
    }

    if(!hasComponent)
    {
        LOG_ERR("Syntax error: Expected command.\n");

        free(command.tokens.data);
        free(command.redirects.data);

        return NULL;
    }

    Ast *ast = createAst(AST_SIMPLE_COMMAND);

    if(!ast)
    {
        free(command.tokens.data);
        free(command.redirects.data);

        return NULL;
    }

    ast->commands = command;

    return ast;
}

static Ast *parseSubshell(Parser *this)
{
    if(!match(this, TOKEN_LPAREN))
    {
        LOG_ERR("Internal error: expected '('.\n");
        return NULL;
    }

    consumeNewlines(this);

    Ast *body = NULL;

    if(!check(this, TOKEN_RPAREN))
    {
        body = parseList(this);

        if(!body)
            return NULL;
    }

    consumeNewlines(this);

    if(!match(this, TOKEN_RPAREN))
    {
        LOG_ERR("Syntax error: Expected ')'.\n");
        return NULL;
    }

    AstRedirects redirects = {0};

    if(!parseRedirects(this, &redirects))
    {
        free(redirects.data);
        return NULL;
    }

    Ast *ast = createAst(AST_SUBSHELL);

    if(!ast)
    {
        free(redirects.data);
        return NULL;
    }

    ast->subshell.body = body;
    ast->subshell.redirects = redirects;

    return ast;
}

static Ast *parseBraceGroup(Parser *this)
{
    if(!match(this, TOKEN_LBRACE))
    {
        LOG_ERR("Internal error: Expected '{'\n");
        return NULL;
    }

    consumeNewlines(this);

    Ast *body = NULL;

    if(!check(this, TOKEN_RBRACE))
    {
        body = parseList(this);

        if(!body)
            return NULL;
    }

    consumeNewlines(this);

    if(!match(this, TOKEN_RBRACE))
    {
        LOG_ERR("Syntax error: Expected '}\n");
        return NULL;
    }

    AstRedirects redirects = {0};

    if(!parseRedirects(this, &redirects))
    {
        free(redirects.data);
        return NULL;
    }

    Ast *ast = createAst(AST_SUBSHELL);

    if(!ast)
    {
        free(redirects.data);
        return NULL;
    }

    ast->groups.body = body;
    ast->groups.redirects = redirects;

    return ast;
}

Ast *Parser_parse(Parser *this)
{
    consumeNewlines(this);

    if(check(this, TOKEN_EOF))
        return NULL;

    Ast *root = parseList(this);

    if(!root)
        return NULL;

    consumeNewlines(this);

    if(!check(this, TOKEN_EOF))
    {
        LOG_ERR("Syntax error: Unexpected token.\n");
        return NULL;
    }

    return root;
}