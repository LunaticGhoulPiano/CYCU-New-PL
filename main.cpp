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
bool gIsFirstSExpInputed = false; // to record if the first legal <S-exp> has been read

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

/* Keyword types */
// define all types of primitives and funcitons
enum class KeywordType {
    /* primitive types */
    // minimal types
    INTEGER,
    FLOAT,
    STRING,
    ERROR_OBJECT,
    BOOLEAN,
    SYMBOL,
    // complex primitives
    REAL,
    NUMBER, // same as REAL
    // these are also complex primitives, but should be judgement actions, not the defined types
    ATOM, // can be any of the primitive types
    NIL, // can be boolean, aka nil, which is a minimal type, or any of the functions that return the minimal type NIL, i.e. ()
    LIST, // can be a minimal type NIL, or a function (e.g. CONSTRUCTOR::LIST, SEQUENCING_AND_FUNCTIONAL_COMPOSITION::BEGIN) (<S-exp>s)
    PAIR, // is a kind of LIST, be the number of <S-exp>s in (<S-exp>s) should be even
    /* function types */
    CONSTRUCTOR,
    BYPASS_EVALUATION,
    BINDING,
    PART_ACCESSOR,
    PRIMITIVE_PREDICATE,
    OPERATION,
    EQIVALENCE_TESTER,
    SEQUENCING_AND_FUNCTIONAL_COMPOSITION,
    CONDITIONAL,
    READ,
    DISPLAY,
    LAMBDA,
    VERBOSE,
    EVALUATION,
    CONVERT_TO_STRING,
    ERROR_OBJECT_OPERATION,
    CLEAN_ENVIRONMENT,
    EXIT
};

/* Argument number mode */
// define the number of arguments that a function accepts
enum class ARGUMENT_NUMBER_MODE {
    AT_LEAST, // argument number >= n
    MUST_BE, // argument number == n
    SPECIFIC // argument number is one of n1, n2, ..., nk
};

/* KeywordInfo */
// define the informations of primitives and functions
struct KeywordInfo {
    bool isPrimitive;
    ARGUMENT_NUMBER_MODE arg_mode;
    std::vector<int> arg_nums;
    KeywordType functionType, returnType;
};

/* Keywords */
std::unordered_map<std::string, KeywordInfo> gKeywords = {
    // primitives
    {"#t", {true}},
    {"nil", {true}},
    // functions
    {"cons", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::CONSTRUCTOR, KeywordType::PAIR}},
    {"list", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {0}, KeywordType::CONSTRUCTOR, KeywordType::LIST}},
    {"quote", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::BYPASS_EVALUATION}}, // returnType can be any of primitives
    {"define", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::BINDING}}, // returnType can be any of primitives
    {"let", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::BINDING}}, // returnType can be any of primitives
    {"set!", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::BINDING}}, // returnType can be any of primitives
    {"car", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PART_ACCESSOR}}, // returnType can be any of primitives
    {"cdr", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PART_ACCESSOR}}, // returnType can be any of primitives
    {"atom?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"pair?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"list?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"null?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"integer?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"real?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"number?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"string?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"boolean?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"symbol?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"+", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::NUMBER}},
    {"-", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::NUMBER}},
    {"*", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::NUMBER}},
    {"/", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::NUMBER}},
    {"not", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"and", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION}}, // returnType can be NUMBER or BOOLEAN
    {"or", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION}}, // returnType can be NUMBER or BOOLEAN
    {">", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {">=", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"<", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"<=", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"=", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"string-append", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::STRING}},
    {"string>?", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"string<?", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"string=?", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"eqv?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::EQIVALENCE_TESTER, KeywordType::BOOLEAN}},
    {"equal?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::EQIVALENCE_TESTER, KeywordType::BOOLEAN}},
    {"begin", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {1}, KeywordType::SEQUENCING_AND_FUNCTIONAL_COMPOSITION}}, // returnType can be any of primitives
    {"if", {false, ARGUMENT_NUMBER_MODE::SPECIFIC, {2, 3}, KeywordType::CONDITIONAL}}, // returnType can be any of primitives
    {"else", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {1}, KeywordType::CONDITIONAL}}, // returnType can be any of primitives // special case
    {"cond", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {1}, KeywordType::CONDITIONAL}}, // returnType can be any of primitives
    {"read", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::READ}}, // returnType can be any of primitives
    {"write", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::DISPLAY}}, // returnType can be any of primitives
    {"display-string", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::DISPLAY}}, // returnType can be STRING or ERROR_OBJECT
    {"newline", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::DISPLAY, KeywordType::NIL}},
    {"lambda", {false, ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::LAMBDA}}, // returnType can be any of primitives
    {"verbose", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::VERBOSE, KeywordType::BOOLEAN}},
    {"verbose?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::VERBOSE, KeywordType::BOOLEAN}},
    {"eval", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::EVALUATION}}, // returnType can be any of primitives
    {"symbol->string", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::CONVERT_TO_STRING, KeywordType::STRING}},
    {"number->string", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::CONVERT_TO_STRING, KeywordType::STRING}},
    {"create-error-object", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::ERROR_OBJECT_OPERATION, KeywordType::ERROR_OBJECT}},
    {"error-object?", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::ERROR_OBJECT_OPERATION, KeywordType::BOOLEAN}},
    {"clean-environment", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::CLEAN_ENVIRONMENT}},
    {"exit", {false, ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::EXIT}}
};

