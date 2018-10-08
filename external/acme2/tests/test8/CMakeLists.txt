DEFINE_TEST("exe->staticA->staticB")

ACME_MODULE(
    NAME                    executable_static_static_staticB
    TYPE                    STATIC_LIBRARY

    FILES_SOURCE            test8_staticB/test8_staticB.cpp
    INCLUDE_BASE            test8_staticB/include
    FILES_PRIVATE_HEADER     test8_staticB/include/*.h
)

ACME_MODULE(
    NAME                    executable_static_static_staticA
    TYPE                    STATIC_LIBRARY

    FILES_SOURCE            test8_staticA/test8_staticA.cpp
    INCLUDE_BASE            test8_staticA/include
    FILES_PRIVATE_HEADER     test8_staticA/include/*.h

    DEPENDENCIES            executable_static_static_staticB
)

ACME_MODULE(
    NAME                    executable_static_static_exe
    TYPE                    BINARY

    FILES_SOURCE            test8_executable/executable_static_object_executable.cpp
    INCLUDE_BASE            test8_executable/include
    FILES_PRIVATE_HEADER     test8_executable/include/*.h

    DEPENDENCIES            executable_static_static_staticA
)

ASSERT_TARGET_EXISTS(executable_static_static_staticB "target for static library B must exist")
ASSERT_TARGET_EXISTS(executable_static_static_staticA "target for static library A must exist")
ASSERT_TARGET_EXISTS(executable_static_static_exe "target for executable must exist")

# static library B
ASSERT_INCLUDES_CONTAIN(     executable_static_static_staticB "test8_staticB/include" "static library B must have its own include directories")
ASSERT_INCLUDES_DONT_CONTAIN(executable_static_static_staticB "test8_staticA" "static library B must not have static library A includes")
ASSERT_INCLUDES_DONT_CONTAIN(executable_static_static_staticB "test8_executable" "static library B must not have executables includes")
ASSERT_SOURCES_CONTAIN(      executable_static_static_staticB "test8_staticB/test8_staticB.cpp" "static library B must have its source file")

# static library A
ASSERT_INCLUDES_CONTAIN(     executable_static_static_staticA "test8_staticA/include" "static library A must have its own include directories")
ASSERT_INCLUDES_CONTAIN(     executable_static_static_staticA "test8_staticB/include" "static library A must have static library B includes")
ASSERT_INCLUDES_DONT_CONTAIN(executable_static_static_staticA "test8_executable" "static library A must not have exe includes")
ASSERT_SOURCES_CONTAIN(      executable_static_static_staticA "test8_staticA/test8_staticA.cpp" "static library A must have its source file")

#executable
ASSERT_INCLUDES_CONTAIN(executable_static_static_exe "test8_executable/include" "executable must have its own include-dirs")
ASSERT_INCLUDES_CONTAIN(executable_static_static_exe "test8_staticA/include" "executable must have static library A include-dirs")
ASSERT_INCLUDES_CONTAIN(executable_static_static_exe "test8_staticB/include" "executable must have static library B include-dirs")
ASSERT_LINK_LIBRARIES_CONTAIN(executable_static_static_exe "test8_staticA" "executable must link static library A automatically")
ASSERT_INTERFACE_LINK_LIBRARIES_CONTAIN(executable_static_static_staticA "test8_staticB" "library A must expose dependency to static library B")

END_TEST()
