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
        int lineNum = 0, columnNum = 0;
        std::vector<Token> tokens;
        std::unordered_map<char, char> escape_map = {{'t', '\t'}, {'n', '\n'}, {'\\', '\\'}, {'\"', '\"'}};

        bool isWhiteSpace(char ch) {
            return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r');
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
                        if (token.value[0] == '\"') throw NoClosingQuote(token.line, token.column, token.value);
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
                    /* 
                    TO FIX:
                    "1'\"'2'\\"'3'\\\"'4'\\\\"'5'\\\\\"'" ; 再測一次
                    */
                    char next_ch = std::cin.peek();
                    if (next_ch == 'n' || next_ch == '\"' || next_ch == 't' || next_ch == '\\') {
                        if (prev_ch != '\'') { // escape char not in ''
                            if (escape_map.count(next_ch)) {
                                // not shure is 'v', 'f', 'r' need to be replaced
                                std::cin.get(); // skip the next char that will be replace
                                columnNum++;
                                prev_ch = next_ch;
                                ch = escape_map[next_ch];
                                if (next_ch == '\"') { // to avoid storing double-quotes
                                    token.value += ch;
                                    continue;
                                }
                            }
                        }
                        else { // escape must in ''
                            if (next_ch != '\\') { // odd backslash, ex. "'\"'" -> '\"'
                                // not shure is 'v', 'f', 'r' need to be replaced
                                if (escape_map.count(next_ch)) {
                                    std::cin.get(ch); // skip the next char that will be replace
                                    columnNum++;
                                    prev_ch = ch;
                                    ch = escape_map[prev_ch];
                                    if (ch == '\"') { // to avoid storing double-quotes
                                        token.value += ch;
                                        continue;
                                    }
                                }
                            }
                            else { // even backslash, ex. "'\\n'" -> '\\' + '\n' 
                                std::cin.get(); // skip even number of backslash
                                columnNum++;
                                prev_ch = '\'';
                                token.value += ch;
                                continue;
                            }
                        }
                    }
                    // else just a backslash
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
            //std::cout << "out" << std::endl;
            lexer.readAndTokenize();
            //lexer.printAllTokens();
        } catch (CorrectExit &c) {
            std::cout << c.what() << std::endl;
            break;
        } catch (NoMoreInput &e) {
            std::cerr << e.what() << std::endl;
            break;
        } catch (UnexpectedToken &e) {
            std::cerr << e.what() << std::endl;
            //break;
        } catch (NoClosingQuote &e) {
            std::cerr << e.what() << std::endl;
            //break;
        } catch (NoRightParen &e) {
            std::cerr << e.what() << std::endl;
            //break;
        } catch (...) {
            std::cerr << "Unknown error" << std::endl;
            //break;
        }
    }
    return 0;
}