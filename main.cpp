#include <iostream>
#include <exception>
#include <vector>
#include <stack>
#include <string>
#include <unordered_map>
#include <memory>

std::string gTestNum; // note that it is int + '\n'

/* Token Types */
enum class TokenType {
    LEFT_PAREN, // '('
    RIGHT_PAREN, // ')'
    INT, // e.g., '123', '+123', '-123'
    STRING, // "string's (example)." (strings do not extend across lines)
    DOT, // '.'
    FLOAT, // '123.567', '123.', '.567', '+123.4', '-.123'
    NIL, // 'nil' or '#f', but not 'NIL' nor 'nIL'
    T, // 't' or '#t', but not 'T' nor '#T'
    QUOTE, // '
    SYMBOL // a consecutive sequence of printable characters that are
        // not numbers, strings, #t or nil, and do not contain 
        // '(', ')', single-quote, double-quote, semi-colon and 
        // white-spaces ; 
        // Symbols are case-sensitive 
        // (i.e., uppercase and lowercase are different);
};

/* Token structure */
struct Token {
    TokenType type;
    std::vector<std::string> values;
    std::string value = "";
};

/* S-Expression Abstract Syntax Tree */
class ASTNode {
    public:
        virtual ~ASTNode() = default;
        virtual void print() const = 0;
};

class AtomNode: public ASTNode {
    public:
        Token token;

        explicit AtomNode(const Token &t): token(t) {}
        void print() const override {
            std::cout << token.value;
        }
};

class ListNode: public ASTNode {
    public:
        std::vector<std::unique_ptr<ASTNode>> elements;

        void add(std::unique_ptr<ASTNode> node) {
            elements.push_back(std::move(node));
        }
        void print() const override { // pretty print
            std::cout << "( ";
            for (const auto &e: elements) {
                e->print();
                std::cout << " ";
            }
            std::cout << ")";
        }
};

class QuoteNode: public ASTNode {
    public:
        std::unique_ptr<ASTNode> quoted;

        explicit QuoteNode(std::unique_ptr<ASTNode> node): quoted(std::move(node)) {}
        void print() const override { // pretty print
            std::cout << "(quote ";
            quoted->print();
            std::cout << ")";
        }
};

class DotPairNode: public ASTNode {
    public:
        std::unique_ptr<ASTNode> before;
        std::unique_ptr<ASTNode> after;
    
        DotPairNode(std::unique_ptr<ASTNode> sub_before, std::unique_ptr<ASTNode> sub_after): before(std::move(sub_before)), after(std::move(sub_after)) {}
        void print() const override { // pretty print
            std::cout << "( ";
            before->print();
            std::cout << " . ";
            after->print();
            std::cout << " )";
        }
};

/* Error Exceptions */
class BaseException: public std::exception {
    protected:
        std::string message = "";
    public:
        explicit BaseException(const std::string &msg): message(msg) {}
        const char *what() const noexcept override {
            return message.c_str();
        }
};

class CorrectExit: public BaseException {
    public:
        CorrectExit():
            BaseException("Thanks for using OurScheme!") {}
};

// ERROR (unexpected token) : atom or '(' expected when token at Line X Column Y is >>...<<
class UnexpectedToken: public BaseException {
    public:
        UnexpectedToken(int line, int column, const std::string &token):
            BaseException("\n> ERROR (unexpected token) : atom or '(' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<\n") {}
};

// ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<
class NoRightParen: public BaseException {
    public:
        NoRightParen(int line, int column, const std::string &token):
            BaseException("\n> ERROR (unexpected token) : ')' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<\n") {}
};

// ERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y
class NoClosingQuote: public BaseException {
    public:
        NoClosingQuote(int line, int column):
            BaseException("\n> ERROR (no closing quote) : END-OF-LINE encountered at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + "\n") {}
};

// ERROR (no more input) : END-OF-FILE encountered
class NoMoreInput: public BaseException {
    public:
        NoMoreInput(): BaseException("\n> ERROR (no more input) : END-OF-FILE encountered\n") {}
};

/* S-Expression Lexer */
// Syntax of OurScheme:
// <S-exp> ::= <ATOM> | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | QUOTE <S-exp>
// <ATOM>  ::= SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN RIGHT-PAREN
class S_Exp_Lexer {
    
