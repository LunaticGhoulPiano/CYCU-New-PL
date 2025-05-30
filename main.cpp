#include <iostream>
#include <iomanip>
#include <sstream>
#include <exception>
#include <vector>
#include <stack>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <regex>
#include <cctype>

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
    ARGUMENT_NUMBER_MODE arg_mode;
    std::vector<int> arg_nums;
    KeywordType functionType, returnType;
};

/* Keywords */
static std::unordered_map<std::string, KeywordInfo> gKeywords = {
    // {function name, {arg_mode, arg_nums, function type, return type}}
    {"cons", {ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::CONSTRUCTOR, KeywordType::PAIR}},
    {"list", {ARGUMENT_NUMBER_MODE::AT_LEAST, {0}, KeywordType::CONSTRUCTOR, KeywordType::LIST}},
    {"quote", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::BYPASS_EVALUATION}}, // returnType can be any of primitives
    {"define", {ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::BINDING}}, // returnType can be any of primitives
    {"let", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::BINDING}}, // returnType can be any of primitives
    //{"set!", {ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::BINDING}}, // returnType can be any of primitives
    {"car", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PART_ACCESSOR}}, // returnType can be any of primitives
    {"cdr", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PART_ACCESSOR}}, // returnType can be any of primitives
    {"atom?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"pair?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"list?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"null?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"integer?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"real?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"number?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"string?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"boolean?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"symbol?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::PRIMITIVE_PREDICATE, KeywordType::BOOLEAN}},
    {"+", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::NUMBER}},
    {"-", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::NUMBER}},
    {"*", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::NUMBER}},
    {"/", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::NUMBER}},
    {"not", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"and", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION}}, // returnType can be NUMBER or BOOLEAN
    {"or", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION}}, // returnType can be NUMBER or BOOLEAN
    {">", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {">=", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"<", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"<=", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"=", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"string-append", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::STRING}},
    {"string>?", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"string<?", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"string=?", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::OPERATION, KeywordType::BOOLEAN}},
    {"eqv?", {ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::EQIVALENCE_TESTER, KeywordType::BOOLEAN}},
    {"equal?", {ARGUMENT_NUMBER_MODE::MUST_BE, {2}, KeywordType::EQIVALENCE_TESTER, KeywordType::BOOLEAN}},
    {"begin", {ARGUMENT_NUMBER_MODE::AT_LEAST, {1}, KeywordType::SEQUENCING_AND_FUNCTIONAL_COMPOSITION}}, // returnType can be any of primitives
    {"if", {ARGUMENT_NUMBER_MODE::SPECIFIC, {2, 3}, KeywordType::CONDITIONAL}}, // returnType can be any of primitives
    {"cond", {ARGUMENT_NUMBER_MODE::AT_LEAST, {1}, KeywordType::CONDITIONAL}}, // returnType can be any of primitives
    //{"read", {ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::READ}}, // returnType can be any of primitives
    //{"write", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::DISPLAY}}, // returnType can be any of primitives
    //{"display-string", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::DISPLAY}}, // returnType can be STRING or ERROR_OBJECT
    //{"newline", {ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::DISPLAY, KeywordType::NIL}},
    {"lambda", {ARGUMENT_NUMBER_MODE::AT_LEAST, {2}, KeywordType::LAMBDA}}, // returnType can be any of primitives
    {"verbose", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::VERBOSE, KeywordType::BOOLEAN}},
    {"verbose?", {ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::VERBOSE, KeywordType::BOOLEAN}},
    //{"eval", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::EVALUATION}}, // returnType can be any of primitives
    //{"symbol->string", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::CONVERT_TO_STRING, KeywordType::STRING}},
    //{"number->string", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::CONVERT_TO_STRING, KeywordType::STRING}},
    //{"create-error-object", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::ERROR_OBJECT_OPERATION, KeywordType::ERROR_OBJECT}},
    //{"error-object?", {ARGUMENT_NUMBER_MODE::MUST_BE, {1}, KeywordType::ERROR_OBJECT_OPERATION, KeywordType::BOOLEAN}},
    {"clean-environment", {ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::CLEAN_ENVIRONMENT}},
    {"exit", {ARGUMENT_NUMBER_MODE::MUST_BE, {0}, KeywordType::EXIT}}
};

/* Token structure */
struct Token {
    TokenType type = TokenType::NIL;
    std::string value = "";
    Token(): type(TokenType::NIL), value("") {}
    Token(TokenType t, const std::string& v): type(t), value(v) {}
};

/* Binding types */
enum class BindingType {
    MID,
    ATOM_BUT_NOT_SYMBOL,
    PRIMITIVE_FUNCTION,
    USER_FUNCTION
};

struct AST; // forward declaration

/* Binding structure */
struct Binding {
    bool isRoot = false;
    bool isFirstNode = false;
    bool isReturnOfQuote = false;
    KeywordType dataType = KeywordType::NIL;
    BindingType bindingType;
    std::string value;
    Binding(BindingType t = BindingType::ATOM_BUT_NOT_SYMBOL, std::string v = ""): bindingType(t), value(v) {}
};

/* AST structure */
struct AST {
    bool isAtom = false;
    Token token;
    Binding binding = Binding();
    std::shared_ptr<AST> left = nullptr, right = nullptr;
    AST() {} // only used for inintializing result node
    AST(Token t) : isAtom(true), token(std::move(t)) {}
    AST(std::shared_ptr<AST> l, std::shared_ptr<AST> r) : isAtom(false), left(std::move(l)), right(std::move(r)) {}
    bool isEndNode() { return left == nullptr && right == nullptr; }
};

/* Debugger */
class Debugger {
    public:
        std::string getTokenType(Token token) {
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

        void printTokenType(Token token) {
            std::cout << getTokenType(token) << std::endl;
        }

        std::string getBindingType(BindingType type) {
            if (type == BindingType::MID) return "MID";
            else if (type == BindingType::ATOM_BUT_NOT_SYMBOL) return "ATOM_BUT_NOT_SYMBOL";
            else if (type == BindingType::PRIMITIVE_FUNCTION) return "PRIMITIVE_FUNCTION";
            else if (type == BindingType::USER_FUNCTION) return "USER_FUNCTION";
            else return "ERROR: didn't judged!";
        }

        std::string getKeywordType(KeywordType keywordType) {
            if (keywordType == KeywordType::INTEGER) return "INTEGER";
            else if (keywordType == KeywordType::FLOAT) return "FLOAT";
            else if (keywordType == KeywordType::STRING) return "STRING";
            else if (keywordType == KeywordType::ERROR_OBJECT) return "ERROR_OBJECT";
            else if (keywordType == KeywordType::BOOLEAN) return "BOOLEAN";
            else if (keywordType == KeywordType::SYMBOL) return "SYMBOL";
            else if (keywordType == KeywordType::REAL) return "REAL";
            else if (keywordType == KeywordType::NUMBER) return "NUMBER";
            else if (keywordType == KeywordType::ATOM) return "ATOM";
            else if (keywordType == KeywordType::NIL) return "NIL";
            else if (keywordType == KeywordType::LIST) return "LIST";
            else if (keywordType == KeywordType::PAIR) return "PAIR";
            else if (keywordType == KeywordType::CONSTRUCTOR) return "CONSTRUCTOR";
            else if (keywordType == KeywordType::BYPASS_EVALUATION) return "BYPASS_EVALUATION";
            else if (keywordType == KeywordType::BINDING) return "BINDING";
            else if (keywordType == KeywordType::PART_ACCESSOR) return "PART_ACCESSOR";
            else if (keywordType == KeywordType::PRIMITIVE_PREDICATE) return "PRIMITIVE_PREDICATE";
            else if (keywordType == KeywordType::OPERATION) return "OPERATION";
            else if (keywordType == KeywordType::EQIVALENCE_TESTER) return "EQIVALENCE_TESTER";
            else if (keywordType == KeywordType::SEQUENCING_AND_FUNCTIONAL_COMPOSITION) return "SEQUENCING_AND_FUNCTIONAL_COMPOSITION";
            else if (keywordType == KeywordType::CONDITIONAL) return "CONDITIONAL";
            else if (keywordType == KeywordType::READ) return "READ";
            else if (keywordType == KeywordType::DISPLAY) return "DISPLAY";
            else if (keywordType == KeywordType::LAMBDA) return "LAMBDA";
            else if (keywordType == KeywordType::VERBOSE) return "VERBOSE";
            else if (keywordType == KeywordType::EVALUATION) return "EVALUATION";
            else if (keywordType == KeywordType::CONVERT_TO_STRING) return "CONVERT_TO_STRING";
            else if (keywordType == KeywordType::ERROR_OBJECT_OPERATION) return "ERROR_OBJECT_OPERATION";
            else if (keywordType == KeywordType::CLEAN_ENVIRONMENT) return "CLEAN_ENVIRONMENT";
            else if (keywordType == KeywordType::EXIT) return "EXIT";
            else return "";
        }

        void debugPrintAST(const std::shared_ptr<AST> node, int depth = 0, const std::string &prefix = "AST_root") {
            if (! node) return;

            std::string indent(depth * 2, ' ');
            std::cout << indent << prefix;

            if (node->isAtom) {
                std::cout << " (isAtom = true, atom = \"" << node->token.value << "\", token type = " << getTokenType(node->token);
                // if is keyword and is defined, print the binding content
                if (gKeywords.find(node->token.value) != gKeywords.end())
                    std::cout << ", keyword type = " << getKeywordType(gKeywords[node->token.value].functionType);
                std::cout << ")\n";
            }
            else {
                std::cout << " (isAtom = false)\n";
                debugPrintAST(node->left, depth + 1, "|--- left  -> ");
                debugPrintAST(node->right, depth + 1, "|--- right -> ");
            }
        }
};
Debugger gDebugger;

/* Error Exceptions */
class OurSchemeException: public std::exception { // base class
    protected:
        std::string message = "";

