#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(internal)

if(ramses-sdk_TEXT_SUPPORT)
    list(APPEND CLIENT_IMPL_SOURCES     impl/text/*.h
                                        impl/text/*.cpp)
endif()

if(ramses-sdk_ENABLE_LOGIC)
    list(APPEND CLIENT_IMPL_SOURCES     impl/logic/*.h
                                        impl/logic/*.cpp)
endif()

createModule(
    NAME                    ramses-client
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    SRC_FILES               impl/*.h
                            impl/*.cpp
                            ${CLIENT_IMPL_SOURCES}
    DEPENDENCIES            ramses-client-internal
                            ramses-framework
)
