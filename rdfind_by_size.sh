#!/bin/bash

#set -x

DIRLIST="./dir1 ./dir2 ./dir3"
RDFIND=rdfind

DRYRUN_VAL="false"
REMOVEIDENTINODE_VAL="true"

MAX_SIZE=$(( 1000 * 1000000 ))	# 1000 megabytes = 1 gig
# Walk down in size creating groups of files in size bands to work on
for i in 20 10 5 4 3 2 1 0.75 0.5 0.25 0; do
    if [ -e stoprdfind ]
    then
	exit
    else
	echo "Proceeding..."
    fi
    MIN_SIZE=$( echo "scale=0; ($i * 1000000)/1" | bc)  # i * megabyte
    printf "MAX_SIZE = %'d\n" $MAX_SIZE
    printf "MIN_SIZE = %'d\n" $MIN_SIZE
    echo " .. "
    DATESTR=$(date +%Y%m%d_%H:%M:%S)
    # The following command can be dispatched to multiple machines or cores to run in parallel
    { time ${RDFIND} -dryrun ${DRYRUN_VAL} -removeidentinode ${REMOVEIDENTINODE_VAL} -makehardlinks true -checksum sha1 -outputname results_min_${MIN_SIZE}_max_${MAX_SIZE}_${DATESTR}.txt -minFileSize ${MIN_SIZE} -maxFileSize ${MAX_SIZE} ${DIRLIST} ; } 2>&1 | tee -a results_min_${MIN_SIZE}_max_${MAX_SIZE}_${DATESTR}.log

    MAX_SIZE=$MIN_SIZE 		# step down to next size
done
exit
