#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys
import os
from pathlib import Path
import argparse
import json
import subprocess
import re
import html
import github3


def get_sdkroot():
    """Get sdk root folder by asking git"""
    return subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                   cwd=os.path.dirname(os.path.realpath(__file__))).decode('utf-8').strip()


def load_coverage(fp):
    with open(fp, 'r') as f:
        content = json.load(f)
    assert content['type'] == 'llvm.coverage.json.export'
    assert content['version'].startswith('2')

    sdkroot = get_sdkroot()
    res = {}
    for f in content['data'][0]['functions']:
        try:
            fname = str(Path(f['filenames'][0]).relative_to(sdkroot))
            e = res.get(fname, [])
            e.append(f)
            res[fname] = e
        except ValueError:
            pass
    return res


def merge_file_regions(regions):
    regstate = {}
    for reg in regions:
        line_start, col_start, line_end, col_end, cnt, file_id, exp_file_id, reg_kind = reg
        if file_id != 0 or exp_file_id != 0 or reg_kind != 0:
            continue
        key = (line_start, col_start, line_end, col_end)
        if key in regstate:
            existing_reg = regstate[key]
            existing_reg[4] += cnt
            regstate[key] = existing_reg
        else:
            regstate[key] = [line_start, col_start, line_end, col_end, cnt]
    return sorted(regstate.values(), key=lambda x: (x[0], x[1]))


def line_coverage_for_file(cov, fname):
    if fname not in cov:
        return {}
    raw_regions = []
    for e in cov[fname]:
        raw_regions.extend(e['regions'])
    regions = merge_file_regions(raw_regions)
    covmap = {}
    for reg in regions:
        line_start, col_start, line_end, col_end, cnt = reg
        for line_idx in range(line_start, line_end + 1):
            covmap[line_idx] = cnt
    return covmap


def create_annotated_patch_html(patch, cov):
    html_template = """
<html>
  <head>
    <style>
    table td { border: 0; }
    .lnum { padding-right: 30px; padding-left: 10px; text-align: right; color: grey; }
    .cnt { padding-right: 10px; padding-left: 10px; text-align: right; border-collapse: separate; border-radius: 5px; }
    .cntgood { background-color: #e0e0ff; }
    .cntbad { background-color: LightCoral; }
    .minus3 { color: red; }
    .plus3 { color: green; }
    .comment { color: blue; }
    .minus { color: red; }
    .plus { color: green; }
    .at { color: purple; }
    .line { padding-left: 20px; }
    .linebad { background-color: #fad1d0; border-collapse: separate; border-radius: 5px; }
    .text {}
    </style>
  </head>
  <body>
    <pre>
    <table>
      <thead>
        <tr><th>Line</th><th>Coverage</th><th></th>
      </thead>
      <tbody>
    %ROWS%
      </tbody>
    <table>
    </pre>
  </body>
</html>
"""
    file_re = re.compile(r'\+\+\+ b/(.*)')
    hunk_start_re = re.compile(r'^@@ -\d+(?:,\d+)? \+(\d+)')

    rows = []
    current_idx = 0
    totalTestedLines = 0
    totalUntestedLines = 0
    for line in patch:
        escaped_line = html.escape(line).replace(' ', '&nbsp;')

        need_cnt = need_cov = False
        if line.startswith('+++ /dev/null'):
            # file was deleted
            cls = 'plus3'
        elif line.startswith('+++ '):
            cls = 'plus3'
            current_file = file_re.search(line).group(1)
            file_cov = line_coverage_for_file(cov, current_file)
        elif line.startswith('--- '):
            cls = 'minus3'
        elif line.startswith('-'):
            cls = 'minus'
        elif line.startswith('+'):
            cls = 'plus'
            need_cnt = True
            need_cov = True
        elif line.startswith(' '):
            cls = 'text'
            need_cnt = True
        elif line.startswith('@@ '):
            cls = 'at'
            current_idx = int(hunk_start_re.search(line).group(1))
        else:
            cls = 'comment'

        if need_cnt:
            cnt = file_cov.get(current_idx, '')
            cnt_cls = 'cnt'
            if cnt != '':
                if cnt > 0:
                    cnt_cls += ' cntgood'
                    totalTestedLines += 1
                else:
                    cnt_cls += ' cntbad'
                    if need_cov:
                        cls += ' linebad'
                        totalUntestedLines += 1

            rows.append(f'<tr><td class="lnum">{current_idx}</td><td class="{cnt_cls}">{cnt}</td><td class="line {cls}">{escaped_line}</td></tr>')
            current_idx += 1
        else:
            rows.append(f'<tr><td class="lnum"></td><td class="cnt"></td><td class="line {cls}">{escaped_line}</td></tr>')

    doc = html_template.replace('%ROWS%', '\n'.join(rows))
    return {
        'html_report': doc,
        'tested_lines': totalTestedLines,
        'untested_lines': totalUntestedLines
    }


def get_api_token():
    """Read github API token from file"""
    api_token_file = os.path.expanduser("~/.github-token")
    if not os.path.exists(api_token_file):
        raise Exception("Token file not found at " + api_token_file)
    with open(api_token_file, 'r') as f:
        return f.read().strip()


def get_github_pr(github_url, owner, repo, pr_num):
    gh = github3.github.GitHubEnterprise(url=github_url, token=get_api_token())
    repo = gh.repository(owner, repo)
    return repo.pull_request(pr_num)


def download_patch_from_pr(pr):
    return pr.diff().decode('utf-8')


def post_issue_comment(link_base, html, pr, annotated_patch):
    html_file = Path(html).name
    body = f'Check out [PR coverage]({link_base}/{html_file})\n\nCoverage report \
        summary [tested:{annotated_patch["tested_lines"]}] [untested:{annotated_patch["untested_lines"]}]'
    pr.create_comment(body)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--coverage', required=True, help='llvm-cov coverage json export')
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--diff-file', help='Path to diff file')
    group.add_argument('--pr', type=int, help='Github PR#')
    parser.add_argument('--github-url', required=True, help='Github URL')
    parser.add_argument('--repo-owner', required=True, help='Project owner (org)')
    parser.add_argument('--repo-name', required=True, help='Project name (repo)')
    parser.add_argument('--html-output', required=True, help='Location of html output file')
    parser.add_argument('--post-log-link-in-issue', required=False, default=None,
                        help='Post link to <arg>/<html-output name> to PR given in --pr')
    args = parser.parse_args()

    if args.post_log_link_in_issue and not args.pr:
        parser.error('--post-log-link-in-issue require --pr')

    cov = load_coverage(args.coverage)
    if args.diff_file:
        patch = Path(args.diff_file).read_text()
    elif args.pr:
        pr = get_github_pr(github_url=args.github_url, owner=args.repo_owner, repo=args.repo_name, pr_num=args.pr)
        patch = download_patch_from_pr(pr)

    annotated_patch = create_annotated_patch_html(patch.split('\n'), cov)
    Path(args.html_output).write_text(annotated_patch['html_report'])

    if args.post_log_link_in_issue:
        post_issue_comment(args.post_log_link_in_issue, args.html_output, pr, annotated_patch)


if __name__ == '__main__':
    sys.exit(main())
