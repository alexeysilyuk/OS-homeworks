#!/bin/bash
backupDir="../backup/"
if [ ! -d "$backupDir" ];then	
	echo "\""$backupDir"\" folder created"
	mkdir "$backupDir"
else
	rm -r "$backupDir"
	echo "\""$backupDir"\" folder created"
	mkdir "$backupDir"
fi

echo "Starting backup..."

for i in *; do
	if [ -d "$i" ]; then
		mkdir "$backupDir""$i"
		echo "created folder \""$i"\""
		for j in ./$i/*; do
			echo "$j >>> ../backup/"$j""
			cp "$j" ../backup/"$j"
		done
	else
		echo ""$i" >>>	../backup/"$i""
		cp "$i" ../backup/"$i"
	fi

done
