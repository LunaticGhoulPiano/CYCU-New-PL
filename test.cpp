#include <iostream>
#include <exception>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

// Types
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

// Token structure
struct Token {
    TokenType type;
    std::string value;
};

// S-Expression Abstract Syntax Tree
/*
<S-exp> ::= <ATOM> 
            | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
            | QUOTE <S-exp>
<ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
            | NIL | T | LEFT-PAREN RIGHT-PAREN
*/

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print() const = 0;  // 纯虚函数，必须由子类实现
};

class AtomNode : public ASTNode {
public:
    Token token;

    explicit AtomNode(const Token& tok) : token(tok) {}

    void print() const override {
        std::cout << token.value;  // 直接输出 Token 值
    }
};

class ListNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> elements;

    void addElement(std::unique_ptr<ASTNode> node) {
        elements.push_back(std::move(node));
    }

    void print() const override {
        std::cout << "( ";
        for (const auto& elem : elements) {
            elem->print();
            std::cout << " ";
        }
        std::cout << ")";
    }
};

class QuoteNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> quoted;

    explicit QuoteNode(std::unique_ptr<ASTNode> node) : quoted(std::move(node)) {}

    void print() const override {
        std::cout << "(quote ";
        quoted->print();
        std::cout << ")";
    }
};

class DotPairNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> first;
    std::unique_ptr<ASTNode> second;

    DotPairNode(std::unique_ptr<ASTNode> car, std::unique_ptr<ASTNode> cdr)
        : first(std::move(car)), second(std::move(cdr)) {}

    void print() const override {
        std::cout << "( ";
        first->print();
        std::cout << " . ";
        second->print();
        std::cout << " )";
    }
};

// Recursive Descent Parser
class Parser {
    std::vector<Token> tokens;
    size_t pos = 0;

    Token peek() {
        return (pos < tokens.size()) ? tokens[pos] : Token{TokenType::SYMBOL, ""};
    }

    Token advance() {
        return tokens[pos++];
    }

    std::unique_ptr<ASTNode> parseAtom() {
        Token tok = advance();
        return std::make_unique<AtomNode>(tok);
    }

    std::unique_ptr<ASTNode> parseList() {
        auto list = std::make_unique<ListNode>();
        advance();  // 吃掉 '('

        while (peek().type != TokenType::RIGHT_PARENT) {
            if (peek().type == TokenType::DOT) {
                advance();
                list->addElement(parseExpression());  // 解析 '.' 后的表达式
                break;
            }
            list->addElement(parseExpression());
        }

        advance();  // 吃掉 ')'
        return list;
    }

    std::unique_ptr<ASTNode> parseQuote() {
        advance();  // 吃掉 QUOTE (')

        return std::make_unique<QuoteNode>(parseExpression());
    }

public:
    explicit Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}

    std::unique_ptr<ASTNode> parseExpression() {
        Token tok = peek();
        if (tok.type == TokenType::LEFT_PARENT) return parseList();
        if (tok.type == TokenType::QUOTE) return parseQuote();
        return parseAtom();
    }

    std::unique_ptr<ASTNode> parse() {
        return parseExpression();
    }
};

int main() {
    std::vector<Token> tokens = {
        {TokenType::QUOTE, "'"},
        {TokenType::LEFT_PARENT, "("},
        {TokenType::SYMBOL, "a"},
        {TokenType::SYMBOL, "b"},
        {TokenType::RIGHT_PARENT, ")"},
    };

    Parser parser(tokens);
    auto ast = parser.parse();

    ast->print();  // 输出： (quote ( a b ))
    return 0;
}