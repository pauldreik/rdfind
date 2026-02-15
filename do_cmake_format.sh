#!/bin/sh
#
# cmake autoformat. apt install cmake-format

set -e

cmake-format -i inofficial_cmake/CMakeLists.txt
