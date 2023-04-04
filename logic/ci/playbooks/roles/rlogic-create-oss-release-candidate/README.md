#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

This role creates an open source release candidate. This involves having a separate
checkout of the logic engine, which comes from a different (internal) repository,
executing a python script on it to remove proprietary code, substitutes external
dependencies with their unpatched upstream versions, and creates a commit with
info about the release candidate.

Required variables:
git_root_oss: path to folder where the oss mirror was checked out