/* Token structure */
struct Token {
    TokenType type = TokenType::NIL;
    std::string value = "";
    Token(): type(TokenType::NIL), value("") {}
    Token(TokenType t, const std::string& v): type(t), value(v) {}
};

/* AST structure */
struct AST {
    bool isAtom = false;
    Token token;
    std::shared_ptr<AST> left = nullptr, right = nullptr;
    AST(Token t) : isAtom(true), token(std::move(t)) {}
    AST(std::shared_ptr<AST> l, std::shared_ptr<AST> r) : isAtom(false), left(std::move(l)), right(std::move(r)) {}
    bool isEndNode() { return left == nullptr && right == nullptr; }
};

/* Debugger */
class Debugger {
    public:
        std::string getType(Token token) {
            if (token.type == TokenType::LEFT_PAREN) return "LEFT_PAREN";
            else if (token.type == TokenType::RIGHT_PAREN) return "RIGHT_PAREN";
            else if (token.type == TokenType::INT) return "INT";
            else if (token.type == TokenType::STRING) return "STRING";
            else if (token.type == TokenType::DOT) return "DOT";
            else if (token.type == TokenType::FLOAT) return "FLOAT";
            else if (token.type == TokenType::NIL) return "NIL";
            else if (token.type == TokenType::T) return "T";
            else if (token.type == TokenType::QUOTE) return "QUOTE";
            else if (token.type == TokenType::SYMBOL) return "SYMBOL";
            else return "ERROR: didn't judged!";
        }

        void printType(Token token) {
            std::cout << getType(token) << std::endl;
        }

        void debugPrintAST(const std::shared_ptr<AST> node, int depth = 0, const std::string &prefix = "AST_root") {
            if (! node) return;

            std::string indent(depth * 2, ' ');
            std::cout << indent << prefix;

            if (node->isAtom) std::cout << " (isAtom = true, atom = \"" << node->token.value << "\", type = " << getType(node->token) << ")\n";    
            else {
                std::cout << " (isAtom = false)\n";
                debugPrintAST(node->left, depth + 1, "|--- left  -> ");
                debugPrintAST(node->right, depth + 1, "|--- right -> ");
            }
        }
};
Debugger gDebugger;

/* Error Exceptions */
class ExitException: public std::exception { // Common usage
    protected:
        std::string message = "";

    public:
        explicit ExitException(const std::string &msg): message(msg) {}
        const char *what() const noexcept override {
            return message.c_str();
        }

        static ExitException CorrectExit() { // Exit, implemented in project 1
            return ExitException("\nThanks for using OurScheme!");
        }