    public:
        explicit OurSchemeException(const std::string &msg): message(msg) {}

        const char *what() const noexcept override {
            return message.c_str();
        }
};

class ExitException: public OurSchemeException {
        std::string message = "";

    public:
        explicit ExitException(const std::string &msg): OurSchemeException(msg) {}

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

class SyntaxException: public OurSchemeException {
    protected:
        std::string message = "";

    public:
        explicit SyntaxException(const std::string &msg): OurSchemeException(msg) {}

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

class SemanticException: public OurSchemeException {
    protected:
        std::string message = "";

    public:
        explicit SemanticException(const std::string &msg): OurSchemeException(msg) {}

        // level error
        static SemanticException LevelError(std::string func_name) { // implemented in project 2
            std::string upper;
            for (char c: func_name) upper += toupper(c);
            return SemanticException("ERROR (level of " + upper + ")\n"); // clean-environment, define, exit
        }

        // format error
        static SemanticException FormatError(std::string func_name, std::string s_exp) { // implemented in project 2
            std::string upper;
            for (char c: func_name) upper += (('a' <= c && c <= 'z') ? toupper(c): c); // convert to uppercase
            return SemanticException("ERROR (" + upper + " format) : " + s_exp); // proj 2: cond, define, proj 3: let, lambda, proj 4: set!
        }

        // symbol error
        static SemanticException UnboundSymbol(std::string symbol) { // implemented in project 2
            return SemanticException("ERROR (unbound symbol) : " + symbol + "\n");
        }

        // argument error
        static SemanticException IncorrectNumOfArgs(std::string arg) { // implemented in project 2
            return SemanticException("ERROR (incorrect number of arguments) : " + arg + "\n");
        }
        
        static SemanticException IncorrectArgType(std::string op, std::string s_exp) { // implemented in project 2
            return SemanticException("ERROR (" + op +" with incorrect argument type) : " + s_exp);
        }

        static SemanticException NonList(std::string s_exp) { // implemented in project 2
            return SemanticException("ERROR (non-list) : " + s_exp);
        }

        // not function name error
        static SemanticException NonFunction(std::string s_exp) { // implemented in project 2
            return SemanticException("ERROR (attempt to apply non-function) : " + s_exp);
        }
};

class RuntimeException: public OurSchemeException {
    protected:
        std::string message = "";

    public:
        explicit RuntimeException(const std::string &msg): OurSchemeException(msg) {}

        // function error
        static RuntimeException DivisionByZero() { // implemented in project 2
            return RuntimeException("ERROR (division by zero) : /\n");
        }

