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
    TokenType type = TokenType::NIL;
    std::string value = "";
    Token() : type(TokenType::NIL), value("") {}
    Token(TokenType t, const std::string& v) : type(t), value(v) {}
};

/* AST structure */
struct AST {
    bool isAtom = false;
    Token atom;
    std::shared_ptr<AST> left = nullptr, right = nullptr;
    AST(Token t) : isAtom(true), atom(std::move(t)) {}
    AST(std::shared_ptr<AST> l, std::shared_ptr<AST> r) : isAtom(false), left(std::move(l)), right(std::move(r)) {}
};

/* Error Exceptions */
class ExitException: public std::exception { // Common usage
    protected:
        std::string message = "";
    public:
        explicit ExitException(const std::string &msg): message(msg) {}
        const char *what() const noexcept override {
            return message.c_str();
        }

        static ExitException CorrectExit() { // Exit
            return ExitException("\n> \nThanks for using OurScheme!");
        }

        static ExitException NoMoreInput() { // EOF & Exit
            return ExitException("\n> ERROR (no more input) : END-OF-FILE encountered\nThanks for using OurScheme!");
        }
};

class SyntaxException: public std::exception { // Project 1
    protected:
        std::string message = "";
    public:
        explicit SyntaxException(const std::string &msg): message(msg) {}
        const char *what() const noexcept override {
            return message.c_str();
        }

        static SyntaxException UnexpectedToken(int line, int column, const std::string token) {
            return SyntaxException("\n> ERROR (unexpected token) : atom or '(' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<\n");
        }

        static SyntaxException NoRightParen(int line, int column, const std::string token) {
            return SyntaxException("\n> ERROR (unexpected token) : ')' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<\n");
        }

        static SyntaxException NoClosingQuote(int line, int column) {
            return SyntaxException("\n> ERROR (no closing quote) : END-OF-LINE encountered at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + "\n");
        }
};

class SemanticException: public std::exception { // Project 2
    protected:
        std::string message = "";
    public:
        explicit SemanticException(const std::string &msg): message(msg) {}
        const char *what() const noexcept override {
            return message.c_str();
        }
};

/* S-Expression Evaluator */
class S_Exp_Evaluator {
    private:
        //
    public:
        //
};

/* S-Expression Parser */
// <S-exp> ::= <ATOM>
//             | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
//             | QUOTE <S-exp>
         
// <ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
//             | NIL | T | LEFT-PAREN RIGHT-PAREN
class S_Exp_Parser {
    private:
        enum class LIST_MODE {
            NO_DOT,
            WITH_DOT,
            QUOTE
        };
    
        // store list modes and lists
        std::stack<std::pair<LIST_MODE, std::vector<std::shared_ptr<AST>>>> lists_info; // first: mode, second: list
        // check num of <S-exp> after DOT
        std::stack<std::pair<bool, int>> dot_info; // first: isDOTStart, second: <S-exp> after DOT
        S_Exp_Evaluator evaluator;
    
    public:
        void resetInfos() { // when a <S-exp> ended or error encountered
            lists_info = std::stack<std::pair<LIST_MODE, std::vector<std::shared_ptr<AST>>>>();
            dot_info = std::stack<std::pair<bool, int>>();
        }

        std::shared_ptr<AST> makeList(const std::vector<std::shared_ptr<AST>> &tree_root, // the part before DOT (car)
            const std::shared_ptr<AST> &cdr = std::make_shared<AST>(Token{TokenType::NIL, "nil"})) { // the part after DOT (cdr)
            std::shared_ptr<AST> res = cdr;
            for (int i = static_cast<int>(tree_root.size()) - 1; i >= 0; --i) res = std::make_shared<AST>(tree_root[i], res);
            return res;
        }

