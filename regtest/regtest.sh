#!/bin/bash

trap 'echo "FAILED (rc: $?)"' EXIT
set -eu

for i in *.s; do
    echo --- compiling: $i ---
    ./as88 $i
done

for i in *.# *.$ *.88; do
    echo --- comparing: $i ---
    cmp -s $i ref/$i
done

rm *.# *.$ *.88

trap - EXIT

echo ======
echo PASSED