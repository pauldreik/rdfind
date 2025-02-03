#!/bin/sh

set -e

me=$(basename "$0")

if ! which shfmt >/dev/null 2>&1; then
  echo "$me: please install shfmt"
  exit 1
fi

git ls-files | grep -E ".sh$" | xargs shfmt --indent 2 \
  --binary-next-line \
  --case-indent \
  --write