        static ExitException NoMoreInput() { // EOF & Exit, implemented in project 1
            return ExitException("ERROR (no more input) : END-OF-FILE encountered\nThanks for using OurScheme!");
        }

        static ExitException NoMoreInputWhileRead() { // implemented in project 4, but current might be incorrect outputs
            return ExitException("ERROR : END-OF-FILE encountered when there should be more input\nThanks for using OurScheme!");
        }
};

class SyntaxException: public std::exception {
    protected:
        std::string message = "";

    public:
        explicit SyntaxException(const std::string &msg): message(msg) {}
        const char *what() const noexcept override {
            return message.c_str();
        }

        static SyntaxException UnexpectedToken(int line, int column, const std::string token) { // unexpected token, implemented in project 1
            return SyntaxException("ERROR (unexpected token) : atom or '(' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<\n");
        }

        static SyntaxException NoRightParen(int line, int column, const std::string token) { // no RP, implemented in project 1
            return SyntaxException("ERROR (unexpected token) : ')' expected when token at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + " is >>" + token + "<<\n");
        }

        static SyntaxException NoClosingQuote(int line, int column) { // no second \", implemented in project 1
            return SyntaxException("ERROR (no closing quote) : END-OF-LINE encountered at Line "
                + std::to_string(line) + " Column " + std::to_string(column) + "\n");
        }
};

class SemanticException: public std::exception {
    protected:
        std::string message = "";

    public:
        explicit SemanticException(const std::string &msg): message(msg) {}
        const char *what() const noexcept override {
            return message.c_str();
        }

        // level error
        static SemanticException LevelOfCleanEnv() { // implemented in project 2
            return SemanticException("ERROR (level of CLEAN-ENVIRONMENT)\n");
        }

        static SemanticException LevelOfDefine() { // implemented in project 2
            return SemanticException("ERROR (level of DEFINE)\n");
        }

        static SemanticException LevelOfExit() { // implemented in project 2
            return SemanticException("ERROR (level of EXIT)\n");
        }

        // format error
        static SemanticException CondFormat(std::string s_exp) { // implemented in project 2
            return SemanticException("ERROR (COND format) : " + s_exp + "\n");
        }

        static SemanticException DefineFormat(std::string s_exp) { // implemented in project 2
            return SemanticException("ERROR (DEFINE format) : " + s_exp + "\n");
        }

        static SemanticException SetFormat(std::string s_exp) { // implemented in project 4
            return SemanticException("ERROR (SET! format) : " + s_exp + "\n");
        }

        static SemanticException LetFormat(std::string s_exp) { // implemented in project 3
            return SemanticException("ERROR (LET format) : " + s_exp + "\n");
        }

        static SemanticException LambdaFormat(std::string s_exp) { // implemented in project 3
            return SemanticException("ERROR (LAMBDA format) : " + s_exp + "\n");
        }

        // symbol error
        static SemanticException UnboundSymbol(std::string symbol) { // implemented in project 2
            return SemanticException("ERROR (unbound symbol) : " + symbol + "\n");
        }

        // argument error
        static SemanticException IncorrectNumOfArgs(std::string arg) { // implemented in project 2
            return SemanticException("ERROR (incorrect number of arguments) : " + arg + "\n");
        }
        
        static SemanticException IncorrectCarArgType(std::string op, std::string arg) { // implemented in project 2
            return SemanticException("ERROR (" + op +" with incorrect argument type) : " + arg + "\n");
        }

        static SemanticException NonList(std::string s_exp) { // implemented in project 2
            return SemanticException("ERROR (non-list) : " + s_exp + "\n");
        }

        // function error
        static SemanticException NonFunction(std::string arg) { // implemented in project 2
            return SemanticException("ERROR (attempt to apply non-function) : " + arg + "\n");
        }
        
        static SemanticException NoReturnValue(std::string s_exp) { // implemented in project 2
            return SemanticException("ERROR (no return value) : " + s_exp + "\n");
        }
};

class RuntimeException: public std::exception {
    protected:
        std::string message = "";

