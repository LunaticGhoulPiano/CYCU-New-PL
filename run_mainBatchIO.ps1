# set project serial number
$projectName = "project3"

# compile
g++ main.cpp -std=c++2a -o main.exe

# set paths
$testDir = "./self_tests/$projectName/CRLF"
$exePath = "./main.exe"
$outputDir = "$testDir/test_outputs_main"
$logFile = "$outputDir/error_test_cases.txt"

# create output folder if not exists
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

# initialize log file (overwrite if exists)
if (Test-Path $logFile) {
    Remove-Item $logFile
}

# run test cases
Get-ChildItem "$testDir/*.in" | ForEach-Object {
    $filename = $_.BaseName
    $inputFile = $_.FullName
    $outputFile = "$outputDir/$filename.bug"
    $correctAnswerFile = "$testDir/$filename.out"

    # run the program and produce .bug
    Get-Content $inputFile | & $exePath > $outputFile

    # compare .bug and .out
    if (Compare-Object (Get-Content $outputFile) (Get-Content $correctAnswerFile) -SyncWindow 0) {
        $filenameWithExt = "$filename.bug"
        Add-Content -Path $logFile -Value $filenameWithExt
        Write-Host "[main.cpp]: $filenameWithExt is not equal to $filename.out."
    }
    else { # no error, remove .bug file
        Remove-Item $outputFile
    }
}

# check if $outputDir is empty and delete it if so
if ((Get-ChildItem $outputDir).Count -eq 0) {
    Remove-Item $outputDir -Force
    Write-Host "[main.cpp]: All tests are correct!"
}
else {
    $errorCount = (Get-Content $logFile).Count
    Write-Host "[main.cpp]: $errorCount error case(s) are found!"
}