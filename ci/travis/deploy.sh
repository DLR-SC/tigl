#!/bin/bash

# only deploy nightly builds, not regular builds 
if [[ "$TRAVIS_OS_NAME" == osx && -n "$FTP_PASSWORD" && "$TIGL_NIGHTLY" == "ON" ]]; then
    files=( "*.dmg" )
    PACKAGE="${files[0]}"
    ln -s $PACKAGE TiGL-Nightly-Latest-Darwin.dmg
    sshpass -p $FTP_PASSWORD scp -o StrictHostKeyChecking=no $PACKAGE TiGL-Nightly-Latest-Darwin.dmg $FTP_USER@frs.sourceforge.net:/home/frs/project/tigl/Nightlies-macOS
fi
