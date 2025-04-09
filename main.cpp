#include <iostream>
#include <iomanip>
#include <sstream>
#include <exception>
#include <vector>
#include <stack>
#include <string>
#include <unordered_map>
#include <memory>
#include <regex>

std::string gTestNum; // note that it is int + '\n'

/* Token types */
enum class Token_Type {
    UDF, // to avoid undefined enum, set this to constructor and reset
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
    Token_Type type = Token_Type::NIL;
    std::string value = "";
    Token() : type(Token_Type::NIL), value("") {}
    Token(Token_Type t, const std::string& v) : type(t), value(v) {}
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

/* AST structure */
struct AST {
    bool isAtom = false;
    Token atom;
    std::shared_ptr<AST> left = nullptr, right = nullptr;
    AST(Token t) : isAtom(true), atom(std::move(t)) {}
    AST(std::shared_ptr<AST> l, std::shared_ptr<AST> r) : isAtom(false), left(std::move(l)), right(std::move(r)) {}
};
/* S-Expression Parser */
// <S-exp> ::= <ATOM>
//             | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
//             | QUOTE <S-exp>
         
// <ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
//             | NIL | T | LEFT-PAREN RIGHT-PAREN
class S_Exp_Parser {
    private:
        enum class Mode { NORMAL, WAIT_DOT_CDR };
    
        std::stack<std::vector<std::shared_ptr<AST>>> list_stack;
        std::stack<Mode> mode_stack;
        std::shared_ptr<AST> current;
    
    public:

        void parse(const Token &token) {
            if (token.type == Token_Type::LEFT_PAREN) {
                list_stack.push({});
                mode_stack.push(Mode::NORMAL);
            } else if (token.type == Token_Type::RIGHT_PAREN) {
                if (list_stack.empty()) {
                    std::cerr << "Unmatched )\n";
                    return;
                }
                auto nodes = list_stack.top(); list_stack.pop();
                auto mode = mode_stack.top(); mode_stack.pop();
    
                std::shared_ptr<AST> ast = nullptr;
                if (mode == Mode::WAIT_DOT_CDR) {
                    if (nodes.size() < 2) {
                        std::cerr << "Invalid dotted pair\n";
                        return;
                    }
                    auto cdr = nodes.back();  // 最後一個是 dot 後的 cdr
                    nodes.pop_back();         // 剩下的是 list 的部分
                    ast = makeDottedList(nodes, cdr);
                } else {
                    ast = makeList(nodes);
                }
    
                if (!list_stack.empty()) {
                    list_stack.top().push_back(ast);
                } else {
                    current = ast;
                    printAST(current);
                }
            } else if (token.type == Token_Type::DOT) {
                if (mode_stack.empty() || mode_stack.top() != Mode::NORMAL) {
                    std::cerr << "Unexpected dot\n";
                    return;
                }
                mode_stack.top() = Mode::WAIT_DOT_CDR;
            } else {
                auto node = std::make_shared<AST>(token);
                if (!list_stack.empty()) {
                    list_stack.top().push_back(node);
                } else {
                    current = node;
                    printAST(current);
                }
            }
        }
    
        std::shared_ptr<AST> makeList(const std::vector<std::shared_ptr<AST>>& nodes) {
            std::shared_ptr<AST> nil = std::make_shared<AST>(Token{Token_Type::NIL, "nil"});
            std::shared_ptr<AST> res = nil;
            for (int i = static_cast<int>(nodes.size()) - 1; i >= 0; --i) {
                res = std::make_shared<AST>(nodes[i], res);
            }
            return res;
        }
    
        std::shared_ptr<AST> makeDottedList(const std::vector<std::shared_ptr<AST>>& nodes, const std::shared_ptr<AST>& cdr) {
            std::shared_ptr<AST> res = cdr;
            for (int i = static_cast<int>(nodes.size()) - 1; i >= 0; --i) {
                res = std::make_shared<AST>(nodes[i], res);
            }
            return res;
        }
    
