# Project2 整理
## 未通過隱測之題號
5, 7, 15
## 重構
- ```main()```會無限執行```S_Exp_Lexer()::readAndTokenize()```，有例外會先印出訊息，再根據例外類型決定是否結束程式，或繼續執行```S_Exp_Lexer()::readAndTokenize()```
- ```S_Exp_Lexer::readAndTokenize()```重寫邏輯
## 簡述
- ```S_Exp_Lexer::readAndTokenize()```不斷讀取char
- ```S_Exp_Lexer::saveAToken()```將一個合法的token在parse後存入AST
- ```S_Exp_Parser::parseAndBuildAST()```對傳入的legal token進行parse，並在一個```<S-exp>```結束時透過```S_Exp_Parser::endSExp()```判斷是否為main ```<S-exp>```結束還是sub ```<S-exp>```結束，若sub ```<S-exp>```結束則繼續parse，否則為合法main ```<S-exp>```，在執行後重設
- ```S_Exp_Executor::run()```先標記所有樹或子樹的root node及root->left node，再將main ```<S-exp>```進行evaluate，最後印出執行結果
- ```S_Exp_Executor::evaluate()```檢查main ```<S-exp>```是否有semantic error，沒問題的話就執行
- ```S_Exp_Executor::execute()```檢查main ```<S-exp>```是否有semantic error，沒問題的話根據root node的keyword type進行對應的函數操作，執行時會檢查是否有runtime error
- 所有正確的return value會被儲存到current node，即傳入一個main ```<S-exp>```的root node，最後正確執行結果會將此root node變成return node
## 架構
### ```enum class TokenType```
- 定義token的type
### ```enum class KeywordType```
- 定義了所有keyword
    1. minimal types:
        - 最小單位的keyword
            ```
            INTEGER, FLOAT, STRING, ERROR_OBJECT, BOOLEAN, SYMBOL
            ```
    2. complex primitives:
        - 基於minimal type的複合型別
            ```
            REAL, NUMBER: 基於INT, FLOAT
            ATOM: 任意的minimal types
            NIL: BOOLEAN
            LIST: non-dotted pair的list
            PAIR: dotted pair
            ```
    3. function types:
        - primitive function的type，用此作為hash map的key去map到對應的執行
            ```
            CONSTRUCTOR, BYPASS_EVALUATION, BINDING,
            PART_ACCESSOR, PRIMITIVE_PREDICATE, OPERATION,
            EQIVALENCE_TESTER, SEQUENCING_AND_FUNCTIONAL_COMPOSITION, CONDITIONAL,
            READ, DISPLAY, LAMBDA,
            VERBOSE, EVALUATION, CONVERT_TO_STRING,
            ERROR_OBJECT_OPERATION, CLEAN_ENVIRONMENT, EXIT
            ```
### ```enum class ARGUMENT_NUMBER_MODE```
- 定義了函數的參數模式
    1. ```AT_LEAST```: 至少n個
    2. ```MUST_BE```: 必為n個
    3. ```SPECIFIC```: 可為任一```{n1, n2, ... nn}```個
### ```struct KeywordInfo```
- 定義了一個keyword的相關資訊
    1. ```ARGUMENT_NUMBER_MODE arg_mode```: 函數的參數模式
    2. ```std::vector<int> arg_nums```: 可接受的參數數量，僅在```arg_mode == ARGUMENT_NUMBER_MODE::SPECIFIC```時有多個，否則僅有一個值
    3. ```KeywordType functionType```: 定義此keyword所屬的function type
### ```struct Token```
- 定義了一個token node的結構
    1. ```TokenType type```: 此token的type
    2. ```std::string value```: 此token的value
### ```enum class BindingType```
- 定義了一個AST node的binding type
    ```
    MID: 一棵AST中非leaf的node, 在tokenizing會被視為nil
    ATOM_BUT_NOT_SYMBOL: 並非function的symbol
    PRIMITIVE_FUNCTION: primitive function, 即keywords
    USER_FUNCTION: 使用者定義的function
    ```
