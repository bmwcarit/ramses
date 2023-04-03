#!/bin/bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# Have to have dot (graphviz) installed for this to work
# Files has .txt extension so that git tools can see it as plain text
dot -Tpng $SCRIPT_DIR/arch_diagram.dot.txt -o arch_diagram.png