    public:
        explicit RuntimeException(const std::string &msg): message(msg) {}
        const char *what() const noexcept override {
            return message.c_str();
        }

        // function error
        static RuntimeException DivisionByZero() { // implemented in project 2
            return RuntimeException("ERROR (division by zero) : /\n");
        }
};

/* Global output printer */
class Printer { // all outputs are dealed here
    public:

        void printPrompt() {
            std::cout << "\n> ";
        }

        void printSExp(std::string s_exp) {
            std::cout << s_exp;
            printPrompt(); // print the next prompt
        }

        void printError(const std::exception &e) {
            std::cout << e.what();
            // the next prompt will be printed in S_Exp_Lexer()::readAndTokenize()
        }

        void printResult(std::string result) {
            std::cout << result;
            printPrompt();
        }

        std::string getprettifiedSExp(const std::shared_ptr<AST> &cur, std::string s_exp = "", int depth = 0, bool isRoot = true, bool isFirstTokenOfLine = true) { // recursively append into the string
            if (cur != nullptr) {
                if (cur->isAtom) { // <S-exp> ::= <ATOM>
                    if (cur->token.type == TokenType::QUOTE) s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + "quote\n");
                    else s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + cur->token.value + "\n");
                }
                else { // <S-exp> ::= LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | <S-exp> ::= QUOTE <S-exp>
                    // LP: new list started
                    if (isRoot) {
                        s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + "(");
                        depth++;
                        isFirstTokenOfLine = false;
                    }
                    // car
                    s_exp = getprettifiedSExp(cur->left, s_exp, depth, true, isFirstTokenOfLine);
                    // cdr
                    if (cur->right && cur->right->isAtom && cur->right->token.type != TokenType::NIL) {
                        s_exp += (std::string(depth * 2, ' ') + ".\n");
                        s_exp = getprettifiedSExp(cur->right, s_exp, depth, true, true);
                    }
                    else if (cur->right && ! cur->right->isAtom) s_exp = getprettifiedSExp(cur->right, s_exp, depth, false, true);
                    else if (! cur->right || cur->right->token.type == TokenType::NIL) ; // nothing
                    else {
                        s_exp += (std::string(depth * 2, ' ') + ".\n");
                        s_exp = getprettifiedSExp(cur->right, s_exp, depth, true, true);
                    }
                    // RP: cur list ended
                    if (isRoot) {
                        depth--;
                        s_exp += (std::string(depth * 2, ' ') + ")\n");
                    }
                }
            }

            return s_exp;
        }
};
Printer gPrinter;

/* S-Expression Evaluator */
class S_Exp_Executor {
    private:
        std::unordered_map<std::string, KeywordType> env; // to store the user-defined bindings

        bool isKeyword(const std::string &str) {
            return gKeywords.find(str) != gKeywords.end();
        }

        bool isDefined(const std::string &str) {
            return env.find(str) != env.end();
        }

        bool isAtomAFunctionName(std::string atom) {
            // if the atom is a symbol that is bound to a function, will not check the bindings (cuz it should check outside)
            if (atom != "#t" && atom != "nil" && atom != "else" && isKeyword(atom)) return true;
            else return false;
        }

        /* functions */
        // KeywordType::CONSTRUCTOR
        std::shared_ptr<AST> constructPair() { // cons
            //
        }

        std::shared_ptr<AST> constructList() { // list
            //
        }

        // KeywordType::BYPASS_EVALUATION

        // KeywordType::BINDING

        // KeywordType::PART_ACCESSOR

        // KeywordType::PRIMITIVE_PREDICATE        

        // KeywordType::OPERATION

        // KeywordType::EQIVALENCE_TESTER

        // KeywordType::SEQUENCING_AND_FUNCTIONAL_COMPOSITION

        // KeywordType::CONDITIONAL

        // KeywordType::READ

        // KeywordType::DISPLAY

        // KeywordType::LAMBDA

        // KeywordType::VERBOSE

        // KeywordType::EVALUATION

