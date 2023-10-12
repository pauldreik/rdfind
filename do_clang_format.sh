#!/bin/sh
#
#executes clang format
#
#
# copyright 2017 Paul Dreik
# Distributed under GPL v 2.0 or later, at your option.
# See LICENSE for further details.

#find clang format (usually named clang-format-3.x or clang-format, who knows)
CLANGFORMAT=$(find /usr/local/bin /usr/bin -executable -name "clang-format*" |grep -v -- -diff |sort -g |tail -n1)

if [ ! -x "$CLANGFORMAT" ] ; then
   echo failed finding clangformat
   exit 1
else
   echo "found clang format: $CLANGFORMAT"
fi

find . -maxdepth 1 -type f \( -name "*.h" -o -name "*.cpp" -o -name "*.cc" -o -name "*.hh" \) -print0 | \
   xargs -0 -n1 "$CLANGFORMAT" -i
