#!/usr/bin/python

import argparse
import subprocess
import os
import shutil


def run(cmd):
    try:
        p = subprocess.Popen(cmd, shell=True)
        p.communicate()
        status = p.returncode
        if status:
            raise
    except:
        print "An error has occured while running the following shell command:"
        print cmd
        exit(1)


def main():

    parser = argparse.ArgumentParser(description='Convert and upload change log to google code wiki and create PDF file')
    parser.add_argument('-i', '--input', default='../../ChangeLog', help='TIGL change log file')
    parser.add_argument('--local', action='store_true', help='only local files (no upload)')
    args = parser.parse_args()

    # create valid RST file
    print "Convert to RST..."
    rstfile = 'ChangeLog.rst'
    run('python changeLogToRST.py -i %s -o %s' % (args.input, rstfile))

    # convert to wiki format ('pip install wikir' necessary)
    print "Convert to wiki syntax..."
    wikifile = 'LastChanges.wiki'
    wikifile_abspath = os.path.abspath(wikifile)
    run('wikir ChangeLog.rst > %s' % wikifile)

    # add sub-headline to wikie
    print 'Add sub-headline to wiki file...'
    f = open(wikifile,  'r')
    lines = f.readlines()
    f.close()
    f = open(wikifile,  'w')
    f.write("#summary log of the last changes made in TIGL.\n")
    for line in lines:
        f.write(line)
    f.close()

    if not args.local:
        # check out tigl.wiki
        print "Check out tigl.wiki repo..."
        if os.path.exists('tigl.wiki'):
            shutil.rmtree('tigl.wiki')
        run('git clone https://code.google.com/p/tigl.wiki/')
    
        # overwrite LastChanges.wiki
        print 'Overwrite LastChanges.wiki...'
        os.chdir('tigl.wiki')
        shutil.move(wikifile_abspath, wikifile)
        # push repo
        print 'Upload changed LastChanges.wiki...'
        run('git add %s' % wikifile)
        run("git commit -m 'Updated ChangeLog'")
        run("git push")
        os.chdir('../')

    # Clean up
    print 'Clean up...'
    os.remove(rstfile)
    if not args.local:
        shutil.rmtree('tigl.wiki')
    else:
        os.remove(wikifile)

if __name__ == "__main__":
    main()