        void checkExit(const std::shared_ptr<AST> &tree_root) {
            if (! tree_root || tree_root->isAtom) return;
            if (tree_root->left && tree_root->left->isAtom && tree_root->left->atom.type == TokenType::SYMBOL && tree_root->left->atom.value == "exit"
                && (! tree_root->right || (tree_root->right->isAtom && tree_root->right->atom.type == TokenType::NIL))) throw ExitException::CorrectExit();
        }

        void endSExp(std::shared_ptr<AST> cur_node) {
            std::cout << "\n> ";
            printAST(cur_node);
            resetInfos();
        }

        bool parseAndBuildAST(const Token &token, int lineNum, int columnNum) {
            // check number of <S-exp> after DOT
            if (! dot_info.empty() && dot_info.top().first && dot_info.top().second == 1 && token.type != TokenType::RIGHT_PAREN) {
                resetInfos();
                throw SyntaxException::NoRightParen(lineNum, columnNum - token.value.length() + 1, token.value); // columnNum: the first char's pos of the token
            }
            // process token
            if (token.type == TokenType::QUOTE) lists_info.push({LIST_MODE::QUOTE, {std::make_shared<AST>(Token{TokenType::QUOTE, ""})}});
            else if (token.type == TokenType::DOT) {
                if (lists_info.empty() // start with DOT
                    || lists_info.top().first != LIST_MODE::NO_DOT // QUOTE + DOT or DOT + DOT
                    || lists_info.top().second.empty()) { // no <S-exp> before DOT, ex. > (.
                    resetInfos();
                    throw SyntaxException::UnexpectedToken(lineNum, columnNum, token.value);
                }
                lists_info.top().first = LIST_MODE::WITH_DOT;
                dot_info.push({true, 0}); // start counting <S-exp> after DOT
            }
            else if (token.type == TokenType::LEFT_PAREN) lists_info.push({LIST_MODE::NO_DOT, {}}); // push a new list into stack
            else if (token.type == TokenType::RIGHT_PAREN) {
                if (lists_info.empty() ||
                    (lists_info.top().first == LIST_MODE::WITH_DOT && ! dot_info.empty() && dot_info.top().first && dot_info.top().second == 0)) {
                    resetInfos();
                    throw SyntaxException::UnexpectedToken(lineNum, columnNum, token.value);
                }
                
                // get current list
                auto [cur_list_mode, cur_list] = lists_info.top();
                lists_info.pop();
                if (cur_list_mode == LIST_MODE::WITH_DOT) dot_info.pop();

                // build AST
                std::shared_ptr<AST> cur_node = nullptr;
                if (cur_list_mode == LIST_MODE::NO_DOT) cur_node = makeList(cur_list); // (cons car nil)
                else { // (cons car cdr)
                    // get cdr (part after DOT)
                    auto cdr = cur_list.back();
                    cur_list.pop_back();
                    // make dotted pair
                    cur_node = makeList(cur_list, cdr);
                }
                
                // NOTED: always check if lists_mode's top is quote when a <S-exp> ended (check the prev if quote)
                while (! lists_info.empty() && lists_info.top().first == LIST_MODE::QUOTE) {
                    // get quote
                    auto quote_list = std::move(lists_info.top().second);
                    lists_info.pop();
                    // make quote
                    cur_node = makeList({std::make_shared<AST>(Token{TokenType::QUOTE, ""}), cur_node});
                }

                // end a dotted-pair
                if (! lists_info.empty()) {
                    lists_info.top().second.push_back(cur_node);
                    if (lists_info.top().first == LIST_MODE::WITH_DOT) dot_info.top().second++;
                }
                else { // <S-exp> ended
                    checkExit(cur_node); // check if car == "exit" && cdr == "nil"
                    endSExp(cur_node);
                }
            }
            else {
                auto cur_node = std::make_shared<AST>(token); // <ATOM>

                // NOTED: always check if lists_mode's top is quote when a <S-exp> ended (check the prev if quote)
                while (! lists_info.empty() && lists_info.top().first == LIST_MODE::QUOTE) {
                    // get quote
                    auto quote_list = std::move(lists_info.top().second);
                    lists_info.pop();
                    // make quote
                    cur_node = makeList({std::make_shared<AST>(Token{TokenType::QUOTE, ""}), cur_node});
                }
        
                if (! lists_info.empty()) {
                    lists_info.top().second.push_back(cur_node);
                    if (lists_info.top().first == LIST_MODE::WITH_DOT) dot_info.top().second++;
                }
                else endSExp(cur_node); // <S-exp> ended
            }

            return lists_info.empty(); // if the whole <S-exp> ended
        }

