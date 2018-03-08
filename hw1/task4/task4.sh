#!/bin/bash

poem="$1"
if [ -f "$poem" ]; then
	grep -Eo '\<[a-Z]{3,8}\>' "$poem"| tr '[A-Z]' '[a-z]' | sort -u 1> "$poem".dict
else
	echo "Poem with name \""$poem"\" not exists"
fi

