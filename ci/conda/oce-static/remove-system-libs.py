#!/usr/bin/env python

# @author: Martin Siggel <martin.siggel@dlr.de>
#
# This script fixes the cmake exports file by
# removing explicit linking to system libraries

import sys, re

def remove_absolute_paths(line):
    """
    Removes libraries from the line that are found under /usr
    """

    if sys.platform == 'win32':
        return line
    elif sys.platform == 'darwin':
        return re.sub('/Applications/[-_a-zA-Z0-9/.]+.framework[;]?', '', line)
    else:
        return re.sub('/usr/[-_a-zA-Z0-9/]+.so[;]?', '', line)

def fix_paths(filename):
    with open(filename) as f:
        lines = f.readlines()

        # just select lines containing string IMPORTED_LINK_INTERFACE_LIBRARIES
        for i, line in enumerate(lines):
            if "IMPORTED_LINK_INTERFACE_LIBRARIES" in line or "INTERFACE_LINK_LIBRARIES" in line:
                lines[i] = remove_absolute_paths(line)

    fout = open(filename,'w')
    fout.write("".join(lines))
    fout.close()

if __name__ == "__main__":
    assert(len(sys.argv) == 2)

    filename = sys.argv[1]
    fix_paths(filename)
