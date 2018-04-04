#!/usr/bin/env bash

# Driver script to reproduce known bugs where different invariants can be output
# by pagai (which are not consistent with each other).
# The different outputs to get should be named as follows:
#
# If the test file is name "test.c" then the different outputs to reproduce
# shoule be named "test.out.1.c", "test.out.2.c", "test.out.3.c", etc.

if [ $# -lt 2 ]
then
    echo "Usage: $0 PAGAI_EXE SOURCE [EXTRA_PAGAI_ARGUMENTS...]"
    exit 1
fi

pagai_exec="$1"
shift
source_file="$1"
shift

source_basename_we="$(basename -s .c -- ${source_file})"
source_dirname="$(dirname ${source_file})"
tmp_file="/tmp/pagai_reproduce_bug_${source_basename_we}.out.c"

output_got_array=(true) # fake index 0 (output files to got starts from 1)
for exp_output in "$source_dirname/$source_basename_we".out.*.c
do
    output_got_array+=(false)
done

# Return "true" only of all expected files have been obtained
check_all_done() {
    for got in ${output_got_array[@]}
    do
        if ! $got
        then
            return 1
        fi
    done
    return 0
}

# Print files that haven't been reproduced
print_non_reproduced_outputs() {
    count=0
    for got in ${output_got_array[@]}
    do
        if ! $got
        then
            echo " >  ${source_dirname}/${source_basename_we}.out.${count}.c"
        fi
        count=$((count + 1))
    done
}

# Update "output_got_array" with current "tmp_file"
# Return "true" only if "tmp_file" matched one of the expected outputs
check_all_diff() {
    count=0
    for got in ${output_got_array[@]}
    do
        if ! $got && cmp -s "$tmp_file" "${source_dirname}/${source_basename_we}.out.${count}.c"
        then
            output_got_array[$count]=true
            echo "Reproduced file: ${source_dirname}/${source_basename_we}.out.${count}.c"
            return 0
        fi
        count=$((count + 1))
    done
    return 1
}

COUNT_MAX=200 # max number of executions to get all the required outputs

for cnt in $(seq 1 $COUNT_MAX)
do
    "$pagai_exec" -i "$source_file" -o "$tmp_file" "$@" > /dev/null

    if ! check_all_diff
    then
        sleep 0.01 # just to let some random stuff do something
    fi

    if check_all_done
    then
        exit 0
    fi
done

echo "Impossible to reproduce the following outputs after $COUNT_MAX tries:"
print_non_reproduced_outputs
exit 1

rm -f "$tmp_file"
