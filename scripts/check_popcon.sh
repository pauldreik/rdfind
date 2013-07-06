#!/bin/sh
echo "will now check debians web page..."
#checks rdfind on popularity contest page
debian_addr=http://popcon.debian.org/by_inst.gz
debian_row=`wget -q -O - $debian_addr |gunzip |grep rdfind`

echo "will now check ubuntus web page..."
ubuntu_addr=http://popcon.ubuntu.com/by_inst.gz
ubuntu_row=`wget -q -O - $ubuntu_addr |gunzip |grep rdfind`

echo "status on debian is "
echo $debian_row

echo "status on ubuntu is "
echo $ubuntu_row

