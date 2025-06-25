# Project3 簡易整理及描述（因為過了所以懶了）
- 忘記哪些是新增的是用來幹嘛的，反正看註解你會看得懂
- verbose相關的兩個指令完成，剩下的應該都有bug沒修（verbose好像是project4才會出現，但既然document在project3提到，實作也簡單，那我就做了）
- 因為在Project2中沒有symbol被binding為一個todo function tree的情況，所以是會對存在enviroment中的東西進行改動；Project3因為會有```(define func 'do-something)```或```(define (func) (begin 'do-something))```的情況所以搞了一個duplicate的function用來複製存在environment中的tree以避免改動到原本的東西（會需要reuse它）
- Project2只考慮到global variable，Project3要考慮local所以以下為思想：
    - Global variable (```globalVars```) 就是單純的hash map，一個symbol(key)對應到一個shared pointer(```<S-exp>``` tree)
    - Local variable (```localVars```) 也是hash map，但是一個symbol(key)對應到一個stack，當此key symbol有新的binding時就push，並且每次get binding都是get the top of stack，直到離開定義此新binding的(sub)tree就將top給pop掉，並且當此local variable的stack在pop完是empty時就將此symbol從local hash map中移除
- 在最主要的```run()```中：
    ```C++
    void run(std::shared_ptr<AST> &root) {
        try {
            labelRootAndFirstNode(root); // 遞迴地將一個main <S-exp>中的root和第一個sub <S-exp>（如果有）標記起來
            evaluate(root); // 因為是直譯器所以一旦evaluate完一個todo <S-exp>就要直接執行了
            if (! verbose && isCleanOrDefine) gPrinter.printPrompt(); // 根據verbose或其他case決定要不要印東西
            else gPrinter.printResult(gPrinter.getprettifiedSExp(false, root));
            isCleanOrDefine = false;
        }
        catch (OurSchemeException &e) {
            localVars.clear(); // 一個main <S-exp>結束及遇到error記得清掉local variables
            throw;
        }
    }
    ```
- ```evaluate()```:
    ```C++
    if (cur == nullptr) return; // 不用解釋了吧
    else if (cur->token.type != TokenType::NIL || (cur->token.type == TokenType::NIL && cur->isEndNode())) {} // ATOM(end node, leaf node, 隨便你怎麼稱呼)或是中繼節點（即非end node的nil）
    else if (cur->token.type == TokenType::NIL) {// 中繼節點，left必為end node，right可為end node或中繼節點
        /* Step 1. 中繼節點接中繼節點，即LP + LP + ...
           ex. (((begin 'return)))
           先消化掉內層的LP sub tree再執行一些操作 */
        /* Step 2-1. left node是function name
            如果是user-defined function就先把它變成begin
            進入primitive function execution */
        /* Step 2-2. left node是非function name的atom */
        /* Step 3. 最後當一個main <S-exp>結束要pop all local variables */
    }
    ```
- ```execute()```:
    ```C++
    // 檢查level，即current <S-exp> tree是main還是sub
    // 搞定quote
    // 先保存執行前的pretty print，以便作為error message丟出
    // 特殊情況特殊對待：if, cond, and..., etc.
    // else根據prim_func_map的function type執行對應的操作，並因為call by reference因此沒有return value（我直接把執行完的結果存進cur就不用搞不同的return了）
    ```
- 很多special case都是"處理完"轉換成```(begin ...)```再丟入```execute()```根據```prim_func_map```的function type執行對應的execution，```begin```很好用
- bug沒修，要用的話自己看著辦