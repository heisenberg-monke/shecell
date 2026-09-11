#ifndef LEXER_H
#define LEXER_H

#include "utils.h"

typedef enum TokenType
{
    TOKEN_WORD,

    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_APPEND,
    TOKEN_HEREDOC,

    TOKEN_LPAREN,
    TOKEN_LBRACE,
    TOKEN_RPAREN,
    TOKEN_RBRACE,

    TOKEN_EOF,
    TOKEN_ERROR,

    TOKEN_PIPE,
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_AMPERSAND,
    TOKEN_SEMICOLON,

    TOKEN_NEWLINE,
    
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

