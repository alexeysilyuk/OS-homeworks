#!/bin/bash
path=$PWD
dir="${path%/**}"
echo "Changing permission in folder : \""$dir"\""
find -maxdepth 1 -path "./*" -type f -name "*.sh" -exec chmod 750 {} \;
find -maxdepth 1 -path "./*" -type f ! -name "*.sh" -exec chmod 710 {} \;
find -maxdepth 1 -path "./*" -type d -exec chmod 640 {} \;
