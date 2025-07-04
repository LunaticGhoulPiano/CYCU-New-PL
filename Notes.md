# 上課筆記
- 考試會從這裡面出題
## Midterm
### Q&A
#### 1
##### Q:
為何要BNF->EBNF?
##### A:
避免left recursive（無窮遞迴）
#### 2
##### Q:
為何scanf後都沒再動, array最後印出來卻跟原先要讀進來的內容不同?
##### A:
因可能發生別的變數index out of range, 多出來的內容被蓋到屬於此array的地方
#### 3
##### Q:
為何現在不可在scanf時把&拿掉?
##### A:
如果原先int a = 0的話, 讀入拿掉&就會把數字放到0這個位置 => 有可能會直接去使用到系統區域, 或不可存取的區域, 因此拿掉&就相當於直接存取的概念
#### 4
##### Q:
程式碼有何問題 ?
```C
struct Student {
    int score;
    struct Student *next;
} ; // struct Student

struct Student *head = NULL, *temp = NULL;
int score = 0;

scanf("%d", &score);
while (score !- -99999) {
    temp = new Student;
    temp->next = NULL;
    temp->score = score;
    head = temp;
    scanf("%d", &score);
}
```
##### A:
這可能會有memory leak的問題
因為head一直指向新生成的空間, 而之前生成的空間都沒有任何指標紀錄, 導致沒有人能找到過去生成的內容, 因此生成的空間會永遠存在而無法回收記憶體
(無法單靠garbage collection處理) 
**structt的結構宣告不站和記憶體空間**
#### 5
##### Q:
程式碼有何問題 ? (Runtime Error)
```C
struct Student {
    Str30 name;
    int id;
    struct Student *next;
};

typedef Student * StudentPtr;
Str30 inputStr;
StudentPtr newPtr;
newPtr = new Student;

scanf("%s", inputStr);
while (strcmp(inputStr, "-1")) {
    strcpy(newPtr->name, inputStr);
    printf("%s\n", newPtr->name);
    scanf("%d", &newPtr->id);
    printf("%d\n", newPtr->id);
    newPtr = newPtr->next;
    scanf("%s", inputStr);
} // while
```
##### A:
因倒數第三行直接指向next會發生找不到值的狀況
#### 6
##### Q:
程式碼有何問題 ? (Compile Error)
```C
#include <stdio.h>

int main() {
    int num[5] = {1, 2};
    printf("num: %p\n", num);
    printf("*num: %d\n", *num);
    printf("num++: %p\n", num++);

    return 0;
}
```
##### A:
第一行印出整個陣列的第一個位置
第二行印出整個陣列的第一個位置裡的值
第三行把num值改變違反常數在執行中被改變的規則 
#### 7
##### Q:
pointer + integer的意思是什麼？
##### A:
pointer目前所指的位置往前4(一格大小，即int大小) * b 個bytes（根據p的型別決定移動一格的大小）
#### 8
##### Q:
變數被宣告(int i)時, 是在哪個階段被配置空間的 ? ( compile階段 or runtime階段)
##### A:
compile階段, compiler在作文法分析時, 同時程式會告訴compiler需要預留空間(但不代表這塊空間就永遠是屬於該變數的), 但若在之後該空間被其他東西佔走後, 在真正需要用到該變數(i)時, 就要重新分配空間, 也就是真正進入到runtime階段時, 才會確定該變數會使用到哪一塊空間, 此方式為Dynamic Allocation 

(除非加上static(靜態)宣告, 才會在告訴compiler需要預留空間時, 就確定把這塊空間只留給該變數(i)使用, 但很有可能一直沒有去使用該空間(變數 i 沒有用到)而造成空間上的浪費, 此方式為Static Allocation)
#### 9
##### Q:
```.exe```檔如何做分配
##### A:
1. code
2. data (已初始化之static / global 變數)
3. block started by symbol (未做初始化之static / global 變數)
4. new() (heap⬇)
|
|                               中間這塊空間就是使用者可以使用的部分空間
|
5. function call (stack⬆)
⚠️ 箭頭為該空間的生長方向 
#### 10
##### Q:
Why array ought to be passed by reference? (為什麼陣列應該以「Pass by Reference」方式傳遞，而不是整個陣列傳遞？)
##### A:
主要是由於記憶體管理與效能考量, 以及 C/C++ 語言設計上的特性, C 語言的function預設是「Pass by Value」, 這表示function的參數會建立一份新的副本, 這對陣列來說會帶來幾個問題 :
1. 沒有效率 : 陣列通常很大, 複製成本太高(在stack上要長出的空間會根據傳過來的參數大小而有所影響，也就是可能會佔太多空間) , 並且在 C/C++ 中, function參數無法以值傳遞的方式接收整個陣列, 因當陣列名稱作為function參數時, 它會自動退化（decay）為指標, 也就是說 arr[1000] 其實就相當於 int* arr , 因而不支援直接複製整個陣列作為函數參數
2. 陣列名稱「本質上就是指標」=>「類似傳Reference」的方式傳遞
3. 真的想要「傳Value」該怎搞？ => 需要在function內手動複製它
#### 11
##### Q:
Why we should not return an array, but we can return, e.g. char p = new char [100] ; (為什麼不能回傳陣列, 但可以回傳指標（e.g. char* p = new char[100];）？)
##### A:
主要與記憶體管理與變數的生命週期有關
```C
#include <stdio.h>

int *getArray() {
    int arr[5] = {1, 2, 3, 4, 5}; // local variable "arr" in stack
    return arr; // error: return一個即將被釋放的記憶體位址
}

int main() {
    int *p = getArray(); // p會指向無效記憶體
    printf("%d\n", p[0]); // 未定義行為（可能當機/垃圾值）
    return 0;
}
```
1. 陣列在function內部是區域變數, 會在function執行結束時被銷毀 : 在function內部宣告一個陣列時, 該陣列會被分配在 stack 記憶體中. 屬於區域變數,  因此當function執行結束後，區域變數的記憶體會被自動釋放, 導致該陣列的記憶體無效, 也就是還沒回傳回去就已經被消失了(會造成未定義行為)
2. 正確方法 1：回傳動態分配的記憶體（Heap）, 如果記憶體是動態分配的（malloc() or new()）, 它就不會在function執行結束後被自動釋放, 這樣外部程式仍然可以使用該記憶體, 需要手動 free() 或 delete[] 來避免記憶體洩漏
3. 正確方法 2 : 使用 static 宣告陣列
4. new char[100]的作法是建立在heap上, 所以不會因function return的時後就被消失, 而是要手動清除

