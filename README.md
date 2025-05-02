# CYCU-New-PL (中原資工程式語言project - OurScheme)
## 修課相關
- 本人修大四下畢業班，夏老大（夏延德）授課，計分方式與大三不同
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
- 支援```C/C++```, ```Java```, ```Python```
- 平時分數計算：每堂課會有4分
    1. 出席分
    2. So so
    3. Ok
    4. Great
    0. Not good
- Project分數到80可以不用出席

## What is OurScheme?
- 它是中原資工大三下的**程式語言**課程Project，會有```OurC```與```OurScheme```兩種每年輪替，OurScheme是比較簡單的。
- ```OurC```要寫簡化版的```C```compiler, ```OurScheme```要寫簡化版的```Scheme```interpreter（一個使用Python實作的Scheme interpreter[見此](https://github.com/vladimirfomene/scheme-interpreter)，線上IDE[請點此](https://www.jdoodle.com/execute-scheme-online)）
- [Scheme](https://zh.wikipedia.org/zh-tw/Scheme)是[LISP](https://zh.wikipedia.org/wiki/LISP)的其中一種方言([dialect](https://en.wikipedia.org/wiki/Programming_language#Dialects,_flavors_and_implementations))，而```OurScheme```是此門課程將```Scheme```簡化後的方言（同理，```OurC```也是```C```的簡化方言）。
- 詳細實現內容請見[./AboutProject底下的OurSchemeProj{1/2/3/4}-UTF-8.txt](./AboutProject/)，現在新版的測試系統不會以```HowToWriteOurScheme.doc```為準，例如```ERROR (level of CLEAN-ENVIRONMENT) / ERROR (level of DEFINE) / ERROR (level of EXIT)```就不需實作。

## Commands
- Windows Powershell:
    - Warning:
        - 不要用cmd，也不要用```.bat```撰寫單元測試腳本，會有CRLF問題（你的程式輸出檔有時會在最後被插入換行，有時又有幾個```<S-exp>```的output被吃掉）
    - 編譯：
        ```powershell
        # g++ {程式檔本體} -std={C++版本} -o {編譯輸出檔路徑}
        g++ ./main.cpp -std=c++2a -o ./main # current progress
        g++ ./CompleteCodeByProjects./project1.cpp -std=c++2a -o ./project1 # complete project1
        # ...其餘同理
        ```
    - 執行：
        - Interactive I/O:
            ```powershell
            # {執行檔路徑}
            ./main.exe # current progress
            ./project1.exe # complete project1
            # ...其餘同理
            ```
        - Batch I/O (acceptance tests):
            - 所有的input檔案以```.in```為副檔名，正確output檔案以```.out```為副檔名，自己的程式輸出我習慣以```.bug```為副檔名
            1. Single test case: 僅測試單一指定測資
                ```powershell
                # 以project1的test1.in為例
                # Get-Content {輸入檔路徑} | {執行檔路徑} > {輸出檔路徑}
                Get-Content ./self_tests./project1./test1.in | ./main.exe > ./self_tests./project1./main_test1.bug
                Get-Content ./self_tests./project1./test1.in | ./project1.exe > ./self_tests./project1./project1_test1.bug
                # 執行完自行與test1.out比對
                ```
            2. Multiple test cases: 測試整個project中的所有測資（以project1為例）
                - 請見```run_mainBatchIO.ps1```, ```run_project1BatchIO.ps1```, ```run_project2BatchIO.ps1```等powershell腳本
                - 因為main.cpp是最新進度，所以請在```run_mainBatchIO.ps1```的```$testDir = "./self_tests/project2" # change the number of your current-working project here```修改你要測試的project
                - 其餘```.ps1```腳本是針對特定project的測試，會使用```{指定project}.cpp```去測試
                - 執行腳本：
                    ```powershell
                    # {powershell腳本路徑}
                    ./run_mainBatchIO.ps1 # current progress
                    ./run_project1BatchIO.ps1 # complete project1
                    # ...其餘同理
                    ```
                - 它會先編譯```{指定程式本體}.cpp```為```{指定執行檔}.exe```，再將```./self_tests./{指定project}```底下的```{所有測資}.in```送進你的程式執行檔，並將輸出寫入到```.self_tests./{指定project}./test_outputs_{去掉副檔名的指定程式名稱}./{所有測資}.bug```，再以```./self_tests./{指定project}./{所有測資}.out```和你的程式輸出之```.bug```比對
                - 若所有測資皆正確，會在terminal顯示訊息，並將```./self_tests./{指定project}./test_outputs_{去掉副檔名的指定程式名稱}```刪除
                - 若有測資錯誤，會在terminal顯示訊息，並將與```.out```不同的```.bug```輸出檔檔名寫入```./self_tests./{指定project}./test_outputs_{去掉副檔名的指定程式名稱}./error_test_cases.txt```，其餘正確的不會保留
                - 範例terminal輸出(main.cpp, project1)：
                    ```powershell
                    # 1. 正確輸出，完全沒有錯誤
                    [main.cpp]: All tests are correct!
                    # 2. test3和test7錯誤
                    ## 2-1. error_test_cases.txt會寫入test3.bug與test7.bug（各檔名自成一行）
                    ## 2-2. 在./self_tests./project1./test_outputs_main中會有三個檔案：error_test_cases.txt, test3.bug與test7.bug
                    [main.cpp]: test3.bug is not equal to test1.out.
                    [main.cpp]: test7.bug is not equal to test2.out.
                    [main.cpp]: 2 error case(s) are found!
                    ```
            - 如果想要順便學用powershell寫測試腳本，可以參考[Pester官方文檔](https://pester.dev/docs/quick-start)及[微軟的測試檔解釋](https://devblogs.microsoft.com/scripting/unit-testing-powershell-code-with-pester/)
    - 比對：
        - 在大二的**組合語言與嵌入式系統**課程使用的[WinMerge](https://winmerge.org/downloads/?lang=zh_tw)是最熟悉的老熟人
        - 但我自己的習慣是使用VScode內建的比對功能
            ![image](https://github.com/LunaticGhoulPiano/CYCU-New-PL/blob/master/pictures/choose_to_compare.jpeg?raw=true)
            ![image](https://github.com/LunaticGhoulPiano/CYCU-New-PL/blob/master/pictures/compare_with_chosen.jpeg?raw=true)
            ![image](https://github.com/LunaticGhoulPiano/CYCU-New-PL/blob/master/pictures/native_compare.jpeg?raw=true)
        - 你也可以使用extension，例如[Compare Folders](https://marketplace.visualstudio.com/items?itemName=moshfeu.compare-folders)：
            ![image](https://github.com/LunaticGhoulPiano/CYCU-New-PL/blob/master/pictures/compare_folders.jpeg?raw=true)

## Structure
```
CYCU-New-PL
├──2025-02-19-hsia-Notes2025-02-19-hsia-Notes // 夏老大的筆記
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
├──AboutProject // 整個project的所有說明文檔
│ ├──HowToCorrectlyGetNextToken-UTF-8.txt
│ ├──HowToWriteOurScheme.doc
│ ├──OurSchemeIntro.doc
│ ├──OurSchemeProj1-UTF-8.txt
│ ├──OurSchemeProj2-UTF-8.txt
│ ├──OurSchemeProj3-UTF-8.txt
│ ├──OurSchemeProj4-UTF-8.txt
│ ├──ThreeLisps-UTF-8.txt
│ └──Tree的建造-UTF-8.txt
├──AboutRecursiveDescentParsing // 夏老大的parsing筆記
│ ├──NeedOfFIRSTset.txt
│ └──RecursiveDescentParsing-Intro.doc
├──CompleteCodeByProjects // 對應project的完整程式碼及說明文件，一個project完成才會丟
│ ├──Project1.cpp
│ └──Project1.md
├──pictures // 說明用圖片
│ ├──choose_to_compare.jpeg
│ ├──compare_folders.jpeg
│ ├──compare_with_chosen.jpeg
│ └──native_compare.jpeg
├──self_tests // 自訂義的測資，部分為測試系統官方的，理論上全過就能過測試系統
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
│     ├──test1.in
│     ├──test1.out
│     ├──test2.in
│     ├──test2.out
│     ├──test3.in
│     └──test3.out
├──.gitignore
├──README.md // overview
├──main.cpp // 我的最新project進度
├──run_mainBatchIO.ps1 // main.cpp的multi-test-cases測試腳本
├──run_project1BatchIO.ps1 // project1.cpp的multi-test-cases測試腳本
└──run_project2BatchIO.ps1 // project2.cpp的multi-test-cases測試腳本
```

# 我的實作
## [Project 1](./CompleteCodeByProjects/Project1.md)