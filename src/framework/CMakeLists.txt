#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(internal)

createModule(
    NAME                    ramses-framework
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    SRC_FILES               impl/*.h
                            impl/*.cpp
    DEPENDENCIES            ramses-framework-internal
    )
