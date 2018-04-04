#!/usr/bin/env bash

# Driver script for non-regression tests (compare output with expected output)

if [ $# -lt 3 ]
then
    echo "Usage: $0 PAGAI_EXE SOURCE OUTPUT [EXTRA_PAGAI_ARGUMENTS...]"
    exit 1
fi

pagai_exec="$1"
shift
source_file="$1"
shift
output_file="$1"
shift

"$pagai_exec" -i "$source_file" -o "$output_file" "$@"

expected_file_we="$(dirname $source_file)/$(basename -s .c -- $source_file).expected"

# Unique file
if [ -f "$expected_file_we".c ]
then
    diff "$output_file" "$expected_file_we".c

# Mutiple possible valid files
else
    for possible in "$expected_file_we".*.c
    do
        if cmp -s "$output_file" "$possible"
        then
            exit 0
        fi
    done

    exit 1
fi
