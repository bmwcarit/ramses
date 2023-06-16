#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

function(add_example)
    cmake_parse_arguments(
        EXAMPLE             # Prefix of parsed args
        "MAKE_TEST"         # Options
        "NAME"              # Single-value args
        "FILES"             # Multi-value-args
        ${ARGN}
    )

    createModule(
        NAME                    ${EXAMPLE_NAME}
        TYPE                    BINARY
        INCLUDE_PATHS           shared
        ENABLE_INSTALL          OFF
        SRC_FILES               ${EXAMPLE_FILES}
        DEPENDENCIES            ramses-shared-lib
    )

    if(TARGET ${EXAMPLE_NAME} AND ramses-sdk_BUILD_TESTS AND ${EXAMPLE_MAKE_TEST})
        MakeTestFromTarget(
            TARGET ${EXAMPLE_NAME}
            SUFFIX UNITTEST)
    endif()
endfunction()

add_example(NAME 00_minimal               FILES 00_minimal/main.cpp               MAKE_TEST)
add_example(NAME 01a_primitive_properties FILES 01a_primitive_properties/main.cpp MAKE_TEST)
add_example(NAME 01b_struct_properties    FILES 01b_struct_properties/main.cpp    MAKE_TEST)
add_example(NAME 01c_array_properties     FILES 01c_array_properties/main.cpp     MAKE_TEST)
add_example(NAME 02_errors_compile_time   FILES 02_errors_compile_time/main.cpp   MAKE_TEST)
add_example(NAME 03_errors_runtime        FILES 03_errors_runtime/main.cpp        MAKE_TEST)
# Can't execute ramses runtime example as test - it has a loop which blocks test automation and wastes resources
add_example(NAME 04_ramses_scene          FILES 04_ramses_scene/main.cpp)
add_example(NAME 05_serialization         FILES 05_serialization/main.cpp         MAKE_TEST)
add_example(NAME 07_links                 FILES 07_links/main.cpp                 MAKE_TEST)
add_example(NAME 09_modules               FILES 09_modules/main.cpp               MAKE_TEST)
add_example(NAME 10_globals               FILES 10_globals/main.cpp               MAKE_TEST)
add_example(NAME 11_interfaces            FILES 11_interfaces/main.cpp            MAKE_TEST)

# Can't execute ramses animation examples as test - they have a loop which blocks test automation and wastes resources
add_example(NAME 08a_static_animation     FILES 08a_static_animation/main.cpp)
add_example(NAME 08b_dynamic_animation    FILES 08b_dynamic_animation/main.cpp)
# Can't execute as test - it is essentially animation with loop which blocks test automation and wastes resources
add_example(NAME 14_skinbinding           FILES 14_skinbinding/main.cpp)
add_example(NAME 12_anchor_point          FILES 12_anchor_point/main.cpp)
add_example(NAME 13_render_order          FILES 13_render_order/main.cpp)
add_example(NAME 15_meshnodebinding       FILES 15_meshnodebinding/main.cpp)
