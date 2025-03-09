#include <iostream>
#include <exception>
#include <vector>
#include <stack>
#include <string>
//#include <sstream>
#include <unordered_map>
#include <memory>

std::string gTestNum; // note that it is int + '\n'

/* Token Types*/
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
    std::string value = "";
    int line = -1;
    int column = -1;
    void reset() {
        value = "";
        line = -1;
        column = -1;
    }
    void setPos(int lineNum, int columnNum) {
        line = lineNum;
        column = columnNum;
    }
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
            BaseException("> ERROR (unexpected token) : atom or '(' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<\n\n") {}
};

// ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<
class NoRightParen: public BaseException {
    public:
        NoRightParen(int line, int column, const std::string &token):
            BaseException("> ERROR (unexpected token) : ')' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<\n\n") {}
};

// ERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y
class NoClosingQuote: public BaseException {
    public:
        NoClosingQuote(int line, int column):
            BaseException("> ERROR (no closing quote) : END-OF-LINE encountered at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + "\n\n") {}
};

// ERROR (no more input) : END-OF-FILE encountered
class NoMoreInput: public BaseException {
    public:
        NoMoreInput(): BaseException("> ERROR (no more input) : END-OF-FILE encountered\n\n") {}
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
            for (auto token: tokens) std::cout << "token: " << token.value << " at Line " << token.line << " Column " << token.column << std::endl;
        }

        void saveAToken(Token &token) {
            if (token.value == "") return;
            judgeTokenType(token);
            std::cout << token.value << "\n" << std::endl;
            //std::cout << "save token: ->" << token.value << "<- at Line " << token.line << " Column " << token.column << std::endl;
            tokens.push_back(token);
            token.reset();
            int s = tokens.size();
            if (s >= 3 && tokens[s-3].value == "(" && tokens[s-2].value == "exit" && tokens[s-1].value == ")") throw CorrectExit();
        }

        void read() {
            Token token; // to store single token
            std::vector<Token> lineBuffer; // clear when line error or save when line ok
            lineNum = 1;
            columnNum = 0;
            ch = '\0';
            prev_ch = '\0';
            if (printInputSign) std::cout << "> ";
            printInputSign = true;

            while (std::cin.get(ch)) {
                std::cout << "->" << ch << "<-" << std::endl;
                if (ch == ';') {
                    if (! token.value.empty()) {
                        if (token.value[0] != '\"') {
                            // saveAToken(token);
                            while (ch != '\n') std::cin.get(ch);
                        }
                        else { // in the double-quote
                            token.value += ch;
                            columnNum++;
                        }
                    }
                    else while (ch != '\n') std::cin.get(ch);
                }
                else if (isWhiteSpace(ch)) {
                    if (token.value.empty()) { // between complete tokens
                        if (ch == '\n') {
                            lineNum++;
                            columnNum = 0;
                        }
                        else columnNum++;
                    }
                    else { // incomplete token still inputing
                        if (ch == '\n') {
                            switch (token.value[0]) {
                                case '\"': {
                                    columnNum++;
                                    throw NoClosingQuote(token.line, token.column);
                                }
                                case '(': {
                                    // because still not meat the coressponding ')'
                                    // so only a line-feed, still inputing a double-quote
                                    lineNum++;
                                    columnNum = 0;
                                }
                                case '\'': {
                                    // because still not meat the end of single-quote (ATOM or RIGHT-PAREN)
                                    // so only a line-feed, still inputing a double-quote
                                    lineNum++;
                                    columnNum = 0;
                                }
                            }
                        }
                        else {
                            //
                        }
                    }
                }
                else if (ch == '(') {
                    //
                }
                else if (ch == ')') {
                    //
                }
                else if (ch == '\"' && ! token.value.empty()) {
                    if (token.value[0] == '\"') {
                        token.value += ch;
                        saveAToken(token);
                        columnNum = 0;
                    }
                    // else throw NoClosingQuote(token.line, token.column, token.value);
                }
                else if (ch == '\'') {
                    //
                }
                else if (ch == '\\') {
                    //
                }
                else if (ch == '+' || ch == '-' || ch == '.') {
                    //
                }
                else token.value += ch;
            }
            throw CorrectExit();
        }

