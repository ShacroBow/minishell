#!/usr/bin/env bash
# rename_bonus.sh – add “_bonus” before the .c extension on every *.c file
# run from the directory where you want the renaming to start

find . -type f -name "*.c" ! -name "*_bonus.c" -print0 | while IFS= read -r -d '' f
do
	dir="$(dirname "$f")"
	base="$(basename "$f" .c)"
	mv -- "$f" "$dir/${base}_bonus.c"
done
