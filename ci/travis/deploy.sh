#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == osx && -n "$FTP_PASSWORD" ]]; then
    files=( "*.dmg" )
    PACKAGE="${files[0]}"
    ln -s $PACKAGE TiGL-LatestNightly-Darwin.dmg
    sshpass -p $FTP_PASSWORD scp -o StrictHostKeyChecking=no $PACKAGE TiGL-LatestNightly-Darwin.dmg $FTP_USER@frs.sourceforge.net:/home/frs/project/tigl/Nightlies-macOS
fi
