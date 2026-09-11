#include "lexer.h"
#include "logger.h"
#include "utils.h"

#include <ctype.h>
#include <string.h>

static bool atEnd(const Lexer *this) {
    return this->pos >= this->input.length;
}

static char peekN(const Lexer *this, size_t n) {
    return this->pos + n >= this->input.length ? '\0' : this->input.begin[this->pos + n];
}

static char peek(const Lexer *this) {
    return peekN(this, 0);
}

// static char peekNext(const Lexer *this) {
//     return peekN(this, 1);
// }

static char advance(Lexer *this) {
    return atEnd(this) ? '\0' : this->input.begin[this->pos++];
}

static bool match(Lexer *this, char expected) 
{
    if(peek(this) != expected)
        return false;

    return ++this->pos;
}

static Token createToken(const Lexer *this, TokenType type, size_t start)
{
    return (Token)
    {
        .value.begin = this->input.begin + start,
        .value.length = this->pos - start,
        .type = type
    };
}

static bool isBlank(char c) {
    return c == ' ' || c == '\t';
}

static bool isMetachar(char c) 
{
    switch(c)
    {
        case '|':
        case '&':
        case ';':
        case '(':
        case ')':
        case '<':
        case '>':
            return true;

        default:
            return false;
    }
}

static bool singleQuote(Lexer *this)
{
    advance(this);

    while(!atEnd(this))
    {
        if(advance(this) == '\'')
            return true;
    }

    LOG_ERR("Unterminated single quote");
    return false;
}

static bool doubleQuote(Lexer *this)
{
    advance(this);

    while(!atEnd(this))
    {
        char c = advance(this);

        if(c == '"')
            return true;

        if(c == '\\')
        {
            char next = peek(this);

            switch(next)
            {
                case '\n':
                case '$':
                case '`':
                case '"':
                case '\\':
                {
                    advance(this);
                    continue;
                }
            }
        }
    }

    LOG_ERR("Unterminated double quote");
    return false;
}

static bool escape(Lexer *this)
{
    advance(this);

    if(atEnd(this))
    {
        LOG_ERR("Backslash at the end of input");
        return false;
    }

    advance(this);
    return true;
}

static void skipComment(Lexer *this)
{
    advance(this);

    while(!atEnd(this) && peek(this) != '\n')
        advance(this);
}

static Token word(Lexer *this)
{
    size_t start = this->pos;

    while(!atEnd(this))
    {
        char c = peek(this);

        if(isspace(c) || isMetachar(c))
            break;

        if(c == '#')
        {
            advance(this);
            continue;
        }

        if(c == '\\')
        {
            if(!escape(this))
                return createToken(this, TOKEN_ERROR, start);

            continue;
        }

        if(c == '\'')
        {
            if(!singleQuote(this))
                return createToken(this, TOKEN_ERROR, start);

            continue;
        }

        if(c == '"')
        {
            if(!doubleQuote(this))
                return createToken(this, TOKEN_ERROR, start);

            continue;
        }

        advance(this);
    }

    return createToken(this, TOKEN_WORD, start);
}

static Token operator(Lexer *this)
{
    size_t start = this->pos;
    char c = peek(this);

    if(c == '\n')
    {
        advance(this);
        return createToken(this, TOKEN_NEWLINE, start);
    }

    if(c == '|')
    {
        advance(this);

        if(match(this, '|'))
            return createToken(this, TOKEN_OR, start);

        return createToken(this, TOKEN_PIPE, start);
    }

    if(c == '&')
    {
        advance(this);

        if(match(this, '&'))
            return createToken(this, TOKEN_AND, start);

        return createToken(this, TOKEN_AMPERSAND, start);
    }

    if(c == ';')
    {
        advance(this);

        if(match(this, ';'))
            return createToken(this, TOKEN_DSEMI, start);

        return createToken(this, TOKEN_SEMICOLON, start);
    }

    if(c == '<')
    {
        advance(this);

        if(match(this, '<'))
            return createToken(this, TOKEN_HEREDOC, start);

        if(match(this, '&'))
            return createToken(this, TOKEN_DUP_IN, start);

        return createToken(this, TOKEN_REDIRECT_IN, start);
    }

    if(c == '>')
    {
        advance(this);

        if(match(this, '>'))
            return createToken(this, TOKEN_APPEND, start);

        if(match(this, '&'))
            return createToken(this, TOKEN_DUP_OUT, start);

        return createToken(this, TOKEN_REDIRECT_OUT, start);
    }

    if(c == '(')
    {
        advance(this);
        return createToken(this, TOKEN_LPAREN, start);
    }

    if(c == ')')
    {
        advance(this);
        return createToken(this, TOKEN_RPAREN, start);
    }

    if(c == '{')
    {
        advance(this);
        return createToken(this, TOKEN_LBRACE, start);
    }

    if(c == '}')
    {
        advance(this);
        return createToken(this, TOKEN_RBRACE, start);
    }

    LOG_ERR("Unknown shell operator: %c", advance(this));

    return createToken(this, TOKEN_ERROR, start);
}

void Lexer_tokenize(Lexer *this, const char *input)
{
    this->input.begin = input;
    this->input.length = strlen(input);

    while(!atEnd(this))
    {
        while(!atEnd(this) && isBlank(peek(this)))
            advance(this);

        if(atEnd(this))
            break;

        char c = peek(this);

        if(c == '#')
        {
            skipComment(this);
            continue;
        }

        if(c == '\n' || isMetachar(c))
        {
            Token token = operator(this);
            VEC_PUSH(&this->tokens, token);

            if(token.type == TOKEN_ERROR)
                break;

            continue;
        }

        Token token = word(this);
        VEC_PUSH(&this->tokens, token);

        if(token.type == TOKEN_ERROR)
            break;
    }

    VEC_PUSH(&this->tokens, createToken(this, TOKEN_EOF, this->pos));
    LOG_INFO("Generated %zu tokens \n", this->tokens.size);
}

const char *TokenType_toStr(TokenType type)
{
    switch (type)
    {
        case TOKEN_WORD:            return "WORD";

        case TOKEN_PIPE:            return "PIPE";
        case TOKEN_OR:              return "OR";
        case TOKEN_AMPERSAND:       return "AMPERSAND";
        case TOKEN_AND:             return "AND";
        case TOKEN_SEMICOLON:       return "SEMICOLON";
        case TOKEN_DSEMI:           return "DSEMI";
        case TOKEN_NEWLINE:         return "NEWLINE";

        case TOKEN_REDIRECT_IN:     return "REDIRECT IN";
        case TOKEN_REDIRECT_OUT:    return "REDIRECT OUT";
        case TOKEN_APPEND:          return "APPEND";
        case TOKEN_HEREDOC:         return "HEREDOC";
        case TOKEN_DUP_IN:          return "DUP IN";
        case TOKEN_DUP_OUT:         return "DUP OUT";

        case TOKEN_LPAREN:          return "LPAREN";
        case TOKEN_RPAREN:          return "RPAREN";
        case TOKEN_LBRACE:          return "LBRACE";
        case TOKEN_RBRACE:          return "RBRACE";

        case TOKEN_EOF:             return "EOF";
        case TOKEN_ERROR:           return "ERROR";
    }

    return "UNKNOWN";
}