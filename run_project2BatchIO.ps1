# compile
Push-Location "CompleteCodeByProjects"
g++ project2.cpp -std=c++2a -o ..\project2.exe
Pop-Location

# set paths
$testDir = "./self_tests/project2"
$exePath = "./project2.exe"
$outputDir = "$testDir/test_outputs_project2"
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
        Write-Host "[project2.cpp]: $filenameWithExt is not equal to $filename.out."
    }
}

# check if $outputDir is empty and delete it if so
if ((Get-ChildItem $outputDir).Count -eq 0) {
    Remove-Item $outputDir -Force
    Write-Host "[project2.cpp]: All tests are correct!"
}
else {
    $errorCount = (Get-Content $logFile).Count
    Write-Host "[project2.cpp]: $errorCount error case(s) are found!"
}