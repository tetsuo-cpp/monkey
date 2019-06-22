#!/bin/bash

current_dir=`basename $PWD`
if [ "$current_dir" != "monkey" ]; then
    echo "format.sh must be executed from the monkey project root"
    exit 1
fi

git ls-files | grep ".*\.\(cpp\|h\)" | xargs clang-format -i
