#include <iostream>
#include <exception>
#include <vector>
#include <string>
#include <memory>

/* Token Types*/
enum class TokenType {
    LEFT_PARENT, // '('
    RIGHT_PARENT, // ')'
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
    std::string value;
    int line;
    int column;
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

// ERROR (unexpected token) : atom or '(' expected when token at Line X Column Y is >>...<<
class UnexpectedToken: public BaseException {
    public:
        UnexpectedToken(int line, int column, const std::string &token):
            BaseException("ERROR (unexpected token) : atom or '(' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<") {}
};

// ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<
class NoRightParen: public BaseException {
    public:
        NoRightParen(int line, int column, const std::string &token):
            BaseException("ERROR (unexpected token) : ')' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<") {}
};

// ERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y
class NoClosingQuote: public BaseException {
    public:
        NoClosingQuote(int line, int column, const std::string &token):
            BaseException("ERROR (no closing quote) : END-OF-LINE encountered at Line "
                + std::to_string(line) + " Column " + std::to_string(column)) {}
};

// ERROR (no more input) : END-OF-FILE encountered
class NoMoreInput: public BaseException {
    public:
        NoMoreInput(): BaseException("ERROR (no more input) : END-OF-FILE encountered") {}
};

/* Lexer */
// Syntax of OurScheme:
// <S-exp> ::= <ATOM> | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | QUOTE <S-exp>
// <ATOM>  ::= SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN RIGHT-PAREN
class S_Exp_Lexer {
    private:
        std::string buffer;
    public:
        S_Exp_Lexer() {
            buffer = "";
        }
        void read() {
            buffer = "";
            std::getline(std::cin, buffer);
        }

        void syntaxAnalysis() {}
};

/* S-Expression Recursive Descent Parser */
class S_Exp_Parser {
    public:
        std::vector<Token> tokens;
};

/* Main Read-Eval-Print-Loop */
int main() {
    std::cout << "Welcome to OurScheme!" << std::endl;
    S_Exp_Lexer lexer;
    S_Exp_Parser parser;
    while (true) {
        std::cout << "> ";
        lexer.read();
        try {
            lexer.syntaxAnalysis();
        } catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
        
    }
    return 0;
}