        void printAST(const std::shared_ptr<AST>& node, int indent = 0, bool isRoot = true) {
            std::string pad(indent * 2, ' ');
            if (node == nullptr) return;
    
            if (node->isAtom) {
                std::cout << pad << node->atom.value << "\n";
                return;
            }
    
            if (isRoot) std::cout << pad << "(\n";
            printAST(node->left, indent + 1, true);
    
            if (node->right && node->right->isAtom && node->right->atom.type != Token_Type::NIL) {
                std::cout << std::string((indent + 1) * 2, ' ') << ".\n";
                printAST(node->right, indent + 1, true);
                std::cout << pad << ")\n";
            } else if (node->right && !node->right->isAtom) {
                printAST(node->right, indent, false);
                if (isRoot) std::cout << pad << ")\n";
            } else if (!node->right || node->right->atom.type == Token_Type::NIL) {
                if (isRoot) std::cout << pad << ")\n";
            } else {
                std::cout << std::string((indent + 1) * 2, ' ') << ".\n";
                printAST(node->right, indent + 1, true);
                std::cout << pad << ")\n";
            }
        }
    
        std::shared_ptr<AST> getResult() const {
            return current;
        }
};
    

/* S-Expression Lexer */
class S_Exp_Lexer {
    
    private:
        char ch, prev_ch;
        int lineNum = 1, columnNum = 0;
        std::vector<Token> exit_buffer;
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

        bool isInt(const std::string &str) {
            std::regex pattern("^[-+]?\\d+$");
            return std::regex_match(str, pattern);
        }

        bool isFloat(const std::string &str) {
            std::regex pattern("^[-+]?((\\d+\\.\\d*)|(\\.\\d+))$");
            return std::regex_match(str, pattern);
        }

        void printType(Token token) {
            if (token.type == Token_Type::LEFT_PAREN) std::cout << "LEFT_PAREN\n";
            else if (token.type == Token_Type::RIGHT_PAREN) std::cout << "RIGHT_PAREN\n";
            else if (token.type == Token_Type::INT) std::cout << "INT\n";
            else if (token.type == Token_Type::STRING) std::cout << "STRING\n";
            else if (token.type == Token_Type::DOT) std::cout << "DOT\n";
            else if (token.type == Token_Type::FLOAT) std::cout << "FLOAT\n";
            else if (token.type == Token_Type::NIL) std::cout << "NIL\n";
            else if (token.type == Token_Type::T) std::cout << "T\n";
            else if (token.type == Token_Type::QUOTE) std::cout << "QUOTE\n";
            else if (token.type == Token_Type::SYMBOL) std::cout << "SYMBOL\n";
            else if (token.type == Token_Type::UDF) std::cout << "UDF\n";
            else std::cout << "ERROR: didn't judged!\n";
        }

        void judgeType(Token &token) {
            if (isInt(token.value) || isFloat(token.value)) {
                std::istringstream iss(token.value);
                std::stringstream oss;
                // formating int and float, also round float to "%.3f"
                if (token.value.find('.') != std::string::npos) {
                    token.type = Token_Type::FLOAT;
                    double value;
                    iss >> value;
                    oss << std::fixed << std::setprecision(3) << value;
                }
                else {
                    token.type = Token_Type::INT;
                    int value;
                    iss >> value;
                    oss << value;
                }
                token.value = oss.str();
            }
            else if (token.value == "(") token.type = Token_Type::LEFT_PAREN;
            else if (token.value == ")") {
                // deal the "()" here
                /*
                if (buffer.size() > 0 && buffer[buffer.size()-1].type == Token_Type::LEFT_PAREN) {
                    buffer.pop_back();
                    token.value = "nil";
                    token.type = Token_Type::NIL;
                }
                else token.type = Token_Type::RIGHT_PAREN;
                */
                token.type = Token_Type::RIGHT_PAREN;
            }
            else if (token.value == ".") token.type = Token_Type::DOT;
            else if (token.value[0] == '\"' && token.value[token.value.length()-1] == '\"') token.type = Token_Type::STRING;
            else if (token.value == "\'") token.type = Token_Type::QUOTE;
            else if (token.value == "nil" || token.value == "#f") {
                token.type = Token_Type::NIL;
                token.value = "nil";
            }
            else if (token.value == "t" || token.value == "#t") {
                token.type = Token_Type::T;
                token.value = "#t";
            }
            else token.type = Token_Type::SYMBOL;
        }

