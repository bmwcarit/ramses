#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

This publishes an open source release candidate which was created by
rlogic-create-oss-release-candidate.

Required variables:
git_root_oss: path to folder where the oss mirror was checked out

Other requirements:
* must be triggered with a tag, not with a branch
* tag must be on a protected branch
* tag must match version in top-level CMakeLists (vX.Y.Z) -> X, Y, Z must be equal to major, minor and patch numbers
* tag version must be mentioned in the changelog.md
