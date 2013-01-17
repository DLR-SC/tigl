#!/bin/bash

pwdir=`pwd`
tmpdir=`mktemp -d`
cd $tmpdir

prefix=http://download.opensuse.org/repositories/home:/martinsiggel/SLE_11_SP1/x86_64/
wget ls $prefix

filelist=`cat index.html | grep  rpm | awk '{print $7}' |  cut -d'"' -f 2`
#bin_file_list= ``
for file in $filelist; do
	if [[ $file != *devel* ]] && [[ $file != *demo* ]] && [[ $file != *32bit* ]]
	then
		bin_file_list+=($file)
	fi
done

for file in ${bin_file_list[@]}; do
	wget $prefix/$file
	rpm2cpio $file | cpio -idmv
done

tar -czf $pwdir/TIGL-2.0.4-SLED11_x86_64.tar.gz usr/

rm -rf $tmpdir