    private:
        char ch, prev_ch;
        int lineNum = 1, columnNum = 0;
        std::vector<Token> tokens;
        std::unordered_map<char, char> escape_map = {{'t', '\t'}, {'n', '\n'}, {'\\', '\\'}, {'\"', '\"'}};

        bool isWhiteSpace(char ch) {
            return (ch == ' ' || ch == '\t' || ch == '\n');
            //return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r');
        }

        bool isDigit(char ch) {
            return ('0' <= ch && ch <= '9');
        }

        bool isAlphabet(char ch) {
            return (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'));
        }

        bool isEscape(char ch) {
            return (ch == 'n' || ch == '\"' || ch == 't'|| ch == '\\');
        }

        bool isSeperator(char ch) {
            return (isWhiteSpace(ch) || ch == '(' || ch == ')' || ch == '\'' || ch == '\"' || ch == ';');
        }

        void judgeTokenType(Token &token) {
            //
        }

    public:
        bool printInputSign = true;
        
        S_Exp_Lexer() {
            ch = '\0';
            prev_ch = '\0';
        }

        void printAllTokens() {
            for (auto token: tokens) std::cout << "token: " << token.value << std::endl;
        }

        void saveAToken(Token &token) {
            //std::cout << "save token: ->" << token.value << "<- at Line " << token.line << " Column " << token.column << std::endl;
            tokens.push_back(token);
            token.value = "";
            lineNum = 1;
            columnNum = 0;

            // just to correctly quit the system
            int s = tokens.size();
            if (s >= 3 && tokens[s-3].value == "(" && tokens[s-2].value == "exit" && tokens[s-1].value == ")") throw CorrectExit();
            if (s >= 1 && tokens[s-1].value == "(exit)") throw CorrectExit();
        }

        void read() {
            Token tokenBuffer; // to store single token
            std::stack<char> parenStack;
            lineNum = 1;
            columnNum = 0;
            ch = '\0';
            prev_ch = '\0';
            if (printInputSign) std::cout << "\n> "; // init input prompt
            printInputSign = true;

            while (std::cin.get(ch)) {
                if (ch == ';') {
                    if (! tokenBuffer.value.empty()) {
                        if (tokenBuffer.value[0] != '\"') {
                            while (ch != '\n') std::cin.get(ch);
                            std::cout << "\n> "; // input prompt be printed when new line encountered
                        }
                        else { // in the double-quote
                            tokenBuffer.value += ch;
                            columnNum++;
                        }
                    }
                    else while (std::cin.peek() != '\n') std::cin.get(ch);
                }
                else if (isWhiteSpace(ch)) {
                    if (tokenBuffer.value.empty()) { // between complete tokens
                        if (ch == '\n') {
                            lineNum = 1;
                            columnNum = 0;
                        }
                        else columnNum++; // skip ' ' or '\t'
                    }
                    else { // incomplete token still inputing
                        if (ch == '\n') {
                            if (tokenBuffer.value[0] == '\"') {
                                columnNum++;
                                throw NoClosingQuote(lineNum, columnNum);
                            }
                            else {
                                // may have error here
                                // because still not meat the coressponding ')'
                                // so only a line-feed, still inputing a double-quote
                                std::cout << "\n> " << tokenBuffer.value << "\n"; // output prompt
                                saveAToken(tokenBuffer);
                                lineNum++;
                                std::cout << "\n> "; // input prompt be printed when new line encountered
                            }
                        }
                        else { // ' ' or '\t'
                            if (tokenBuffer.value[0] != '\"') { // token finished, store and reset
                                std::cout << "\n> " << tokenBuffer.value << "\n"; // output prompt
                                saveAToken(tokenBuffer);
                                columnNum++; // the position of white space
                                if (std::cin.peek() == '\n') std::cout << "\n> "; // input prompt be printed when new line encountered
                            }
                            else { // store into double-quote
                                tokenBuffer.value += ch;
                                columnNum++;
                            }
                        }
                    }
                }
                else if (ch == '(') {
                    if (! (tokenBuffer.value != "" && tokenBuffer.value[0] == '\"')) parenStack.push(ch); // if not in double-quote
                    tokenBuffer.value += ch;
                    columnNum++;
                }
                else if (ch == ')') {
                    if (tokenBuffer.value != "" && tokenBuffer.value[0] == '\"') {
                        tokenBuffer.value += ch;
                        columnNum++;
                    }
                    else {
                        columnNum++;
                        if (parenStack.empty()) {
                            while (std::cin.peek() != '\n') std::cin.get(ch);
                            throw UnexpectedToken(lineNum, columnNum, ")");
                        }
                        else {
                            parenStack.pop();
                            if (parenStack.empty()) {
                                tokenBuffer.value += ch;
                                std::cout << "\n> " << tokenBuffer.value << "\n"; // output prompt
                                saveAToken(tokenBuffer);
                                if (std::cin.peek() == '\n') std::cout << "\n> "; // input prompt be printed when new line encountered
                            }
                            else {
                                //
                            }
                        }
                    }
                }
                else if (ch == '\"' && ! tokenBuffer.value.empty()) {
                    if (tokenBuffer.value[0] == '\"') { // end of double-quote
                        tokenBuffer.value += ch;
                        std::cout << "\n> " << tokenBuffer.value << "\n"; // output prompt
                        saveAToken(tokenBuffer);
                       if (std::cin.peek() == '\n') std::cout << "\n> "; // input prompt be printed when new line encountered
                    }
                    else {
                        std::cout << "\n> " << tokenBuffer.value << "\n"; // output prompt
                        saveAToken(tokenBuffer); // store the token before double-quote, ex. > 4"
                        tokenBuffer.value += ch;
                        columnNum++;
                        //if (std::cin.peek() == '\n') std::cout << "\n9> "; // input prompt be printed when new line encountered
                    }
                }
                else if (ch == '\'') {
                    if (tokenBuffer.value != "" && tokenBuffer.value[0] == '\"') {
                        tokenBuffer.value += ch;
                        columnNum++;
                    }
                    else {
                        //
                    }
                }
                else if (ch == '\\') {
                    if (tokenBuffer.value != "" && tokenBuffer.value[0] == '\"' && escape_map.count(std::cin.peek())) { // in double-string, escape
                        // ch + next_ch = "\t" or "\n" or "\\" or "\""
                        columnNum++; // first backslash
                        ch = std::cin.get();
                        tokenBuffer.value += escape_map[ch];
                        columnNum++;
                    }
                    else { // just a normal backslash
                        tokenBuffer.value += ch;
                        columnNum++;
                    }
                }
                else if (ch == '+' || ch == '-' || ch == '.') {
                    if (tokenBuffer.value != "") {
                        if (tokenBuffer.value[0] == '\"') {
                            tokenBuffer.value += ch;
                            columnNum++;
                        }
                        else {
                            if (ch == '.') {
                                if (isDigit(tokenBuffer.value[0])) { // float
                                    tokenBuffer.value += ch;
                                    columnNum++;
                                }
                                else { // symbol
                                    //
                                }
                            }
                            else {
                                //
                            }
                        }
                    }
                    else {
                        //
                    }
                }
                else {
                    tokenBuffer.value += ch;
                    columnNum++;
                }
            }
        }
};

/* S-Expression Recursive Descent Parser */
class S_Exp_Parser {};

/* Main Read-Eval-Print-Loop */
int main() {
    std::getline(std::cin, gTestNum);
    std::cout << "Welcome to OurScheme!" << std::endl;
    S_Exp_Lexer lexer;
    S_Exp_Parser parser;
    while (true) {
        try {
            lexer.read();
        } catch (CorrectExit &c) {
            std::cout << c.what();
            break;
        } catch (NoMoreInput &e) {
            std::cout << e.what();
            break;
        } catch (UnexpectedToken &e) {
            std::cout << e.what();
            //break;
        } catch (NoClosingQuote &e) {
            std::cout << e.what();
            //break;
        } catch (NoRightParen &e) {
            std::cout << e.what();
            //break;
        } catch (...) {
            std::cout << "Unknown error" << std::endl;
            //break;
        }
    }
    return 0;
}