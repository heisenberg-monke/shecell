#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct Parser
{
    const Tokens *tokens;
    size_t pos;
}
Parser;

Ast *Parser_parse(Parser *this);

#endif