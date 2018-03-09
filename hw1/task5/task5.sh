#!/bin/bash
rm *.o *.exe *.s #remove old files

for file in $(find . -name "*.c"); do
	filename=$(basename "$file")
	filename="${filename%.*}"
	gcc -Os -std=gnu11 -c "$filename.c" 
done

for file in $(find . -name "*.c"); do
	filename=$(basename "$file")
	filename="${filename%.*}"
	gcc -Os -Wall -S -masm=intel "$filename.c" -Os
done

for file in $(find . -name "*.o"); do
	filename=$(basename "$file")
	filename="${filename%.*}"
	gcc -Os -Wall  -g "$file" -o "$filename".exe  
done




