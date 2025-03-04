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
        
        // TODO
        void judgeAndSetTokenType(Token &token) {
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
            judgeAndSetTokenType(token);
            std::cout << "save token: ->" << token.value << "<- at Line " << token.line << " Column " << token.column << std::endl;
            tokens.push_back(token);
            token.reset();
        }

        void readAndTokenize() {
            std::cout << "> ";
            Token token;
            lineNum++;
            ch = '\0';
            prev_ch = '\0';
            
            while (std::cin.get(ch)) {
                columnNum++;

                if (isWhiteSpace(ch)) {
                    if (ch == '\n') {
                        if (token.value[0] == '\"') throw NoClosingQuote(token.line, token.column, token.value);
                        else {
                            if (token.value != "") saveAToken(token);
                            std::cout << "> "; // may have bug when empty line
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
                if (ch == ';' && token.value == "") {
                    if (! token.value.empty()) saveAToken(token);
                    if (! tokens.empty() || token.value != "") lineNum++;
                    columnNum = 0;
                    while (ch != '\n') std::cin.get(ch); // skip the rest of the line
                    prev_ch = ch;
                    continue;
                }
                if (ch == '\\') { // escape
                    char next_ch = std::cin.peek();
                    if (next_ch == 'n' || next_ch == '\"' || next_ch == 't' || next_ch == '\\') {
                        std::cout << "Next: " << next_ch << std::endl;
                        std::cout << "Cur: " << ch << std::endl;
                        std::cout << "Prev: " << prev_ch << std::endl;
                        /*
                        if (next_ch == '\"') {
                            std::cout << "token = " << token.value << std::endl;
                        }
                        if (prev_ch == '\\') {
                            prev_ch = ch;
                            std::cin.get(ch); // '\\n' -> '\n', '\\\"' -> '\"'
                        }
                        */
                        if (prev_ch != '\'') { // ex. "There is an ENTER HERE>>\nSee?!" -> replace to the true white space value
                            // not shure is 'v', 'f', 'r' need to be replaced
                            std::unordered_map<char, char> escape_map = {{'t', '\t'}, {'n', '\n'}, {'\\', '\\'}, {'\"', '\"'}};
                            if (escape_map.count(next_ch)) {
                                std::cin.get(); // skip the next char that will be replace
                                prev_ch = next_ch;
                                ch = escape_map[next_ch];
                                if (next_ch == '\"') { // to avoid storing double-quotes
                                    token.value += ch;
                                    continue;
                                }
                            }
                        }
                        else {
                            /*
                            prev_ch == '\''
                            cur_ch == '\\'
                            next_ch == 'n' || '"' || 't' || '\\'
                            -> '\n
                            -> '\"
                            -> '\t
                            -> '\\

                            '\"' -> '"'
                            '\\"' -> '\"'
                            '\\n' -> '\n'
                            '\\t' -> '\t'
                            */
                           /*
                           try this:
                           "a: \n b: '\n' c: '\\n' d: '\"' e: '\\"' f: '\\\"' "
                           */
                            if (next_ch == '\\') {
                               
                            }
                        }
                    }
                    // else just a backslash
                }

                //std::cout << "token=: " << token.value << " at Line " << lineNum << " Column " << columnNum << "" << std::endl;

                prev_ch = ch;
                token.value += ch;

                // end of a double-quote
                // 
                if (token.value.length() >= 2 && token.value[0] == '\"' && token.value[token.value.length() - 1] == '\"') saveAToken(token);
                // 
                if (ch == '(' || ch == ')' || ch == '.' || ch == '#') saveAToken(token);

                // record token's position by the first char in token.value
                if (token.line == -1 && token.column == -1) token.setPos(lineNum, columnNum);
            }

            saveAToken(token);
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
            std::cout << "out" << std::endl;
            lexer.readAndTokenize();
            lexer.printAllTokens();
        } catch (NoMoreInput &e) {
            std::cerr << e.what() << std::endl;
            //break;
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

    std::cout << "Thanks for using OurScheme!" << std::endl;
    return 0;
}