#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(ramses-sdk_TEXT_SUPPORT)
    list(APPEND CLIENT_INTERNAL_LIBS    freetype
                                        harfbuzz)
endif()

if(ramses-sdk_ENABLE_LOGIC)
    list(APPEND CLIENT_INTERNAL_SOURCES logic/*.h
                                        logic/*.cpp
                                        logic/flatbuffers/generated/*.h
                                        logic/flatbuffers/schemas/*.fbs)
    list(APPEND CLIENT_INTERNAL_LIBS    sol2::sol2
                                        lua::lua
                                        ramses::flatbuffers
                                        fmt::fmt)
endif()

createModule(
    NAME                    ramses-client-internal
    TYPE                    OBJECT
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           ${PROJECT_SOURCE_DIR}/src/client

    SRC_FILES               *.h
                            *.cpp
                            glslEffectBlock/*.h
                            glslEffectBlock/*.cpp
                            ClientCommands/*.h
                            ClientCommands/*.cpp
                            ${CLIENT_INTERNAL_SOURCES}
    DEPENDENCIES            ramses-api
                            ramses-framework-internal
                            ramses-glslang
                            ${CLIENT_INTERNAL_LIBS}
)

if(TARGET FlatbufGen)
    add_dependencies(ramses-client-internal FlatbufGen)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-client-internal PRIVATE "-Wsuggest-override")
endif()
