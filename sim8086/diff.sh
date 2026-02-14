#!/bin/bash

listing="$1"

asm_out=$(mktemp)
bin_out=$(mktemp)

./sim8086 "$listing" > "$asm_out"
nasm "$asm_out" -o "$bin_out"

diff "$listing" "$bin_out"
