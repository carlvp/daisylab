#!/bin/bash
# Export base64 encoded png from svg
files=$(printf "%s\n" $* | sort -V)
for f in $files ; do
    echo
    echo \# $(basename $f .svg).png
    echo \'$(basename $f .svg).png\': \'\'\'
    inkscape -f $f -d 96 -e - | base64
    echo \'\'\',
done
