#ifndef AST_H
#define AST_H

#include "lexer.h"

typedef enum RedirectType
{
    REDIRECT_IN,
    REDIRECT_OUT,
    REDIRECT_APPEND,
    REDIRECT_HEREDOC   
}
RedirectType;

typedef struct AstRedirect
{
    RedirectType type;
    const Token *target;
}
AstRedirect;

typedef enum AstType
{
    AST_LIST,
    AST_AND,
    AST_OR,
    AST_ASYNC,
    AST_PIPELINE,
    AST_SIMPLE_COMMAND,
    AST_SUBSHELL,
    AST_BRACE_GROUP
}
AstType;

typedef struct Ast Ast;

typedef struct AstList 
{
    Ast **data;
    size_t size;
    size_t capacity;
}
AstList;

typedef struct AstAnd
{
    Ast *left;
    Ast *right;
}
AstAnd;

typedef AstAnd AstOr;

typedef struct AstAsync {
    Ast *body;
}
AstAsync;

typedef struct AstList AstPipeline;

typedef struct AstRedirects
{
    AstRedirect *data;
    size_t size;
    size_t capacity;
} AstRedirects;

typedef struct ASTSimpleCommand
{
    Tokens tokens;
    AstRedirects redirects;
}
ASTSimpleCommand;

typedef struct AstSubshell
{
    Ast *body;
    AstRedirects redirects;
}
AstSubshell;

typedef AstSubshell AstBraceGroup;

struct Ast
{
    AstType type;

    union  {
        AstList list;
        AstAnd and;
        AstOr or;
        AstAsync async;
        AstPipeline pipeline;
        ASTSimpleCommand commands;
        AstSubshell subshell;
        AstBraceGroup groups;
    };
};

void Ast_print(const Ast *ast, size_t depth);

#endif