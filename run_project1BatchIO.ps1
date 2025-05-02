# compile
Push-Location "Complete_Code_by_Projects"
g++ project1.cpp -std=c++2a -o ..\project1.exe
Pop-Location

# set paths
$testDir = "./self_tests/project1"
$exePath = "./project1.exe"
$outputDir = "$testDir/test_outputs_project1"
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
        Write-Host $filenameWithExt
    }
}

Write-Host "All tests completed."