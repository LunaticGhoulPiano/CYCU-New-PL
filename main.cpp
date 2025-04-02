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

/* Atom Types */
enum class AtomType {
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

/* ATOM structure */
struct Atom {
    AtomType type;
    std::string value;
    void reset() {
        type = static_cast<AtomType>(-1);
        value = "";
    }
};

/* S-Expression structure (while reading into buffer) */
struct BufferSExp {
    std::vector<Atom> tokens;
    void reset() {
        tokens.clear();
    }
};

/* S-Expression Abstract Syntax Tree */
/*
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
            // std::cout << token.value;
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
*/

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
// Syntax of OurScheme:
// <S-exp> ::= <ATOM> | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | QUOTE <S-exp>
// <ATOM>  ::= SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN RIGHT-PAREN
class S_Exp_Lexer {
    
    private:
        char ch, prev_ch;
        int lineNum = 1, columnNum = 0;
        std::vector<BufferSExp> sexps;
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

        void printType(Atom atom) {
            if (atom.type == AtomType::LEFT_PAREN) std::cout << "LEFT_PAREN\n";
            else if (atom.type == AtomType::RIGHT_PAREN) std::cout << "RIGHT_PAREN\n";
            else if (atom.type == AtomType::INT) std::cout << "INT\n";
            else if (atom.type == AtomType::STRING) std::cout << "STRING\n";
            else if (atom.type == AtomType::DOT) std::cout << "DOT\n";
            else if (atom.type == AtomType::FLOAT) std::cout << "FLOAT\n";
            else if (atom.type == AtomType::NIL) std::cout << "NIL\n";
            else if (atom.type == AtomType::T) std::cout << "T\n";
            else if (atom.type == AtomType::QUOTE) std::cout << "QUOTE\n";
            else if (atom.type == AtomType::SYMBOL) std::cout << "SYMBOL\n";
            else std::cout << "ERROR_TYPE!\n";
        }

        void saveAnAtom(Atom &atom, BufferSExp &buffer) {
            // judge type
            if (isInt(atom.value) || isFloat(atom.value)) {
                std::istringstream iss(atom.value);
                std::stringstream oss;
                // formating int and float, also round float to "%.3f"
                if (atom.value.find('.') != std::string::npos) {
                    atom.type = AtomType::FLOAT;
                    double value;
                    iss >> value;
                    oss << std::fixed << std::setprecision(3) << value;
                }
                else {
                    atom.type = AtomType::INT;
                    int value;
                    iss >> value;
                    oss << value;
                }
                atom.value = oss.str();
            }
            else if (atom.value == "(") atom.type = AtomType::LEFT_PAREN;
            else if (atom.value == ")") {
                // deal the "()" here
                if (buffer.tokens.size() > 0 && buffer.tokens[buffer.tokens.size()-1].type == AtomType::LEFT_PAREN) {
                    buffer.tokens.pop_back();
                    atom.value = "nil";
                    atom.type = AtomType::NIL;
                }
                else atom.type = AtomType::RIGHT_PAREN;
            }
            else if (atom.value == ".") atom.type = AtomType::DOT;
            else if (atom.value[0] == '\"' && atom.value[atom.value.length()-1] == '\"') atom.type = AtomType::STRING;
            else if (atom.value == "\'") atom.type = AtomType::QUOTE;
            else if (atom.value == "nil" || atom.value == "#f") {
                atom.type = AtomType::NIL;
                atom.value = "nil";
            }
            else if (atom.value == "t" || atom.value == "#t") {
                atom.type = AtomType::T;
                atom.value = "#t";
            }
            else atom.type = AtomType::SYMBOL;

            std::cout << "\n> save an atom: ->" << atom.value << "<-\n";
            printType(atom);
            buffer.tokens.push_back(atom);
            atom.reset();

            // debug
            int size = buffer.tokens.size();
            std::cout << "Cur tokens:\n";
            for (int i = 0; i < size; i++) {
                std::cout << buffer.tokens[i].value << " ";
                printType(buffer.tokens[i]);
            }
            if (size >= 3 && buffer.tokens[size - 3].value == "(" && buffer.tokens[size - 2].value == "exit" && buffer.tokens[size - 1].value == ")") {
                std::cout << "\nThanks for using OurScheme!\n";
                system("pause");
                system("pause");
                exit(0);
            }
        }

    public:
        bool allow_newline_in_token = false;
        
        S_Exp_Lexer() {
            ch = '\0';
            prev_ch = '\0';
        }

