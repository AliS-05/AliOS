#include "token.h"

const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case IDENTIFIER: return "IDENTIFIER";
        case REGISTER:   return "REGISTER";
        case NUMBER:     return "NUMBER";

        case COMMA:      return "COMMA";
        case PLUS:       return "PLUS";
        case MINUS:      return "MINUS";
        case STAR:       return "STAR";
        case COLON:      return "COLON";

        case LBRACKET:   return "LBRACKET";
        case RBRACKET:   return "RBRACKET";

        case NEWLINE:    return "NEWLINE";

        case TOK_EOF:        return "EOF";

        default: return "UNKNOWN";
    }
}
