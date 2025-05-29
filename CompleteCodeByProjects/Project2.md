# Project2 整理
## 未通過隱測之題號
5, 7, 15

## Tokenizer重寫
```c++
void lexer::tokenize() {
    reset token;
    reset AST;
    reset stack; // for LP and RP
    reset ch;
    reset position to (line = 1, column = 0);
    while (std::cin.get(ch)) {
        if (ch is ';') {
            if (current token is empty) {
                eat the remainings until '\n';
                if (current AST is empty) line = 1;
                else line++;
                column = 0;
            }
            else if (current token is non-closed string) {
                append into current token;
                column++;
            }
            else {
                try {
                    store the current token into AST;
                }
                catch (syntax error) { // lexer and parser error
                    eat the remainings until '\n';
                    throw error message to main();
                }
                
                if (AST is completed) {
                    try {
                        evaluate and execute the AST;
                    }
                    catch (semantic or runtime error) { // executor error
                        eat the remainings until '\n';
                        throw error message to main();
                    }
                }
                else eat the remainings until '\n';
                
                if (current AST is empty) line = 1;
                else line++;
                column = 0;
            }
        }
        else if (ch is ' ' or '\t' or '\n') {
            if (ch == '\n') {
                if (current token is empty) {
                    if (current AST is empty) line = 1;
                    else line++;
                    column = 0;
                }
                else if (current token is non-closed string) {
                    // cuz the closing '\"' should at cur column + 1
                    column++;
                    throw error message;
                }
                else {
                    try {
                        store the current token into AST;
                    }
                    catch (syntax error) {
                        // don't eat a line cuz '\n' already be read into ch
                        throw error message;
                    }

                    if (AST is completed) {
                        try {
                            evaluate and execute the AST;
                        }
                        catch (semantic or runtime error) {
                            // don't eat a line cuz '\n' already be read into ch
                            throw error message;
                        }
                    }

                    if (current AST is empty) line = 1;
                    else line++;
                    column = 0;
                }
            }
            else {
                if (current token is empty) column++;
                else if (current token is non-closed string) {
                    append into current token;
                    column++;
                }
                else {
                    try {
                        store the current token into AST;
                    }
                    catch (syntax error) {
                        eat the remainings until '\n';
                        throw error message;
                    }

                    if (AST is completed) {
                        try {
                            evaluate and execute the AST;
                        }
                        catch (semantic or runtime error) {
                            put back to std::cin;
                            throw error message to main();
                        }
                    }
                    
                    if (current AST is empty) line = 1, column = 1; // column = 1 fot the current char
                    else column++;
                }
            }
        }
        else if (ch is '(') {
            if (current token is empty) {
                push into stack;
                current token = "(";
                column++;
                try {
                    store the current token into AST;
                }
                catch (syntax error) {
                    eat the remainings until '\n';
                    throw error message;
                }
                if (AST is completed) {
                    try {
                        evaluate and execute the AST;
                    }
                    catch (semantic or runtime error) {
                        put back to std::cin;
                        throw error message to main();
                    }
                }
            }
            else if (current token is non-closed string) {
                append into current token;
                column++;
            }
            else {
                try {
                    store the current token into AST;
                }
                catch (syntax error) {
                    eat the remainings until '\n';
                    throw error message;
                }

                if (AST is completed) {
                    try {
                        evaluate and execute the AST;
                    }
                    catch (semantic or runtime error) {
                        put back to std::cin;
                        throw error message to main();
                    }
                    line = 1, column = 1;
                }
                else column++;

                push into stack;
                current token = "(";
                try {
                    store the current token into AST;
                }
                catch (syntax error) { // not happen
                    eat the remainings until '\n';
                    throw error message;
                }
                if (AST is completed) { // not happen
                    try {
                        evaluate and execute the AST;
                    }
                    catch (semantic or runtime error) {
                        put back to std::cin;
                        throw error message to main();
                    }
                }
            }
        }
        else if (ch is ')') {
            if (current token is empty) {
                current token = "(";
                column++;

                if (stack is empty) {
                    eat the remainings until '\n';
                    throw error message;
                }
                else {
                    pop the corresponding '(' from stack;
                    try {
                        store the current token into AST;
                    }
                    catch (syntax error) { // not happen
                        eat the remainings until '\n';
                        throw error message;
                    }
                    if (AST is completed) {
                        try {
                            evaluate and execute the AST;
                        }
                        catch (semantic or runtime error) {
                            // should NOT eat a line
                            throw;
                        }

                        line = 1, column = 0; // reset
                    }
                }
            }
            else if (current token is non-closed string) {
                append into current token;
                column++;
            }
            else {
                try {
                    store the current token into AST;
                }
                catch (syntax error) { // ex. .)
                    eat the remainings until '\n';
                    throw error message;
                }
                
                if (AST is completed) {
                    try {
                        evaluate and execute the AST;
                    }
                    catch (semantic or runtime error) { // ex. abc)
                        put back to std::cin;
                        throw error message to main();
                    }
                }

                current token = ")";
                if (current AST is empty) line = 1, column = 1;
                else column++;

                if (stack is empty) {
                    eat the remainings until '\n';
                    throw error message;
                }
                else {
                    pop the corresponding '(' from stack;
                    try {
                        store the current token into AST;
                    }
                    catch (syntax error) { // not happen (occur when the stack is empty)
                        eat the remainings until '\n';
                        throw error message;
                    }
                    if (AST is completed) {
                        try {
                            evaluate and execute the AST;
                        }
                        catch (semantic or runtime error) {
                            // should NOT eat a line
                            throw;
                        }

                        line = 1, column = 0; // reset
                    }
                }
            }
        }
        else if (ch is '\\') {
            if (current token is non-closed string and the next char is 't' or 'n' or '\\' or '\"') {
                ch = the next char; // ex. 't'
                append the converted escape char into the current token; // token += '\t'
                column += 2; // '\\' and 't'
            }
            else {
                append into current token;
                column++;
            }
        }
        else if (ch is '\'') {
            if (current token is empty) {
                token = "\'";
                column++;

                try {
                    store the current token into AST;
                }
                catch (syntax error) { // not happen
                    eat the remainings until '\n';
                    throw error message;
                }
            }
            else if (current token is non-closed string) {
                append into current token;
                column++;
            }
            else {
                try {
                    store the current token into AST; // |
                } //                                     v
                catch (syntax error) { // ex. '(a . bbbb c'd)
                    eat the remainings until '\n';
                    throw error message;
                }
                
                if (AST is completed) {
                    try {
                        evaluate and execute the AST;
                    }
                    catch (semantic or runtime error) { // not happen
                        put back to std::cin;
                        throw error message to main();
                    }
                }

                current token = "\'";
                if (AST is completed) column = 1;
                else column++;

                try {
                    store the current token into AST;
                }
                // should not have errors
            }
        }
        else if (ch is '\"') {
            if (token is empty) {
                append into current token;
                column++;
            }
            else if (current token is non-closed string) {
                append into current token;
                column++;

                try {
                    store the current token into AST;
                }
            }
        }
    }
}

int main() {
    exception e;
    do {
        e = reset_exception_to_empty();
        try {
            lexer::tokenize();
        }
        catch (OurScheme error) {
            print error message;
            apply error to e;
        }
    } while (e is not exit exception);
}
```