#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import click

import os
from pathlib import Path


class Common:
    def __init__(self):
        pass


@click.group()
@click.pass_context
def cli(ctx):
    ctx.obj = Common()


@cli.command()
@click.pass_obj
@click.argument('path')
@click.option('-e', '--encoding', default='utf-8')
def includes(conf, path, encoding):
    patterns = [
        ["#include \"ramses-framework-api/", "#include \"ramses/framework/"],
        ["#include <ramses-framework-api/", "#include <ramses/framework/"],

        ["#include \"ramses-client.h\"", "#include \"ramses/client/ramses-client.h\""],
        ["#include <ramses-client.h>", "#include <ramses/client/ramses-client.h>"],
        ["#include \"ramses-utils.h\"", "#include \"ramses/client/ramses-utils.h\""],
        ["#include <ramses-utils.h>", "#include <ramses/client/ramses-utils.h>"],
        ["#include \"ramses-client-api/", "#include \"ramses/client/"],
        ["#include <ramses-client-api/", "#include <ramses/client/"],

        ["#include \"ramses-text.h\"", "#include \"ramses/client/text/ramses-text.h\""],
        ["#include <ramses-text.h>", "#include <ramses/client/text/ramses-text.h>"],
        ["#include \"ramses-text-api/", "#include \"ramses/client/text/"],
        ["#include <ramses-text-api/", "#include <ramses/client/text/"],

        ["#include \"ramses-logic/", "#include \"ramses/client/logic/"],
        ["#include <ramses-logic/", "#include <ramses/client/logic/"],

        ["#include \"ramses-renderer-api/", "#include \"ramses/renderer/"],
        ["#include <ramses-renderer-api/", "#include <ramses/renderer/"],

        ["#include \"ramses/client/ERotationType.h\"", "#include \"ramses/framework/ERotationType.h\""],
        ["#include <ramses/client/ERotationType.h", "#include <ramses/framework/ERotationType.h"],
        ["#include \"ramses/client/EVisibilityMode.h", "#include \"ramses/framework/EVisibilityMode.h"],
        ["#include <ramses/client/EVisibilityMode.h", "#include <ramses/framework/EVisibilityMode.h"],
        ["#include \"ramses/client/EScenePublicationMode.h\"", "#include \"ramses/framework/EScenePublicationMode.h\""],
        ["#include <ramses/client/EScenePublicationMode.h", "#include <ramses/framework/EScenePublicationMode.h"],
    ]
    replace_patterns_in_files(conf, path, patterns, encoding)


def replace_patterns_in_files(conf, path, patterns, encoding):
    rootpath = Path(path)
    if not rootpath.exists():
        raise Exception(f"No folder '{rootpath}' found!")

    for root, _dirs, files in os.walk(rootpath):
        for filename in files:
            file = Path(root) / filename

            isExternal = ((rootpath / "external") in file.parents)
            if isExternal:
                print(f"Skipping file {file} in {root}...")
                continue

            if filename.endswith(".cpp") or filename.endswith(".h"):
                print(f"Processing file {file}...")

                with open(file, 'r', encoding=encoding) as f:
                    filedata = f.read()

                # Replace the target string
                for p in patterns:
                    filedata = filedata.replace(p[0], p[1])

                # Write the file out again
                with open(file, 'w', encoding=encoding) as f:
                    f.write(filedata)


if __name__ == '__main__':

    cli()
