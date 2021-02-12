#!/bin/sh
# Investigate what happen when symlinking fails.
#


set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

assert_eq() {
  message="$1"
  actual="$2"
  expected="$3"
  if [ "$expected" != "$actual" ]; then
    dbgecho "ASSERTION FAILED [$message], expected '$expected' but was '$actual'"
    exit 1
  fi
}

make_files() {
  head -c 1000000 </dev/urandom > a
  ln a a1
  ln a a2
  cp a A
  ln A A1
  ln A A2

  head -c 1000000 </dev/urandom > b
  ln b b1
  ln b b2
  cp b B
  ln B B1
  ln B B2
}

verify_result() {
  assert_eq "number of files" $(ls | wc -l) 12
  assert_eq "number of hardlinks to a" $(stat -c %h $datadir/a) 6
  assert_eq "number of hardlinks to b" $(stat -c %h $datadir/b) 6
  if cmp --silent $datadir/a $datadir/b; then
    dbgecho "Files should be different"
    exit 1
  fi
}

# <expected_reported_reduction> <expected_actual_reduction> <expected_created_hardlinks> [rdfind_option...]
verify_reduction_with_options() {
  expected_reported="$1"
  expected_reduction="$2"
  expected_links="$3"
  shift 3

  size_before=$(du -m "$datadir" | cut -f1)

  $rdfind -makehardlinks true "$@" "$datadir" | tee rdfind.out

  assert_eq "reported reduction" "$(cat rdfind.out | grep Totally | sed 's/Totally, \([^ ]*\).*/\1/')" "$expected_reported"
  assert_eq "number of creaded hardlinks" "$(cat rdfind.out | grep Making | sed 's/Making \([^ ]*\).*/\1/')" "$expected_links"

  rm rdfind.out results.txt
  size_after=$(du -m "$datadir" | cut -f1)
  assert_eq "actual reduction" $(( $size_before - $size_after )) "$expected_reduction"
}

# Default behavior
# requires multiple runs and reports incorrect reduction
make_files
verify_reduction_with_options 2 0 2 -removeidentinode true
verify_reduction_with_options 2 0 2 -removeidentinode true
verify_reduction_with_options 2 2 2 -removeidentinode true
verify_result
verify_reduction_with_options 0 0 0 -removeidentinode true
verify_result

# removeidentinode false
# requires single run but reports incorrect reduction and number of created links
# also does too much work and keeps repeating it all even when no reduction can be gained
reset_teststate
make_files
verify_reduction_with_options 10 2 10 -removeidentinode false
verify_result
verify_reduction_with_options 10 0 10 -removeidentinode false
verify_result

# rememberidentinode true
# requires single run and reports correct statistics
reset_teststate
make_files
verify_reduction_with_options 2 2 6 -rememberidentinode true
verify_result
verify_reduction_with_options 0 0 0 -rememberidentinode true
verify_result