### ```struct Binding```
- 定義了一個AST node對應的binding之結構
    1. ```bool isRoot```: 是否為一個```<S-exp>```的root（只要是```<S-exp>```的root都算，並非僅有最外層的main ```<S-exp>```之root
    2. ```bool isFirstNode```: 是否為一個```<S-exp>```的第一個left node，當root的binding type為```MID```時，root->left的value應為一個function name
    3. ```isReturnOfQuote```: 是否為quote執行後的returun，例如：
        ```Scheme
        ((quote list)) ; ERROR (attempt to apply non-function) : list
        ((list)) ; nil
        ```
    4. ```KeywordType dataType```: 此token的keyword type
    5. ```BindingType bindingtype```: 此AST node的binding type
    6. ```std::string value```: 在binding後的value，例如：
        ```Scheme
        list ; after binding, token.value = "list", binding value = "#<procedure list>"
        ```
### ```struct AST```
- 定義了AST的節點結構
    1. ```bool isAtom```: 是否為atom
    2. ```Token token```: 此節點的token
    3. ```Binding binding```: 此節點的binding
    4. ```std::shared_ptr<AST> left = nullptr, right = nullptr```: 左右子節點
    5. ```bool isEndNode()```: 是否為leaf node
### ```class Debugger```
- ```Debugger gDebugger```: 全域使用
### ```class OurSchemeException: public std::exception```
- 定義專屬於此interpreter的exceptions
### ```class ExitException: public OurSchemeException```
- 定義需要結束程式的exceptions
    1. ```CorrectExit```: 正常退出
    2. ```NoMoreInput```: EOF
    3. ```NoMoreInputWhileRead```: project 4的error，還不知道要幹啥，先定義好
### ```class SyntaxException: public OurSchemeException```
- 定義了在lexing與parsing時的語法錯誤
    1. ```UnexpectedToken```: ```atom or '(' expected```
    2. ```NoRightParen```: ```')' expected```
    3. ```NoClosingQuote```: 一個```\"```沒有對應的```\"```