        void read() {
            BufferSExp buffer; // to store single token
            Atom atom;
            std::stack<char> parenStack;
            lineNum = 1;
            columnNum = 0;
            ch = '\0';
            prev_ch = '\0';
            if (! allow_newline_in_token) std::cout << "\nA> "; // init input prompt

            while (std::cin.get(ch)) {
                if (ch == ';') {
                    if (atom.value != "") {
                        if (atom.value[0] == '\"') { // in string
                            atom.value += ch;
                            columnNum++;
                        }
                        else {
                            // save an atom
                            saveAnAtom(atom, buffer);

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
                    if (atom.value == "") {
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
                            if (atom.value[0] == '\"') {
                                columnNum++;
                                throw NoClosingQuote(lineNum, columnNum);
                            }
                            else {
                                // save a atom
                                saveAnAtom(atom, buffer);

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
                            if (atom.value[0] == '\"') {
                                atom.value += ch;
                                columnNum++;
                            }
                            else {
                                // save an atom
                                saveAnAtom(atom, buffer);

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
                    if (atom.value != "") {
                        if (atom.value[0] == '\"') { // in string
                            atom.value += ch;
                            columnNum++;
                        }
                        else {
                            // save an atom
                            saveAnAtom(atom, buffer);
                            // set position
                            columnNum = 0;

                            // set new parenStack
                            parenStack.push(ch);
                            // save an atom
                            atom.value += ch;
                            saveAnAtom(atom, buffer);

                            // set position
                            columnNum = 0;
                        }
                    }
                    else {
                        parenStack.push(ch);
                        
                        // save an atom
                        atom.value += ch;
                        saveAnAtom(atom, buffer);

                        // set position
                        columnNum = 0;
                    }
                }
                else if (ch == ')') { // judge both end of a single-quote or in string
                    if (atom.value != "") {
                        if (atom.value[0] == '\"') { // in string
                            atom.value += ch;
                            columnNum++;
                        }
                        else {
                            // save an atom
                            saveAnAtom(atom, buffer);
                            // set position
                            columnNum = 0;

                            // check parenStack
                            if (parenStack.empty()) {
                                while (std::cin.get() != '\n') std::cin.get(ch);
                                throw UnexpectedToken(lineNum, columnNum, ")");
                            }
                            else {
                                parenStack.pop();
                                // save an atom
                                atom.value += ch;
                                saveAnAtom(atom, buffer);

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
                            // save an atom
                            atom.value += ch;
                            saveAnAtom(atom, buffer);

                            // set position
                            columnNum = 0;
                        }
                    }
                }
                else if (ch == '\"') {
                    if (atom.value == "") { // the start of a string
                        atom.value += ch;
                        columnNum++;
                    }
                    else {
                        if (atom.value[0] == '\"') { // end of a string
                            // save a atom
                            atom.value += ch;
                            saveAnAtom(atom, buffer);
    
                            // set position
                            if (allow_newline_in_token) lineNum++;
                            else lineNum = 1;
                            columnNum = 0;
                        }
                        else { // ex. > "" ""asf"" -> temp = "asf", ch = '\"'
                            // save an atom
                            saveAnAtom(atom, buffer);
                            columnNum = 0;
    
                            // set the new starting string's char and position
                            atom.value += ch;
                            columnNum++;
                        }
                    }
                }
                else if (ch == '\'') {
                    if (atom.value == "") { // the start of a single-quote
                        // save an atom
                        columnNum++;
                        atom.value += ch;
                        saveAnAtom(atom, buffer);

                        // set position
                        columnNum = 0;
                    }
                    else {
                        if (atom.value[0] == '\"') { // in string
                            atom.value += ch;
                            columnNum++;
                        }
                        else {
                            // save an atom
                            saveAnAtom(atom, buffer);
                            columnNum = 0;
                            
                            // save an atom
                            columnNum++;
                            atom.value += ch;
                            saveAnAtom(atom, buffer);
                            columnNum = 0;
                        }
                    }
                }
                else if (ch == '\\') {
                    if (atom.value != "" && atom.value[0] == '\"' && escape_map.count(std::cin.peek())) { // in string, escape
                        // ch + next_ch = "\t" or "\n" or "\\" or "\""
                        columnNum++; // first backslash
                        ch = std::cin.get();
                        atom.value += escape_map[ch];
                        columnNum++;
                    }
                    else { // just a normal backslash
                        atom.value += ch;
                        columnNum++;
                    }
                }
                else {
                    atom.value += ch;
                    columnNum++;
                }
            }
        }
};

/* S-Expression Recursive Descent Parser */
class S_Exp_Parser {
    private:
        //
    public:
        //
};

/* Main Read-Eval-Print-Loop */
int main() {
    std::getline(std::cin, gTestNum);
    std::cout << "Welcome to OurScheme!" << std::endl;
    S_Exp_Lexer lexer;
    S_Exp_Parser parser;
    while (true) {
        try {
            lexer.read();
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