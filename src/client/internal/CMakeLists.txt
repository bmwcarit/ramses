#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(ramses-sdk_TEXT_SUPPORT)
    list(APPEND TEXT_INTERNAL_LIBS      freetype
                                        harfbuzz)
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
                            logic/*.h
                            logic/*.cpp
                            logic/flatbuffers/generated/*.h
                            logic/flatbuffers/schemas/*.fbs
    DEPENDENCIES            ramses-api
                            ramses-framework-internal
                            ramses-glslang
                            sol2::sol2
                            lua::lua
                            ramses::flatbuffers
                            fmt::fmt
                            ${TEXT_INTERNAL_LIBS}
)

if(TARGET FlatbufGen)
    add_dependencies(ramses-client-internal FlatbufGen)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-client-internal PRIVATE "-Wsuggest-override")
endif()
