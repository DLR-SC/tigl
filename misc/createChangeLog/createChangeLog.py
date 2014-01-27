#!/usr/bin/python

import argparse
import subprocess
import datetime


def run(cmd):
    return subprocess.check_output(cmd, shell=True)


# TIGL changelog (subset of restructured text)
class changelog:

    def __init__(self):
        self.body = ''
        self.tab = '    '

    def save(self, filename):
        f = open(filename, 'w')
        f.write(self.body)
        f.close()

    def plot(self):
        print self.body

    def blank(self):
        self.body += '\n'

    def date(self):
        self.body += datetime.datetime.now().strftime("%d/%m/%Y") + '\n'

    def headline(self, text):
        self.body += text + '\n'
        for l in range(0, len(text)):
            self.body += '='

    def subheadline(self, text):
        self.body += text + '\n'
        for l in range(0, len(text)):
            self.body += '-'
        self.body += '\n'

    def item(self, text):
        self.body += self.tab
        self.body += '- '
        self.body += text + '\n'

    def subitem(self, text):
        self.body += self.tab
        self.body += self.tab
        self.body += '- '
        self.body += text + '\n'

    def subsubitem(self, text):
        self.body += self.tab
        self.body += self.tab
        self.body += self.tab
        self.body += '- '
        self.body += text + '\n'

    def items(self, name, items):
        if items:
            self.body += name + '\n'
            for it in items:
                self.subitem(it)
            self.blank()

    def subitems(self, name, items):
        if items:
            self.body += self.tab
            self.body += name + '\n'
            for it in items:
                self.subitem(it)
            self.blank()

# FILTER
def isCategoryWord(word, buzzwords, prefixes=[], suffixes=[], contains=[]):
    boolCont = []
    for prefix in prefixes:
        boolCont.append(word.startswith(prefix))
    for suffix in suffixes:
        boolCont.append(word.endswith(suffix))
    for contain in contains:
        boolCont.append(contain in word and word != contain)
    for buzzword in buzzwords:
        boolCont.append(word.lower == buzzword.lower)
    return any(boolCont)

def isCategoryLine(line, operator, buzzwords, prefixes=[], suffixes=[], contains=[]):
    boolCont = []
    for word in line:
        boolCont.append(isCategoryWord(word, buzzwords, prefixes, suffixes, contains))
    return operator(boolCont)


# decorate code words e.g. tigl -> `tigl`
def decorate(logs):
    codePrefixes = ['tixi', 'tigl', 'CCPACS', 'ITigl', 'CTigl']
    codeSuffixes = ['.sh', '.cpp', '.h']
    codeContains = ['Wing', 'Fuselage']
    codeBuzzwords = []
    logs_decorated = []
    for line in logs:
        tiglwords = []
        for word in line.split():
            if isCategoryWord(word, codeBuzzwords, codePrefixes, codeSuffixes, codeContains):
                tiglwords.append(word)
        new_line = line
        for tword in tiglwords:
            new_line = new_line.replace(' ' + tword + ' ', ' ``' + tword + '`` ')
        logs_decorated.append(new_line)
    logs = logs_decorated
    return logs

# get fixes if fixed occurs (case insensitive)
def getFixes(logs):
    fixBuzzwords = ['fixes', 'fixed', 'bugfix', 'bugfixes', 'fix']
    fixes = []
    for line in logs:
        if isCategoryLine(line, any, fixBuzzwords):
            fixes.append(line)
    return fixes

# get API changes
def getChangedAPI(logs):
    changesBuzzwords = ['changed', 'api']
    changes = []
    for line in logs:
        if isCategoryLine(line, all, changesBuzzwords):
            changes.append(line)
    return changes


# get new API function
def getNewAPI(logs):
    newBuzzwords = ['new', 'api']
    new = []
    for line in logs:
        if isCategoryLine(line, all, newBuzzwords):
            new.append(line)
    return new


def main():

    parser = argparse.ArgumentParser(description='Creates TIGL release notes')
    parser.add_argument('-n', '--new_tag', help='tag of the new release', required=True)
    parser.add_argument('-o', '--tag', help='tag of the latest release')
    parser.add_argument('-f', '--file', default='../../ChangeLog', help='Changelog file to prepend to')
    parser.add_argument('--dry', action='store_true', help='Dry run, print out draft')

    args = parser.parse_args()

    # check if on master branch
    branch = run("git rev-parse --abbrev-ref HEAD").rsplit('\n')[0]
    print branch
    if branch != 'master':
        print "You are not on the master branch! Break."
        exit(1)

    # get relevant commit messages
    last_commit = run("git rev-list %s  | head -n 1" % args.tag).rsplit('\n')[0]
    current_commit = run("git rev-parse HEAD").rsplit('\n')[0]

    # get commit message heads
    logs = run("git log --oneline %s..%s" % (last_commit, current_commit)).split('\n')
    logs = [l.partition(" ")[2] for l in logs if l]

    # get commit messages and
    logs = run("git log --format='###seperator###%%s -- %%b' %s..%s" % (last_commit, current_commit)).split('###seperator###')
    # skip message body if non-existent
    logs = [l.rstrip(' -- \n') for l in logs if l]
    # remove newlines
    logs = [l.replace('\n', ' ') for l in logs]

    # get normal commit message heads
    #logs = run("git log --oneline --no-merges %s..%s" % (last_commit, current_commit)).split('\n')
    #logs = [l.partition(" ")[2] for l in logs if l]
    # get merge commit message heads
    #merges = run("git log --oneline --merges %s..%s" % (last_commit, current_commit)).split('\n')
    #merges = [l.partition(" ")[2] for l in merges if l]
    # filter the merges into master branch
    # e.g. get 'MacCalculation' from "421218c Merge branch 'MacCalculation'"
    # and filter out "d29b6c8 Merge branch 'master' of https://code.google.com/p/tigl"
    #merges = [l.split(' ')[2] for l in merges if len(l.split(' '))==3 ]

    # decorate code words in logs
    logs = decorate(logs)

    # split between tigl and tigl-viewer messages
    viewer = filter(lambda l: 'tiglviewer' in l.lower(), logs)
    logs = filter(lambda l: 'tiglviewer' not in l.lower(), logs)

    # filter out changed api messages
    changed_api = getChangedAPI(logs)
    # filter out fix messages
    fixes = getFixes(logs)
    # filter out new api messages
    new_api = getNewAPI(logs)
    # the rest is general changes
    general_changes = [l for l in logs if not l in changed_api + new_api + fixes]

    # sort messages alphabetically
    changed_api.sort()
    fixes.sort()
    new_api.sort()
    general_changes.sort()
    viewer.sort()

    cl = changelog()
    cl.subheadline("Version " + args.new_tag)
    cl.date()
    cl.blank()
    cl.subitems('- Changed API:', changed_api)
    cl.subitems('- General changes:', general_changes)
    cl.subitems('- New API functions:', new_api)
    cl.subitems('- Fixes:', fixes)
    cl.subitems('- TiGLViewer:', viewer)
    cl.blank()
    #cl.save('ChangeLog.tmp')
    if args.dry:
        cl.plot()
    else:
        # prepend to changelog file
        f = open(args.file,  'r')
        lines = f.readlines()
        f.close()
        f = open(args.file,  'w')
        f.write(cl.body)
        for l in lines:
            f.write(l)
        f.close()
        print "written new change log to ", args.file

if __name__ == "__main__":
    main()
