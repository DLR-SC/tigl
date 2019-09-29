#!/bin/bash

# only deploy nightly builds, not regular builds 
if [[ "$TRAVIS_OS_NAME" == osx && -n "$FTP_PASSWORD" && "$TIGL_NIGHTLY" == "ON" ]]; then
    files=( "*.dmg" )
    PACKAGE="${files[0]}"
    ln -s $PACKAGE TiGL-Nightly-Latest-Darwin.dmg
    sshpass -p $FTP_PASSWORD scp -o StrictHostKeyChecking=no $PACKAGE TiGL-Nightly-Latest-Darwin.dmg $FTP_USER@frs.sourceforge.net:/home/frs/project/tigl/Nightlies-macOS
fi

# only deploy documentation on release builds
if [[ "$TRAVIS_OS_NAME" == linux && -n "$FTP_PASSWORD"  && -n "$TRAVIS_TAG" ]]; then
    sshpass -p $FTP_PASSWORD scp -r -o StrictHostKeyChecking=no install/share/doc/tigl3/html/* $FTP_USER@frs.sourceforge.net:/home/project-web/tigl/htdocs/Doc
fi
