#!/usr/bin/env python

#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import glob
import os
import sys
import subprocess
import re

def explode_path(path):
    rest, last = os.path.split(path)
    res = [last]
    while len(rest) > 0:
        rest, last = os.path.split(rest)
        res = [last] + res
    return res

def path_starts_with(path, start):
    ex = explode_path(path)
    return len(ex) > 0 and ex[0] == start

def path_starts_with_any_of(path, startList):
    for entry in startList:
        if path_starts_with(path, entry):
            return True
    return False

def get_files_in_directory_recursive(path, ext = None):
    res = []
    for (dir, _, files) in os.walk(path):
        for f in files:
            if not ext or os.path.splitext(f)[1] == ext:
                res.append(os.path.join(dir, f))
    return res

def get_versioned_ramses_include_dir(dir):
    entries = os.listdir(dir)
    version = None
    result = None
    for e in entries:
        full = os.path.join(dir, e)
        m = re.match(r'^ramses-(\d+\.\d+)$', e)
        if m:
            if result:
                print "ERROR: duplicate ramses directory", full, "already found", result
                exit(1)
            else:
                version = m.group(1)
                result = full
        else:
            print "ERROR: Unexpected directory", full
            exit(1)
    if not result:
        print "ERROR: did not find ramses version directory in", dir
        exit(1)

    # check version matches dir
    versionFile = os.path.join(result, "ramses-version")
    if os.path.isfile(versionFile):
        with open(versionFile, 'r') as f:
            content = f.read()
            m = re.search(r'RAMSES_VERSION={}\.'.format(re.escape(version)), content, re.MULTILINE)
            if not m:
                print "ERROR: ramses-version file invalid or does not match directory version", version
                exit(1)
    else:
        print "ERROR: version file not found at", versionFile
        exit(1)
    return result


def get_installed_include_dir(installDir):
    fullIncludeDir = os.path.join(installDir, 'include')
    fullDir = get_versioned_ramses_include_dir(fullIncludeDir)
    return fullDir

def get_installed_includes(installIncludeDir):
    allFiles = get_files_in_directory_recursive(installIncludeDir)
    relFiles = [os.path.relpath(h, installIncludeDir) for h in allFiles]
    filteredFiles = [h for h in relFiles if not path_starts_with_any_of(h, ['ramses-version'])]
    return filteredFiles

def get_source_api_headers(srcDir):
    baseDirs = []
    pattern = re.compile('.*/ramses-[^/]+-api/include$')
    for dirpath, dirs, _ in os.walk(srcDir):
        for d in dirs:
            c = os.path.join(dirpath, d)
            if pattern.match(c):
                baseDirs.append(c)

    res = []
    for dir in baseDirs:
        headers = get_files_in_directory_recursive(dir, '.h')
        res += [os.path.relpath(h, dir) for h in headers]
    return res


# --- main ---

if len(sys.argv) != 3:
    print 'Install header check: Arguments missing'
    print 'Usage: ' + sys.argv[0] + ' <ramses-sdk> <install-base>'
    sys.exit(1)

print "Run install header check"

srcDir = sys.argv[1]
installDir = sys.argv[2]

installedIncludeDir = get_installed_include_dir(installDir)

installedHeaders = get_installed_includes(installedIncludeDir)
srcApiHeaders = get_source_api_headers(srcDir)

# check wich headers are unexpected and which are missing
print "Check missing/too many headers"
installedSet = set(installedHeaders)
srcSet = set(srcApiHeaders)

unexpected = list(installedSet - srcSet)
missing = list(srcSet - installedSet)

if len(unexpected) > 0:
    print 'ERROR: Headers should not be installed\n  ' + '\n  '.join(unexpected)
if len(missing) > 0:
    print 'ERROR: Headers are missing from installation\n  ' + '\n  '.join(missing)
if len(unexpected) > 0 or len(missing) > 0:
    sys.exit(1)

# check installed header dependencies
print "Check header dependencies"
numDependencyErrors = 0
for h in installedHeaders:
    cmd = 'g++ -MM -I"{}" "{}/{}"'.format(installedIncludeDir, installedIncludeDir, h)
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out = p.communicate()
    if p.returncode > 0:
        print 'Header dependency check failed for: ', h
        print cmd
        print out[0]
        print out[1]
        numDependencyErrors += 1

if numDependencyErrors > 0:
    print "ERROR: found dependency errors in installed headers"
    sys.exit(1)

# check installed header with pedantic
print "Check headers with pedantic"
numPedanticErrors = 0
for h in installedHeaders:
    cmd = 'g++ -Werror -pedantic -I"{}" "{}/{}" -o /tmp/ramses-pedantic-header.o'.format(installedIncludeDir, installedIncludeDir, h)
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out = p.communicate()
    if p.returncode > 0:
        print 'Header pedantic check failed for: ', h
        print cmd
        print out[0]
        print out[1]
        numPedanticErrors += 1

if numPedanticErrors > 0:
    print "ERROR: found errors with pedantic in installed headers"
    sys.exit(1)

print "Done"

# normal exit
sys.exit(0)
