#  -------------------------------------------------------------------------
#  Copyright (C) 2024 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# This library solves the problem of allowing ramses-renderer to have
# dependency on Platform, while still allowing unit testing
# of ramses-renderer without that depdendency on Platform
# The library allows unit test to link to ramses-renderer-impl
# without producing linker error

createModule(
    NAME                    PlatformFactoryFake
    TYPE                    STATIC_LIBRARY
    SRC_FILES               *.h
                            *.cpp
    DEPENDENCIES            ramses-renderer-internal
)
