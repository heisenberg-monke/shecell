#ifndef LEXER_H
#define LEXER_H

#include "utils.h"

typedef enum TokenType
{
    TOKEN_WORD,

    TOKEN_PIPE,
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_AMPERSAND,
    TOKEN_SEMICOLON,
    TOKEN_DSEMI,

    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_APPEND,
    TOKEN_HEREDOC,
    TOKEN_DUP_IN,
    TOKEN_DUP_OUT,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_NEWLINE,
    
    TOKEN_EOF,
    TOKEN_ERROR
}
TokenType;

typedef struct Token
{
    StringView value;
    TokenType type;
}
Token;

typedef struct Tokens
{
    Token *data;
    size_t size;
    size_t capacity;
}
Tokens;

typedef struct Lexer
{
    Tokens tokens;
    StringView input;
    size_t pos;
}
Lexer;

const char *TokenType_toStr(TokenType type);

void Lexer_tokenize(Lexer *lexer, const char *input);

static inline void Lexer_destroy(Lexer *this) {
    free(this->tokens.data);
}

#endif