        void printAST(const std::shared_ptr<AST> &cur, bool isRoot = true, int depth = 0, bool isFirstTokenOfLine = true) { // recursively print
            if (cur == nullptr) return;
            // ATOM
            if (cur->isAtom) {
                if (cur->atom.type == TokenType::QUOTE) std::cout << (isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") << "quote\n";
                else std::cout << (isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") << cur->atom.value << "\n";
                return;
            }
            // LP: new list started
            if (isRoot) {
                std::cout << (isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") << "(";
                depth++;
                isFirstTokenOfLine = false;
            }
            // car
            printAST(cur->left, true, depth, isFirstTokenOfLine);
            // cdr
            if (cur->right && cur->right->isAtom && cur->right->atom.type != TokenType::NIL) {
                std::cout << std::string(depth * 2, ' ') << ".\n";
                printAST(cur->right, true, depth);
            }
            else if (cur->right && ! cur->right->isAtom) printAST(cur->right, false, depth);
            else if (! cur->right || cur->right->atom.type == TokenType::NIL) ; // nothing
            else {
                std::cout << std::string(depth * 2, ' ') << ".\n";
                printAST(cur->right, true, depth);
            }
            // RP: cur list ended
            if (isRoot) {
                depth--;
                std::cout << std::string(depth * 2, ' ') << ")\n";
            }
        }
};    

/* S-Expression Lexer */
class S_Exp_Lexer {
    private:
        char ch;
        int lineNum = 1, columnNum = 0;
        std::unordered_map<char, char> escape_map = {{'t', '\t'}, {'n', '\n'}, {'\\', '\\'}, {'\"', '\"'}};
        S_Exp_Parser parser;

        bool isWhiteSpace(char ch) {
            return (ch == ' ' || ch == '\t' || ch == '\n');
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

        bool isInt(const std::string &str) {
            std::regex pattern("^[-+]?\\d+$");
            return std::regex_match(str, pattern);
        }

        bool isFloat(const std::string &str) {
            std::regex pattern("^[-+]?((\\d+\\.\\d*)|(\\.\\d+))$");
            return std::regex_match(str, pattern);
        }

        void judgeType(Token &token) {
            if (isInt(token.value) || isFloat(token.value)) {
                std::istringstream iss(token.value);
                std::stringstream oss;
                // formating int and float, also round float to "%.3f"
                if (token.value.find('.') != std::string::npos) {
                    token.type = TokenType::FLOAT;
                    double value;
                    iss >> value;
                    oss << std::fixed << std::setprecision(3) << value;
                }
                else {
                    token.type = TokenType::INT;
                    int value;
                    iss >> value;
                    oss << value;
                }
                token.value = oss.str();
            }
            else if (token.value == "(") token.type = TokenType::LEFT_PAREN;
            else if (token.value == ")") token.type = TokenType::RIGHT_PAREN;
            else if (token.value == ".") token.type = TokenType::DOT;
            else if (token.value[0] == '\"' && token.value[token.value.length()-1] == '\"') token.type = TokenType::STRING;
            else if (token.value == "\'") token.type = TokenType::QUOTE;
            else if (token.value == "nil" || token.value == "#f") {
                token.type = TokenType::NIL;
                token.value = "nil";
            }
            else if (token.value == "t" || token.value == "#t") {
                token.type = TokenType::T;
                token.value = "#t";
            }
            else token.type = TokenType::SYMBOL;
        }

        void eatALine() {
            std::string useless_line;
            std::getline(std::cin, useless_line);
        }

        bool saveAToken(Token &token, int lineNum, int columnNum, bool eat = true) {
            try {
                judgeType(token);
                bool res = parser.parseAndBuildAST(token, lineNum, columnNum);
                token = Token(); // reset
                return res;
            }
            catch (ExitException &e) { // CorrectExit, no need to eat a line
                parser.resetInfos();
                throw;
            }
            catch (SyntaxException &e) {
                parser.resetInfos();
                if (eat) eatALine(); // eat: if need to eat a line when error encountered
                throw;
            }
        }

    public:
        bool s_exp_ended = false;
        
        S_Exp_Lexer() {
            ch = '\0';
        }

        void readAndTokenize() {
            Token token;
            std::stack<char> parenStack;
            lineNum = 1;
            columnNum = 0;
            ch = '\0';
            bool start = false;
            s_exp_ended = false;

            while (std::cin.get(ch)) {
                start = true;

                if (ch == ';') {
                    if (token.value == "") {
                        eatALine();
                        if (s_exp_ended) {
                            lineNum = 1;
                            s_exp_ended = false;
                        }
                        else lineNum++;
                        columnNum = 0;
                    }
                    else {
                        if (token.value[0] == '\"') { // in STRING
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            s_exp_ended = saveAToken(token, lineNum, columnNum);
                            eatALine();
                            if (s_exp_ended) {
                                lineNum = 1;
                                s_exp_ended = false;
                            }
                            else lineNum++;
                            columnNum = 0;
                        }
                    }
                }
                else if (isWhiteSpace(ch)) {
                    if (ch == '\n') {
                        if (token.value == "") {
                            if (s_exp_ended) {
                                lineNum = 1;
                                s_exp_ended = false;
                            }
                            else lineNum++;
                            columnNum = 0;
                        }
                        else {
                            if (token.value[0] == '\"') { // newline while in STRING
                                columnNum++;
                                parser.resetInfos();
                                throw SyntaxException::NoClosingQuote(lineNum, columnNum);
                            }
                            else {
                                s_exp_ended = saveAToken(token, lineNum, columnNum, false);
                                
                                if (s_exp_ended) {
                                    lineNum = 1;
                                    s_exp_ended = false;
                                }
                                else lineNum++;
                                columnNum = 0;
                            }
                        }
                    }
                    else { // ' ' or '\t'
                        if (token.value == "") columnNum++;
                        else {
                            if (token.value[0] == '\"') { // in STRING
                                token.value += ch;
                                columnNum++;
                            }
                            else {
                                s_exp_ended = saveAToken(token, lineNum, columnNum);
                                
                                if (s_exp_ended) {
                                    lineNum = 1;
                                    columnNum = 1; // 1 for ' ' or '\t'
                                }
                                else columnNum++;
                            }
                        }
                    }
                }
                else if (ch == '(') {
                    if (token.value == "") {
                        parenStack.push(ch);
                        token.value += ch; // "("
                        columnNum++; // ex. "   f   (((.\n" -> ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 7 is >>.<<
                        s_exp_ended = saveAToken(token, lineNum, columnNum); // must be false
                    }
                    else {
                        if (token.value[0] == '\"') { // in STRING
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save previous
                            s_exp_ended = saveAToken(token, lineNum, columnNum);
                            
                            // save current
                            parenStack.push(ch);
                            token.value += ch; // "("
                            columnNum++; // ex. "123A((.\n" -> ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 3 is >>.<<
                            s_exp_ended = saveAToken(token, lineNum, columnNum); // must be false
                        }
                    }
                }
                else if (ch == ')') {
                    if (token.value == "") {
                        if (parenStack.empty()) { // no LP before RP
                            eatALine();
                            columnNum++;
                            parser.resetInfos();
                            throw SyntaxException::UnexpectedToken(lineNum, columnNum, ")");
                        }
                        else {
                            parenStack.pop();
                            token.value += ch; // ")"
                            columnNum++;
                            s_exp_ended = saveAToken(token, lineNum, columnNum);
                            if (s_exp_ended) {
                                lineNum = 1;
                                columnNum = 0;
                            }
                        }
                    }
                    else {
                        if (token.value[0] == '\"') { // in STRING
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save previous
                            s_exp_ended = saveAToken(token, lineNum, columnNum);
                            if (s_exp_ended) {
                                lineNum = 1;
                                columnNum = 0;
                            }
                            // save current
                            if (parenStack.empty()) { // no LP before RP
                                eatALine();
                                columnNum++;
                                parser.resetInfos();
                                throw SyntaxException::UnexpectedToken(lineNum, columnNum, ")");
                            }
                            else {
                                parenStack.pop();
                                token.value += ch; // ")"
                                columnNum++;
                                s_exp_ended = saveAToken(token, lineNum, columnNum);
                                if (s_exp_ended) {
                                    lineNum = 1;
                                    columnNum = 0;
                                }
                            }
                        }
                    }
                }
                else if (ch == '\\') {
                    if (token.value != "" && token.value[0] == '\"' && escape_map.count(std::cin.peek())) { // in STRING and legal escape
                        columnNum++; // first backslash
                        ch = std::cin.get(); // legal escape mode after backslash: 't' or 'n' or '\\' or '\"'
                        token.value += escape_map[ch];
                        columnNum++;
                    }
                    else {
                        token.value += ch;
                        columnNum++;
                    }
                }
                else if (ch == '\'') {
                    if (token.value == "") {
                        token.value += ch;
                        columnNum++;
                        s_exp_ended = saveAToken(token, lineNum, columnNum); // must be false
                    }
                    else {
                        if (token.value[0] == '\"') { // in STRING
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            s_exp_ended = saveAToken(token, lineNum, columnNum);
                            
                            token.value += ch; // "\'"
                            if (s_exp_ended) columnNum = 1;
                            else columnNum++;
                            s_exp_ended = saveAToken(token, lineNum, columnNum);
                        }
                    }
                }
                else if (ch == '\"') {
                    if (token.value == "") { // the start of a STRING
                        token.value += ch;
                        columnNum++; // cuz may have whitespace before so use ++ not set to 1
                    }
                    else {
                        if (token.value[0] == '\"') { // the end of a STRING
                            token.value += ch;
                            columnNum++;
                            s_exp_ended = saveAToken(token, lineNum, columnNum);
                            
                            if (s_exp_ended) columnNum = 0;
                        }
                        else { // token + STRING, with no whitespace ex. > asf"
                            // save previous
                            s_exp_ended = saveAToken(token, lineNum, columnNum);
                            
                            // the start of a STRING
                            token.value += ch;
                            if (s_exp_ended) columnNum = 1; // no whitespace so set to 1, ex. > asf" -> ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 2
                            else columnNum++; // ex. > (asf" -> ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 6
                        }
                    }
                }
                else {
                    token.value += ch;
                    columnNum++;
                    s_exp_ended = false;
                }
            }

            if (! start) throw ExitException::NoMoreInput();
        }
};

/* Main Read-Eval-Print-Loop */
int main() {
    std::getline(std::cin, gTestNum);
    std::cout << "Welcome to OurScheme!\n";
    S_Exp_Lexer lexer;
    while (true) {
        try {
            lexer.readAndTokenize();
        }
        catch (ExitException &e) {
            std::cout << e.what();
            break;
        }
        catch (SyntaxException &e) {
            std::cout << e.what();
        }
        catch (...) {
            std::cout << "Unknown error" << std::endl;
            break;
        }
    }
    return 0;
}