        // KeywordType::CONVERT_TO_STRING

        // KeywordType::ERROR_OBJECT_OPERATION

        // KeywordType::CLEAN_ENVIRONMENT

        // KeywordType::EXIT

    public:
        std::string evaluate(std::shared_ptr<AST> cur) {
            // TODO: add the to-return-value (string or function in env) into AST and return std::shared_ptr<AST>
            // or just make a new struct that has AST and the above values
            if (cur->isAtom) {
                if (cur->token.type != TokenType::SYMBOL) return cur->token.value;
                else {
                    if (isAtomAFunctionName(cur->token.value)) return ("#<procedure " + cur->token.value + ">\n");
                    else {
                        // check the binding
                        if (! isDefined(cur->token.value)) throw SemanticException::UnboundSymbol(cur->token.value);
                        else {
                            // TODO: get the bind of symbol, i.e. return gPrinter.getprettifiedSExp(env[cur->token.value]) or and internal function
                        }
                    }
                }
            }
            else { // functions
                //
            }
        }

        void execute(std::shared_ptr<AST> root) {
            // TODO: if return a internal function, execute it
            gPrinter.printResult(evaluate(root)); // temp
        }
};

/* Below comments are deprecated codes */
// S-Expression Evaluator
/*
class S_Exp_Evaluator {
    private:
        
        // just to record how to use std::functional
        //std::unordered_map<std::string, std::function<double(double, double)>> binary_operators = {
        //    {"+", [](double a, double b) { return a + b; }}, // a + b
        //    {"-", [](double a, double b) { return a - b; }}, // a - b
        //    {"*", [](double a, double b) { return a * b; }}, // a * b
        //    {"/", [](double a, double b) { return a / b; }}, // a / b
        //    {"<", [](double a, double b) { return a < b; }}, // a < b
        //    {">", [](double a, double b) { return a > b; }}, // a > b
        //    {"<=", [](double a, double b) { return a <= b; }}, // a <= b
        //    {">=", [](double a, double b) { return a >= b; }}, // a >= b
        //    {"=", [](double a, double b) { return a == b; }}, // a == b
        //    {"!=", [](double a, double b) { return a != b; }}, // a != b
        //};

        std::unordered_map<std::string, std::shared_ptr<AST>> env;

        bool isKeyword(const std::string &str) {
            return keywords.find(str) != keywords.end();
        }

        bool isDefined(const std::string &str) {
            return env.find(str) != env.end();
        }

        std::string prettyWriteSExp(const std::shared_ptr<AST> &cur, std::string s_exp = "", int depth = 0, bool isRoot = true, bool isFirstTokenOfLine = true) { // recursively print
            if (cur != nullptr) {
                if (cur->isAtom) { // <S-exp> ::= <ATOM>
                    if (cur->token.type == TokenType::QUOTE) s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + "quote\n");
                    else s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + cur->token.value + "\n");
                }
                else { // <S-exp> ::= LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | <S-exp> ::= QUOTE <S-exp>
                    // LP: new list started
                    if (isRoot) {
                        s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + "(");
                        depth++;
                        isFirstTokenOfLine = false;
                    }
                    // car
                    s_exp = prettyWriteSExp(cur->left, s_exp, depth, true, isFirstTokenOfLine);
                    // cdr
                    if (cur->right && cur->right->isAtom && cur->right->token.type != TokenType::NIL) {
                        s_exp += (std::string(depth * 2, ' ') + ".\n");
                        s_exp = prettyWriteSExp(cur->right, s_exp, depth, true, true);
                    }
                    else if (cur->right && ! cur->right->isAtom) s_exp = prettyWriteSExp(cur->right, s_exp, depth, false, true);
                    else if (! cur->right || cur->right->token.type == TokenType::NIL) ; // nothing
                    else {
                        s_exp += (std::string(depth * 2, ' ') + ".\n");
                        s_exp = prettyWriteSExp(cur->right, s_exp, depth, true, true);
                    }
                    // RP: cur list ended
                    if (isRoot) {
                        depth--;
                        s_exp += (std::string(depth * 2, ' ') + ")\n");
                    }
                }
            }

            return s_exp;
        }

        void executeByKeywords(std::shared_ptr<AST> root) {
            switch(keywords[root->left->token.value].first) {
                case KEYWORD_TYPE::CONSTRUCTOR: {
                    //
                    break;
                }
                case KEYWORD_TYPE::QUOTE: {
                    //
                    break;
                }
                case KEYWORD_TYPE::DEFINE: {
                    //
                    break;
                }
                case KEYWORD_TYPE::PART_ACCESSOR: {
                    //
                    break;
                }
                case KEYWORD_TYPE::PRIMITIVE_PREDICATE: {
                    //
                    break;
                }
                case KEYWORD_TYPE::OPERATOR: {
                    //
                    break;
                }
                case KEYWORD_TYPE::EQIVALENCE_TESTER: {
                    //
                    break;
                }
                case KEYWORD_TYPE::SEQUENCING_AND_FUNCTIONAL_COMPOSITION: {
                    //
                    break;
                }
                case KEYWORD_TYPE::CONDITIONAL_OPERATOR: {
                    //
                    break;
                }
                case KEYWORD_TYPE::CLEAN_ENVIRONMENT: {
                    env.clear();
                    std::cout << "\n> environment cleaned\n";
                    break;
                }
            }
        }

        void checkLegalSExp(std::shared_ptr<AST> root, int layer) { // recursively judge
            int increased_layer = layer;
            std::shared_ptr<AST> temp = root;
            
            //while ({()}), i.e., there are ((((( ... then <keyword> {<arg>s} ... )))))
            //after while, temp->left must be a symbol or <keyword>
            //ex. (((just_a_atom) test a))
            //    ^ ^     ^
            //   /   \    |
            // root temp temp->left
            //  0     2   3 // increased_layer
            while (temp->left->token.type == TokenType::NIL) { // while LP
                increased_layer++;
                temp = temp->left;
            }

            // judge if the root->left is a <keyword> or a bound/unbound symbol
            // if (<keyword> {<arg>s}) -> root->left must be <keyword>
            // if ({(<keyword> {<arg>s)}) -> root->left must be NIL
            if (! isKeyword(root->left->token.value)) {
                if (root->left->isAtom) {
                    //
                }
                else {
                    //
                }
            }
            
            // count count each number of argument of a complete non-<ATOM> <S-exp>s from the current layer (current <S-exp>)
            int arg_num = 0;
            std::shared_ptr<AST> temp = root->right;
            while (temp->right != nullptr) {
                arg_num++;
                temp = temp->right;
            }

            std::cout << "\n> " << root->left->token.value << ": there are " << arg_num << " arguments.\n"; // just for debug
            
            // judge the current layer
            switch (keywords[root->left->token.value].second.first) {
                case KEYWORD_NUM_MODE::AT_LEAST: {
                    if (arg_num < keywords[root->left->token.value].second.second[0]) throw SemanticException::IncorrectNumOfArgs(root->left->token.value);
                    break;
                }
                case KEYWORD_NUM_MODE::ONLY: {
                    if (arg_num != keywords[root->left->token.value].second.second[0]) throw SemanticException::IncorrectNumOfArgs(root->left->token.value);
                    break;
                }
                case KEYWORD_NUM_MODE::SPECIFIC: {
                    const std::vector<int> &arg_nums = keywords[root->left->token.value].second.second; // legal numbers
                    if (std::find(arg_nums.begin(), arg_nums.end(), arg_num) == arg_nums.end()) throw SemanticException::IncorrectNumOfArgs(root->left->token.value);
                    break;
                }
            }

            // if there are sub <S-exp>, judge the following
            if (root->right->left != nullptr && ! root->right->left->isAtom) checkLegalSExp(root->right->left, layer + 2);
            if (root->right->right != nullptr && root->right->right->left != nullptr
                && (! root->right->right->left->isAtom
                    || (root->right->right->left->token.type == TokenType::NIL
                        && root->right->right->left->left != nullptr))) checkLegalSExp(root->right->right->left, layer + 3);
        }

    public:
        void evaluate(std::shared_ptr<AST> root) {
            if (! root->isAtom) {
                checkLegalSExp(root, 0);
                //if (root->left->token.type == TokenType::QUOTE) { // QUOTE: "quote"
                //    std::cout << "\n> " << prettyWriteSExp(root); // temp
                //}
                std::cout << "\n> " << prettyWriteSExp(root); // just print it
            }
            else { // <ATOM>
                if (isKeyword(root->token.value)) std::cout << "\n> #<procedure " << root->token.value << ">\n";
                else { // <ATOM> is not keyword
                    if (root->token.type == TokenType::SYMBOL) { // <ATOM> is SYMBOL
                        if (! isDefined(root->token.value)) throw SemanticException::UnboundSymbol(root->token.value);
                        else {
                            // TODO: define SYMBOL
                        }
                    }
                    else std::cout << "\n> " << prettyWriteSExp(root); // just print it
                }
            }
        }
    
};
*/

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
        S_Exp_Executor executor;

        std::shared_ptr<AST> makeList(const std::vector<std::shared_ptr<AST>> &tree_root, // the part before DOT (car)
            const std::shared_ptr<AST> &cdr = std::make_shared<AST>(Token{TokenType::NIL, "nil"})) { // the part after DOT (cdr)
            std::shared_ptr<AST> res = cdr;
            for (int i = static_cast<int>(tree_root.size()) - 1; i >= 0; --i) res = std::make_shared<AST>(tree_root[i], res);
            return res;
        }

        void checkExit(const std::shared_ptr<AST> &tree_root) {
            if (! tree_root || tree_root->isAtom) return;
            if (tree_root->left && tree_root->left->isAtom && tree_root->left->token.type == TokenType::SYMBOL && tree_root->left->token.value == "exit"
                && (! tree_root->right || (tree_root->right->isAtom && tree_root->right->token.type == TokenType::NIL))) throw ExitException::CorrectExit();
        }
    
    public:
        void resetInfos() { // when a <S-exp> ended or error encountered
            lists_info = std::stack<std::pair<LIST_MODE, std::vector<std::shared_ptr<AST>>>>();
            dot_info = std::stack<std::pair<bool, int>>();
        }

        void endSExp(std::shared_ptr<AST> cur_node, bool isAtom) {
            // NOTED: always check if lists_mode's top is quote when a <S-exp> ended (check the prev if quote)
            while (! lists_info.empty() && lists_info.top().first == LIST_MODE::QUOTE) {
                // get quote
                auto quote_list = std::move(lists_info.top().second);
                lists_info.pop();
                // make quote
                cur_node = makeList({std::make_shared<AST>(Token{TokenType::QUOTE, ""}), cur_node});
            }

            // current <S-exp> ended
            if (! lists_info.empty()) { // current <S-exp> is not the most outer <S-exp>
                lists_info.top().second.emplace_back(cur_node);
                if (lists_info.top().first == LIST_MODE::WITH_DOT) dot_info.top().second++;
            }
            else { // current <S-exp> is the most outer <S-exp>
                if (! isAtom) checkExit(cur_node); // check if car == "exit" && cdr == "nil"
                executor.execute(cur_node);
                if (! gIsFirstSExpInputed) gIsFirstSExpInputed = true; // record the first legal <S-exp> is activated
                resetInfos();
            }
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
                
                endSExp(cur_node, false);
            }
            else {
                auto cur_node = std::make_shared<AST>(token); // <ATOM>
                endSExp(cur_node, true);
            }

            return lists_info.empty(); // if the whole <S-exp> ended
        }
};    

