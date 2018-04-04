#!/usr/bin/env bash

# Driver script for tests using assertions (check that all assertions are proved by pagai)

if [ $# -lt 2 ]
then
    echo "Usage: $0 PAGAI_EXE SOURCE [EXTRA_PAGAI_ARGUMENTS...]"
    exit 1
fi

pagai_exec="$1"
shift
source_file="$1"
shift

"$pagai_exec" -i "$source_file" "$@" | grep "/\* assert " | grep -v "not proved"
