DEFINE_TEST("Executable depending on static lib")

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    test2_staticLibrary
    TYPE                    STATIC_LIBRARY

    FILES_SOURCE            test2_staticLibrary.cpp
    INCLUDE_BASE            test2_include
    FILES_PRIVATE_HEADER     test2_include/*.h
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    test2_executable
    TYPE                    BINARY

    FILES_SOURCE            test2.cpp

    INCLUDE_BASE            test2_executable
    FILES_PRIVATE_HEADER     test2_executable/*.h

    DEPENDENCIES            test2_staticLibrary
)

ASSERT_TARGET_EXISTS(test2_staticLibrary "target for static lib must exist")
ASSERT_TARGET_EXISTS(test2_executable "target for executable must exist")

# check the static lib
ASSERT_INCLUDES_CONTAIN(test2_staticLibrary "test2_include" "lib must include its own include-dirs")
ASSERT_INCLUDES_DONT_CONTAIN(test2_staticLibrary "test2_executable" "lib must not get includes of executable")
ASSERT_SOURCES_CONTAIN(test2_staticLibrary "test2_staticLibrary.cpp" "lib must contain its source")
ASSERT_SOURCES_DONT_CONTAIN(test2_staticLibrary "test2.cpp" "lib must not contain executable's source")

# check executable (transitive properties)
ASSERT_LINK_LIBRARIES_CONTAIN(test2_executable "test2_staticLibrary" "executable must link static lib automatically")

END_TEST()