### 觀念
- 指標變數
    *b就是b這個位置裡的值去找出相對位置裡的值
    b 就是這個指標變數裡存的值
    &b 就是這個指標變數在記憶體中的位置
- 正交性
    primitive不多 = 基本資料型態（語法）不多
    conbine不多 = 因能用到的工具不多, 所以拼湊部方式就不多
    任意複雜度都可組裝 = 語法之間可以交互使用，組合出來都可trace
    => 因此文法不會太多or雜
- 配置方法 Auto / Controlled
    Auto : call function時, 此時記憶體就會自動配置空間放在stack上
    Controlled : malloc() / new() => 使用者主動去要空間放在heap上 
    大多數programming language在於一般沒有用static宣告的變數就會被放在stack上, 因為那些變數就是被放在function裡面(e.g. 放在main function就是放在一個function內), 因此而隨著call function時被配置, 除非是全域變數(global)才會放在heap上, 因那是程式的一開始就會存在的變數
- OS會每隔一段時間進行garbage collection(處理破碎空間的問題)
- Activation Record (放在stack上)
    在call function時, 系統會在stack上自動長出一個activaion record, 內容包含 : 
    1. Variable (該function的local變數 / pass parameter別人傳給該function的變數)
    2. Return address (要return回呼叫該function的位置)
    3. Next (指向該function去呼叫的function)
    4. Previous (指向呼叫該function的function)
- Scope
    def. : 變數的勢力範圍, 當使用某變數, 判斷該變數在哪裡被宣告
    白話 : 找該變數被哪個function包住, 找不到就一層一層向外找

    Static : 變數放在的結構，可以看出哪邊可以用到它（依照程式結構決定變數的勢力範圍）
    ```
    |--- main() {
    | int i, j;
    | ......
    | |--- sub() {
    | | int i;
    | | printf("%d%d", i, j);
    | | ......
    | |--- } // sub()
    | ......
    | printf("%d%d", i, j);
    | ......
    |--- } // main()
    ```
    Dynamic : 從當前的function 去找是否有該變數，沒有就往stack下找 （依照function被呼叫的次序, i.e. stack）
    ```
    printf("%d%d", i, j);

    |------|
    | sub  | -> Local definition: i
    | sub  | -> Local definition: i
    | main | -> Local definition: i, j
    |------|

    |--- AA() {
    | |--- BB() {
    | | |--- CC() {          |----|
    | | | ...                | DD |
    | | | use i;             | CC |
    | | | ...                | BB |
    | | |--- } // CC()       | AA |
    | | |--- DD() {          |----|
    | | | ...
    | | | use i;
    | | | ...
    | | |--- } // DD()
    | |--- }
    |--- } // AA()
    ```
- Static
    ```C++
    #include <iostream>
    using namespace std;

    class Example {
        public:
            static int count; // 宣告static成員變數
            Example() { // 每次創建物件，count都會增加
                count++;
            }
    };

    int Example::count = 0; // 必須在類別外初始化
    
    int main() {
        Example obj1, obj2, obj3;
        cout << "Number of objects created: " << Example::count << endl; // 3
        return 0;
    }
    ```
    1. static global variable : 可見範圍僅限於該程式檔案，不能被其他程式檔案存取, 可防止其他 .c 檔案中的全域變數名稱衝突, 也就是提高封裝性
    2. static local variable : 只能在該function內存取（作用域不變）, 生命週期會維持到程式執行結束, 不會在function return時釋放空間（不像普通區域變數）, 而且初始化只執行一次, 之後該變數的值就會被保留,  因此可以在function多次呼叫之間記住變數的值(e.g. 用於計數), 也就避免了每次呼叫function時重置變數, 提高效率
    3. static data member : 屬於整個類別, 而非特定物件（instance）, 因此所有物件共享同一個 static 變數, 改變其值會影響所有物件, 所以必須在類別外單獨定義, 否則會導致未定義參考錯誤, 此方式允許所有類別物件共用同一個變數, 節省記憶體, 並且可以作為計數器, 記錄類別已創建多少個物件
- Operator Overloading (運算子多載)
    def. 賦予 +, -, * , / 新的意義
    白話 : 除了原先有的功能外, 再附加給該符號新的功能(e.g. '+' 原先只有把數字相加功能, 而overloading 可以附加把字串接起來的功能)

    &a => '&' 是位置運算子, 也就是變數a在記憶體中的位置
    a & b => '&' 是位元運算子, 做a和b的AND運算

    由上述例子可知Operator Overloading會造成可讀性降低, 以及編譯程式的偵錯能力降低, 因可能漏寫了某些東西, 但始終能過compile
- Function Overloading (函式多載)
    def. 每次給相同名稱function的參數 (個數/型別) 不同, 都能做處理
    白話 : 以cin為例, 可發現每次使用cin函式不需要多寫要cin幾個參數, 而且還能直接讀入不同型別的參數, 都不須額外做處理就能完成目的
