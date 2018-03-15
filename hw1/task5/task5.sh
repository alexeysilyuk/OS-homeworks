#!/bin/bash
rm *.o runfile *.s #remove old files

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

gcc -Os -Wall  -g *.o -o runfile  





