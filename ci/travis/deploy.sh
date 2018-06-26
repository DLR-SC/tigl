# only deploy documentation on release builds
if [[ "$TRAVIS_OS_NAME" == linux && -n "$FTP_PASSWORD"  && -n "$TRAVIS_TAG" ]]; then
    sshpass -p $FTP_PASSWORD scp -r -o StrictHostKeyChecking=no install/share/doc/tigl/html/* $FTP_USER@frs.sourceforge.net:/home/project-web/tigl/htdocs/Doc
fi