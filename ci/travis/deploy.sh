#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == osx && -n "$FTP_PASSWORD" ]]; then
    sshpass -p $FTP_PASSWORD scp -o StrictHostKeyChecking=no *.dmg $FTP_USER@frs.sourceforge.net:/home/frs/project/tigl/Nightlies-macOS
fi
