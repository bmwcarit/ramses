#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(internal/RendererLib)
add_subdirectory(internal/Platform)

# This object library solves the problem of ramses-renderer
# dependency on Platform, while still allowing unit testing
# of ramses-renderer without that depdendency on Platform
# Since this is an object library it does not need to link to Platform.
# Only ramses-renderer has to link to Platform
# A fake platform factory lib is added to unit tests, which provides a fake implementation
# of PlatformFactory, that basically does nothing, but allows
# the tests to link to ramses-renderer-impl without linker error
createModule(
    NAME                    ramses-renderer-impl
    TYPE                    OBJECT
    ENABLE_INSTALL          OFF
    SRC_FILES               impl/*.h
                            impl/*.cpp
    DEPENDENCIES            ramses-renderer-internal
                            ramses-framework
)

createModule(
    NAME                    ramses-renderer
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    DEPENDENCIES            Platform
                            ramses-renderer-impl
)
