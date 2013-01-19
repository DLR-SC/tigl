#!/bin/bash
#
# Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
#
# Created: 2013-01-17 Martin Siggel <martin.siggel@dlr.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @brief This script creates an all-inclusive binary package for TIGL.
# It downloads the required binaries from our openbuildservice page
# and copies them into on tar.gz file

VERSION=2.0.4
NAME=TIGL-$VERSION-SLED11_x86_64

pwdir=`pwd`
tmpdir=`mktemp -d`
cd $tmpdir

prefix=http://download.opensuse.org/repositories/home:/martinsiggel/SLE_11_SP1/x86_64/

# get list of available files to download 
wget $prefix/
filelist=`cat index.html | grep  rpm | awk '{print $7}' |  cut -d'"' -f 2`

# select required files
for file in $filelist; do
	#opencascade	
	if [[ $file == OCE*.rpm ]] && [[ $file != OCE-devel* ]]
	then
		bin_file_list+=($file)
	fi
	
	#ftgl
	if [[ $file == libftgl2*.rpm ]] && [[ $file != libftgl2-32bit*.rpm ]]
	then
		bin_file_list+=($file)
	fi

	#TIXI
	if [[ $file == libTIXI2*.rpm ]] 
	then
		bin_file_list+=($file)
	fi
	
	#TIGL
	if [[ $file == libTIGL2*.rpm ]] || [[ $file == tigl-*.rpm ]]
	then
		bin_file_list+=($file)
	fi

done

#download and extract files
for file in ${bin_file_list[@]}; do
	wget $prefix/$file
	rpm2cpio $file | cpio -idmv
done

mv usr/ $NAME
cd $NAME

#create start script for tiglviewer
echo "#!/bin/bash" > tiglviewer.sh
echo 'CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"' >> tiglviewer.sh
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CURDIR/lib64/' >> tiglviewer.sh
echo 'export CSF_GraphicShr=$CURDIR/lib64/libTKOpenGl.so.5' >> tiglviewer.sh
echo '$CURDIR/bin/TIGLViewer' >> tiglviewer.sh
chmod +x tiglviewer.sh

cd ..

tar -czf $pwdir/$NAME.tar.gz $NAME

rm -rf $tmpdir
