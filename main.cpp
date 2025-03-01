#include <iostream>
#include <exception>
#include <vector>
#include <stack>
#include <string>
#include <sstream>
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

/* Char structure */
// for tokenization
struct SubToken {
    char value;
    int line;
    int column;
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

/* S-Expression Lexer */
// Syntax of OurScheme:
// <S-exp> ::= <ATOM> | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | QUOTE <S-exp>
// <ATOM>  ::= SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN RIGHT-PAREN
class S_Exp_Lexer {
    
    private:
        char ch, prev_ch;
        std::string line;
        std::stringstream buffer;
        int lineNum = 0, columnNum = 0;
        bool isFirstComment = false;
        std::vector<SubToken> subTokens;

        bool isWhiteSpace(char ch) {
            return (ch == ' ' || ch == '\t');
            //return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r');
        }

        bool isDigit(char ch) {
            return ('0' <= ch && ch <= '9');
        }

        bool isEscape(char ch) {
            return (ch == 'n' || ch == '\"' || ch == 't'|| ch == '\\');
        }

    public:
        void init() {
            ch = '\0';
            line = "";
            prev_ch = '\0';
            buffer.str("");
            buffer.clear();
            subTokens.clear();
            isFirstComment = false;
        }

        std::vector<SubToken> read() {
            if (! isFirstComment) std::cout << "> ";
            if (! std::getline(std::cin, line)) throw NoMoreInput();
            init();
            buffer << line;
            lineNum++;
            columnNum = 0;
            
            while (buffer.get(ch)) {
                columnNum++;
                if (subTokens.empty() && ch == ';') {
                    isFirstComment = true; // has error, to be tested and fixed
                    break;
                }
                if (subTokens.empty() && isWhiteSpace(ch)) continue;
                if (ch == '\\') {
                    char next_ch = buffer.peek();
                    if (isEscape(next_ch)) {
                        if (prev_ch == '\\' && next_ch == '\\') continue; // '\\\"' -> '\"'
                        if (next_ch == 't' && prev_ch != '\\') { // \t -> tab
                            ch = '\t';
                            buffer.get();
                            columnNum++;
                        }
                        else if (next_ch == 'n' && prev_ch != '\\') { // \n -> new line
                            ch = '\n';
                            buffer.get();
                            columnNum++;
                        }
                        else if (prev_ch != '\\') {
                            prev_ch = ch;
                            continue;
                        }
                    }
                }
                subTokens.push_back({ch, lineNum, columnNum});
                prev_ch = ch;
            }

            return subTokens;
        }

        bool tokenize(std::vector<SubToken> subTokens) {
            if (line == "(exit)") return false;
            std::cout << line << "\n" << std::endl;
            return true;
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
            if (! lexer.tokenize(lexer.read())) break;
        } catch (NoMoreInput &e) {
            std::cerr << e.what() << std::endl;
            break;
        } catch (UnexpectedToken &e) {
            std::cerr << e.what() << std::endl;
            break;
        } catch (NoClosingQuote &e) {
            std::cerr << e.what() << std::endl;
            break;
        } catch (NoRightParen &e) {
            std::cerr << e.what() << std::endl;
            break;
        } catch (...) {
            std::cerr << "Unknown error" << std::endl;
            break;
        }
    }

    std::cout << "Thanks for using OurScheme!" << std::endl;
    return 0;
}