- Regular Expression
    1. ```'.'``` => 可代替所有字元
    2. ```'*'``` => 可repeat 0 or 多次
    3. ```'+'``` => 可repeat 1 or 多次
    4. ```'\ escape'``` => 清除原先特殊意義使其成為純文字 (e.g. 在printf("")中如果要印 " 字元的話, 要先清除原先的意義才能印得出來)
    5. ```[abcd]``` or ```[a-d]``` => 只能選此區段間的內容
    6. ```^[abcd]``` or ```^[a-d]``` => 比較開頭 (開頭必須是abcd)
    7. ```[abcd]$``` => 比對結尾 (結尾必須是abcd)
- Extend Regular Expression
    1. ```'?'``` => 比對前一個or不比對
    2. ```'|'``` => or
    3. ```(...)``` => grouping
- Regular Expression實例解釋
    1. Variable
        - ```[a-zA-Z][a-zA-Z_0-9]* |  _[a-zA-Z][a-zA-Z_0-9]```
        - => 規則在```|```處分為前後兩個, 前面的規則是```[a-zA-Z]```為開頭字須為```a-z```或```A-Z```其中之一, 第二個字開始```[a-zA-Z_0-9]*```(可重複0次到多次)必須為```a-z```或```A-Z```或```_```或```0-9```其中之一, 如此才符合文法規則 (就像在命名變數的規則), 而後面的規則是如果以```_```開頭的話後面接著字的規則就如同前述
    2. Number
        - ```[0-9]+?[0-9]* | [0-9]*.[0-9]+```
        - => 規則在```|```處分為前後兩個, 前面的規則是開頭字必須為```0-9```其中之一, 第二個是```.```代表被清除意義後的```.```(也就是成為純小數點), 接下來的```?```是指比對前一個字 or 不比對 (相當於前面的```.```可有可無), 最後的```[0-9]*```就是可以還有0或多個數字, 而後面的規則是以被清除意義的```.```開頭後面必須要有1到多個數字才符合規則
- Readability / Writability / Reliability
    - Readability : 程式是否容易讓人理解內容
    - Writability : 分成兩個 => Abstraction (process) 只要知道它會做到甚麼事, 實際它怎麼做的我們不需要知道 / Abstraction (data) 不需要知道data怎麼被處理的, 如使用vector只需知道怎麼新增和刪除data, 不必知道data存進去的時候空間是否已滿(事實上可能是去產生更大空間把原先空間裡的data搬過去), 且它內部如何塞data的我們也不需要知道
    - Reliability : 程式發生問題時, 是否可很快知道問題在哪, 也包括type checking (由compiler做型別檢查, 如把 int 傳入只收 float 的function, 就會由compiler做轉換型別, 若compiler無法幫助做轉型就會跳出error message, 並要求你自己做轉型)以及exception handling 還有aliasing
    - *手動轉型(cast)例子 : (int) a
- 轉型 Trouble
    - int 是以二進位解讀
    - float 是以 IEEE754 解讀
    - 所以像在 fp = [float *]ip 手動轉型的方式會造成 fp 現在指向 a (0A00)這個記憶體位置, 但它仍然是 float*, 所以會在將 a 位置內的值取出時當作 float 解釋  (以IEEE754解讀), 因而造成印出來時和預想的數值不一樣
    - 所以直接以值之間做轉型才不會和以透過指標轉型的方式精準度差這麼多
    - ⚠️cast不是萬能的
- Exception Handling
    1. 預防錯誤擴散 : 發生錯誤時, 希望只發生在該function而不影響到其他地方
    2. 提供錯誤恢復機制 :  要知道如何recover
    3. 良好錯誤報告和診斷 : 要知道發生錯誤時, 如何做事後追查
    4. 強制錯誤處理 : 強制開發者要處理可能導致錯誤的狀況 (先把例外處理寫好)
    5. 分離錯誤處理邏輯 : 像是多寫一個 else , 發生錯誤時就進這個 else 暫時性做處理, 避免讓整個系統整個炸掉
- Aliasing
    ```C
    int final_score = 30;
    intPtr class1 = NULL, class2 = NULL;
    class1 = malloc(sizeof(int));
    class2 = class1;
    *class2 = 2;
    final_score = final_score * *class1;
    printf("final_score = %d\n", final_score);
    // ...
    free(class1);
    class1 = NULL;
    final_score = final_score * *class2;
    ```
    1. 難以追蹤和預測的行為 (e.g. class1和class2都指向同一空間, 兩個指標都能對該空間內容做修改, 很難追蹤到是誰做了內容修改)
    2. 誤用導致的錯誤 (e.g. class1可能把該空間free()掉了, 造成class2變成指向一個空指標, 可能要後續一直執行到class2才會發現錯誤)
    3. 測試和維護的困難 (e.g. 如果程式規模夠大就會很難測試錯誤並維護)
- Dereferencing
    - def. 以 *ptr 為例, 就是去取 ptr 存的記憶體位置裡面的值 , 又分成Explicit (外顯) 和 Implicit (不外顯)
    - Explicit : 以 * 去取值 (透過記憶體位置) 出來讓人一看就知道是透過甚麼方式在取值
    - Implicit :  如 student.score 看不出來是用甚麼方式取出 student 結構中的 score 的值的
- Language Design Trade offs
    - Reliability v.s. Cost of Execution : e.g. index range checking功能難以兼顧兩個特質, 如果interpreter不做此功能則Reliability下降, 但相對Cost of Execution就不會這麼高
    - Writability v.s. Reliability : e.g. pointer arithmetic以C語言來說可用pointer++來做memory access (Writability好處), 但此做法可能會導致memory leak (Reliability降低) ; 若以Java來說會強制以reference的方式, 不可直接做memory access (Writability較低), 但因有garbage collection / 繼承等OOP特性 (Reliability較高)
    - 總結 : 語言的設計都難以同時兼顧兩邊的特性
- Implementation methods
    1. Compiler v.s. Interpreter :
        - 效率 (執行層面) => Compiler較優 (執行.exe檔), 因Interpreter每次執行都要重跑
        - 平台 => Compiler 很依賴平台 (OS and Hardware), 而Interpreter則可跨平台 (因每次都是重新跑一次)
        - 彈性 => Compiler是一次將整個程式碼做執行, 而Interpreter則是逐行做, 有錯誤就停下來
    2. C preprocessor (e.g. meaning of #include) : 針對程式中有用到 # 的地方, 會在程式碼送入compiler前預先處理, 例如程式有include函式庫, C preprocessor就會去找出整個程式碼有用到該函式庫的函式去加入
    - ```<iostream>```是系統函式庫 / "AAA.h"是從當前目錄去找這個user函式庫
    3. Macro : 是指可以用 #define 來定義一段程式碼 (e.g. #define MAX(a,b) (a>b ? a:b) 意思是如果 a > b, 就回傳a, 否則回傳b ), 也是在送入compiler前讓preprocessor做預處理, 把程式碼中有寫到 MAX(a,b) 的部份去做定義內容的代換, 但這個預處理並不會做參數的型別檢查 (較容易有副作用)
    - 而 Function 執行時機點與之的差別是會等到 runtime 才執行, 且比較起來Macro速度會比較快, 因在.exe檔中, Macro的程式碼早已代換完成做執行, 而 Function 則是在呼叫到時, 才會開始在 stack 上長出空間做執行, 且執行完還要花時間把空間釋放掉, 也因此 Macro 程式碼會更耗空間 (因每個地方做完代換後可能程式內容會變更多行, 以空間換時間的概念) 
- Programming Environment
    - def. 抽象化概念讓使用者認為透過IDE就能讓程式執行, 實際上是由IDE來跟OS溝通以達成目的
    - C (for OS and Hardware) : 很依賴平台且要手動做記憶體管理
    - JAVA (JVM) : 平台依賴性低且有garbage collection的特性
## Final
### Q&A
#### 1
##### Q
Is it possible for a variable NOT to have a name?
##### A
透過指標或reference的方式去指向或參考, 存取的那塊空間本身是一個變數但是它沒有名字 (風險是指標或reference之後沒有再指向該空間, 那該空間就會持續存在而不會被回收)
#### 2
##### Q
Is it possible for a variable NOT to have an address? ( = does not exist)
##### A
在compile階段會針對程式要使用的的變數(非static)先個別預留空間可以做使用, 但還沒有分配確切的空間位置給它 or 把變數放在暫存器中
#### 3
##### Q
Is it possible for a variable  To have different addresses at different times?
##### A
Function呼叫, 根據被呼叫時, 才在stack上長出空間而具備記憶體位置, function中的區域變數隨著function在不同時間被呼叫時所配置的記憶體位置會有所不同 or 塞資料進vector中, 因vector需要更大空間時, 而會把內容複製過去新空間 
#### 4
##### Q
Is it possible for a variable  NOT to have a value?
##### A
一般變數宣告時若沒有賦予值的話IDE會給予一個預設的初始值(正常應該是一個亂數值) 
#### 5
##### Q
Can a variable be used without knowing its type?
##### A
可以, 也就是Dynamic Type Binding的狀況, 因會依照當下執行到的那行程式碼來決定變數的型別(也就是根據賦予的值來知道是什麼型別), 所以還沒執行到該行程式碼時, 是不需要知道那行的變數是屬於什麼型別
#### 6
##### Q
How is the type of a variable determined? (esp. dynamic type binding 5.4.2.2)
##### A
根據前一題, 就是看賦予的值來知道是什麼型別
#### 7
##### Q
What is the lifetime of a variable? global, local?
##### A
- Local: 執行區塊時建立，結束立即銷毀
- Global: 程式執行期間
- Static: 程式執行期間

### 觀念
- Call Back
    做法 : 把一個function 1當作參數傳入另一function 2, 並在之後某個時間點(function 2執行過程中)去呼叫（回呼）這個function 1的做法
    - Example 1
        ```JS
        function firstFunc(CB) {
            console.log('First Function');
            CB();
        }

        function secondFunc() {
            console.log('Second Function');
        }

        firstFunc(secondFunc);

        /*
        執行順序：
        First Function
        Second Function
        */
        ```
    - Example 2
        ```JS
        // ex2
        console.log('Start');
        setTimeout(() => {
            console.log('Inside Timeout');
        }, 3000);

        console.log('End');

        /*
        執行順序：
        Start
        End
        // 等三秒
        Inside Timeout
        */
        ```
- Describing Syntax
    - Lexeme : 未經處理過的input內容
    - Token : input內容經過處理區分後的結果
- Context Free Grammar
    - def. 每一條文法規則都與其他條文法互不影響, 不會有需要先經過其他條B or C or D文法才能導出A文法內容(單一條文法規則不會被先決條件限制的概念)
- Production (Rule)
    - def. 從BNF或EBNF之中抽出其中一條文法就屬於Production
    - e.g.
        ```EBNF
        <term> -> <term> '*' <factor> | <factor>
        ```
        含有兩個Production, 一個為
        ```EBNF
        <term> -> <term> '*' <factor>
        ```
        另一個為
        ```EBNF
        <term> -> <factor>
        ```
- Terminal & Non-Terminal
    - Terminal : 沒有被<>包起來的部分(亦是不可再分解的意思)
    - Non-Terminal : 文法中有被<>包起來的部分就屬於
- Start Symbol
    - def. 根據regular expression把input內容(lexeme)判斷是否可以轉成token
    - *參考之前針對regular expression所寫之筆記
- Syntactical Analyzer (Parser)
    - def. 將已轉換成一整串token的內容根據文法規則判斷是否符合文法
- Parse Tree
    - def. 每抓進來判斷文法的token所走的路徑
- Leftmost & Rightmost
    1. Leftmost : 例如```<term> -> <term> '*' <factor>```就是從最左邊的```<term>```開始對token慢慢一個一個往右過去判斷而不是直接從右邊的```<factor>```開始判斷
     2. Rightmost : 和Leftmost判斷方式相反, 由最右邊的```<factor>```開始對token慢慢一個一個往左過去判斷
    ```EBNF
    <assign> => ID '=' <exp>
    => ID '=' <exp> '+' <term>
    => ID '=' <term> '+' <term>
    => ID '=' <factor> '+' <term>
    => ID '=' ID '+' <term>
    => ID '=' ID '+' <factor>
    => ID '=' ID '+' ID
    ```
- Derivation
    - 將token串根據文法規則去做判斷並逐步展開的過程, 如下圖的整個過程就是, 一次僅將一個non-terminal symbol做展開(代換), 直到都展開為terminal symbol
- Ambiguity (混淆不清)
    - 根據文法規則可以推導出2顆以上parse tree的狀況
- Ambiguous Grammar
    - e.g. Dangling Else Problem
        - Q:
        ```
        if (a) then if (b) then do_something() else do_otherthing()
        =>此時的else do_otherthing()到底要屬於if (a)不成立該做的還是if (b)不成立該做的 ?
        ```
        A:
        ```
        要是沒有做出區分, 則Compiler無法判斷出什麼情況下else該搭配if (a)還是if (b)
        ```

    - 前人解決辦法 :
        - ALGO60 : 使用Begin和End
        - ALGO68 : 使用IF和FI
        - PASCAL : 找else最相近的if搭配
        - ADA : 使用IF和ENDIF
- Operator Precedence
    - Left Associative : 由左至右的運算, 如普通運算(a + b)
    - Right Associative : 由右至左的運算, 如次方計算 / 運算式(a = b + c會先算出b+c再把結果給a)
    - e.g. a = b = c的執行細節為先把(b = c)這個運算式的結果得到後, 再把該結果給a, 要注意不是直接去把b的值取出來給a
- State Diagram (狀態圖)
    - 表達一個系統的狀態, 如input內容讀進來後是經過如何的各種處理而得到結果(可以照順序畫成一個一個狀態), 通常在開發前的初步設計會用到, 從圖就能看出明確的各個功能運作的順序
- Finite Automata
    - 更加凸顯系統內的一個狀態, 用來描述例如「一個系統如何根據輸入字元, 一步步改變內部狀態」, 簡單來說就是一種只有「有限個狀態」的機器, 根據輸入字元依規則從一個狀態轉移到另一個狀態, 最後決定是否接受這個輸入
    - 也就是狀態圖其中一個階段的圖中, 該圖細部的每一個條件處理狀態, 可能確認a後才能做b, 並且確認完b, 才確定此一階段狀態圖的result
- Recursive Decent Parsing
    - 不可以有left recursive, 且要注意文法規則中, 不可以有任意兩條規則的第一個token規則相同(交集), 否則遇到時會不知道該走哪一條文法
- Variable
    - 由meomory cell(可想像成memory是由一格一格組成的)的集合來儲存資訊
    1. 由變數的角度來看data abstraction : 對使用者而言, 只是寫了一行變數宣告而已, 但其實IDE會把程式碼轉換成機器碼去跟OS要空間來分配給變數, 這部分是如何做到的使用者並不需要知道
    2. Six-Tuple of attributes : name (使用者賦予的變數名) / address (變數的記憶體位置) /  type (資料型別, 決定變數值的範圍以及可作用在該變數之運算, 如long long 範圍更大還有不會在char型別之間用 ' + ' 來做運算) / value (變數關聯記憶體之內容) /  lifetime (變數擁有記憶體的時間, 如區域變數的時間就是從被定義到function結束) / scope (在程式中可以存取某變數的有效範圍, 如定義在function中的話, 只要離開function時就不在有效範圍內了)
    - *同一個名稱在不同的地方或不同的時間可能對應到不同的變數 : 指的是在不同function裡如果有被命名到相同名字的變數, 則實際上在記憶體中是對應到不同個變數
    - *別名(Aliases) : 不同的變數名稱引用同一個記憶體位址, 可發揮共用和節省空間以及高效率存取陣列內容的效果, 但相對造成可靠度低
- Binding (綁定)
    - 一個程式實體(Program entity)與其屬性發生關聯(association), 程式實體可從小(一個變數)到大(一整支程式)都算是一個實體
    - e.g. int x : 讓x跟int的屬性綁定關係
    - 以下為Binding Time時機 : 
        1. 設計：binding可能的型別( x = x + 10), 由開發者決定x要綁定的型別
        2. 實作：binding 用何種硬體實作, 可想成如用加法器來執行程式中的加法
        3. 編譯：binding 型別by Programmer(在設計階段決定後, 才在編譯時把型別和變數綁定) or Compiler(有些語言可以動態調整型別, 如交給編譯器依據當下來決定型別而不是先在設計階段時決定)
        4. 載入：binding specific memory location, obj 載入到記憶體(合成.exe檔時先決定要把程式預設放在什麼位置, 不過這還不是實際上要擺進去記憶體時的位置)
        5. 連結：binding memory address for external variable (等載入完後, 針對外部變數, 在linking時才確定各變數的位置)
        6. Call Function：local definition (function中的區域變數原先在設計階段決定型別, 但實際上是在被呼叫時才做綁定), pass parameters (預設要傳入哪幾個參數到function裡, 當呼叫function時傳入的參數才在此時做綁定型別和值等) etc.
        7. Any time：binding any value (動態綁定)
        - *不同語言的binding time不盡相同
- Static Binding
    - 程式未執行前就已經先把型別決定(綁定)好, 也表示變數型別已固定, 如全域變數或靜態變數
- Dynamic Binding
    - 程式在執行的當下型別被不斷做變動, 根據當下變數後面被賦予的內容來動態決定型別(不用特別寫要讓變數轉型的程式)
    - *利用static binding可以早就決定好型別, 因此用該變數在做運算時, 可以確定是用正確的型別, 代表程式也會較易讀, trace code會容易了解變數屬於的型別, 而避免使用dynamic binding可能會造成還要多做型別判斷來確定變數是正確使用的狀態, 在trace code時會難以當下看到該變數就確定是屬於什麼型別
- Type Binding
    - 綁定型別就相當於綁定變數的大小(byte)和範圍(value)
    - 例子 : typedef 用來定義一個使用者更好看的懂的型別
- Static Type Binding
    - def. 在程式執行以前就已經決定了綁定的型別
    1. 分成外顯式(explicit)和內隱式(implicit)宣告, 外顯為明確寫出型別, 內隱為不用事先宣告型別, 會根據當下給予什麼樣的值來賦予型別, 被賦予型別後就不可再被調整, 也就是在後續的程式中, 該變數都會維持被賦予的型別不會再更動
    2. 內隱式的優點為不用事先決定好型別(彈性較高), 只是缺點就是降低了程式的可讀性和可靠性(可能在程式中要用到某個變數但不小心拼錯了, 會被認為是在宣告新的變數而不是error, 此外也難以debug)還有需額外空間來記住該變數是什麼型別以及在後續trace code時可能會有錯誤, 例如變數a = 5(被賦予interger型別), 在後續想要做a = a + 1.5就會因a是屬於int而運算完只存了6這個值而不是6.5
    3. 內隱式宣告需保留空間上的彈性, 要預留空間以防變數被賦予值時突然需大量空間(如被賦予為struct), 造成空間不夠使用
- Dynamic Type Binding
    - 程式語言不用事先定義好型別, 在過程中依據當下給予的值來轉換型別, 因此每次執行到某行定義時, 都要先判斷變數是屬於什麼類別
    1. Dynamic較適合Interpreter, 因執行上為一行一行執行, 會根據當下執行到的那行來判斷當時變數的狀態, 也就是目前變數後方被給予的內容就是當下該變數會屬於的型別, 不必管後面程式會怎麼變動該變數, 但對於Compiler而言會看整體程式的正確性, 若在Compiler用Dynamic Type Binding就會導致Compiler的型別檢查失去意義, 因型別可以不斷被調整, 所以Compiler看到變數都只能認為是合理使用
    2. Dynamic能增加程式的彈性, 但相對降低了程式的可讀性和可靠性
- Binding Problems
    1. Spell error : 因Dynamic不用事先宣告, 所以要使用已被宣告過的某變數時不小心拼錯該變數名, 就會在當下被視為是在做宣告一個新的變數, 而這對Compiler而言也會是合理的, 因此原變數不但沒被使用到還反而多宣告了一個新變數, 而這也會導致很難找出錯誤之處(debug難度高)
    2. Declaration of b :  用IDE開發的優點是可以做compile和做執行並且幫忙跟機器打交道(把程式翻成機器碼去做執行), 而用text editor(文字編輯器)沒有compiler而是以純文字做編輯, 之後再透過GCC做compile, 相對IDE在找錯誤上較不便
- Storage binding
    1. Allocation為佔有空間的行為,deallocation為把空間回收
    2. static variable : 變數會保留在heap中不會被清掉或重置, 優點為在compiler階段就會分配好空間, 但缺點為若是不常使用到該變數就會很浪費空間, 且若是有每次使用都需要重置的狀況就無法支援遞迴
        - global variables : 只能在該檔案使用不可跨檔案
        - history-sensitive variables ( local variables) : 可看到最後一次被使用後的狀態
    3. stack-dynamic variable : 放在stack上動態產生的變數, 也就是區域變數, 當function進入或離開stack中, 變數就會跟著進出, 優點是function被呼叫時才會產生並佔有記憶體, 且可以支援遞迴, 但缺點為function若反覆被呼叫的話就會需要一直做配置/釋放記憶體空間, 且不會保留變數上一次最後被執行的狀態
    4. explicit heap-dynamic variable : 顯性, 在heap上動態產生的變數, 例如使用new的方式產生的變數, 操作方式為透過pointer或reference, 而優點為可以自己決定要何時手動要配置空間, 但相對缺點為無法直接針對這塊空間, 所以若沒有釋放掉記憶體就把pointer指向別處, 這塊記憶體就會有memory leak的問題
    5. implicit heap-dynamic variable : 隱性, 在heap上動態產生的變數, 不需寫出來須要配置空間, 也就是優點是會自動分配空間, 例如str = “Hi” + ‘ ‘ + “ICE” ; (使用 '=' 的方式),  相對也會自動釋放空間, 但缺點為無法手動控制這塊空間, 且實作上的成本還有多了要判斷“Hi” + ‘ ‘ + “ICE”這部分需要多少空間來存放
- Scope
    - def. 一個變數所能執行的範圍, 根據敘述式來決定所能夠執行的範圍
    - *Referencing environment : 在程式碼裡, 隨機指定執行到的某一行, 當時可以參照到的變數有哪些 (區域變數/外部全域變數/外部檔案的變數)
    1. Static scoping : 查找變數是根據程式本身被哪些程式所包住 (程式的架構) 來做決定範圍, 若往外層找始終找不到就代表該變數未被定義
    - *hole in scope : 全域變數和區域變數發生衝突 (命名相同) 時, 在function內所用到的變數是來自於它的區域變數
    2. Dynamic scoping : 根據stack堆疊的先後順序來決定範圍, 單純看誰呼叫誰的順序, 一直往下層找直到找到有宣告該變數的地方就是了 (而不是看整個結構中, 當前stack上的function中用到的該變數是利用來自何處定義的值)
    - *package-scoped : 表示變數僅能在該package (檔案) 中使用
    - *要清晰知道global, local, “static” global/local, public/private/protected/package-scoped等加上修飾字會對變數的scope造成什麼影響
- Global Scope
    1. 解決全域和區域變數衝突的狀況 (Python) : 例如當全域裡的一個條件式為成立才會定義變數A, 但在緊接著條件式結束後會執行印A的行為就可能會出錯, 因先前條件式要是發生不成立, 變數A就不會被定義到, 當然這時就會印不出變數A的值, 因此變數A應該要在先前就先定義好而不是在區域中才做定義
    2. 在function中要用到外部 (全域) 變數A的狀況 (Python) : 要在function中使用變數A前先加上一行global A, 否則沒加的話預設會認為是要取區域變數A來做使用, 因此global A可以用來解決區域和全域變數名稱相同時, 使用上衝突的問題
    3. “visible” and “hidden” ( C/C++) : 不是指直接在變數A前加上visible或hidden修飾字, 而是指全域變數和區域變數衝突時, 會有其中一個被 "隱藏起來" 的效果 , 使狀況成為只有一個變數為可視的,  而在function中執行的狀況是全域變數A成為hidden而區域變數A成為visible, 若是不想使用區域變數A而是想要使用全域變數A的狀況下, 只要在變數前加上修飾字 (::A) 就可以成功使用到全域變數A
    4. “visible” and “hidden” (JAVA) : 在function中執行的狀況是全域變數A成為hidden而區域變數A成為visible, 若是不想使用區域變數A而是想要使用全域變數A的狀況下, 只要在變數前加上修飾字 (super A) 就可以成功使用到全域變數A (概念是多個子class繼承父class時, 各個子class分別要用到父class的變數之情況)
- C/C++ declarations vs. definitions
    - declarations (宣告) : 告訴compiler未來會用到什麼工具, 當compiler在做文法檢查時, 就會去程式找是否有對應的定義,  以 int x 而言就是一種宣告 (不配置記憶體), 放在如.h檔裡 (像是工具列表), 平常只要include這個.h檔就可以去使用裡面的工具, 讓compiler可以透過include進來的清單去找到某某工具再去把該工具的主體取出來使用
    - definitions (定義) : 在.cpp主體中 (會配置記憶體), compiler會在整個程式找是否有對應的定義, 找不到就會去include進來的工具列表找
    - use of ‘extern’ : 不同程式檔要merge時, 希望有可以讓每個檔案都能共同使用的變數 (合成一個.exe檔時可以共用), 所以在變數加上extern (extern int i) 告訴compiler這個變數是在其他檔案裡, 在文法檢查時把該變數視為存在
    - *若某個檔案裡不小心把變數多加上了static的話就會變成在compile時, 其他檔案會找不到該變數定義
    - *若每個檔案都各自定義了相同名字的變數, 在合併成.exe檔時就會出問題, 會被認為是在做重複定義, 而解決方式就是各自加上static, 就會變成都僅在各自的檔案內做使用
- Named Constants
    - define: 有名稱的常數用數值來取代, 增加維護的方便性, 且不須表明出型別, 當compile時透過C-processor來把程式碼中所有該名稱的變數用define的數值去取代
    - const : 宣告一個常數, 會表明出變數型別,和define一樣只要值固定下來後, 就不可再被修改
    - *兩者差異為define不會佔有記憶體位置, 而const則會佔有記憶體且兩者都不可被修改, 但const相對較安全 (因有表明型別)
    - *define可能會有命名衝突, 因也是可以讓多個檔案共用, 若兩個檔案都定義到相同名稱的變數就會在連結時發生錯誤
    - *‘readonly’ dynamic binding of value (for C#) : 常數看似可以做改變 (public readonlyint X;) , 只要一旦綁定了一個值 (X = a + 1;) 後, 就依然不可再被改變值
- Data Types
    - Variables : 使用前做宣告 (方便了解和追查變數在程式中的型別是什麼或是哪邊有用錯), 例如 (int a = 3 *b;) 就可以明顯看出a的型別而不需先確定b的型別才能知道a的, 為了方便除錯, 盡可能以區域變數做使用而不仰賴static或全域變數, 因變數一旦產生就會讓全部程式做使用, 難以保證變數是否會有失控的可能 ; 提到變數名稱要能想的到是在做什麼行為 ; 開發環境 (所用IDE), 可以透過IDE來追查變數在什麼地方被使用
    - Type = value + operations : 變數在被賦予型別時, 就相當於限縮了value的範圍 (最小/最大值), 以及限縮了運用方式 (operations), 如int就只能用int的operation
    - Many data types : 早期程式語言有很多型別, 為了因應各種情況去開發更多新型別, 難以界定之處就是不知道該給多少空間, 而導致可能會開發太多新資料型別, 現代則是因應各種需求去產生不同的結構 or class來解決儲存的問題
    - Coercion : 內隱式型別轉換, 由compiler在編譯時轉, 例如傳int到只收float的function
    - Cast : 外顯式型別轉換, 由使用者明確寫出要轉的型別, 例如(int) a, 但要注意結果可能會不如預期 or 精準度誤差
- Primitive data type
    - def. 基本資料型別, 變數被宣告後會有固定空間大小且可以直接從變數所在位置把值取出就屬之, 若如pointer是透過間接存取的方式就不屬於基本資料型別
    - *以 Java 而言, String 並不是基本資料型別, 它是物件（reference type）, 且 String 常值會存放在 String Pool 由變數指向
- Character String
    1. 是Primitive嗎 : 大部分語言都不是, 因string光在空間大小就無法在宣告確定
    2. C/C++ : 通常是用字元陣列來存, 字串結尾以 \0作為結束, 因此通常宣告大小應該要多宣告一格存放 \0
    3. BASIC : 把string視為基本資料型別, 在變數名稱後加上$內定為字串變數
    4. String Length : 根據語言不同而特性有所不同, 若是static string length就是在程式執行前就確定字串長度, 而以C/C++而言是屬於limited dynamic string length, 可大範圍擴窗字元陣列, 但有上限
- User-defined ordinal types (enum)
    1. 和 #dhefine 比較 : enum是把各個常數包在一起, 而 define 則是散在外部 ; enum有型別, 而 define 則沒有
2. 型別安全 : 因enum有型別, 所以若以enum作為 function 接收的變數時, 就會去檢查傳進來的參數是否屬於enum中的其中一員, 反則就會在編譯時報錯
3. Scope控制 : 只要各個enum的名稱不同, 即使內部存放元素的名字相同也不會造成衝突 ; 若是 define 則是會在各個檔案合成時, 只要發現有不同檔案 define 了相同名字的內容就會發生衝突 (程式碼也不知道該使用哪一個檔案的)
- Array
    1. Type of subscript : 陣列的 subscript 指的是索引 (index), 可以用來表達要存取的位置或是要宣告的陣列大小
    2. Associative array (關聯式陣列) : 以 key + value 來儲存, 每個 key 值會對應到不同 value, 所以可以用 key 值來找到 value , 而不是以 index 來找到 value
    3. Index out of range : 由 IDE 來決定要不要加入此檢查機制, 若不檢查則執行效率高, 但相對的可靠度降低
- Array的種類
    1. static array : 在compile階段就決定記憶體位置和大小
    2. fixed stack-dynamic array : 動態產生固定大小的陣列, 放在 stack 上, 例如在 function 內用到的 array, 跟著 function 被呼叫和結束而產生和消失
        ```C
        void F(int a) {
            int arr[5];
        }
        ```
    3. stack-dynamic array : 動態產生不固定大小的陣列, 放在 stack 上, 例如在  function 內用到的 array, 跟著 function 被呼叫和結束而產生和消失
        ```C
        void F( int a ) {
            int arr[a];
        }
        ```
    4. fixed heap dynamic : 動態產生固定大小的陣列, 放在 heap 上, 例如以 new() 來產生, 需要透過自己手動來做刪除, 否則會一直存在到程式結束為止, 所以有機會造成 memory leak
        ```C
        int size = 10;
        int[] arr = new int[size];
        ```
    5. heap-dynamic array : 動態產生不固定大小的陣列, 放在 heap 上, 例如以 new() 來產生, 需要透過自己手動來做刪除, 否則會一直存在到程式結束為止, 所以有機會造成 memory leak
        ```C
        ArrayList<Integer> list = new ArrayList<>();
        list.add(1);
        list.add(2);
        list.add(3);
        ```
- Array Operation
    - Slice (切片) : 以取出陣列局部部分的方式來做使用, 例如以Python而言
        ```C
        arr = [10, 20, 30, 40, 50]
        print(arr[1:4])  # [20, 30, 40]
        ```
        - 只包含 start index, 但不包含 end index
- Jagged Array
    - def. 讓陣列中的每一格大小都可以不同
        ```C
        jagged = [ [1, 2], [3, 4, 5], [6, 7, 8, 9] ]
        ```
- Computation of element address
    1. 引用陣列位置方式 : 如以指標常數 a 為例子, 利用 a[3] 的方式也能用 *(a+3) 來存取到相同陣列中的位置
    2. Row Major 和 Column Major : 以行或列開始數, 同一個陣列中的位置所會被讀到的順序會不同, 所以資料排列在記憶體的方式也會不同
- Dangling pointer
    1. Tombstones : 由此來負責產生空間 (不是由指標來產生), 指標僅用來記錄Tombstones的位置, 當要存取被刪掉的空間, 就會透過Tombstones發現該空間已不存在而去把指向此Tombstones的指標設為NULL
    2. Locks-and-keys : 指標產生空間時, 同時也產生一個lock值在該空間中, 而這個lock值也會記錄在指標上 (key值), 每當要存取時就要先檢查指標的key值是否對得上該空間的lock值, 只要對不上就表示無法存取該空間 (代表可能該空間已被釋放)
- Lost heap-dynamic variables
    - 動態產生的 variable 並放在 heap 上 (用new / malloc), 若指向他的指標 or Reference 在此空間未被釋放前就改指向其他地方, 就會導致此空間無人指向而成為 garbage
- Heap Management
    - def. garbage collection 機制 (為了解決 memory leak)
    1. Mark sweep : 在空間分配出去時, 去記錄哪個指標去指向了該空間 (做 mark 的概念), 而在每隔一段時間執行 garbage collection 時, 去檢查該表中每個空間是否有指標指向 (做 mark ), 若沒有就代表無人使用
    2. Reference count : 在空間分配出去時, 紀錄指向該空間的指標數量, 而在每隔一段時間執行 garbage collection 時, 去檢查指向該空間的指標數量是否為 0, 如果為 0 就代表目前無人指向, 可以做空間釋放