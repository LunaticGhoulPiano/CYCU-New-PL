#!/bin/bash

# set project serial number
project_name="project1"

# compile
cd CompleteCodeByProjects || exit
g++ "$project_name.cpp" -std=c++2a -o "../$project_name"
cd - > /dev/null

# set paths
test_dir="./self_tests/$project_name/LF"
exe_path="./$project_name"
output_dir="$test_dir/test_outputs_$project_name"
log_file="$output_dir/error_test_cases.txt"

# create output folder if not exists
mkdir -p "$output_dir"

# initialize log file (overwrite if exists)
[ -f "$log_file" ] && rm "$log_file"

# run test cases
for input_file in "$test_dir"/*.in; do
    filename=$(basename "$input_file" .in)
    output_file="$output_dir/$filename.bug"
    correct_answer_file="$test_dir/$filename.out"

    # check if the file has newline at the end
    if [ "$(tail -c 1 "$input_file" | od -a | awk '{print $2}')" = "nl" ]; then
        "$exe_path" < "$input_file" > "$output_file"
    else # if no, concatenate newline temporarily
        (cat "$input_file"; printf '\n') | "$exe_path" > "$output_file"
    fi

    # compare .bug and .out
    if ! diff -q "$output_file" "$correct_answer_file" > /dev/null; then
        echo "$filename.bug" >> "$log_file"
        echo "[$project_name.cpp]: $filename.bug is not equal to $filename.out."
    else # no error, remove .bug file
        rm "$output_file"
    fi
done

# check if $outputDir is empty and delete it if so
if [ -z "$(ls -A "$output_dir")" ]; then
    rm -r "$output_dir"
    echo "[$project_name.cpp]: All tests are correct!"
else
    error_count=$(wc -l < "$log_file")
    echo "[$project_name.cpp]: $error_count error case(s) are found!"
fi
