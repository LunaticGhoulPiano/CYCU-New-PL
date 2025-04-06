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
    Token_Type type;
    std::string value;
    void reset() {
        type = static_cast<Token_Type>(-1);
        value = "";
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
// <ATOM>  ::= SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN RIGHT-PAREN
class S_Exp_Lexer {
    
    private:
        char ch, prev_ch;
        int lineNum = 1, columnNum = 0;
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
            else std::cout << "ERROR_TYPE!\n";
        }

        void judgeType(Token &token, std::vector<Token> &buffer) {
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
                if (buffer.size() > 0 && buffer[buffer.size()-1].type == Token_Type::LEFT_PAREN) {
                    buffer.pop_back();
                    token.value = "nil";
                    token.type = Token_Type::NIL;
                }
                else token.type = Token_Type::RIGHT_PAREN;
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

        void saveAToken(Token &token, std::vector<Token> &buffer) {
            // judge type
            judgeType(token, buffer);

            // push into buffer
            // Should replace the method into build and push a node into tree and parse,
            // rather then just simply push into a vector.
            // Should check if tree is empty;
            // if empty then build tree,
            // then parse (test) the current atom to match s-expression;
            // if no match, return false;
            // else continue, then return true at the end.
            std::cout << "\n> push a token: ->" << token.value << "<-\n";
            printType(token);
            buffer.push_back(token); // should replace to parser.parse(atom);
            token.reset();

            // debug
            int size = buffer.size();
            std::cout << "Cur tokens:\n";
            for (int i = 0; i < size; i++) {
                std::cout << buffer[i].value << " ";
                printType(buffer[i]);
            }
            if (size >= 3 && buffer[size - 3].value == "(" && buffer[size - 2].value == "exit" && buffer[size - 1].value == ")") {
                std::cout << "\nThanks for using OurScheme!\n";
                system("pause");
                system("pause");
                exit(0);
            }

            // return if a S-exp end
        }

    public:
        bool allow_newline_in_token = false;
        
        S_Exp_Lexer() {
            ch = '\0';
            prev_ch = '\0';
        }

        void readAndTokenize() {
            std::vector<Token> buffer; // to store single token
            Token token;
            std::stack<char> parenStack;
            lineNum = 1;
            columnNum = 0;
            ch = '\0';
            prev_ch = '\0';
            if (! allow_newline_in_token) std::cout << "\nA> "; // init input prompt

            while (std::cin.get(ch)) {
                if (ch == ';') {
                    if (token.value != "") {
                        if (token.value[0] == '\"') { // in string
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save a token
                            saveAToken(token, buffer);

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
                                saveAToken(token, buffer);

                                // set position
                                if (allow_newline_in_token) lineNum++;
                                else {
                                    lineNum = 1;
                                    std::cout << "\nD> ";
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
                                saveAToken(token, buffer);

                                // set position
                                if (allow_newline_in_token) lineNum++;
                                else {
                                    lineNum = 1;
                                    std::cout << "\nE> ";
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
                            saveAToken(token, buffer);
                            // set position
                            columnNum = 0;

                            // set new parenStack
                            parenStack.push(ch);
                            // save a token
                            token.value += ch;
                            saveAToken(token, buffer);

                            // set position
                            columnNum = 0;
                        }
                    }
                    else {
                        parenStack.push(ch);
                        
                        // save a token
                        token.value += ch;
                        saveAToken(token, buffer);

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
                            saveAToken(token, buffer);
                            // set position
                            columnNum = 0;

                            // check parenStack
                            if (parenStack.empty()) {
                                while (std::cin.get() != '\n') std::cin.get(ch);
                                throw UnexpectedToken(lineNum, columnNum, ")");
                            }
                            else {
                                parenStack.pop();
                                // save a token
                                token.value += ch;
                                saveAToken(token, buffer);

                                // set position
                                columnNum = 0;
                            }
                        }
                    }
                    else {
                        // check parenStack
                        if (parenStack.empty()) {
                            columnNum++;
                            while (std::cin.peek() != '\n') std::cin.get(ch);
                            throw UnexpectedToken(lineNum, columnNum, ")");
                        }
                        else {
                            parenStack.pop();
                            // save a token
                            token.value += ch;
                            saveAToken(token, buffer);

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
                            saveAToken(token, buffer);
    
                            // set position
                            if (allow_newline_in_token) lineNum++;
                            else lineNum = 1;
                            columnNum = 0;
                        }
                        else { // ex. > "" ""asf"" -> temp = "asf", ch = '\"'
                            // save a token
                            saveAToken(token, buffer);
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
                        saveAToken(token, buffer);

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
                            saveAToken(token, buffer);
                            columnNum = 0;
                            
                            // save a token
                            columnNum++;
                            token.value += ch;
                            saveAToken(token, buffer);
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
        }
};

/* Main Read-Eval-Print-Loop */
int main() {
    std::getline(std::cin, gTestNum);
    std::cout << "Welcome to OurScheme!" << std::endl;
    S_Exp_Lexer lexer;
    while (true) {
        try {
            lexer.readAndTokenize();
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