/* S-Expression Lexer */
class S_Exp_Lexer {
    private:
        char ch;
        int lineNum = 1, columnNum = 0;
        bool isSExpEnded = false;
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
            catch (SyntaxException &e) {
                parser.resetInfos();
                if (eat) eatALine(); // eat: if need to eat a line when error encountered
                throw;
            }
            catch (...) { // if error encountered, need to reset
                parser.resetInfos();
                throw;
            }
        }

    public:
        S_Exp_Lexer() {
            ch = '\0';
        }

        void readAndTokenize() { // all inputs are dealed here
            Token token;
            std::stack<char> parenStack;
            lineNum = 1;
            columnNum = 0;
            ch = '\0';
            bool start = false;
            isSExpEnded = false;

            // because peek() will need a input while Interactive I/O at the beginning
            // so in the following (i.e. not the first input) can use peek()
            if (! gIsFirstSExpInputed || std::cin.peek() != EOF) gPrinter.printPrompt();

            while (std::cin.get(ch)) {
                start = true;

                if (ch == ';') {
                    if (token.value == "") {
                        eatALine();
                        if (isSExpEnded) {
                            lineNum = 1;
                            isSExpEnded = false;
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
                            isSExpEnded = saveAToken(token, lineNum, columnNum);
                            eatALine();
                            if (isSExpEnded) {
                                lineNum = 1;
                                isSExpEnded = false;
                            }
                            else lineNum++;
                            columnNum = 0;
                        }
                    }
                }
                else if (isWhiteSpace(ch)) {
                    if (ch == '\n') {
                        if (token.value == "") {
                            if (isSExpEnded) {
                                lineNum = 1;
                                isSExpEnded = false;
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
                                isSExpEnded = saveAToken(token, lineNum, columnNum, false);
                                
                                if (isSExpEnded) {
                                    lineNum = 1;
                                    isSExpEnded = false;
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
                                isSExpEnded = saveAToken(token, lineNum, columnNum);
                                
                                if (isSExpEnded) {
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
                        isSExpEnded = saveAToken(token, lineNum, columnNum); // must be false
                    }
                    else {
                        if (token.value[0] == '\"') { // in STRING
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save previous
                            isSExpEnded = saveAToken(token, lineNum, columnNum);
                            
                            // save current
                            parenStack.push(ch);
                            token.value += ch; // "("
                            columnNum++; // ex. "123A((.\n" -> ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 3 is >>.<<
                            isSExpEnded = saveAToken(token, lineNum, columnNum); // must be false
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
                            isSExpEnded = saveAToken(token, lineNum, columnNum);
                            if (isSExpEnded) {
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
                            isSExpEnded = saveAToken(token, lineNum, columnNum);
                            if (isSExpEnded) {
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
                                isSExpEnded = saveAToken(token, lineNum, columnNum);
                                if (isSExpEnded) {
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
                        isSExpEnded = saveAToken(token, lineNum, columnNum); // must be false
                    }
                    else {
                        if (token.value[0] == '\"') { // in STRING
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            isSExpEnded = saveAToken(token, lineNum, columnNum);
                            
                            token.value += ch; // "\'"
                            if (isSExpEnded) columnNum = 1;
                            else columnNum++;
                            isSExpEnded = saveAToken(token, lineNum, columnNum);
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
                            isSExpEnded = saveAToken(token, lineNum, columnNum);
                            
                            if (isSExpEnded) columnNum = 0;
                        }
                        else { // token + STRING, with no whitespace ex. > asf"
                            // save previous
                            isSExpEnded = saveAToken(token, lineNum, columnNum);
                            
                            // the start of a STRING
                            token.value += ch;
                            if (isSExpEnded) columnNum = 1; // no whitespace so set to 1, ex. > asf" -> ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 2
                            else columnNum++; // ex. > (asf" -> ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 6
                        }
                    }
                }
                else {
                    token.value += ch;
                    columnNum++;
                    isSExpEnded = false;
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
            gPrinter.printError(e);
            break;
        }
        catch (SyntaxException &e) {
            gPrinter.printError(e);
        }
        catch (SemanticException &e) {
            gPrinter.printError(e);
        }
        catch (...) {
            std::cout << "Unknown error" << std::endl;
            break;
        }
    }
    return 0;
}