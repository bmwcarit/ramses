#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(ramses-sdk_TEXT_SUPPORT)
    list(APPEND CLIENT_SOURCES      impl/text/*.h
                                    impl/text/*.cpp)
    list(APPEND CLIENT_LIBS         freetype
                                    harfbuzz)
endif()

if(ramses-sdk_ENABLE_LOGIC)
    list(APPEND CLIENT_SOURCES      impl/logic/*.h
                                    impl/logic/*.cpp
                                    internal/logic/*.h
                                    internal/logic/*.cpp
                                    internal/logic/flatbuffers/generated/*.h
                                    internal/logic/flatbuffers/schemas/*.fbs)
    list(APPEND CLIENT_LIBS         sol2::sol2
                                    lua::lua
                                    ramses::flatbuffers
                                    fmt::fmt)
endif()

createModule(
    NAME                    ramses-client
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           .

    SRC_FILES               impl/*.h
                            impl/*.cpp
                            internal/*.h
                            internal/*.cpp
                            internal/glslEffectBlock/*.h
                            internal/glslEffectBlock/*.cpp
                            internal/ClientCommands/*.h
                            internal/ClientCommands/*.cpp
                            ${CLIENT_SOURCES}
    DEPENDENCIES            ramses-api
                            ramses-framework
                            ramses-glslang
                            ${CLIENT_LIBS}
)

if(TARGET FlatbufGen)
    add_dependencies(ramses-client FlatbufGen)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-client PRIVATE "-Wsuggest-override")
endif()