        // deprecate
        void readAndTokenize() {
            std::cout << "> ";
            Token token;
            lineNum++;
            ch = '\0';
            prev_ch = '\0';

            // avoid invisible char
            // TODO: error must fix 'c' + Ctrl+Z + 'c' -> "cc", should be "c" "c"
            if (std::cin.peek() == EOF) {
                std::cin.clear();
                throw NoMoreInput();
            }

            while (std::cin.get(ch)) {
                columnNum++;
                if (isWhiteSpace(ch)) {
                    if (ch == '\n') {
                        if (token.value[0] == '\"') throw NoClosingQuote(token.line, token.column);
                        else {
                            if (token.value != "") saveAToken(token);
                            std::cout << "> "; // have bug when empty line or start with ;
                            lineNum++;
                            columnNum = 0;
                        }
                        prev_ch = ch;
                        continue;
                    }
                    else if (token.value == "") {
                        prev_ch = ch;
                        continue; // don't save whitespace
                    }
                    else if (token.value != "" && token.value[0] != '\"') {
                        saveAToken(token); // if whitespace not in double-qupte, skip it
                        prev_ch = ch;
                        continue;
                    }
                    // else DON'T continue, it may store into a double-quote
                }
                if (ch == ';') {
                    if (! token.value.empty()) saveAToken(token);
                    if (! tokens.empty() || token.value != "") lineNum++;
                    columnNum = 0;
                    while (std::cin.peek() != '\n') std::cin.get(ch); // skip the rest of the line
                    prev_ch = ch;
                    continue;
                }
                if (ch == '\\') { // escape
                    char next_ch = std::cin.peek();

                }
                if (ch == '(' || ch == ')' || ch == '\'') {
                    if (token.value == "") {
                        prev_ch = ch;
                        token.value += ch;
                        saveAToken(token);
                        continue;
                    }
                    else if (token.value[0] != '\"') {
                        // save the token before '(' or ')'
                        saveAToken(token);
                        // save '(' or ')' as a single token
                        prev_ch = ch;
                        token.value += ch;
                        saveAToken(token);
                        continue;
                    }
                }

                prev_ch = ch;
                token.value += ch;

                // end of a double-quote
                if (token.value.length() >= 2 && token.value[0] == '\"' && token.value[token.value.length() - 1] == '\"') saveAToken(token);

                // record token's position by the first char in token.value
                if (token.line == -1 && token.column == -1) token.setPos(lineNum, columnNum);
            }

            saveAToken(token);
            if (tokens.empty()) throw NoMoreInput();
        }
};

/* S-Expression Recursive Descent Parser */
class S_Exp_Parser {};

/* Main Read-Eval-Print-Loop */
int main() {
    std::getline(std::cin, gTestNum);
    std::cout << "Welcome to OurScheme!\n" << std::endl;
    S_Exp_Lexer lexer;
    S_Exp_Parser parser;
    while (true) {
        try {
            lexer.read();
            //std::cout << "out" << std::endl;
            //lexer.readAndTokenize();
            //lexer.printAllTokens();
        } catch (CorrectExit &c) {
            std::cout << c.what();
            break;
        } catch (NoMoreInput &e) {
            std::cerr << e.what();
            break;
        } catch (UnexpectedToken &e) {
            std::cerr << e.what();
            //break;
        } catch (NoClosingQuote &e) {
            std::cerr << e.what();
            //break;
        } catch (NoRightParen &e) {
            std::cerr << e.what();
            //break;
        } catch (...) {
            std::cerr << "Unknown error" << std::endl;
            //break;
        }
    }
    return 0;
}