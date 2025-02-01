cmake_minimum_required(VERSION 3.25)

file(READ ../VERSION.txt RDFIND_VERSION)
string(STRIP "${RDFIND_VERSION}" RDFIND_VERSION)

string(REGEX REPLACE "\\.next$" ".999" PROJECT_VERSION ${RDFIND_VERSION})
project(rdfind VERSION "${PROJECT_VERSION}")

find_package(PkgConfig)
pkg_check_modules(nettle REQUIRED nettle)

pkg_check_modules(xxhash IMPORTED_TARGET libxxhash)

if(xxhash_FOUND)
    set(HAVE_LIBXXHASH 1)
else()
    set(HAVE_LIBXXHASH 0)
endif()


configure_file(config.h.in config.h @ONLY)

add_executable(rdfind
    ../Checksum.cc
    ../Checksum.hh
    ../CmdlineParser.cc
    ../CmdlineParser.hh
    ../Dirlist.cc
    ../Dirlist.hh
    ../EasyRandom.cc
    ../EasyRandom.hh
    ../Fileinfo.cc
    ../Fileinfo.hh
    ../rdfind.cc
    ../RdfindDebug.hh
    ../Rdutil.cc
    ../Rdutil.hh
    ../UndoableUnlink.cc
    ../UndoableUnlink.hh)
target_include_directories(rdfind PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(rdfind PRIVATE ..)

target_compile_features(rdfind PRIVATE cxx_std_17)
target_link_libraries(rdfind nettle)
if(xxhash_FOUND)
target_link_libraries(rdfind PkgConfig::xxhash)
endif()
target_compile_options(rdfind PRIVATE -Wall -Wextra -Wpedantic)