        void saveAToken(Token &token, S_Exp_Parser &parser) {
            exit_buffer.push_back(token);
            // judge type
            judgeType(token);

            // push into buffer
            // Should replace the method into build and push a node into tree and parse,
            // rather then just simply push into a vector.
            // Should check if tree is empty;
            // if empty then build tree,
            // then parse (test) the current atom to match s-expression;
            // if no match, return false;
            // else continue, then return true at the end.
            // std::cout << "\n> push a token: ->" << token.value << "<-\n";
            // printType(token);
            parser.parse(token);
            /*
            buffer.push_back(token); // should replace to parser.parse(atom);
            std::cout << "\n> " << token.value << "\n\n";
            */
            token = Token(); // reset
            if (exit_buffer.size() >= 3 && exit_buffer[exit_buffer.size() - 3].value == "(" && exit_buffer[exit_buffer.size() - 2].value == "exit" && exit_buffer[exit_buffer.size() - 1].value == ")") {
                throw CorrectExit();
            }
        }

    public:
        bool allow_newline_in_token = false;
        
        S_Exp_Lexer() {
            ch = '\0';
            prev_ch = '\0';
        }

        void readAndTokenize(S_Exp_Parser &parser) {
            Token token;
            std::stack<char> parenStack;
            lineNum = 1;
            columnNum = 0;
            ch = '\0';
            prev_ch = '\0';
            bool start = false;
            if (! allow_newline_in_token) std::cout << "> "; // init input prompt

            while (std::cin.get(ch)) {
                start = true;
                if (ch == ';') {
                    if (token.value != "") {
                        if (token.value[0] == '\"') { // in string
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save a token
                            saveAToken(token, parser);

                            // read until new line
                            while (ch != '\n') std::cin.get(ch);

                            /*
                            // set position
                            if (allow_newline_in_token) lineNum++;
                            else {
                                lineNum = 1;
                                std::cout << "\nB> ";
                            }
                            */
                            lineNum++;
                            columnNum = 0;
                        }
                    }
                    else {
                        while (std::cin.peek() != '\n') std::cin.get(ch);
                        columnNum = 0;
                    }
                }
                else if (isWhiteSpace(ch)) {
                    if (token.value == "") {
                        if (ch == '\n') {
                            // set position
                            /*
                            if (allow_newline_in_token) lineNum++;
                            else {
                                lineNum = 1;
                                std::cout << "\nC> ";
                            }
                            */
                            lineNum++;
                            columnNum = 0;
                        }
                        else columnNum++;
                    }
                    else { // incomplete token still inputing
                        if (ch == '\n') {
                            if (token.value[0] == '\"') {
                                columnNum++;
                                throw NoClosingQuote(lineNum, columnNum);
                            }
                            else {
                                // save a token
                                saveAToken(token, parser);

                                // set position
                                if (allow_newline_in_token) lineNum++;
                                else {
                                    lineNum = 1;
                                    //std::cout << "\nD> ";
                                }
                                columnNum = 0;
                            }
                        }
                        else {
                            if (token.value[0] == '\"') {
                                token.value += ch;
                                columnNum++;
                            }
                            else {
                                // save a token
                                saveAToken(token, parser);

                                // set position
                                if (allow_newline_in_token) lineNum++;
                                else {
                                    lineNum = 1;
                                    //std::cout << "\nE> ";
                                }
                                columnNum = 0;
                            }
                        }
                    }
                }
                else if (ch == '(') { // judge both start of a single-quote or in string
                    if (token.value != "") {
                        if (token.value[0] == '\"') { // in string
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save an token
                            saveAToken(token, parser);

                            // set new parenStack
                            parenStack.push(ch);
                            
                            // save a token
                            token.value += ch;
                            saveAToken(token, parser);

                            // set position
                            columnNum = 0;
                        }
                    }
                    else {
                        // set new parenStack
                        parenStack.push(ch);
                        // save a token
                        token.value += ch;
                        saveAToken(token, parser);

                        // set position
                        columnNum = 0;
                    }
                }
                else if (ch == ')') { // judge both end of a single-quote or in string
                    if (token.value != "") {
                        if (token.value[0] == '\"') { // in string
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save a token
                            saveAToken(token, parser);
                            // set position
                            columnNum = 1;

                            // check parenStack
                            if (parenStack.empty()) {
                                std::string s;
                                std::getline(std::cin, s);
                                throw UnexpectedToken(lineNum, columnNum, ")");
                            }
                            else {
                                parenStack.pop();
                                // save a token
                                token.value += ch;
                                saveAToken(token, parser);

                                // set position
                                columnNum = 0;
                            }
                        }
                    }
                    else {
                        // check parenStack
                        if (parenStack.empty()) {
                            columnNum++;
                            std::string s;
                            std::getline(std::cin, s);
                            throw UnexpectedToken(lineNum, columnNum, ")");
                        }
                        else {
                            parenStack.pop();
                            // save a token
                            token.value += ch;
                            saveAToken(token, parser);

                            // set position
                            columnNum = 0;
                        }
                    }
                }
                else if (ch == '\"') {
                    if (token.value == "") { // the start of a string
                        token.value += ch;
                        columnNum++;
                    }
                    else {
                        if (token.value[0] == '\"') { // end of a string
                            // save a token
                            token.value += ch;
                            saveAToken(token, parser);
    
                            // set position
                            if (allow_newline_in_token) lineNum++;
                            else lineNum = 1;
                            columnNum = 0;
                        }
                        else { // ex. > "" ""asf"" -> temp = "asf", ch = '\"'
                            // save a token
                            saveAToken(token, parser);
                            columnNum = 0;
    
                            // set the new starting string's char and position
                            token.value += ch;
                            columnNum++;
                        }
                    }
                }
                else if (ch == '\'') {
                    if (token.value == "") { // the start of a single-quote
                        // save a token
                        columnNum++;
                        token.value += ch;
                        saveAToken(token, parser);

                        // set position
                        columnNum = 0;
                    }
                    else {
                        if (token.value[0] == '\"') { // in string
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save a token
                            saveAToken(token, parser);
                            columnNum = 0;
                            
                            // save a token
                            columnNum++;
                            token.value += ch;
                            saveAToken(token, parser);
                            columnNum = 0;
                        }
                    }
                }
                else if (ch == '\\') {
                    if (token.value != "" && token.value[0] == '\"' && escape_map.count(std::cin.peek())) { // in string, escape
                        // ch + next_ch = "\t" or "\n" or "\\" or "\""
                        columnNum++; // first backslash
                        ch = std::cin.get();
                        token.value += escape_map[ch];
                        columnNum++;
                    }
                    else { // just a normal backslash
                        token.value += ch;
                        columnNum++;
                    }
                }
                else {
                    token.value += ch;
                    columnNum++;
                }
            }

            if (! start) throw NoMoreInput();
        }
};

/* Main Read-Eval-Print-Loop */
int main() {
    std::getline(std::cin, gTestNum);
    std::cout << "Welcome to OurScheme!\n\n";
    S_Exp_Lexer lexer;
    S_Exp_Parser parser;
    while (true) {
        try {
            lexer.readAndTokenize(parser);
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