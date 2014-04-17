#!/usr/bin/python

import argparse
import os


def main():

    parser = argparse.ArgumentParser(description='Convert TIGL change log to valid restructured text format')
    parser.add_argument('-i', '--input', default='ChangeLog', help='TIGL change log file')
    parser.add_argument('-o', '--output', default='ChangeLog.rst', help='TIGL change log RST file')
    args = parser.parse_args()

    f = open(os.path.normpath(args.input),  'r')
    lines = f.readlines()
    f.close()

    # convert to MD 
    f = open(os.path.normpath(args.output),  'w')
    f.write("Latest Changes\n")
    f.write("==============\n")
    f.write("\n")
    for line in lines:
        l=line.replace("    ", " ")
        f.write(l)
        if line.startswith('    - '):
            f.write('\n')
    f.close()

if __name__ == "__main__":
    main()
