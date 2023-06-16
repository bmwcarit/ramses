#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB src *.cpp)

add_executable(benchmarks
    ${src}
)

target_link_libraries(benchmarks
    PRIVATE
        rlogic::ramses-logic-static
        ramses::google-benchmark-main
        fmt::fmt
        sol2::sol2
        lua::lua
)

target_include_directories(benchmarks
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/logic/lib/flatbuffers>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/logic/lib>
)

folderizeTarget(benchmarks)
