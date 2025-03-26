# CYCU-New-PL (中原資工程式語言project - OurScheme)
## 資料
- 簡報: https://www.canva.com/design/DAGemJ7sLQI/X3aeWBckMJEJuJ5ks3dF7w/edit?utm_content=DAGemJ7sLQI&utm_campaign=designshare&utm_medium=link2&utm_source=sharebutton
- 老大的上傳系統: https://pai.lab715.uk:5001/PL-PostCode/
- 系統只吃一個檔案，所以要全部塞在同一檔案中
- Project deadline修改為與大三相同
- 平時分數計算：每堂課會有4分
    1. 出席分
    2. So so
    3. Ok
    4. Great
    0. Not good
- Project分數到80可以不用出席
- WSL路徑: ```/mnt/c/Users/PoHsu/OneDrive/文件/GitHub/CYCU-New-PL```

## Project 1
- 位置計算與Errors
    - 正常的token:
        - 每次保存一個token之後都要重設
        - lineNum = 1, columnNum = 0
    - ```NoClosingQuote```:
        - lineNum一定是1，因為```""```之間不可以有```\n```
        - columnNum是當前正在判斷的字元位置+1
        - 範例（空格用```_```表示，但greeting的```> ```與Error message的空格會正常印）：
            1.
            ```
            > "123""
            > "123"
            > ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 2
            ```
            ```"123"```讀完存入後重設為(1, 0)
            讀到```"```時它的位置是(1, 1)
            所以預期對應的```"```在(1, 2)
            2.
            ```
            > "123"___"
            > "123"
            > ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 5
            ```
            ```"123"```讀完存入後重設為(1, 0)
            接著讀入三個空格，位置為(1, 1), (1, 2), (1, 3)
            讀到```"```時它的位置是(1, 4)
            所以預期對應的```"```在(1, 5)
            3.
            ```
            > "123"__a_"
            > "123"
            > a
            > ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 3
            ```
            ```"123"```讀完存入後重設為(1, 0)
            接著讀入兩個空格，位置為(1, 1), (1, 2)
            ```a```讀完存入後重設為(1, 0)
            接著讀入一個空格，位置為(1, 1)
            讀到```"```時它的位置是(1, 2)
            所以預期對應的```"```在(1, 3)
            4.
            ```
            > "123"__f__"v
            > "123"
            > f
            > ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 5
            ```
            ```"123"```讀完存入後重設為(1, 0)
            接著讀入兩個空格，位置為(1, 1), (1, 2)
            ```f```讀完存入後重設為(1, 0)
            接著讀入兩個空格，位置為(1, 1), (1, 2)
            讀到```"```時它的位置是(1, 3)
            讀到```v```時它的位置是(1, 4)
            所以預期對應的```"```在(1, 5)
            5.
            ```
            > ______"
            > ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 8
            ```
            連續讀入6個空格，位置為(1, 1) ~ (1, 6)
            讀到```"```時它的位置是(1, 7)
            所以預期對應的```"```在(1, 8)
            6.
            ```
            > """
            > ""
            > ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 2
            ```
            第一個```"```位置為(1, 1)
            第二個```"```位置為(1, 2)
            因為是一個完整個double-quote，因此重設為(1, 0)
            第三個```"```位置為(1, 1)
            所以預期對應的```"```在(1, 2)
    - ```UnexpectedTokenError```:
        - 分為兩種
        - ```atom or '(' expected ...```我把它稱做```UnexpectedTokenError```
        - ```')' expected ...```我把它稱做```NoRightParen```
        - 因為可以在single-quote或```()```之間有換行，因此要注意
        - columnNum不用+1，因為它不是成對的
        1. ```UnexpectedTokenError```範例:
            1.
            ```
            > '
            .
            > ERROR (unexpected token) : atom or '(' expected when token at Line 2 Column 1 is >>.<<
            ```
            因為第一行為```'```加上一個換行，因此在第二行第一個字元```.```時位置為(2, 1)
            2.
            ```
            > '_
            _.
            > ERROR (unexpected token) : atom or '(' expected when token at Line 2 Column 2 is >>.<<
            ```
            因為第二行的```.```在第一個空格之後，所以位置為(2, 2)
            3.
            ```
            > ()___)
            > ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 4 is >>)<<
            ```
            因為第一個```nil```作為一個tokem存入後重設位置
            接著連續輸入三個空格為(1, 1), (1, 2), (1, 3)
            因此```)```的位置為(1, 4)
            4.
            ```
            > (1_2_._;
            )_;
            > ERROR (unexpected token) : atom or '(' expected when token at Line 2 Column 1 is >>)<<
            ```
            因為在第二行第一個字元不符合規範所以就是(2, 1)
        2. ```NoRightParen```範例:
            1.
            ```
            > (1_.()_.(123))
            > ERROR (unexpected token) : ')' expected when token at Line 1 Column 8 is >>.<<
            ```
    - ```NoMoreInput```:
        - EOF