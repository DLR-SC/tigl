#!/usr/bin/python

import argparse


def main():

    parser = argparse.ArgumentParser(description='Convert TIGL change log to valid restructured text format')
    parser.add_argument('-i', '--input', default='ChangeLog', help='TIGL change log file')
    parser.add_argument('-o', '--output', default='ChangeLog.rst', help='TIGL change log RST file')
    args = parser.parse_args()

    f = open(args.input,  'r')
    lines = f.readlines()
    f.close()
    f = open(args.output,  'w')
    for line in lines:
        f.write(line)
        if line.startswith('    - '):
            f.write('\n')
    f.close()

if __name__ == "__main__":
    main()
