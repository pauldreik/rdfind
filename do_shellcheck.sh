#!/bin/sh

set -e

me=$(basename "$0")


echo "$me: run shellcheck on shellscripts"
(
    # use this when all issues are fixed
    # git ls-files | grep -v "^testcases" | grep -E ".sh$" | xargs shellcheck
    shellcheck do_shellcheck.sh do_yamllint.sh
)

echo "$me: run shellcheck on testcases"
(
    # use this when all issues are fixed
    # cd testcases && git ls-files | grep -E ".sh$" | xargs shellcheck -x
)