### ```class SemanticException: public OurSchemeException```
- 定義了語意錯誤，正確```<S-exp>```，執行前檢查
    1. ```LevelError```: ```clean-environment```, ```define```, ```exit```
    2. ```FormatError```:
        - project 2: ```cond```, ```define```
        - project 3: ```let``, ```lambda```
        - project 4: ```set!```
    3. ```UnboundSymbol```: 未定義的symbol
    4. ```IncorrectNumOfArgs```: 不正確的參數數量
    5. ```IncorrectArgType```: 不正確的參數型別
    6. ```NonList```: 此list為dotted pair
    7. ```NonFunction```: root->left的value並非function name
### ```class RuntimeException: public OurSchemeException```
- 定義了執行時的錯誤
    1. ```DivisionByZero```: 除以零
    2. ```NoReturnValue```: 沒有return，因為是直譯器所以為runtime error，但OurC會是語意錯誤
### ```class Printer```
- 定義所有的輸出
    - ```Printer gPrinter```: 全域使用
    1. ```void printPrompt()```: 印出```"\n> "```
    2. ```void printSExp(std::string s_exp)```: 印出```<S-exp>```後```printPrompt()```
    3. ```void printError(const std::exception &e)```: 印出error message (```e.what()```)
    4. ```void printResult(std::string result)```: 印出執行結果後```printPrompt()```
    5. ```std::string getprettifiedSExp(bool useToken, const std::shared_ptr<AST> &cur, std::string s_exp = "", int depth = 0, bool isRoot = true, bool isFirstTokenOfLine = true)```: 遞迴pretty write到一個暫時的```std::string```並回傳此prettified後的```<S-exp>```
### ```class S_Exp_Executor```
- evaluate一個合法的```<S-exp>```，並在它為合法的functionc或atom時execute
#### ```private```
##### ```std::unordered_map<std::string, std::shared_ptr<AST>> globalVars, localVars```
1. ```globalVars```: 全域變數
2. ```localVars```: 區域變數，此階段不會出現
##### ```bool isKeyword(std::string sym)```
- 是否為primitive function name或```#t```或```nil```
##### ```bool isDefined(std::string sym)```
- 是否為全域或區域變數，此階段僅判斷是否為全域變數
- 可選擇是否要檢查binding
##### ```bool isPrimFunc(std::string sym, bool checkBinding = true)```
- 是否為primitive function name
##### ```bool isUserFunc(std::string sym)```
- 是否為使用者自定義的函數，此階段不會被使用
##### ```bool isFunction(std::string sym, bool checkBinding = true)```
- 是否為primitive or user-defined function name
##### ```void checkPureList(std::shared_ptr<AST> cur_node, std::shared_ptr<AST> cur_func, bool useToken = false)```
- 是否為非dotted pair的list
##### ```void checkLevelOfSpecifics(std::string func_name, int level)```
- 檢查```clean-environment```, ```define```及```exit```的level是否正確
##### ```void checkArgumentsNumber(std::shared_ptr<AST> cur)```
- 檢查函數的參數數量是否正確
##### ```void checkArgumentType(std::string func_name, std::shared_ptr<AST> arg)```
- 檢查函數的參數型別是否正確
##### ```KeywordType getNodeMinorBindingDataTypeByTokenValue(std::shared_ptr<AST> cur)```
- 設定current node的binding之keyword type
##### ```void getBinding(std::shared_ptr<AST> &cur, int level, std::string func_name = "", int bypassLevel = -1)```
- bind the current node
##### ```std::shared_ptr<AST> makeBooleanNode(bool b)```
- 回傳一個boolean atom node
##### ```std::shared_ptr<AST> makeBeginNode()```
- 回傳一個begin atom node
##### ```std::unordered_map<KeywordType, std::function<void(std::shared_ptr<AST> &)>> prim_func_map```
- 透過keyword type執行對應的函數
##### ```void construct(std::shared_ptr<AST> &cur)```
- 執行```list```或```cons```
##### ```void bypass(std::shared_ptr<AST> &cur)```
- 執行```quote```
##### ```void bind(std::shared_ptr<AST> &cur)```
- 執行```define```
- 此階段僅定義單一參數，並無定義函數
##### ```void getPart(std::shared_ptr<AST> &cur)```
- 執行```car```或```cdr```
##### ```void judgePrimitivePredicate(std::shared_ptr<AST> &cur)```
- 執行```atom?```, ```pair?```, ```list?```, ```null?```, ```integer?```, ```real?```, ```number?```, ```string?```, ```boolean?```或```symbol?```
##### ```std::unordered_map<std::string, std::function<double(std::string a, std::string b)>> arithmeticAndLogicalAndCompareOperateMap```
- 定義了```+```, ```-```, ```*```, ```/```, ```>```, ```>=```, ```<```, ```<=```, ```=```, ```string>?```, ```string<?```及```string=?```的操作
##### ```std::string arithmeticOperations(std::string op, std::string a, std::string b, bool convertToFloat)```
- 執行```+```, ```-```, ```*```或```/```，並返回formatted後的結果
##### ```void operate(std::shared_ptr<AST> &cur)```
- 執行```+```, ```-```, ```*```, ```not```, ```/```, ```>```, ```>=```, ```<```, ```<=```, ```=```, ```string-append```, ```string>?```, ```string<?```或```string=?```
##### ```bool isStructureTheSame(std::shared_ptr<AST> obj1, std::shared_ptr<AST> obj2)```
- 比對兩個main ```<S-exp>```是否結構與value相同
##### ```void judgeEqivalence(std::shared_ptr<AST> &cur)```
- 執行```eqv?```或```equal```
##### ```void sequence(std::shared_ptr<AST> &cur)```
- 執行```begin```
##### ```void cleanEnvironment(std::shared_ptr<AST> &cur)```
- 執行```clean-environment```
##### ```void exit(std::shared_ptr<AST> &cur)```
- 執行```exit```
##### ```void debugPrintNode(std::string pos, std::shared_ptr<AST> cur, int level)```
- debug用，遞迴印出AST資訊
##### ```void debugPrintAST(std::shared_ptr<AST> &cur, int level, std::string pos = "root")```
- debug用，印出單一節點資訊
##### ```void labelRootAndFirstNode(std::shared_ptr<AST> &cur, int level = 0)```
- 標記一個legal```<S-exp>```的root及root->left
##### ```bool isLegalSymbolToBeBound(std::string sym)```
- 判斷是否為合法的參數名稱
##### ```void execute(std::shared_ptr<AST> &cur, int level, int &bypassLevel)```
- 檢查semantic error，若正確則執行main ```<S-exp>```
##### ```void evaluate(std::shared_ptr<AST> &cur, int level = 0, int bypassLevel = -1, std::string func_name = "")```
- 檢查semantic error，若正確則丟入```execute()```
#### ```public```
##### ```void run(std::shared_ptr<AST> &root)```
- 傳入legal main ```<S-exp>```的root，標記重要節點，evaluate後execute，最後印出結果

## 剩下的參照Project 1，暫時不寫重寫邏輯的部分