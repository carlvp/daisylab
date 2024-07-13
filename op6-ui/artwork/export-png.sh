#!/bin/bash
# Export base64 encoded png from svg
files=$(printf "%s\n" $* | sort -V)
for f in $files ; do
    echo
    echo \# $(basename $f .svg).png, 160x120, color
    echo \'$(basename $f .svg).png\': \'\'\'
    inkscape -f $f -d 48 -e - | base64
    echo \'\'\',
done
