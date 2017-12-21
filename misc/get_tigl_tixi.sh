#!/usr/bin/env bash
#
# Copyright (C) 2015 German Aerospace Center (DLR/SC)
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

#package name
PACKAGE=TIGL


VERSION=UNKNOWN
DIST=NO_DIST
FDIST=NO_DIST
PACK_ARCH=NO_ARCH
ARCH=NO_ARCH
LIBDIR=NO_DIR
PACK_TYPE=NO_TYPE


function printUsage {
    echo "usage: get_tigl.sh <distro> <arch>"
    echo
    echo "Valid distributions:"
    echo "    SLE_11_SP2     Suse Linux Enterprise 11 SP2"
    echo "    SLE_12_SP1     Suse Linux Enterprise 12 SP1"
    echo "    openSUSE_13.1  openSUSE 13.1"
    echo "    ubuntu_12.04   Ubuntu 12.04"
    echo "    ubuntu_14.04   Ubuntu 14.04"
    echo "    fedora_20      Fedora 20"
    echo "    rhel_6         Red Hat Enterprise Linux 6"
    echo "    rhel_7         Red Hat Enterprise Linux 7"
    echo "    centos_6       CentOS 6"
    echo
    echo "Valid architectures:"
    echo "    x86_64        64 Bit"
    echo "    i386          32 Bit"
    echo
}

