#!/bin/sh

set -e

YMLLINT=""
if which yamllint >/dev/null 2>/dev/null; then
  YMLLINT="yamllint"
elif [ -f .venv/bin/yamllint ]; then
  YMLLINT=".venv/bin/yamllint"
else
  echo "could not find yamllint please install"
  echo "for debian based systems: apt -y install libxml2-utils"
  echo "for redhat based systems: dnf install yamllint"
  echo "local install: python3 -m venv .venv && .venv/bin/python3 -m pip install yamllint"
  exit 3
fi

# run this when all issues are fixed
git ls-files | grep -E ".yml$" | xargs "$YMLLINT"
