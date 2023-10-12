#!/bin/sh

set -eu

outdir=$(dirname "$0")/out
me=$(basename "$0")

mkdir -p "$outdir"

args="--enable=all --inconclusive --std=c++11 -I . --quiet --suppress=missingIncludeSystem"

# cppcheck can not produce an xml report and a reulgar text file at the same time, so run twice
cppcheck $args *.cc *.hh --template='{severity}:{file}:{line}:{message}' 2>"$outdir/cppcheck.out"

cppcheck $args *.cc *.hh --xml 2>"$outdir/cppcheck.xml"

cppcheck-htmlreport --source-dir=. --title=rdfind --file="$outdir/cppcheck.xml" --report-dir="$outdir"


#is anything serious found?
if grep --quiet -v -E '^(style|information|performance):' "$outdir/cppcheck.out" ; then
 echo "$me: cppcheck found serious issues. see $outdir/cppcheck.out"
 exit 1
fi

echo "$me: cppcheck passed without serious issues."