function checkArguments {

    # check number of args
    if [ $# -ne 2 ]
    then
	printUsage
        exit 1
    fi

    tmp_dist=$1
    tmp_arch=$2
    #check arch
    if [[ $tmp_arch != i386 && $tmp_arch != x86_64 ]]; then
	echo "Error: Unsupported architecture:" $tmp_arch
	echo
	printUsage
	exit 2
    fi

    LIBDIR=lib

    #check dist
    if [[ $tmp_dist == SLE_11_SP2 ]]; then
    	DIST=SLE_11_SP2
	PACK_TYPE=rpm
	if [[  $tmp_arch == i386 ]]; then
	    PACK_ARCH=i586
        else
            PACK_ARCH=x86_64
            LIBDIR=lib64
	fi
    elif [[ $tmp_dist == SLE_12_SP1 ]]; then
	DIST=SLE_12_SP1
	PACK_TYPE=rpm
	if [[  $tmp_arch == i386 ]]; then
	    echo "Error: x86 architecture not available on SLED 12"
	    exit 1
        else
            PACK_ARCH=x86_64
            LIBDIR=lib64
	fi
    elif [[ $tmp_dist == openSUSE_13.1 ]]; then
        DIST=openSUSE_13.1
        PACK_TYPE=rpm
        if [[  $tmp_arch == i386 ]]; then
            PACK_ARCH=i586
        else
            PACK_ARCH=x86_64
            LIBDIR=lib64
        fi
    elif [[ $tmp_dist == rhel_7 ]]; then
	DIST=RHEL_7
	PACK_TYPE=rpm
	if [[  $tmp_arch == i386 ]]; then
	    echo "Error: x86 architecture not available on RHEL 7"
	    exit 1
        else
            PACK_ARCH=x86_64
            LIBDIR=lib64
	fi
    elif [[ $tmp_dist == rhel_6 ]]; then
    	DIST=RedHat_RHEL-6
	PACK_TYPE=rpm
	if [[  $tmp_arch == i386 ]]; then
	    PACK_ARCH=i686
        else
            PACK_ARCH=x86_64
            LIBDIR=lib64
	fi
   elif [[ $tmp_dist == centos_6 ]]; then
    	DIST=CentOS_CentOS-6
	PACK_TYPE=rpm
	if [[  $tmp_arch == i386 ]]; then
	    PACK_ARCH=i686
        else
            PACK_ARCH=x86_64
            LIBDIR=lib64
	fi
   elif [[ $tmp_dist == fedora_20 ]]; then
    	DIST=Fedora_20
	PACK_TYPE=rpm
	if [[  $tmp_arch == i386 ]]; then
	    PACK_ARCH=i686
        else
            PACK_ARCH=x86_64
            LIBDIR=lib64
	fi
    elif [[ $tmp_dist == ubuntu_12.04 ]]; then
    	DIST=xUbuntu_12.04
	PACK_TYPE=deb
	if [[  $tmp_arch == i386 ]]; then
	    PACK_ARCH=i386
        else
            PACK_ARCH=amd64
	fi
    elif [[ $tmp_dist == ubuntu_14.04 ]]; then
        DIST=xUbuntu_14.04
        PACK_TYPE=deb
        if [[  $tmp_arch == i386 ]]; then
            PACK_ARCH=i386
        else
            PACK_ARCH=amd64
        fi
    else
	echo "Error: Unsupported distribution:" $1
	echo
	printUsage
	exit 3
    fi
    ARCH=$tmp_arch
    FDIST=$tmp_dist
}


download() {
    local url=$1
    echo -n "    "
    wget --progress=dot $url 2>&1 | grep --line-buffered "%" | sed -u -e "s,\.,,g" | awk '{printf("\b\b\b\b%4s", $2)}'
    return $?
}


checkArguments $@

pwdir=`pwd`
tmpdir=`mktemp -d`
cd $tmpdir

prefix=http://download.opensuse.org/repositories/science:/dlr/$DIST/$PACK_ARCH/

echo "Downloading from repository $prefix"
echo "Using working directory: $tmpdir"
echo

# get list of available files to download 
wget -q $prefix/
if [[ $? -ne 0 ]]; then
    echo "Error downloading file list from $prefix"
    exit 7
fi

filelist=`cat index.html | grep  $PACK_TYPE | cut -d'"' -f 4`


if [[ $PACK_TYPE == rpm ]]; then
  # select required files
  for file in $filelist; do
	#opencascade	
	if [[ $file == OCE*.rpm ]] && [[ $file != OCE-devel* ]] && [[ $file != *debuginfo* ]]
	then
		bin_file_list+=($file)
	fi

	#TIXI
	if [[ $file == libTIXI2*.rpm ]]  || [[ $file == tixi-*.rpm ]] && [[ $file != *debuginfo* ]] && [[ $file != *debugsource* ]]
	then
		bin_file_list+=($file)
	fi
	
	#TIGL
	if [[ $file == libTIGL2*.rpm ]] || [[ $file == tigl-*.rpm ]] && [[ $file != *debuginfo* ]]  && [[ $file != *debugsource* ]]
	then
		bin_file_list+=($file)
        	#extract version number        
		if [[ $file == libTIGL2-*.rpm ]]; then
			VERSION=`echo $file | awk '{split($0,array,"-")} END{print array[2]}'`
		fi
	fi
  done
elif [[ $PACK_TYPE == deb ]]; then
  # select required files
  for file in $filelist; do
	#opencascade	
	if [[ $file == liboce-*.deb ]] && [[ $file != liboce*dev* ]] && [[ $file != liboce*ocaf* ]]
	then
		bin_file_list+=($file)
	fi

	#TIXI
	if [[ $file == libtixi2*.deb ]] 
	then
		bin_file_list+=($file)
	fi
	
	#TIGL
	if [[ $file == libtigl*.deb ]] || [[ $file == tigl-*.deb ]]
	then
		bin_file_list+=($file)
        	#extract version number        
		if [[ $file == libtigl2_*.deb ]]; then
			VERSION=`echo $file | awk '{split($0,array,"_")} END{print array[2]}' | awk '{split($0,array,"-")} END{print array[1]}'`
		fi
	fi
  done
else
    echo "Error: unknown package type"
    exit 4
fi

NAME="$PACKAGE-$VERSION-$FDIST-$ARCH"

#download and extract files
for file in ${bin_file_list[@]}; do
	echo -n " - Downloading $prefix$file... "
	download $prefix$file
	if [[ $? -ne 0 ]]; then
	    echo
	    echo "Error downloading $file"
	    exit 7
	fi
	echo

        if [[ $PACK_TYPE == rpm ]]; then
	    rpm2cpio $file | cpio -idm 2> /dev/null
	elif [[ $PACK_TYPE == deb ]]; then
	    ar x $file
	    tar -xf data.tar.*
	else
	    echo "Unknown package type $PACK_TYPE"
	    exit 5
	fi
done

mv usr/ $NAME
cd $NAME

if [[ $DIST != RedHat_RHEL-5 ]]; then 
  #create start script for tiglviewer
  echo "#!/bin/bash" > tiglviewer.sh
  echo 'CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"' >> tiglviewer.sh
  echo 'export LD_LIBRARY_PATH=$CURDIR/'$LIBDIR/':$LD_LIBRARY_PATH' >> tiglviewer.sh
  if [[ $PACK_TYPE == deb ]]; then
      echo 'export LD_LIBRARY_PATH=$CURDIR/'$LIBDIR/$ARCH-linux-gnu/':$LD_LIBRARY_PATH' >> tiglviewer.sh
  fi
  echo '$CURDIR/bin/TIGLViewer' >> tiglviewer.sh
  chmod +x tiglviewer.sh
fi

cd ..


echo
echo "create package $NAME.tar.gz"
tar -czf $pwdir/$NAME.tar.gz $NAME

rm -rf $tmpdir

echo "Package generation done..." 
