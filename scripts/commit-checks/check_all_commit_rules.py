#!/usr/bin/env python

#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

"""
Checks each commit for RAMSES-specific commit rules

"""

import os
import sys
import subprocess
import re

def get_current_commit(sdkroot):
    print "Run get_current_commit"
    process = subprocess.Popen(['git', 'show', '--summary', '--format=%P', 'HEAD'], cwd=sdkroot, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    parentCommits, stderroutput = process.communicate()
    parentCommits = parentCommits.strip()

    if stderroutput != "" or parentCommits == "":
        print "Could not read current Git commit parents! Error: {0}".format(stderroutput)
        return False

    commitListLines = parentCommits.split("\n")
    if len(commitListLines) == 0:
        print "Empty commit list, got {}".format(parentCommits)
        return False
    commitListFirstLine = commitListLines[0]
    commitList = commitListFirstLine.split()

    if len(commitList) > 1:
        currentCommit = commitList[-1]
        print "Change is a merge commit. Using correct parent commit"
    else:
        process = subprocess.Popen(['git', 'rev-parse', 'HEAD'], cwd=sdkroot, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        currentCommit, stderroutput = process.communicate()
        currentCommit = currentCommit.strip()

        if stderroutput != "" or currentCommit == "":
            print "Could not read current Git commit! Error: {0}".format(stderroutput)
            return False
    return currentCommit

def get_current_commit_contents(sdkroot, currentCommit):
    print "Verifying commit: {0}".format(currentCommit)

    process = subprocess.Popen(['git', 'log', '--format=%B', '-n', '1', currentCommit], cwd=sdkroot, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    commitMessage, stderroutput = process.communicate()

    if stderroutput != "" or commitMessage == "":
        print "Could not read current Git commit message! Error: {0}".format(stderroutput)
        return False

    print "Use commit: {}".format(commitMessage.split("\n")[0])

    process = subprocess.Popen(['git', 'diff-tree', '--no-commit-id', '--name-status', '-r', currentCommit], cwd=sdkroot, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    commitFileList, stderroutput = process.communicate()

    if stderroutput != "":
        print "Could not read commit contents! Error: {0}".format(stderroutput)
        return False

    if commitFileList == "":
        print "Warning: Commit has no changed files"

    result = {'message' : commitMessage,
              'touchedFiles' : [],
              'addedFiles' : [],
              'removedFiles' : [],
              'modifiedFiles' : []}

    for l in commitFileList.splitlines():
        stat, name  = l.strip().split('\t')
        result['touchedFiles'].append(name)
        if stat == 'A':
            result['addedFiles'].append(name)
        elif stat == 'D':
            result['removedFiles'].append(name)
        else:
            result['modifiedFiles'].append(name)

    return result

def check_commit_msg_contains_ticket_id(commitInfo):
    print "Run check_commit_msg_contains_ticket_id"

    magicWords = "NoTicket"

    commitMessage = commitInfo['message']
    hasTicketId = re.match("^\[RAMSES-\d+\]\s+", commitMessage)
    hasElvisId = re.match("^\[ELVIS-\d+\]\s+", commitMessage)
    isRevertCommit = re.match("^Revert \"\[RAMSES-\d+\]\s+", commitMessage)
    hasMagicWordsInCommitMessage = magicWords in commitMessage

    if not hasTicketId and not hasElvisId and not isRevertCommit and not hasMagicWordsInCommitMessage:
        errorMessage = ("Commit contains no ticket id in the form of '[RAMSES-xxxx] '.\n" +
                        "    If this change can not be assigned to any ticket, type:\n" +
                        "    \"{magicWords}\"\n" +
                        "    as line in the commit message.")

        print "WARNING: {0}".format(errorMessage.format(magicWords=magicWords))
        return False

    return True

def check_paths_must_have_file_changed_in_commit_rule(commitInfo, magicWords, checkPatterns, specialFile, errorMessage):
    commitMessage, changedFiles = commitInfo['message'], commitInfo['touchedFiles']
    hasViolations = False
    hasSpecialFileEntryEntry = specialFile in changedFiles
    hasMagicWordsInCommitMessage = magicWords in commitMessage

    if not hasSpecialFileEntryEntry and not hasMagicWordsInCommitMessage:
        for filename in changedFiles:
            if any(re.match(p, filename) for p in checkPatterns):
                hasViolations = True
                print "WARNING: {0}: {1}".format(filename, errorMessage.format(magicWords=magicWords, specialFile=specialFile))

    return not hasViolations


def check_api_change_in_changelog(commitInfo):
    print "Run check_api_change_in_changelog"

    errorString = ("Public API file changed, but change is not reflected in {specialFile}.\n" +
                   "    If this change does not have to be documented in the CHANGELOG, type:\n" +
                   "    \"{magicWords}\"\n" +
                   "    as line in the commit message.")
    patterns = ["^client/ramses-client-api/", "^client/ramses-text-api/", "^renderer/ramses-renderer-api/", "^framework/ramses-framework-api/"]
    return check_paths_must_have_file_changed_in_commit_rule(commitInfo, "NoApiChange", patterns, "CHANGELOG.txt", errorString)


def check_network_version_needs_change(commitInfo):
    print "Run check_network_version_needs_change"

    protocolVersionFile = "framework/Communication/TransportCommon/include/TransportCommon/RamsesTransportProtocolVersion.h"
    errorString = ("Format or semantic of network messages might have changed but is not reflected in\n" +
                   "    {specialFile}\n" +
                   "    If you are sure this change does not affect format or meaning of data on the wire at all type:\n" +
                   "    \"{magicWords}\"\n" +
                   "    as line in the commit message.")
    patterns = ["^framework/SceneGraph/Scene/include/Scene/SceneActionCollectionCreator.h",
                "^framework/SceneGraph/Scene/src/SceneActionCollectionCreator.cpp",
                "^framework/SceneGraph/Scene/src/SceneResourceChanges.cpp"]
    return check_paths_must_have_file_changed_in_commit_rule(commitInfo, "NoNetworkChange", patterns, protocolVersionFile, errorString)


def check_forbidden_file_extensions_added(commitInfo):
    print "Run check_forbidden_file_extensions_added"

    forbidden_extensions = ['bmp']
    magicWords = 'AcceptForbiddenFileTypes'

    hasViolations = False
    if magicWords not in commitInfo['message']:
        for f in commitInfo['addedFiles']:
            _, ext = os.path.splitext(f)
            if ext:
                ext = ext.lower()[1:]
                if any(e == ext for e in forbidden_extensions):
                    hasViolations = True
                    print "WARNING: {}: has forbidden extension {}.\n    If you really want to add it use \"{}\".\n    All forbidden extensions: {}".format(f, ext, magicWords, forbidden_extensions)

    return not hasViolations


def check_resource_filename_uniqueness(sdkroot):
    print "Run check_resource_filename_uniqueness"

    cmd = "git ls-files"
    p = subprocess.Popen(cmd, cwd=sdkroot, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()

    if p.returncode != 0 or stderr != "":
        print "Command failed\n  cmd: {}\n  returncode: {}\n  stderr: {}".format(cmd, p.returncode, stderr)
        return False

    resFiles = {}
    for line in stdout.split():
        line = line.strip()
        if "/res/" in line and not line.startswith('external/acme2/tests/'):
            filename = os.path.basename(line)
            resFiles[filename] = resFiles.get(filename, 0) + 1

    multipleRes = []
    for filename, count in resFiles.iteritems():
        if count > 1:
            multipleRes.append("  {} {}".format(count, filename))

    if len(multipleRes) > 0:
        print "WARNING: Found multiple occurences of the following resource files in different locations"
        print "\n".join(multipleRes)
        return False

    return True


import unittest

class TestFoo(unittest.TestCase):
    def test1(self):
        self.assertEqual(1, 2)

def main():
    if len(sys.argv) > 2:
        print """
Usage: check_all_commit_rules.py [<commit-hash>]
"""
        return False

    sdkroot = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".."))
    result = True

    if len(sys.argv) == 2:
        commit = sys.argv[1]
    else:
        commit = get_current_commit(sdkroot)
    commitInfo = commit and get_current_commit_contents(sdkroot, commit)

    if commitInfo:
        result = check_commit_msg_contains_ticket_id(commitInfo) and result
        result = check_api_change_in_changelog(commitInfo) and result
        result = check_network_version_needs_change(commitInfo) and result
        result = check_forbidden_file_extensions_added(commitInfo) and result
    else:
        result = False

    result = check_resource_filename_uniqueness(sdkroot) and result

    if not result:
        print "Commit check found errors"
        commitMessage, changedFiles = commitInfo['message'], commitInfo['touchedFiles']
        print "Used commit message:"
        print commitMessage
        print "Used list of changed files:"
        print "  " + "\n  ".join(changedFiles)
        print "Returning error from commit checker"
        return False

    print "All commit rules fulfilled! You can sleep well."
    return True


if __name__ == "__main__":
    if not main():
        exit(1)
