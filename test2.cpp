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

/* S-Expression Recursive Descent Parser */
// Syntax of OurScheme:
// <S-exp> ::= <ATOM> | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | QUOTE <S-exp>
// <ATOM>  ::= SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN RIGHT-PAREN
class S_Exp_Parser {
    
    private:
        char ch;
        std::string line;
        std::stringstream buffer;
        int lineNum = 0, columnNum = 0;
        std::vector<Token> tokens;

        bool isWhiteSpace(char ch) {
            return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r');
        }

    public:
        S_Exp_Parser() {
            ch = '\0';
            line = "";
            buffer.str("");
            buffer.clear();
        }

        void read() {
            line = "";
            if (! std::getline(std::cin, line)) throw NoMoreInput();
            lineNum++;
            columnNum = 0;
        }
        
        void tokenize() {
            ch = '\0';
            buffer.str("");
            buffer.clear();
            buffer << line;
            tokens.clear();  // 清空 token 列表
        
            std::stack<char> stack;
            std::string tokenStr = "";
            
            while (buffer.get(ch)) {
                columnNum++;
        
                // 跳过空白字符（如果不在字符串内）
                if (isWhiteSpace(ch) && stack.empty()) {
                    if (!tokenStr.empty()) {
                        tokens.push_back({TokenType::SYMBOL, tokenStr, lineNum, columnNum});
                        tokenStr.clear();
                    }
                    continue;
                }
        
                // 处理注释（';' 开头，忽略至行尾）
                if (ch == ';' && stack.empty()) {
                    return;  // 直接结束当前行解析，忽略注释
                }
        
                // 处理左括号 '('
                if (ch == '(' && stack.empty()) {
                    stack.push('(');
                    tokens.push_back({TokenType::LEFT_PARENT, "(", lineNum, columnNum});
                    continue;
                }
        
                // 处理右括号 ')'
                if (ch == ')' && stack.empty()) {
                    if (stack.empty() || stack.top() != '(') {
                        throw NoRightParen(lineNum, columnNum, ")");
                    }
                    stack.pop();
                    tokens.push_back({TokenType::RIGHT_PARENT, ")", lineNum, columnNum});
                    continue;
                }
        
                // 处理单引号 `'`
                if (ch == '\'' && stack.empty()) {
                    tokens.push_back({TokenType::QUOTE, "'", lineNum, columnNum});
                    continue;
                }
        
                // 处理点 '.'
                if (ch == '.' && stack.empty()) {
                    // 判断是否是独立的 `DOT`
                    if (buffer.peek() == EOF || isWhiteSpace(buffer.peek()) || buffer.peek() == ')') {
                        tokens.push_back({TokenType::DOT, ".", lineNum, columnNum});
                        continue;
                    }
                }
        
                // 处理字符串 `"..."`
                if (ch == '"') {
                    if (!stack.empty() && stack.top() == '"') {
                        // 结束字符串
                        stack.pop();
                        tokenStr += '"';
                        tokens.push_back({TokenType::STRING, tokenStr, lineNum, columnNum});
                        tokenStr.clear();
                    } else {
                        // 开始字符串
                        stack.push('"');
                        tokenStr = "\"";
                    }
                    continue;
                }
        
                // 处理字符串内部字符
                if (!stack.empty() && stack.top() == '"') {
                    tokenStr += ch;
                    if (ch == '\\' && buffer.peek() != EOF) { // 处理转义字符
                        buffer.get(ch);
                        columnNum++;
                        tokenStr += ch;
                    }
                    continue;
                }
        
                // 处理数字（整数和浮点数）
                if (isdigit(ch) || ch == '+' || ch == '-' || ch == '.') {
                    tokenStr = ch;
                    bool hasDot = (ch == '.');
                    while (buffer.peek() != EOF && (isdigit(buffer.peek()) || buffer.peek() == '.')) {
                        buffer.get(ch);
                        columnNum++;
                        tokenStr += ch;
                        if (ch == '.') {
                            if (hasDot) {
                                break;  // 多个 `.` 说明是 SYMBOL
                            }
                            hasDot = true;
                        }
                    }
                    if (hasDot) {
                        tokens.push_back({TokenType::FLOAT, tokenStr, lineNum, columnNum});
                    } else {
                        tokens.push_back({TokenType::INT, tokenStr, lineNum, columnNum});
                    }
                    continue;
                }
        
                // 处理布尔值 `#t` 和 `#f`
                if (ch == '#') {
                    tokenStr = "#";
                    if (buffer.get(ch)) {
                        columnNum++;
                        tokenStr += ch;
                        if (tokenStr == "#t") {
                            tokens.push_back({TokenType::T, "#t", lineNum, columnNum});
                        } else if (tokenStr == "#f") {
                            tokens.push_back({TokenType::NIL, "nil", lineNum, columnNum});
                        } else {
                            tokens.push_back({TokenType::SYMBOL, tokenStr, lineNum, columnNum});
                        }
                    }
                    continue;
                }
        
                // 处理符号（SYMBOL）
                if (isprint(ch)) {
                    tokenStr = ch;
                    while (buffer.peek() != EOF && isprint(buffer.peek()) && !isWhiteSpace(buffer.peek()) &&
                           buffer.peek() != '(' && buffer.peek() != ')' && buffer.peek() != '\'' &&
                           buffer.peek() != '"' && buffer.peek() != ';') {
                        buffer.get(ch);
                        columnNum++;
                        tokenStr += ch;
                    }
                    tokens.push_back({TokenType::SYMBOL, tokenStr, lineNum, columnNum});
                    continue;
                }
        
                // 其他未识别字符，报错
                throw UnexpectedToken(lineNum, columnNum, std::string(1, ch));
            }
        
            // **检查未闭合的字符串**
            if (!stack.empty() && stack.top() == '"') {
                throw NoClosingQuote(lineNum, columnNum, tokenStr);
            }
        
            // **检查未匹配的括号**
            if (!stack.empty()) {
                throw NoRightParen(lineNum, columnNum, ")");
            }
        }

        void printt() {
            for (const auto& token : tokens) {
                std::cout << " " << token.value << " " << token.line << " " << token.column << std::endl;
            }
        }
        
};

/* Main Read-Eval-Print-Loop */
int main() {
    std::getline(std::cin, gTestNum);
    std::cout << "Welcome to OurScheme!" << std::endl;
    S_Exp_Parser parser;
    while (true) {
        try {
            std::cout << "> ";
            parser.read();
            parser.tokenize();
            parser.printt();
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
    return 0;
}