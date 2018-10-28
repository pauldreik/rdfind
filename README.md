# Rdfind - redundant data find

Rdfind is a command line tool that finds duplicate files. It is useful for compressing backup directories or just finding duplicate files. It compares files based on their content, NOT on their file names. 

## Continuous integration status
| Status | Description
|-------------|------------------
| [![Total alerts](https://img.shields.io/lgtm/alerts/g/pauldreik/rdfind.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/pauldreik/rdfind/alerts/) | Static analyzer
| [![Build Status](https://travis-ci.org/pauldreik/rdfind.svg?branch=master)](https://travis-ci.org/pauldreik/rdfind) | Builds and executes tests on Ubuntu 14.04 with gcc and clang
| [![Build status](https://ci.appveyor.com/api/projects/status/9crp181omyugf8xk/branch/master?svg=true)](https://ci.appveyor.com/project/pauldreik/rdfind/branch/master) | Builds and executes test on Ubuntu 18.04 with gcc and clang. Also runs address/undefined sanitizers and valgrind.

## Install

### Debian/Ubuntu:

    # apt install rdfind

### Fedora

    # dnf install rdfind

### Mac

If you are on Mac, you can install through [MacPorts](https://www.macports.org/) (direct link to [Portfile](https://github.com/macports/macports-ports/blob/master/sysutils/rdfind/Portfile)). If you want to compile the source yourself, that is fine. Rdfind is written in C++11 and should compile under any \*nix.

### Windows

using cygwin.

## Usage

    rdfind [options] directory_or_file_1 [directory_or_file_2] [directory_or_file_3] ...

Without options, a results file will be created in the current directory. For full options, see [the man page](https://rdfind.pauldreik.se/rdfind.1.html).

## Examples

Basic example, taken from a \*nix environment:
Look for duplicate files in directory /home/pauls/bilder:

    $ rdfind /home/pauls/bilder/
    Now scanning "/home/pauls/bilder", found 3301 files.
    Now have 3301 files in total.
    Removed 0 files due to nonunique device and inode.
    Now removing files with zero size...removed 3 files
    Total size is 2861229059 bytes or 3 Gib
    Now sorting on size:removed 3176 files due to unique sizes.122 files left.
    Now eliminating candidates based on first bytes:removed 8 files.114 files left.
    Now eliminating candidates based on last bytes:removed 12 files.102 files left.
    Now eliminating candidates based on md5 checksum:removed 2 files.100 files left.
    It seems like you have 100 files that are not unique
    Totally, 24 Mib can be reduced.
    Now making results file results.txt
              
It indicates there are 100 files that are not unique. Let us examine them by looking at the newly created results.txt:

    $ cat results.txt
    # Automatically generated
    # duptype id depth size device inode priority name
    DUPTYPE_FIRST_OCCURENCE 960 3 4872 2056 5948858 1 /home/pauls/bilder/digitalkamera/horisontbild/.xvpics/test 001.jpg.gtmp.jpg
    DUPTYPE_WITHIN_SAME_TREE -960 3 4872 2056 5932098 1 /home/pauls/bilder/digitalkamera/horisontbild/.xvpics/test 001.jpg
    .
    (intermediate rows removed)
    .
    DUPTYPE_FIRST_OCCURENCE 1042 2 7904558 2056 6209685 1 /home/pauls/bilder/digitalkamera/skridskotur040103/skridskotur040103 014.avi
    DUPTYPE_WITHIN_SAME_TREE -1042 3 7904558 2056 327923 1 /home/pauls/bilder/digitalkamera/saknat/skridskotur040103/skridskotur040103 014.avi

Consider the last two rows. It says that the file `skridskotur040103 014.avi` exists both in `/home/pauls/bilder/digitalkamera/skridskotur040103/` and `/home/pauls/bilder/digitalkamera/saknat/skridskotur040103/`. I can now remove the one I consider a duplicate by hand if I want to.

## Algorithm

Rdfind uses the following algorithm. If N is the number of files to search through, the effort required is in worst case O(Nlog(N)). Because it sorts files on inodes prior to disk reading, it is quite fast. It also only reads from disk when it is needed.

1. Loop over each argument on the command line. Assign each argument a priority number, in increasing order.
2. For each argument, list the directory contents recursively and assign it to the file list. Assign a directory depth number, starting at 0 for every argument.
3. If the input argument is a file, add it to the file list.
4. Loop over the list, and find out the sizes of all files.
5. If flag -removeidentinode true: Remove items from the list which already are added, based on the combination of inode and device number. A group of files that are hardlinked to the same file are collapsed to one entry. Also see the comment on hardlinks under ”caveats below”!
6. Sort files on size. Remove files from the list, which have unique sizes.
7. Sort on device and inode(speeds up file reading). Read a few bytes from the beginning of each file (first bytes).
8. Remove files from list that have the same size but different first bytes.
9. Sort on device and inode(speeds up file reading). Read a few bytes from the end of each file (last bytes).
10. Remove files from list that have the same size but different last bytes.
11. Sort on device and inode(speeds up file reading). Perform a checksum calculation for each file.
12. Only keep files on the list with the same size and checksum. These are duplicates.
13. Sort list on size, priority number, and depth. The first file for every set of duplicates is considered to be the original.
14. If flag ”-makeresultsfile true”, then print results file (default). 
15. If flag ”-deleteduplicates true”, then delete (unlink) duplicate files. Exit.
16. If flag ”-makesymlinks true”, then replace duplicates with a symbolic link to the original. Exit.
17. If flag ”-makehardlinks true”, then replace duplicates with a hard link to the original. Exit.

## Development

### Building

To build this utility, you need [nettle](https://www.lysator.liu.se/~nisse/nettle/) (on Debian based distros: `apt install nettle-dev`).

### Install from source

Here is how to get and install nettle from source. Please check for the current version before copying the instructions below:

    wget ftp://ftp.lysator.liu.se/pub/security/lsh/nettle-1.14.tar.gz -nc
    wget ftp://ftp.lysator.liu.se/pub/security/lsh/nettle-1.14.tar.gz.asc -nc
    wget ftp://ftp.lysator.liu.se/pub/security/lsh/distribution-key.gpg -nc
    gpg --fast-import distribution-key.gpg                    # omit if you do not want to verify
    gpg --verify nettle-1.14.tar.gz.asc --nettle-1.14.tar.gz  # omit if you do not want to verify
    tar -xzvf nettle-1.14.tar.gz
    ./configure
    make
    sudo make install

 If you install nettle as non-root, you must create a link in the rdfind directory so that rdfind later can do #include "nettle/nettle_header_files.h" correctly. Use for instance the commands

    ln -s nettle-1.14 nettle

### Quality
The following methods are used to maintain code quality:
 - builds without warnings on gcc and clang, even with all the suggested warnings from [cppbestpractices](https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md) enabled. Pass --enable-warnings to configure to turn them on.
 - builds with standards c++11, 14, 17 and 2a
 - tests are written for newly found bugs, first to prove the bug and then to prove that it is fixed. Older bugs do not all have tests.
 - tests are also run through valgrind
 - tests are run on address sanitizer builds
 - tests are run on undefined sanitizer builds
 - tests are run with debug iterators enabled
 - builds are made in default mode (debug) as well as release, and also with the flags suggested by debians hardening helper dpkg-buildflags
 - builds are made with both libstdc++ (gcc) and libc++ (llvm)
 - clang format is used, issue make format to execute it
 - cppcheck has been run manually and relevant issues are fixed
 - [disorderfs](https://packages.debian.org/sid/disorderfs) is used (if available) to verify independence of file system ordering
 
 There is a helper script that does the test build variants, see do_quality_checks.sh in the project root.
 
## Alternatives

There are some interesting alternatives.

- [Duff](http://duff.sourceforge.net/) by Camilla Berglund.
- [Fslint](http://www.pixelbeat.org/fslint/)  by Pádraig Brady

A search on ”finding duplicate files” will give you lots of matches.

### Performance Comparison

Here is a small benchmark. Times are obtained from ”elapsed time” in the time command. The command has been repeated several times in a row, where the result from each run is shown in the table below. As the operating system has a cache for data written/read to the disk, the consecutive calls are faster than the first call. The test computer is a 3 GHz PIV with 1 GB RAM, Maxtor SATA 8 Mb cache, running Mandriva 2006.

| Test case  | duff 0.4 (`time ./duff -rP dir >slask.txt`) | Fslint 2.14 (`time ./findup dir >slask.txt`) | Rdfind 1.1.2 (`time rdfind dir`) |
| ------------- | ------------- | ------------- | ------------- |
| Directory structure with 3,301 files (2,782 Mb jpegs) / 100 files (24 Mb) are redundant | 0:01.55 / 0:01.61 / 0:01.58 | 0:02.59 / 0:02.66 / 0:02.58 | 0:00.49 / 0:00.50 / 0:00.49 |
| Directory structure with 35,871 files (5,325 Mb) / 10,889 files (233 Mb) are redundant | 3:24.90 / 0:46.48 / 0:46.20 / 0:45.31   | 1:26.37 / 1:16.36 / 1:15.38 / 0:53.20 | 0:29.37 / 0:07.81 / 0:06.24 / 0:06.17 |

**Note**: units are minutes:seconds.milliseconds

### Caveats / Features

A group of hardlinked files to a single inode are collapsed to a single entry if `-removeidentinode true`. If you have two equal files (inodes) and two or more hardlinks for one or more of the files, the behaviour might not be what you think. Each group is collapsed to a single entry. That single entry will be hardlinked/symlinked/deleted depending on the options you pass to `rdfind`. This means that rdfind will detect and correct one file at a time. Running multiple times solves the situation. This has been discovered by a user who uses a ”hardlinks and rsync”-type of backup system. There are lots of such backup scripts around using that technique, Apple time machine also uses hardlinks. If a file is moved within the backuped tree, one gets a group of hardlinked files before the move and after the move. Running rdfind on the entire tree has to be done multiple times if -removeidentinode true. To understand the behaviour, here is an example demonstrating the behaviour:

    $ echo abc>a
    $ ln a a1
    $ ln a a2
    $ cp a b
    $ ln b b1
    $ ln b b2
    $ stat --format="name=%n inode=%i nhardlinks=%h" a* b*
    name=a inode=18 nhardlinks=3
    name=a1 inode=18 nhardlinks=3
    name=a2 inode=18 nhardlinks=3
    name=b inode=19 nhardlinks=3
    name=b1 inode=19 nhardlinks=3
    name=b2 inode=19 nhardlinks=3

Everything is as expected.

    $ rdfind -removeidentinode true -makehardlinks true ./a* ./b*
    $ stat --format="name=%n inode=%i nhardlinks=%h" a* b*
    name=a inode=58930 nhardlinks=4
    name=a1 inode=58930 nhardlinks=4
    name=a2 inode=58930 nhardlinks=4
    name=b inode=58930 nhardlinks=4
    name=b1 inode=58931 nhardlinks=2
    name=b2 inode=58931 nhardlinks=2

a, a1 and a2 got collapsed into a single entry. b, b1 and b2 got collapased into a single entry. So rdfind is left with a and b (depending on which of them is received first by the * expansion). It replaces b with a hardlink to a. b1 and b2 are untouched.

If one runs rdfind repeatedly, the issue is resolved, one file being corrected every run.
