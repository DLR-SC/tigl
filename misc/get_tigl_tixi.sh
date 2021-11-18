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

TIGL3VER="3.2"
TIXIVER="3.1"

read_dom () {
    local IFS=\>
    read -d \< ENTITY CONTENT
}

function printUsage {
    echo "usage: get_tigl.sh <distro> <version>(default=3)"
    echo
    echo "Valid distributions:"
    echo "    SLE_12_SP3          Suse Linux Enterprise 12 SP3"
    echo "    openSUSE_Leap_15.2  openSUSE Leap 15.2"
    # echo "    ubuntu_20.04        Ubuntu 20.04"
    echo "    ubuntu_18.04        Ubuntu 18.04"
    echo "    ubuntu_16.04        Ubuntu 16.04"
    echo "    fedora_31           Fedora 31"
    echo "    fedora_32           Fedora 32"
    echo "    rhel_7              Red Hat Enterprise Linux 7"
    echo
    echo "Valid versions:"
    echo "    2             TiGL 2"
    echo "    3             TiGL 3"
}

function checkArguments {

    # check number of args
    if [ $# -lt 1 ] || [ $# -gt 2 ]
    then
    printUsage
        exit 1
    fi

    if [ $# -eq 1 ]
    then
        tmp_ver="3"
    else
        tmp_ver=$2
    fi


    tmp_dist=$1


    LIBDIR=lib

    #check dist
    if [[ $tmp_dist == SLE_12_SP3 ]]; then
        DIST=SLE_12_SP3
        PACK_TYPE=rpm

        PACK_ARCH=x86_64
        LIBDIR=lib64

    elif [[ $tmp_dist == openSUSE_Leap_15.2 ]]; then
        DIST=openSUSE_Leap_15.2
        PACK_TYPE=rpm

        PACK_ARCH=x86_64
        LIBDIR=lib64

    elif [[ $tmp_dist == rhel_7 ]]; then
      DIST=RHEL_7
      PACK_TYPE=rpm
      PACK_ARCH=x86_64
      LIBDIR=lib64
   elif [[ $tmp_dist == fedora_32 ]]; then
        DIST=Fedora_32
        PACK_TYPE=rpm
        PACK_ARCH=x86_64
        LIBDIR=lib64

   elif [[ $tmp_dist == fedora_31 ]]; then
        DIST=fedora_31
        PACK_TYPE=rpm
            PACK_ARCH=x86_64
            LIBDIR=lib64
    elif [[ $tmp_dist == ubuntu_20.04 ]]; then
        DIST=xUbuntu_20.04
        PACK_TYPE=deb
        PACK_ARCH=amd64
    elif [[ $tmp_dist == ubuntu_16.04 ]]; then
        DIST=xUbuntu_16.04
        PACK_TYPE=deb

        PACK_ARCH=amd64
    elif [[ $tmp_dist == ubuntu_18.04 ]]; then
        DIST=xUbuntu_18.04
        PACK_TYPE=deb
        PACK_ARCH=amd64
    else
    echo "Error: Unsupported distribution:" $1
    echo
    printUsage
    exit 3
    fi

    # check version
    if [[ $tmp_ver -ne "2" ]] && [[ $tmp_ver -ne "3" ]]; then
      echo "Error: Unsupported TiGL version:" $tmp_ver
    echo
    printUsage
    exit 4
    fi
    ARCH="x86_64"
    FDIST=$tmp_dist
    TIGLVER=$tmp_ver
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

prefix=https://download.opensuse.org/repositories/science:/dlr/$DIST/$PACK_ARCH/

echo "Downloading from repository $prefix"
echo "Using working directory: $tmpdir"
echo

# get list of available files to download
wget -q $prefix/
if [[ $? -ne 0 ]]; then
    echo "Error downloading file list from $prefix"
    exit 7
fi


# parse index.html for all packages 
filelist=()
while read_dom; do
    if [[ "$CONTENT" == *.$PACK_TYPE ]]
    then
        filelist+=("$CONTENT")
    fi
done < index.html
filelist="${filelist[@]}"

if [[ $PACK_TYPE == rpm ]]; then
    if [[ $TIGLVER -eq "2" ]]; then
        whitelist="OCE-0* libTIGL2* libTIXI2* tigl-devel* tixi-devel* tigl-viewer*"
    elif [[ $TIGLVER -eq "3" ]]; then
        whitelist="libfreeimage* libopencascade7-7* libtigl3-?-$TIGL3VER* libtixi3-?-$TIXIVER* tigl3-devel-$TIGL3VER* tixi3-devel-$TIXIVER* tigl3-viewer-$TIGL3VER*"
    fi
elif [[ $PACK_TYPE == deb ]]; then
    if [[ $TIGLVER -eq "2" ]]; then
        whitelist="liboce-foundation1* liboce-modeling1* liboce-visualization1* libtigl-dev* libtixi-dev* libtigl2* libtixi2* tigl-viewer-$TIGL3VER*"
    elif [[ $TIGLVER -eq "3" ]]; then
        whitelist="libocct-foundation-7* libocct-modeling-algorithms-7*  libocct-modeling-data-7* libocct-visualization-7* libocct-data-exchange-7* libtigl3*_$TIGL3VER* libtixi3*_$TIXIVER* tigl3-viewer_$TIGL3VER*"
    fi
else
    echo "Error: unknown package type"
    exit 4
fi

for file in $filelist; do
    # select required files
    for whitefile in $whitelist; do
        if [[ $file == $whitefile ]]; then
        bin_file_list+=($file)

        #extract version number
        if [[ $file == libTIGL2-*.rpm ]]; then
            VERSION=`echo $file | awk '{split($0,array,"-")} END{print array[2]}'`
        elif [[ $file == libtigl3-?-3*.rpm ]]; then
            VERSION=`echo $file | awk '{split($0,array,"-")} END{print array[3]}'`
        elif [[ $file == libtigl2_*.deb ]] || [[ $file == libtigl3_*.deb ]]; then
            VERSION=`echo $file | awk '{split($0,array,"_")} END{print array[2]}' | awk '{split($0,array,"-")} END{print array[1]}'`
        fi
        fi
    done
done

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


#create start script for tiglviewer
echo "#!/bin/bash" > tiglviewer.sh
echo 'CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"' >> tiglviewer.sh
echo 'export LD_LIBRARY_PATH=$CURDIR/'$LIBDIR/':$LD_LIBRARY_PATH' >> tiglviewer.sh
if [[ $PACK_TYPE == deb ]]; then
    echo 'export LD_LIBRARY_PATH=$CURDIR/'$LIBDIR/$ARCH-linux-gnu/':$LD_LIBRARY_PATH' >> tiglviewer.sh
fi
if [[ $TIGLVER -eq "2" ]]; then
    echo '$CURDIR/bin/TIGLViewer' >> tiglviewer.sh
elif [[ $TIGLVER -eq "3" ]]; then
    echo '$CURDIR/bin/tiglviewer-3' >> tiglviewer.sh
fi
chmod +x tiglviewer.sh


cd ..


echo
echo "create package $NAME.tar.gz"
tar -czf $pwdir/$NAME.tar.gz $NAME

rm -rf $tmpdir

echo "Package generation done..."