        static SemanticException NoReturnValue(std::string s_exp) { // implemented in project 2
            return SemanticException("ERROR (no return value) : " + s_exp);
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

        // use cur->token.value for project 1, cur->binding.value for projecet 2, 3, 4
        std::string getprettifiedSExp(bool useToken, const std::shared_ptr<AST> &cur, std::string s_exp = "", int depth = 0, bool isRoot = true, bool isFirstTokenOfLine = true) { // recursively append into the string
            if (cur != nullptr) {
                if (cur->isAtom) { // <S-exp> ::= <ATOM>
                    if (cur->token.type == TokenType::QUOTE) s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + "quote\n");
                    else s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + (useToken ? cur->token.value : cur->binding.value) + "\n");
                }
                else { // <S-exp> ::= LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | <S-exp> ::= QUOTE <S-exp>
                    // LP: new list started
                    if (isRoot) {
                        s_exp += ((isFirstTokenOfLine ? std::string(depth * 2, ' ') : " ") + "(");
                        depth++;
                        isFirstTokenOfLine = false;
                    }
                    // car
                    s_exp = getprettifiedSExp(useToken, cur->left, s_exp, depth, true, isFirstTokenOfLine);
                    // cdr
                    if (cur->right && cur->right->isAtom && cur->right->token.type != TokenType::NIL) {
                        s_exp += (std::string(depth * 2, ' ') + ".\n");
                        s_exp = getprettifiedSExp(useToken, cur->right, s_exp, depth, true, true);
                    }
                    else if (cur->right && ! cur->right->isAtom) s_exp = getprettifiedSExp(useToken, cur->right, s_exp, depth, false, true);
                    else if (! cur->right || cur->right->token.type == TokenType::NIL) ; // nothing
                    else {
                        s_exp += (std::string(depth * 2, ' ') + ".\n");
                        s_exp = getprettifiedSExp(useToken, cur->right, s_exp, depth, true, true);
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
        bool verbose = true, isCleanOrDefine = false;
        std::unordered_map<std::string, std::shared_ptr<AST>> globalVars, localVars;

        /* judgers */
        bool isKeyword(std::string sym) { return (gKeywords.find(sym) != gKeywords.end() && sym != "#t" && sym != "nil"); }
        bool isDefined(std::string sym) { return (globalVars.find(sym) != globalVars.end()); }
        bool isPrimFunc(std::string sym, bool checkBinding = true) { // must check if sym is keyword, optionally check if sym's binding is primitive function
            return isKeyword(sym) ? true : // check keywords
                ((checkBinding && isDefined(sym) && globalVars[sym]->binding.bindingType == BindingType::PRIMITIVE_FUNCTION) ? true : false); // check bindings
        }
        bool isUserFunc(std::string sym) { return (isDefined(sym) && (globalVars[sym]->binding.bindingType == BindingType::USER_FUNCTION)); }
        bool isFunction(std::string sym, bool checkBinding = true) { return (isPrimFunc(sym, checkBinding) || isUserFunc(sym)) ; }

        /* error checkers */
        void checkPureList(std::shared_ptr<AST> cur_node, std::shared_ptr<AST> cur_func, bool useToken = false) {
            if (cur_node == nullptr || cur_node->isEndNode()) return;
            if (cur_node->right != nullptr && cur_node->right->token.type != TokenType::NIL)
                throw SemanticException::NonList(gPrinter.getprettifiedSExp(useToken, cur_func));
            checkPureList(cur_node->right, cur_func, useToken);
        }

        void checkLevelOfSpecifics(std::string func_name, int level) {
            std::vector<std::string> specifics = {"clean-environment", "define", "exit"};
            if ((std::find(specifics.begin(), specifics.end(), func_name) != specifics.end()) && level != 0)
                throw SemanticException::LevelError(func_name);
        }
        
        void checkArgumentsNumber(std::shared_ptr<AST> cur) {
            if (cur->left->binding.bindingType == BindingType::PRIMITIVE_FUNCTION) { // primitive function
                // count
                std::shared_ptr<AST> temp = cur->right;
                int count = 0;
                while (! temp->isEndNode()) {
                    count++;
                    temp = temp->right;
                }

                // check
                KeywordInfo info = gKeywords[cur->left->token.value];
                switch (info.arg_mode) {
                    case ARGUMENT_NUMBER_MODE::AT_LEAST: {
                        if (count < info.arg_nums[0]) throw SemanticException::IncorrectNumOfArgs(cur->left->token.value);
                        break;
                    }
                    case ARGUMENT_NUMBER_MODE::MUST_BE: {
                        if (count != info.arg_nums[0]) throw SemanticException::IncorrectNumOfArgs(cur->left->token.value);
                        break;
                    }
                    default: { // specific
                        if (std::find(info.arg_nums.begin(), info.arg_nums.end(), count) == info.arg_nums.end())
                            throw SemanticException::IncorrectNumOfArgs(cur->left->token.value);
                        break;
                    }
                }
            }
            else if (cur->left->binding.bindingType == BindingType::USER_FUNCTION) { // user-defined function
                // TODO (project 3)
            }
        }

        void checkArgumentType(std::string func_name, std::shared_ptr<AST> arg) {
            if (func_name == "car" || func_name == "cdr") {
                if (arg->isEndNode()) throw SemanticException::IncorrectArgType(func_name, gPrinter.getprettifiedSExp(false, arg));
            }
            else if (func_name == "+" || func_name == "-" || func_name == "*" || func_name == "/"
                || func_name == ">" || func_name == ">=" || func_name == "<" || func_name == "<=" || func_name == "=") {
                    try { std::stof(arg->binding.value); }
                    catch (...) { throw SemanticException::IncorrectArgType(func_name, gPrinter.getprettifiedSExp(false, arg)); }
                }
            else if (func_name == "string-append" || func_name == "string>?" || func_name == "string<?" || func_name == "string=?") {
                if (arg->binding.value[0] != '\"' || arg->binding.value[arg->binding.value.length() - 1] != '\"')
                    throw SemanticException::IncorrectArgType(func_name, gPrinter.getprettifiedSExp(false, arg));
            }
        }

        /* data & binding setters */
        KeywordType getNodeMinorBindingDataTypeByTokenValue(std::shared_ptr<AST> cur) {
            // judge by token value
            if (std::regex_match(cur->token.value, std::regex("^-?\\d+$"))) return KeywordType::INTEGER;
            else if (std::regex_match(cur->token.value, std::regex("^-?\\d+\\.\\d+$"))) return KeywordType::FLOAT;
            else if (cur->token.type == TokenType::T
                || (cur->token.value == "nil" && cur->binding.bindingType == BindingType::ATOM_BUT_NOT_SYMBOL)) return KeywordType::BOOLEAN;
            else if (cur->token.type == TokenType::STRING) return KeywordType::STRING;
            else return KeywordType::SYMBOL;
        }
        
        void getBinding(std::shared_ptr<AST> &cur, int level, std::string func_name = "", int bypassLevel = -1) {
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;

            // bind
            if (bypassLevel == -1 && isDefined(cur->token.value)) cur = globalVars[cur->token.value]; // get the binding: symbol or user-defined function
            else if (isPrimFunc(cur->token.value)) {
                if ((bypassLevel == -1 || (cur->token.value == "quote" && level == bypassLevel + 1))
                    && ! cur->binding.isReturnOfQuote) cur->binding = Binding(BindingType::PRIMITIVE_FUNCTION, ("#<procedure " + cur->token.value + ">"));
                else cur->binding = Binding(BindingType::PRIMITIVE_FUNCTION, cur->token.value);
            }
            else if (cur->token.type != TokenType::SYMBOL) cur->binding = Binding(BindingType::ATOM_BUT_NOT_SYMBOL, cur->token.value);
            else if (bypassLevel == -1) throw SemanticException::UnboundSymbol(cur->token.value);
            else cur->binding = Binding(BindingType::ATOM_BUT_NOT_SYMBOL, cur->token.value);
            
            // set back to prevent wrong cover
            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
            // set the minor data type of the token in binding
            cur->binding.dataType = getNodeMinorBindingDataTypeByTokenValue(cur);
        }
        
        /* make useful atom nodes */
        std::shared_ptr<AST> makeBooleanNode(bool b) {
            std::shared_ptr<AST> result = std::make_shared<AST>();
            result->isAtom = true; // crucial
            result->token.value = b ? "#t" : "nil";
            result->token.type = b ? TokenType::T : TokenType::NIL;
            result->binding.value = result->token.value;
            result->binding.bindingType = BindingType::ATOM_BUT_NOT_SYMBOL;
            result->binding.dataType = KeywordType::BOOLEAN;
            result->binding.isRoot = true;
            return result;
        }

        std::shared_ptr<AST> makeBeginNode() {
            std::shared_ptr<AST> result = std::make_shared<AST>();
            result->token.value = "begin";
            result->token.type = TokenType::SYMBOL;
            result->left = result->right = nullptr;
            result->isAtom = true;
            result->binding.value = "begin";
            result->binding.bindingType = BindingType::PRIMITIVE_FUNCTION;
            result->binding.dataType = KeywordType::SYMBOL;
            return result;
        }
        
        /* Primitive functions */
        std::unordered_map<KeywordType, std::function<void(std::shared_ptr<AST> &)>> prim_func_map = {
            {KeywordType::CONSTRUCTOR, [this](std::shared_ptr<AST> &cur) { construct(cur); } },
            {KeywordType::BYPASS_EVALUATION, [this](std::shared_ptr<AST> &cur) { bypass(cur); } },
            {KeywordType::BINDING, [this](std::shared_ptr<AST> &cur) { bind(cur); } },
            {KeywordType::PART_ACCESSOR, [this](std::shared_ptr<AST> &cur) { getPart(cur); } },
            {KeywordType::PRIMITIVE_PREDICATE, [this](std::shared_ptr<AST> &cur) { judgePrimitivePredicate(cur); } },
            {KeywordType::OPERATION, [this](std::shared_ptr<AST> &cur) { operate(cur); } },
            {KeywordType::EQIVALENCE_TESTER, [this](std::shared_ptr<AST> &cur) { judgeEqivalence(cur); } },
            {KeywordType::SEQUENCING_AND_FUNCTIONAL_COMPOSITION, [this](std::shared_ptr<AST> &cur) { sequence(cur); } },
            // KeywordType::READ
            // KeywordType::DISPLAY
            // KeywordType::LAMBDA
            {KeywordType::VERBOSE, [this](std::shared_ptr<AST> &cur) { verboseMode(cur); } },
            // KeywordType::EVALUATION
            // KeywordType::CONVERT_TO_STRING
            // KeywordType::ERROR_OBJECT_OPERATION
            {KeywordType::CLEAN_ENVIRONMENT, [this](std::shared_ptr<AST> &cur) { cleanEnvironment(cur); } },
            {KeywordType::EXIT, [this](std::shared_ptr<AST> &cur) { exit(cur); } }
        };

        // counstruct (project 2)
        void construct(std::shared_ptr<AST> &cur) { // project 2: list, cons
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            // init middle nil node
            std::shared_ptr<AST> temp = std::make_shared<AST>();
            temp->token.type = TokenType::NIL;
            temp->token.value = "";
            temp->binding.bindingType = BindingType::MID;
            temp->binding.dataType = KeywordType::NIL;
            temp->binding.isRoot = tempRoot;
            temp->binding.isFirstNode = tempFirstNode;
            temp->binding.isReturnOfQuote = tempQHead;
            
            // construct
            if (cur->left->token.value == "cons") { // return: pair
                temp->left = cur->right->left;
                temp->right = cur->right->right->left;
                cur = temp;
            }
            else { // return: list
                cur = cur->right;
                cur->binding = temp->binding;
                if (cur->isEndNode()) getBinding(cur, 0); // bind if the result is single atom
            }
        }

        // bypass (project 2)
        void bypass(std::shared_ptr<AST> &cur) { // project 2: quote
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            cur = cur->right->left; // pull up
            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
        }

        // bind: define (project 2: symbol, project 3: function), let (project 3), set! (project 4)
        void bind(std::shared_ptr<AST> &cur) {
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            if (cur->left->token.value == "define") {
                std::string sym = "";
                if (cur->right->left->isEndNode()) { // define a symbol (project 2)
                    sym = cur->right->left->token.value;
                    globalVars[cur->right->left->token.value] = cur->right->right->left;
                }
                else { // define a function (project 3)
                    // TODO
                }

                // set defined message node
                cur = std::make_shared<AST>();
                cur->token.value = sym;
                cur->token.type = TokenType::SYMBOL;
                cur->isAtom = true;
                cur->binding.value = sym + " defined";
                cur->binding.bindingType = BindingType::ATOM_BUT_NOT_SYMBOL; // the minimal value of non-function symbol must be bound to an ATOM_BUT_NOT_SYMBOL
                cur->binding.dataType = KeywordType::SYMBOL;

                isCleanOrDefine = true;
            }
            else if (cur->left->token.value == "let") {
                // TODO
            }
            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
        }
        
        // getPart (project 2)
        void getPart(std::shared_ptr<AST> &cur) {
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            if (cur->left->token.value == "car") cur = cur->right->left->left; // car
            else cur = cur->right->left->right; // cdr
            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
        }

        // judgePrimitivePredicate (project 2)
        void judgePrimitivePredicate(std::shared_ptr<AST> &cur) {
            bool result = false;
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            if (cur->left->token.value == "atom?"
                && cur->right->left->isEndNode()) result = true;
            else if (cur->left->token.value == "pair?"
                && ! cur->right->left->isEndNode()) result = true;
            else if (cur->left->token.value == "list?") {
                try {
                    checkPureList(cur->right->left, cur);
                    result = true;
                    if (cur->right->left->isEndNode() && cur->right->left->token.type != TokenType::NIL) result = false;
                }
                catch (...) {} // false
            }
            else if (cur->left->token.value == "null?"
                && cur->right->left->token.type == TokenType::NIL && cur->right->left->isEndNode()) result = true;
            else if (cur->left->token.value == "integer?"
                && cur->right->left->token.type == TokenType::INT) result = true;
            else if ((cur->left->token.value == "real?" || cur->left->token.value == "number?")
                && (cur->right->left->token.type == TokenType::INT || cur->right->left->token.type == TokenType::FLOAT)) result = true;
            else if (cur->left->token.value == "string?"
                && cur->right->left->token.type == TokenType::STRING) result = true;
            else if (cur->left->token.value == "boolean?"
                && (cur->right->left->token.type == TokenType::T
                    || (cur->right->left->token.type == TokenType::NIL && cur->right->left->isEndNode()))) result = true;
            else if (cur->left->token.value == "symbol?") {
               KeywordType minorType = getNodeMinorBindingDataTypeByTokenValue(cur->right->left);
               if (cur->right->left->binding.bindingType == BindingType::ATOM_BUT_NOT_SYMBOL
                    && minorType == KeywordType::SYMBOL
                    && ! isKeyword(cur->right->left->token.value)
                    && cur->right->left->isEndNode()) result = true;
            }

            cur = makeBooleanNode(result);
            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
        }

        // operate (project 2)
        std::unordered_map<std::string, std::function<double(std::string a, std::string b)>> arithmeticAndLogicalAndCompareOperateMap = {
            // result in double
            {"+", [](std::string a, std::string b) { return std::stod(a) + std::stod(b); }},
            {"-", [](std::string a, std::string b) { return std::stod(a) - std::stod(b); }},
            {"*", [](std::string a, std::string b) { return std::stod(a) * std::stod(b); }},
            {"/", [](std::string a, std::string b) { return std::stod(a) / std::stod(b); }},
            // 1 or 0, must be int
            {">", [](std::string a, std::string b) { return int(std::stod(a) > std::stod(b)); }},
            {">=", [](std::string a, std::string b) { return int(std::stod(a) >= std::stod(b)); }},
            {"<", [](std::string a, std::string b) { return int(std::stod(a) < std::stod(b)); }},
            {"<=", [](std::string a, std::string b) { return int(std::stod(a) <= std::stod(b)); }},
            {"=", [](std::string a, std::string b) { return int(std::stod(a) == std::stod(b)); }},
            // string compare
            {"string>?", [](std::string a, std::string b) { return int(a.compare(b) > 0); }},
            {"string<?", [](std::string a, std::string b) { return int(a.compare(b) < 0); }},
            {"string=?", [](std::string a, std::string b) { return int(a.compare(b) == 0); }}
        };

        std::string arithmeticOperations(std::string op, std::string a, std::string b, bool convertToFloat) {
            double r = arithmeticAndLogicalAndCompareOperateMap[op](a, b);
            if (convertToFloat) { // float, round float to "%.3f"
                std::stringstream oss;
                oss << std::fixed << std::setprecision(3) << r;
                return oss.str();
            }
            else return std::to_string(int(r)); // int
        }

        void operate(std::shared_ptr<AST> &cur) {
            // init result node
            std::string op = cur->left->token.value, str_result = "";
            std::shared_ptr<AST> result = cur->right->left, mid = cur->right->right; // use for at least 2 operands
            std::shared_ptr<AST> iter = cur->right; // use for at least 1 operand
            bool compareStatus = false; // for >, >=, <, <=, =
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            
            // operate
            do {
                if (op == "+" || op == "-" || op == "*" || op == "/") { // result must be real(number), int or float
                    bool convertToFloat = false; // if one of the operands is float, result must be float
                    if (result->binding.value.find('.') != std::string::npos
                        || mid->left->binding.value.find('.') != std::string::npos) convertToFloat = true;
                    if (op == "/" && std::stof(mid->left->binding.value) == 0.000000) throw RuntimeException::DivisionByZero(); // division by zero
                    std::string r = arithmeticOperations(op, result->binding.value, mid->left->binding.value, convertToFloat);
                    // set result
                    result = std::make_shared<AST>();
                    result->isAtom = true; // crucial
                    result->token.value = r;
                    result->token.type = convertToFloat ? TokenType::FLOAT : TokenType::INT;
                    result->binding.value = result->token.value;
                    result->binding.bindingType = BindingType::ATOM_BUT_NOT_SYMBOL;
                    result->binding.dataType = convertToFloat ? KeywordType::FLOAT : KeywordType::INTEGER;
                    result->binding.isRoot = true;
                    
                    // check the remainings
                    if (mid->right->isEndNode()) break;
                    else mid = mid->right;
                }
                else if (op == "not") {
                    result = makeBooleanNode(result->binding.value == "nil");
                    break; // must only one argument
                }
                else if (op == ">" || op == ">=" || op == "<" || op == "<=" || op == "=") {
                    compareStatus = bool(arithmeticAndLogicalAndCompareOperateMap[op](iter->left->binding.value, iter->right->left->binding.value));
                    if (iter->right->right->isEndNode() || ! compareStatus) { // return when the last or the first false, cuz the continuous "and" operation
                        result = makeBooleanNode(compareStatus);
                        break;
                    }
                    else iter = iter->right;
                }
                else { // string operations
                    if (op == "string-append") {
                        str_result += iter->left->binding.value.substr(1, iter->left->binding.value.size() - 2);
                        if (iter->right->isEndNode()) {
                            // set result
                            result = std::make_shared<AST>();
                            result->isAtom = true; // crucial
                            result->token.value = "\"" + str_result + "\"";
                            result->token.type = TokenType::STRING;
                            result->binding.value = result->token.value;
                            result->binding.bindingType = BindingType::ATOM_BUT_NOT_SYMBOL;
                            result->binding.dataType = KeywordType::STRING;
                            result->binding.isRoot = true;
                            break;
                        }
                        else iter = iter->right;
                    }
                    else { // same as number comparisons
                        compareStatus = arithmeticAndLogicalAndCompareOperateMap[op](iter->left->binding.value, iter->right->left->binding.value);
                        if (iter->right->right->isEndNode() || ! compareStatus) { // return when the last or the first false, cuz the continuous "and" operation
                            result = makeBooleanNode(compareStatus);
                            break;
                        }
                        else iter = iter->right;
                    }
                }
            } while (true);

            // set result
            cur = result;
            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
        }

        // judgeEqivalence (project 2)
        bool isStructureTheSame(std::shared_ptr<AST> obj1, std::shared_ptr<AST> obj2) {
            // base cases
            if (obj1 == nullptr || obj2 == nullptr) return obj1 == obj2;

            // judge if both are atoms or the node position
            if (obj1->isAtom != obj2->isAtom || obj1->isEndNode() != obj2->isEndNode()) return false;

            // if both are atoms (then only need to use one node to judge if isAtom)
            if (obj1->isAtom) {
                // if value is real number (don't compare type cuz may be int or float), first convert to float, then convert back to string to compare
                // else compare the types and values
                try { return (std::stof(obj1->binding.value) == std::stof(obj2->binding.value)); }
                catch (...) { return ((obj1->token.type == obj2->token.type) && (obj1->binding.value.compare(obj2->binding.value) == 0)); }
            }

            // recursively check left and right
            return isStructureTheSame(obj1->left, obj2->left) && isStructureTheSame(obj1->right, obj2->right);
        }

        void judgeEqivalence(std::shared_ptr<AST> &cur) {
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            std::string func_name = cur->left->token.value;
            // if addresses are the same, must be the same object, must be true
            if (cur->right->left.get() == cur->right->right->left.get()) cur = makeBooleanNode(true);
            // other cases should iterate all nodes to judge equality
            else { // must be the different objects (addresses), judge the structure
                // iterate all nodes
                bool result = isStructureTheSame(cur->right->left, cur->right->right->left);
                if (result && func_name == "eqv?") {
                    if (cur->right->left->isAtom) {
                        if (cur->right->left->binding.dataType == KeywordType::STRING) cur = makeBooleanNode(false);
                        else cur = makeBooleanNode(true);
                    }
                    else cur = makeBooleanNode(false);
                }
                else cur = makeBooleanNode(result);
            }

            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
        }
        
        // sequence (project 2)
        void sequence(std::shared_ptr<AST> &cur) {
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            std::shared_ptr<AST> temp = cur;
            while (temp->right->token.value != "nil" && ! temp->right->isEndNode()) temp = temp->right;
            cur = temp->left;
            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
        }

        // read (project 4)
        // display (project 4)
        // lambda (project 3)

        // verbose (project 3)
        void verboseMode(std::shared_ptr<AST> &cur) {
            if (cur->left->token.value == "verbose") verbose = (cur->right->left->binding.value != "nil");
            cur = makeBooleanNode(verbose);
        }

        // evaluation (project 4)
        // convertToString (project 4)
        // errorObject (project 4)

        // cleanEnvironment (project 2)
        void cleanEnvironment(std::shared_ptr<AST> &cur) {
            globalVars.clear();
            localVars.clear();
            bool tempRoot = cur->binding.isRoot, tempFirstNode = cur->binding.isFirstNode, tempQHead = cur->binding.isReturnOfQuote;
            cur = std::make_shared<AST>();
            cur->token.type = TokenType::SYMBOL;
            cur->token.value = "environment cleaned";
            cur->isAtom = true;
            cur->binding.isRoot = tempRoot;
            cur->binding.isFirstNode = tempFirstNode;
            cur->binding.isReturnOfQuote = tempQHead;
            cur->binding.dataType = KeywordType::SYMBOL;
            cur->binding.bindingType = BindingType::ATOM_BUT_NOT_SYMBOL;
            cur->binding.value = cur->token.value;
            isCleanOrDefine = true;
        }

        // exit (project 1)
        void exit(std::shared_ptr<AST> &cur) {
            throw ExitException::CorrectExit();
        }
        
        /* internal debug functions */

        void debugPrintAST(std::shared_ptr<AST> &cur, int level, std::string pos = "root") {
            if (cur == nullptr) return;
            debugPrintNode(pos, cur, level);
            debugPrintAST(cur->left, level + 1, "left");
            debugPrintAST(cur->right, level + 1, "right");
        }

        void debugPrintNode(std::string pos, std::shared_ptr<AST> cur, int level) {
            std::cout << "\t[level " << level << " " << pos << "]: " << "token = " << cur->token.value << ", token type = " << gDebugger.getTokenType(cur->token);
            std::cout  << ", binding value = " << cur->binding.value << ", binding type = " << gDebugger.getBindingType(cur->binding.bindingType);
            std::cout << ", data type = " << gDebugger.getKeywordType(cur->binding.dataType);
            std::cout << ", is root: " << cur->binding.isRoot << ", is first node: " << cur->binding.isFirstNode;
            std::cout << ", is return of quote: " << cur->binding.isReturnOfQuote << std::endl;
        }
        
        /* label the crucial nodes */
        void labelRootAndFirstNode(std::shared_ptr<AST> &cur, int level = 0) {
            /*
            ex. input: (car (cdr (list 123 (+ 89 00 999) (* 89 889 998 9) "" "test)" + - * /)))
            only car, cdr, list, these 3 nodes are considered as functions
            other nodesare not the first left node of the function (sub tree), or the value not a keyword function name
            */
            // the head of a <S-exp> (sub) tree is not list
            if (cur == nullptr || cur->isAtom) return;
            // first node is function
            if (cur->left != nullptr && cur->left->isAtom) {
                cur->binding.isRoot = true;
                cur->left->binding.isFirstNode = true;
            }
            // first node if non-end-nil, i.e. the head of a <S-exp> (sub) tree
            if(cur->left->token.type == TokenType::NIL && ! cur->left->isEndNode()) {
                cur->binding.isRoot = true;
                cur->left->binding.isFirstNode = true;
                labelRootAndFirstNode(cur->left, level + 1);
            }
            // check right remainings
            std::shared_ptr<AST> remainings = cur->right;
            while (remainings != nullptr && remainings->token.type == TokenType::NIL) {
                // if the remainings is a <S-exp> sub tree, recursively do
                if (remainings->left != nullptr && ! remainings->left->isAtom) labelRootAndFirstNode(remainings->left, level + 1);
                remainings = remainings->right;
            }
        }
        
        /* execution */
        void execute(std::shared_ptr<AST> &cur, int level, int &bypassLevel) {
            checkLevelOfSpecifics(cur->left->token.value, level); // check if level is 0 when specific functions
            if (isPrimFunc(cur->left->token.value)) { // primitive
                // init functions
                if (cur->left->token.value == "quote") cur->binding.isReturnOfQuote = true, bypassLevel = level; // set bypass flags
                evaluate(cur->left, level + 1, bypassLevel); // bind the function name
                
                // special case 1. need to choose a true condition
                if (cur->left->token.value == "if") {
                    // get the original s-exp for error message
                    std::string origErrMsg = gPrinter.getprettifiedSExp(true, cur);

                    // check the number of arguments
                    checkArgumentsNumber(cur);

                    // evaluate the condition
                    evaluate(cur->right->left, level + 2, bypassLevel);

                    // get the result execution(s)
                    if (cur->right->left->binding.value != "nil") cur = cur->right->right->left; // true
                    else {
                        if (cur->right->right->right->isEndNode()) throw RuntimeException::NoReturnValue(origErrMsg);
                        else cur = cur->right->right->right->left; // false
                    }

                    // evaluate the result execution(s)
                    evaluate(cur, level, bypassLevel);
                    return;
                }
                else if (cur->left->token.value == "cond") {
                    // get the original s-exp for error message
                    std::string origErrMsg = gPrinter.getprettifiedSExp(true, cur);
                    try {
                        std::shared_ptr<AST> temp = cur->right, toBeExecuted = nullptr; // iterator, result
                        if (temp->isEndNode()) throw std::runtime_error(""); // (cond)

                        // first check if each condition's format
                        do {
                            // if a sub-condition-execution list is an atom, ex. (cond 1 2 3) -> 1, 2, 3 should be list
                            // or no execution of that condition, ex. (cond (1)), (cond ((1 2)))
                            if (temp->left->isEndNode() || temp->left->right->isEndNode()) throw std::runtime_error("");
                            // if a sub-condition-execution list is not pure list, ex. (cond ("Hi" (cons 5) . 6)) -> ("Hi" (cons 5) . 6) should not be pair
                            try { checkPureList(temp->left, temp->left); }
                            catch (...) { throw SemanticException::FormatError("cond", origErrMsg); }
                            temp = temp->right;
                        } while (temp->right != nullptr);
                        
                        // then evaluate each condition
                        temp = cur->right;
                        int tempLevel = level;
                        do {
                            // if the last condition start with else => must-do => set to true
                            if (temp->right->token.type == TokenType::NIL && temp->right->isEndNode()
                                && temp->left->left->token.value == "else" && temp->left->left->isEndNode())
                                temp->left->left = makeBooleanNode(true);

                            // then evaluate the current condition
                            evaluate(temp->left->left, tempLevel + 2, bypassLevel);
                            
                            if (temp->left->left->binding.value != "nil") { // true
                                // convert the condition node to begin node
                                temp->left->left = makeBeginNode();
                                temp->left->left->binding.isRoot = false;
                                temp->left->left->binding.isReturnOfQuote = false;
                                temp->left->left->binding.isFirstNode = true;
                                // set the begin executions
                                toBeExecuted = temp->left;
                                toBeExecuted->binding.isRoot = true;
                                toBeExecuted->binding.isReturnOfQuote = false;
                                toBeExecuted->binding.isFirstNode = false;
                                break;
                            }
                            temp = temp->right;
                            tempLevel++;
                        } while (temp->right != nullptr);

                        // no true condition found
                        if (toBeExecuted == nullptr) throw RuntimeException::NoReturnValue(origErrMsg);
                        else cur = toBeExecuted;
                    }
                    catch (OurSchemeException &e) { // use the original tokens to throw format error
                        throw; // the error of the condition tree, ex. (cond ((+) 'exec1 'return1) ())
                    }
                    catch (...) { // catch any error
                        throw SemanticException::FormatError("cond", origErrMsg);
                    }

                    // evaluate the begin execution
                    checkPureList(cur, cur);
                    checkArgumentsNumber(cur);
                    evaluate(cur->right, level + 1, bypassLevel, cur->left->token.value); // check & bind arguments
                }
                // special case 2. return the first condition (i.e. don't evaluate the remaining conditions)
                else if (cur->left->token.value == "and" || cur->left->token.value == "or") {
                    // check the number of arguments
                    checkArgumentsNumber(cur);
                    std::string op = cur->left->token.value;
                    std::shared_ptr<AST> iter = cur->right;
                    int tempLevel = level + 1; // + 1 cuz iter = cur->right
                    do {
                        // evaluate the condition
                        evaluate(iter->left, tempLevel + 1, bypassLevel);
                        if (op == "and" && iter->left->binding.value == "nil" // and return the first nil, else return the last that must != nil
                            || op == "or" && iter->left->binding.value != "nil" // or return the first != nil, else return the last that must == nil
                            || iter->right->isEndNode()) { // the last condition
                            cur = iter->left;
                            cur->binding.isRoot = true;
                            break;
                        }
                        else {
                            iter = iter->right;
                            tempLevel++;
                        }
                    } while (true);
                    return;
                }
                // special case 3. with arguments whcich should't be unbound symbol
                else if (cur->left->token.value == "define") {
                    // check number of arguments
                    try { checkArgumentsNumber(cur); }
                    catch (...) { throw SemanticException::FormatError("define", gPrinter.getprettifiedSExp(true, cur)); }

                    // evaluate the symbol name
                    if (cur->right->left->isEndNode()) { // define a symbol (project 2), ex. (define sym (begin list car cdr (real? "str")))
                        // the symbol should not be real (number) / boolean / string / primitive functions
                        bool isNum = false;
                        try { std::stof(cur->right->left->token.value), isNum = true; }
                        catch (...) {}
                        if (isNum || cur->right->left->token.value == "#t" || cur->right->left->token.value == "nil"
                            || (cur->right->left->token.value[0] == '\"' && cur->right->left->token.value[cur->right->left->token.value.size() - 1] == '\"')
                            || isPrimFunc(cur->right->left->token.value, false)) throw SemanticException::FormatError("define", gPrinter.getprettifiedSExp(true, cur));
                        
                        // evaluate the to-bind value
                        evaluate(cur->right->right->left, level + 3, bypassLevel);
                    }
                    else { // define a function (project 3), ex. (define (func x y) (list (+ x y) (- x y) (* x y) (/ x y)))
                        // TODO
                    }
                }
                else if (cur->left->token.value == "let") { // project 3
                    // TODO
                }
                else if (cur->left->token.value == "lambda") { // project 3
                    // TODO
                }
                // else if (cur->left->token.value == "set!") // project 4
                else { // normal function checking
                    checkArgumentsNumber(cur);
                    evaluate(cur->right, level + 1, bypassLevel, cur->left->token.value); // check & bind arguments
                }

                if (bypassLevel == level) bypassLevel = -1; // reset
                if (bypassLevel == -1) prim_func_map[gKeywords[cur->left->token.value].functionType](cur); // execute
            }
            else { // user-defined
                // project 3
            }
        }

        /* evaluation */
        void evaluate(std::shared_ptr<AST> &cur, int level = 0, int bypassLevel = -1, std::string func_name = "") {
            if (cur == nullptr) return;
            //std::cout << "\t[level " << level << "]: " << "cur token: " << cur->token.value << ", token type: " << gDebugger.getTokenType(cur->token) << ", bypass level = " << bypassLevel << ", func_name = " << func_name << std::endl;
            
            // current node is atom
            if (cur->token.type != TokenType::NIL
                || (cur->token.type == TokenType::NIL && cur->isEndNode())) getBinding(cur, level, func_name, bypassLevel); // , bypassLevel, bypass
            // current node is middle (nil) node
            else if (cur->token.type == TokenType::NIL) {
                // set current
                cur->binding.bindingType = BindingType::MID;
                bool hasLeftBinding = false;
                if (bypassLevel == -1) checkPureList(cur, cur, true); // check pure list

                // Step 1. if a new sub tree encountered, ex. ((begin +) "a" b) -> evaluate and execute the sub tree (begin +) -> (+ "a" b)
                if (cur->left->token.type == TokenType::NIL && ! cur->left->isEndNode()) evaluate(cur->left, level + 1, bypassLevel, func_name), hasLeftBinding = true;
                
                // Step 2-1.if cur->left is a function name and also the first left node of the current sub-tree
                if (cur->left->binding.isFirstNode // is the first atom node of a s_exp
                    && bypassLevel == -1 // bypass mode is off, i.e. should be executed
                    && ! cur->left->binding.isReturnOfQuote // ex. ('list) -> (list), but list is a return of quote, which should be non-function error
                    && isFunction(cur->left->token.value)) // then the current binding value is a function name
                    execute(cur, level, bypassLevel);
                else { // Step 2-2. cur->left is atom
                    // if still not bind the left yet (i.e. didn't go to Step 1 before), get the binding (and check unbound symbol error)
                    if (! hasLeftBinding) evaluate(cur->left, level + 1, bypassLevel, func_name);

                    // check non-function error
                    if (cur->left->binding.isFirstNode // is the first left node, which should be a function name normally
                        && bypassLevel == -1 // and the bypass mode is off, i.e. should be executed
                        && (! isFunction(cur->left->token.value) // but the first left node is not a function name
                            || cur->left->binding.isReturnOfQuote)) // or the first left node is a function name that also is a return of quote
                        throw SemanticException::NonFunction(gPrinter.getprettifiedSExp(false, cur->left));
                    
                    // check argument type error
                    if (func_name != "") checkArgumentType(func_name, cur->left); // func_name, arg

                    // bind the remaining arguments
                    evaluate(cur->right, level + 1, bypassLevel, func_name);
                }
            }
            // else neither ATOM nor NIL => nullptr, return
        }

    public:
        /* evaluate, execute, and print the result of a main <S-exp> */
        void run(std::shared_ptr<AST> &root) {
            labelRootAndFirstNode(root);
            evaluate(root);
            // don't print "environment cleaned" and "synmbol defined" when verbose mode on, just print prompt
            if (! verbose && isCleanOrDefine) gPrinter.printPrompt();
            else gPrinter.printResult(gPrinter.getprettifiedSExp(false, root));
            isCleanOrDefine = false;
        }
};

/* S-Expression Parser */
class S_Exp_Parser {
    private:
        enum class LIST_MODE {
            NO_DOT, // list
            WITH_DOT, // dotted pair
            QUOTE // quote
        };
    
        // store list modes and lists
        std::stack<std::pair<LIST_MODE, std::vector<std::shared_ptr<AST>>>> lists_info; // first: mode, second: list
        // check num of <S-exp> after DOT
        std::stack<std::pair<bool, int>> dot_info; // first: isDOTStart, second: <S-exp> after DOT
        S_Exp_Executor executor;

        /* make list or dotted pair or quote */
        std::shared_ptr<AST> makeList(const std::vector<std::shared_ptr<AST>> &tree_root, // the part before DOT (car)
            const std::shared_ptr<AST> &cdr = std::make_shared<AST>(Token{TokenType::NIL, "nil"})) { // the part after DOT (cdr)
            std::shared_ptr<AST> res = cdr;
            for (int i = static_cast<int>(tree_root.size()) - 1; i >= 0; --i) res = std::make_shared<AST>(tree_root[i], res);
            return res;
        }
    
    public:
        /* AST status checker */
        bool isCurTreeEmpty() { return lists_info.empty(); }

        /* reset AST, will be called only when a <S-exp> ended or init in readAndTokenize() (reset or error) */
        void resetInfos() {
            lists_info = std::stack<std::pair<LIST_MODE, std::vector<std::shared_ptr<AST>>>>();
            dot_info = std::stack<std::pair<bool, int>>();
        }

        /* iterate all nodes, check and convert the TokenType::SYMBOL with value "quote" to TokenType::QUOTE, TokenType::QUOTE with value "" to value "quote" */
        void convertToQuote(std::shared_ptr<AST> &cur_node) {
            if (cur_node != nullptr) {
                if (cur_node->token.type == TokenType::SYMBOL && cur_node->token.value == "quote") cur_node->token.type = TokenType::QUOTE;
                else if (cur_node->token.type == TokenType::QUOTE) cur_node->token.value = "quote";
            }
            else return;
            if (cur_node->left != nullptr) convertToQuote(cur_node->left);
            if (cur_node->right != nullptr) convertToQuote(cur_node->right);
        }

        /* end <S-exp>, if a main <S-exp> ended, then call executor, else store into AST */
        void endSExp(std::shared_ptr<AST> &cur_node, bool isAtom) {
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
                convertToQuote(cur_node); // if atom will do nothing
                executor.run(cur_node);
                resetInfos();
            }
        }

        /* parse */
        void parseAndBuildAST(const Token &token, int lineNum, int columnNum) {
            // check number of <S-exp> after DOT
            if (! dot_info.empty() && dot_info.top().first && dot_info.top().second == 1 && token.type != TokenType::RIGHT_PAREN) {
                throw SyntaxException::NoRightParen(lineNum, columnNum - token.value.length() + 1, token.value); // columnNum: the first char's pos of the token
            }
            // process token
            if (token.type == TokenType::QUOTE) lists_info.push({LIST_MODE::QUOTE, {std::make_shared<AST>(Token{TokenType::QUOTE, ""})}});
            else if (token.type == TokenType::DOT) {
                if (lists_info.empty() // start with DOT
                    || lists_info.top().first != LIST_MODE::NO_DOT // QUOTE + DOT or DOT + DOT
                    || lists_info.top().second.empty()) { // no <S-exp> before DOT, ex. > (.
                    throw SyntaxException::UnexpectedToken(lineNum, columnNum, token.value);
                }
                lists_info.top().first = LIST_MODE::WITH_DOT;
                dot_info.push({true, 0}); // start counting <S-exp> after DOT
            }
            else if (token.type == TokenType::LEFT_PAREN) lists_info.push({LIST_MODE::NO_DOT, {}}); // push a new list into stack
            else if (token.type == TokenType::RIGHT_PAREN) {
                if (lists_info.empty()
                    || lists_info.top().first == LIST_MODE::QUOTE
                    || (lists_info.top().first == LIST_MODE::WITH_DOT && ! dot_info.empty() && dot_info.top().first && dot_info.top().second == 0)) {
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
        }
};    

/* S-Expression Lexer */
class S_Exp_Lexer {
    private:
        char ch = '\0';
        int lineNum = 1, columnNum = 0;
        bool isCurLineHasToken = false;
        std::unordered_map<char, char> escape_map = {{'t', '\t'}, {'n', '\n'}, {'\\', '\\'}, {'\"', '\"'}};
        S_Exp_Parser parser;

        /* char judger */
        bool isWhiteSpace(char ch) { return (ch == ' ' || ch == '\t' || ch == '\n'); }

        bool isDigit(char ch) { return ('0' <= ch && ch <= '9'); }

        bool isAlphabet(char ch) { return (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')); }

        bool isEscape(char ch) { return (ch == 'n' || ch == '\"' || ch == 't'|| ch == '\\'); }

        /* real (number) judger */
        bool isInt(const std::string &str) { return std::regex_match(str, std::regex("^[-+]?\\d+$")); }

        bool isFloat(const std::string &str) { return std::regex_match(str, std::regex("^[-+]?((\\d+\\.\\d*)|(\\.\\d+))$")); }

        /* judge and set token type, also format if token is real (number) */
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

        /* line setter */
        void eatALine() {
            std::string useless_line;
            std::getline(std::cin, useless_line);
            isCurLineHasToken = false;
        }

        /* save a token into AST, execute a main <S-exp> if the current AST ended, handle errors */
        void saveAToken(Token &token, int lineNum, int columnNum, char ch) {
            try {
                judgeType(token);
                parser.parseAndBuildAST(token, lineNum, columnNum);
                token = Token(); // reset
            }
            catch (ExitException &e) { // do-nothing
                throw;
            }
            catch (SyntaxException &e) { // lexer & parser error: SyntaxException, must eat the remainings until '\n'
                if (ch != '\n') eatALine(); // '\n' cuz std::cin.get() already eat it to ch
                throw;
            }
            catch (OurSchemeException &e) { // executor error: SemanticException or RuntimeException, may putback the previous read char
                // expect error: a legal <S-exp> with (, ex. unbound_symbol() -> should put back '(' before throwing unbound symnol error
                if (ch == ' ' || ch == '\t' || ch == '(' || ch == '\'') std::cin.putback(ch);
                // if current char is not white space (or non-legal-token, but will not appeared after current execution)
                // record that current line has token inputed to prevent the next char is '\n' or ';' or eatALine()
                if (! isWhiteSpace(ch)) isCurLineHasToken = true;
                throw;
            }
        }

    public:
        /* tokenizer */
        void readAndTokenize() { // all inputs are dealed here
            Token token;
            std::stack<char> parenStack;
            lineNum = 1;
            columnNum = 0;
            ch = '\0';
            parser.resetInfos();

            gPrinter.printPrompt();
            if (std::cin.eof()) throw ExitException::NoMoreInput();
            while (std::cin.get(ch)) {
                if (ch == ';') {
                    if (token.value == "") {
                        // reset if AST is empty, else + 1
                        
                        if (parser.isCurTreeEmpty() && isCurLineHasToken) lineNum = 1;
                        else lineNum++;
                        
                        //lineNum++;
                        columnNum = 0;

                        // ignore the remainings until '\n'
                        eatALine();
                    }
                    else if (token.value[0] == '\"') { // in STRING
                        token.value += ch;
                        columnNum++;
                        isCurLineHasToken = true;
                    }
                    else {
                        // save previous
                        /*
                        if (syntax error) eat the remainings in saveAToken() and throw error; // ex. .;
                        else {
                            store the token into AST;
                            if (the previous <S-exp> is completed) {
                                if (semantic or runtime error) eat the remainings in saveAToken() and throw error; // ex. (ex);
                                else evaluate and eat the remainings in the following line; // ex. 123;
                            }
                            else eat the remainings in the following line; // ex. (123;
                        }
                        */
                        saveAToken(token, lineNum, columnNum, ch);
                        // if AST is empty, reset; else + 1
                        if (parser.isCurTreeEmpty()) lineNum = 1;
                        else lineNum++;
                        columnNum = 0;
                        eatALine();
                    }
                }
                else if (isWhiteSpace(ch)) {
                    if (ch == '\n') {
                        if (token.value == "") {
                            
                            if (parser.isCurTreeEmpty() && isCurLineHasToken) lineNum = 1;
                            else lineNum++;
                            
                            //lineNum++;
                            columnNum = 0;
                            isCurLineHasToken = false;
                        }
                        else if (token.value[0] == '\"') { // newline while in STRING
                            isCurLineHasToken = false;
                            columnNum++;
                            throw SyntaxException::NoClosingQuote(lineNum, columnNum);
                        }
                        else {
                            // no need to putback while executing the previous <S-exp> (i.e. Semantic / Runtime error)
                            // and no need to eat a line while Symtax error
                            // cuz the current char is '\n' itself, which already be consumed from std::cin
                            isCurLineHasToken = false;
                            saveAToken(token, lineNum, columnNum, ch);
                            
                            if (parser.isCurTreeEmpty()) lineNum = 1;
                            else lineNum++;
                            columnNum = 0;
                        }
                    }
                    else { // ' ' or '\t'
                        if (token.value == "") columnNum++;
                        else if (token.value[0] == '\"') { // in STRING
                            token.value += ch;
                            columnNum++;
                            isCurLineHasToken = true;
                        }
                        else {
                            saveAToken(token, lineNum, columnNum, ch);
                            
                            // if current line don't have token and current tree is empty (main <S-exp> didn't start)
                            // columnNum = 1 for recording current whitespace
                            if (parser.isCurTreeEmpty()) lineNum = 1, columnNum = 1;
                            else columnNum++;
                        }                        
                    }
                }
                else if (ch == '(') {
                    if (token.value == "") {
                        columnNum++;
                        parenStack.push(ch);
                        token.value = "(";
                        saveAToken(token, lineNum, columnNum, ch);
                    }
                    else if (token.value[0] == '\"') { // in STRING
                        token.value += ch;
                        columnNum++;
                    }
                    else {
                        // save previous
                        saveAToken(token, lineNum, columnNum, ch);

                        // set position
                        if (parser.isCurTreeEmpty()) lineNum = 1, columnNum = 1;
                        else columnNum++;
                        
                        // save current
                        parenStack.push(ch);
                        token.value = "(";
                        saveAToken(token, lineNum, columnNum, ch);
                    }
                    isCurLineHasToken = true;
                }
                else if (ch == ')') {
                    if (token.value == "") {
                        // set current
                        token.value = ")";
                        columnNum++;

                        // judge if no corresponding LP before RP
                        if (parenStack.empty()) {
                            eatALine();
                            throw SyntaxException::UnexpectedToken(lineNum, columnNum, token.value); // no LP before RP
                        }
                        else {
                            parenStack.pop();
                            saveAToken(token, lineNum, columnNum, ch);

                            // if previous <S-exp> ended, reset position
                            if (parser.isCurTreeEmpty()) lineNum = 1, columnNum = 0;
                        }
                    }
                    else {
                        if (token.value[0] == '\"') { // in STRING
                            token.value += ch;
                            columnNum++;
                        }
                        else {
                            // save previous
                            try { saveAToken(token, lineNum, columnNum, ch); }
                            // special caes: ex. abc). -> put back ')'
                            catch (SemanticException &e) { std::cin.putback(ch); throw; }
                            catch (RuntimeException &e) { std::cin.putback(ch); throw; }

                            // set current
                            token.value = ")";
                            // if previous <S-exp> ended, reset position, and add 1 column for ')'
                            if (parser.isCurTreeEmpty()) lineNum = 1, columnNum = 1;
                            else columnNum++;
                            
                            // save current
                            if (parenStack.empty()) { // no LP before RP, ex. 123).
                                eatALine();
                                throw SyntaxException::UnexpectedToken(lineNum, columnNum, token.value);
                            }
                            else {
                                parenStack.pop();
                                saveAToken(token, lineNum, columnNum, ch);
                                // if previous <S-exp> ended, reset position
                                if (parser.isCurTreeEmpty()) lineNum = 1, columnNum = 0;
                            }
                        }
                    }
                    isCurLineHasToken = true;
                }
                else if (ch == '\\') {
                    if (token.value != "" && token.value[0] == '\"' && escape_map.count(std::cin.peek())) { // in STRING and legal escape
                        ch = std::cin.get(); // legal escape mode after backslash: 't' or 'n' or '\\' or '\"', ex. 't'
                        token.value += escape_map[ch]; // ex. token += '\t'
                        columnNum += 2; // ex. '\\' and 't' -> 2 chars
                    }
                    else {
                        token.value += ch;
                        columnNum++;
                    }
                    isCurLineHasToken = true;
                }
                else if (ch == '\'') {
                    if (token.value == "") {
                        token.value = "\'";
                        columnNum++;
                        saveAToken(token, lineNum, columnNum, ch);
                    }
                    else if (token.value[0] == '\"') { // in STRING
                        token.value += ch;
                        columnNum++;
                    }
                    else {
                        // save previous
                        saveAToken(token, lineNum, columnNum, ch);

                        // set current
                        token.value = "\'";
                        if (parser.isCurTreeEmpty()) columnNum = 1;
                        else columnNum++;
                        saveAToken(token, lineNum, columnNum, ch);
                    }
                    isCurLineHasToken = true;
                }
                else if (ch == '\"') {
                    if (token.value == "") { // the start of a STRING
                        token.value += ch;
                        columnNum++; // cuz may have whitespace before so use ++ not set to 1
                    }
                    else if (token.value[0] == '\"') { // the end of a STRING
                        token.value += ch;
                        columnNum++;
                        // ex. '(1 . "1" "2") -> unexpected ')' at (1, 12) is >>2<<
                        saveAToken(token, lineNum, columnNum, ch);
                        
                        if (parser.isCurTreeEmpty()) columnNum = 0;
                    }
                    else { // token + STRING, with no whitespace ex. > asf"
                        // save previous
                        saveAToken(token, lineNum, columnNum, ch);
                        
                        // set current: the start of a STRING
                        token.value = "\"";
                        if (parser.isCurTreeEmpty()) columnNum = 1;
                        else columnNum++;
                    }
                    isCurLineHasToken = true;
                }
                else {
                    token.value += ch;
                    columnNum++;
                    isCurLineHasToken = true;
                }
            }
            if (std::cin.eof()) throw ExitException::NoMoreInput();
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
        // syntax / semantic / runtime errors
        catch (OurSchemeException &e) {
            gPrinter.printError(e);
        }
        // should not happen
        catch (std::exception &e) {
            std::cout << "std::exception: " << e.what() << std::endl;
            break;
        }
        catch (...) {
            std::cout << "Unknown error\n";
            break;
        }
    }
    return 0;
}