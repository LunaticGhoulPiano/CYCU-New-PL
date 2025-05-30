$projectName = "project3" # set project serial number
$crlfDir = "./self_tests/$projectName/CRLF/"
$lfDir = "/self_tests/$projectName/LF/"

# create LF folder
New-Item -ItemType Directory -Path $lfDir -Force | Out-Null

# iterate all .in and .out
Get-ChildItem -Path $crlfDir -Recurse -Include *.in, *.out -File | ForEach-Object {
    # convert
    $content = Get-Content $_.FullName -Raw
    $lfContent = $content -replace "`r`n", "`n"
    # set output path = current dir + lfDir + filename
    $lfPath = Join-Path (Get-Item .).FullName $lfDir $_.Name
    # write
    Set-Content -Path $lfPath -Value $lfContent -NoNewline
}