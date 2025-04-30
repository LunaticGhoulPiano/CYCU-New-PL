# CYCU-New-PL (中原資工程式語言project - OurScheme)
## 資料與備註
- 本人修大四畢業班，夏老大授課，計分方式與大三不同
- [Project 1 的簡報](https://www.canva.com/design/DAGemJ7sLQI/X3aeWBckMJEJuJ5ks3dF7w/edit)
- 老大的上傳系統：
    - [設定密碼](https://pai.lab715.uk:5001/ChangePassword/)
    - [上傳檔案](https://pai.lab715.uk:5001/PL-PostCode/)
- 線上測試系統：
    - [互動式測試](https://cycu-ice-pl.github.io/website/#/OurScheme)
    - [合法S-Expression建立抽象語法樹的可視化](https://cycu-ice-pl.github.io/website/#/Visualize)
- 主要使用Discord bot測試與計分
    - 系統只會使用output測試，當有```std::cout```沒換行後接```std::cin```時應該不會吃，也因此我的Interactive I/O在印```> ```時會有問題，但使用指令輸出是正確的
    - [Discord bot 連結](https://discord.com/oauth2/authorize?client_id=1265725772094767149)
    - Discord bot 應用程式ID: ```1265725772094767149```
    - 題目數量：
        - Project 1: 10
        - Project 2: 16
        - Project 3: 12
        - Project 4: 10
        - 每題都會有三個測資檔
- 系統只吃一個檔案，所以要全部塞在同一檔案中
- 平時分數計算：每堂課會有4分
    1. 出席分
    2. So so
    3. Ok
    4. Great
    0. Not good
- Project分數到80可以不用出席

## Command
- Windows Powershell:
    - 編譯：
        ```powershell
        g++ ./main.cpp -std=c++2a -o ./main
        ```
    - 執行：
        - Interactive I/O:
            ```powershell
            ./main.exe
            ```
        - Batch I/O:
            - 在```./self_tests./*./```底下的```.in```檔是輸入，```.out```檔是正確輸出，而我習慣以```.bug```檔做為自己程式的輸出
            - 以```./self_tests./project1./test10.in```為例：
                ```powershell
                Get-Content ./self_tests./project1./test10.in | ./main.exe > ./self_tests./project1./test10.bug
                ```

## Structure
```
CYCU-New-PL
├──.gitignore
├──README.md
├──2025-02-19-hsia-Notes2025-02-19-hsia-Notes
│ ├──2025-02-19-ClassNotes.txt
│ ├──mMyShell-n.cpp
│ ├──mMyShell.cpp
│ ├──myShell-01.cpp
│ ├──myShell-01n.cpp
│ ├──myShell-02.cpp
│ ├──myShell-02n.cpp
│ ├──myShell-03.cpp
│ ├──myShell-03n.cpp
│ ├──MyShell-Series-Rationale.txt
│ ├──PL-113-2-MidtermReview-I.txt
│ ├──PL113-2-Project.zip
│ └──Tree的建造-UTF-8.txt
├──AboutProject
│ ├──HowToCorrectlyGetNextToken-UTF-8.txt
│ ├──HowToWriteOurScheme.doc
│ ├──OurSchemeIntro.doc
│ ├──OurSchemeProj1-UTF-8.txt
│ ├──OurSchemeProj2-UTF-8.txt
│ ├──OurSchemeProj3-UTF-8.txt
│ ├──OurSchemeProj4-UTF-8.txt
│ ├──ThreeLisps-UTF-8.txt
│ └──Tree的建造-UTF-8.txt
├──AboutRecursiveDescentParsing
│ ├──NeedOfFIRSTset.txt
│ └──RecursiveDescentParsing-Intro.doc
├──Complete_Code_by_Projects
│ └──Project1.cpp
├──self_tests
│ ├──project1
│ │   ├──test1.in
│ │   ├──test1.out
│ │   ├──test2.in
│ │   ├──test2.out
│ │   ├──test3.in
│ │   ├──test3.out
│ │   ├──test4.in
│ │   ├──test4.out
│ │   ├──test5.in
│ │   ├──test5.out
│ │   ├──test6.in
│ │   ├──test6.out
│ │   ├──test7.in
│ │   ├──test7.out
│ │   ├──test8.in
│ │   ├──test8.out
│ │   ├──test9.in
│ │   ├──test9.out
│ │   ├──test10.in
│ │   └──test10.out
│ └──project2
└──main.cpp
```

## Project 1 整理
- 程式碼請見```./Complete_Code_by_Projects./Project1.cpp```
- Terminal Tokens
    - LEFT-PAREN : ```(```
    - RIGHT-PAREN: ```)```
    - INT: ```123```, ```+123```, ```-123```, ..., etc.
    - FLOAT: ```.0```, ```+.0```, ```-.0```, ```0.```, ```+0.```, ```-0.```, ..., etc.
    - STRING: ```"this is a string"```, **不可能有換行**
    - DOT: ```.```
    - NIT: ```nil```, ```#f```, ```(``` + ```)```
    - T: ```t```, ```#t```
    - QUOTE: ```'```
    - SYMBOL: 以上皆非的, ```3.25a```, ```a.b```, ```#fa```, ```\```, ```..```, ..., etc.
- 文法
    - 使用[擴展巴科斯范式(Extended Backus-Naur Form, EBNF)](https://hackmd.io/@ShenTengTu/HJzCM3aDr)表示法
    - ```{}```表示重複零次或多次
    - ```[]```表示可有可無
    - 文法：
        ```EBNF
        <ATOM>  ::= SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN RIGHT-PAREN
        <S-exp> ::= <ATOM> 
                | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
                | QUOTE <S-exp>
        ```
- 我的設計
    - ```gTestNum```:
        - 直接用```std::string```讀掉整行，以避免還要讀```\n```
    - ```struct Token```:
        - 定義Terminal Token的結構
        - 存有```TokenType type```與```std::string value```
        - Default為```type = NIL```, ```value = ""```
    - ```struct AST```:
        - 定義抽象語法樹的結構
        - 存有```bool isAtom```, ```Token atom```, ```std::shared_ptr<AST> left```與```std::shared_ptr<AST> right```
        - Default為```isAtom = false```, ```atom = Token()```, ```left = nullptr```, ```right = nullptr```
    - ```class Debugger```:
        - 純粹用來debug
        - 以```Debugger gDebugger```全域使用
        - ```std::string getType(Token token) {}```:
            - 以```std::string```回傳```TokenType```
        - ```void printType(Token token) {}```:
            - 印出```TokenType```及```"\n"```
        - ```void debugPrintAST(const std::shared_ptr<AST> node, int depth = 0, const std::string &prefix = "AST_root") {}```:
            - 傳入root node / current node，遞迴印出左右節點
            - 範例：
                - 當輸入為
                    ```
                    (((1) 3 . 4) 8 . ("anlsud h"))
                    ```
                - Pretty print為
                    ```
                    > ( ( ( 1
                        )    
                        3    
                        .    
                        4    
                    )      
                    8      
                    "anlsud h"
                    )
                    ```
                - ```gDebugger.debugPrintAST(cur_node);```印出
                    ```
                    AST_root (isAtom = false)
                    |--- left  ->  (isAtom = false)
                        |--- left  ->  (isAtom = false)
                        |--- left  ->  (isAtom = true, atom = "1", type = INT)
                        |--- right ->  (isAtom = true, atom = "nil", type = NIL)
                        |--- right ->  (isAtom = false)
                        |--- left  ->  (isAtom = true, atom = "3", type = INT)
                        |--- right ->  (isAtom = true, atom = "4", type = INT)
                    |--- right ->  (isAtom = false)
                        |--- left  ->  (isAtom = true, atom = "8", type = INT)
                        |--- right ->  (isAtom = false)
                        |--- left  ->  (isAtom = true, atom = ""anlsud h"", type = STRING)
                        |--- right ->  (isAtom = true, atom = "nil", type = NIL)
                    ```
    - ```class BaseException: public std::exception```:
        - 定義正常退出及四種Error，共五種exceptions的處理方式
    - ```class CorrectExit: public BaseException```:
        - ```\n> \nThanks for using OurScheme!```
        - 以throw exception表示正常退出
        - 在```main()```中會```break```
        - 印完```Thanks for using OurScheme!```不用換行
        - 特別注意**只有當```<S-exp>```完全等於```(```+```exit```+```)```時才可退出**
        - 範例：
            - 當輸入為```(exit)```時才可退出
            - 當輸入為```(exit . (exit))```時不可退出
    - ```class UnexpectedToken: public BaseException```:
        - ```\nERROR (unexpected token) : atom or '(' expected when token at Line X Column Y is >>...<<\n```
        - 需要```Token token```, ```int lineNum```, ```int columnNum```
    - ```class NoRightParen: public BaseException```:
        - ```\nERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<\n```
        - 需要```Token token```, ```int lineNum```, ```int columnNum```
        - 特別注意```columnNum```是**error token的第一個字元的位置**
        - 範例：
            - 當輸入為
                ```
                '( 123
                456 . 789 10_11_12 )
                ```
            - 輸出應為
            ```
            > ERROR (unexpected token) : ')' expected when token at Line 2 Column 11 is >>10_11_12<<
            ```
            - 而非
            ```
            > ERROR (unexpected token) : ')' expected when token at Line 2 Column 18 is >>10_11_12<<
            ```
            - 因為```10_11_12```雖然是合法token，但是根據文法，```DOT```後面可以且僅能有一個```<S-exp>```，因此在印error時還是要先抓完整個token，再印此token的第一個字元的位置
    - ```class NoClosingQuote: public BaseException```:
        - ```\nERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y\n```
        - 需要```int lineNum```, ```int columnNum```
    - ```class NoMoreInput: public BaseException```:
        - ```\n> ERROR (no more input) : END-OF-FILE encountered\nThanks for using OurScheme!```
        - 在```main()```中會```break```
        - 特別注意**印完此error後還要印```Thanks for using OurScheme!```**
        - 印完```Thanks for using OurScheme!```不用換行
    - ```class S_Exp_Parser```:
        - private:
            - ```class LIST_MODE```:
                - 定義```LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN```與```QUOTE <S-exp>```時的處理模式
                - 因為AST是**二元右傾樹**，因此所有事情以```(cons car cdr)```思考
                - [cons, car, cdr的解釋](https://www.reddit.com/r/learnprogramming/comments/169nth/what_do_car_cdr_and_cons_stand_for/):
                    ```
                    cons: Construct, 表示list開始
                    car: Contents of the Address part of Register number, 表示left child
                    cdr: Contents of the Decrement part of Register number, 表示right child，如果<S-exp>沒有```DOT```即為```NIL```，反之為第一個<S-exp>後的剩下那一坨零個或多個的<S-exp>
                    ```
                    - 當**沒有```DOT```**，car就會是LP右邊的**第一個```<S-exp>```**，cdr就會是**剩下的零個或多個<S-exp>**直到RP
                    - 當**有```DOT```**，car就會是LP右邊的**第一個```<S-exp>```**，cdr就會是**DOT右邊的唯一一個<S-exp>**
                    - 範例：
                        - 當輸入為
                            ```
                            (sS1 sS2 sS3)
                            ```
                        - 樹為
                            ```
                              node      // car: sS1, cdr: sS2 sS3
                              / \
                            sS1 node
                                / \
                              sS2 node  // car: sS2, cdr: sS3
                                  / \
                                sS3 nil
                            ```
                        - 當輸入為
                            ```
                            (sS1 sS2 . (sS3 . sS4))
                            ```
                        - 樹為
                            ```
                              node        // car: sS1, cdr: sS2 . (sS3 . sS4)
                              / \
                            sS1 node
                                / \
                              sS2 node    // car: sS2, cdr: (sS3 . sS4)
                                  / \
                                sS3 sS4
                            ```
                - ```NO_DOT``` 範例：
                    ```
                    (1 2 3 (
                        4
                        5 )
                        ( 6 7))
                    ```
                - ```WITH_DOT``` 範例：
                    ```
                    (1 2 (3 a "str" .
                         set)
                      ()
                         nil
                        .
                       DOT)
                    ```
                - ```QUOTE```範例：
                    ```
                    '(
                        '
                        "this"
                        ' "is"
                        '"a"
                        '''''''''"QUOTE"
                    )
                    ```
            - ```std::stack<std::pair<LIST_MODE, std::vector<std::shared_ptr<AST>>>> lists_info;```:
                - 因為stack就是扁平化的樹，因此我使用stack處理```<S-exp>```，並在此正在處理的```<S-exp>```完成時pop出來並加入到樹中
                - 此stack的top永遠是**正在處理的未完成```<S-exp>```**
                - 此stack中的結構：
                    - ```first```: LISTMODE
                    - ```second```: 正在處理的未完成```<S-exp>```
            - ```std::stack<std::pair<bool, int>> dot_info;```:
                - 也是stack，但僅當遇到```.```時才會被壓入
                - 此stack中的結構：
                    - ```first```: ```.```是否開始
                    - ```second```: ```.```後面接的```<S-exp>```的數量
                - 第一次遇到```.```時初始化為```true```與```0```
        - public:
            - ```std::vector<std::shared_ptr<AST>> tree_roots;```:
                - 存放已完成的```<S-exp>s```的roots，因為一個```<S-exp>```就是一棵樹
            - ```void resetInfos() {}```:
                - 重設```lists_info```與```dot_info```
                - 重設時機為**一個完整的```<S-exp>```結束**或**遇到error時**
            - ```std::shared_ptr<AST> makeList(const std::vector<std::shared_ptr<AST>> &tree_root, const std::shared_ptr<AST> &cdr = std::make_shared<AST>(Token{TokenType::NIL, "nil"})) {}```:
                - 在一個```<S-exp>```結束時將stack中pop出的此```<S-exp>```由後向前轉換成```(cons car cdr)```的結構
                - 如果沒有```.```則cdr就設為```nil```
                - 回傳建好的子樹
            - ```void checkExit(const std::shared_ptr<AST> &tree_root) {}```:
                - 檢查```<S-exp>```是否為```(```+```exit```+```)```的結構
                - 注意檢查時機：只有當**一個完整的```<S-exp>```完全等於```(exit)```時**才要檢查並退出
            - ```bool parseAndBuildAST(const Token &token, int lineNum, int columnNum) {}```:
                - **並非recursive descending parsing，而是將樹的概念扁平化成stack進行parse，當```<S-exp>```結束時才加入樹**
                - 為**Top-Down parsing**
                - **每次在tokenizer切出一個token時被呼叫**，即每個獨立的token都會跑一次
                - 回傳**目前的完整```<S-exp>```是否結束```**
            - ```printAST(const std::shared_ptr<AST> &cur, bool isRoot = true, int depth = 0, bool isFirstTokenOfLine = true) {}```:
                - Recursively pretty print
                - 注意縮排及換行
    - ```class S_Exp_Lexer```:
        - private:
            - ```char ch```:
                - 用來從```std::cin```讀字元
            - ```int lineNum = 1, columnNum = 0```:
                - 紀錄每個token的position
                - 跟隨字元更新
                - 注意```ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<```的column是error token的第一個字元的column而非最後一個字元的column
            - ```std::unordered_map<char, char> escape_map = {{'t', '\t'}, {'n', '\n'}, {'\\', '\\'}, {'\"', '\"'}};```:
                - 當遇到跳脫字元```\```時使用
            - ```bool isWhiteSpace(char ch) {}```:
                - 回傳是否為``` ```或```\t```或```\n```
            - ```bool isDigit(char ch) {}```:
                - 回傳是否為字元```0``` ~ ```9```
            - ```bool isAlphabet(char ch) {}```:
                - 回傳是否為字元```a``` ~ ```z```或```A``` ~ ```Z```
            - ```bool isEscape(char ch) {}```:
                - 回傳是否為跳脫字元```\```後方接的字元：```n```或```\"```或```t```或```\\```
            - ```bool isInt(const std::string &str) {}```:
                - 使用正則表達式回傳是否為合法```INT```
            - ```bool isFloat(const std::string &str) {}```:
                - 使用正則表達式回傳是否為合法```FLOAT```
            - ```void judgeType(Token &token) {}```:
                - 判斷並設定```TokenType```
                - ```INT```與```FLOAT```的```%.3f```化會在這邊做
                - ```#f```會在這邊直接被替換成```nil```
                - ```t```會在這邊直接被替換成```#t```
            - ```void eatALine() {}```:
                - 顧名思義，吃掉剩餘的字元直到吃到```\n```後停止
                - 在遇到error或```;```時被呼叫
            - ```bool saveAToken(Token &token, S_Exp_Parser &parser, int lineNum, int columnNum, bool eat = true) {}```:
                - 每次tokenizer切出一個token時都會呼叫
                - 因為遇到error時要吃掉整行，因此```S_Exp_Parser::parseAndBuildAST()```丟出的error會在這邊做中轉
                - 要做的事情：
                    1. 設定```TokenType```
                    2. 將token丟進S_Exp_Parser去parse
                    3. 沒有錯誤的化重設token
                    4. 將```S_Exp_Parser::parseAndBuildAST()```的結果回傳
                - 因為有error中轉，因此有tyy-catch-throw機制
                - 並且預設遇到error都會吃掉整行，除非遇到error當下讀的字元是```\n```就不用吃（如果這邊吃了會多吃）
                - 記得遇到error要拋棄正在建立的樹
        - public:
            - ```bool s_exp_ended = false;```:
                - 接```S_Exp_Parser::parseAndBuildAST()```回傳的**一個完整的```<S-exp>```是否結束**
                - 並且要及其注意更新的時機（指遇到white space時）
            - ```void readAndTokenize(S_Exp_Parser &parser) {}```:
                - 逐個字元從```std::cin```讀取，切token，並在每次切出token時都呼叫```saveAToken()```
                - 會檢查部分的error：
                    1. ```STRING```是否完成（```NoClosingQuote```）
                    2. ```LEFT-PAREN```與```RIGHT-PAREN```是否成對（```UnexpectedToken```）
                    3. 是否有```(exit)```正常退出（```NoMoreInput```）
    - ```int main() {}```:
        - Read-Eval-Print-Loop
        - 有lexer與parser，無窮迴圈使用try-catch執行```lexer.readAndTokenize(parser);```
        - 在```CorrectExit```, ```NoMoreInput```及未知錯誤時break