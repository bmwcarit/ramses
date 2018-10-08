DEFINE_TEST("Executable depending on definition from a find script")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

ACME_MODULE(
    NAME                    test3
    TYPE                    BINARY
    
    FILES_SOURCE            test3.cpp
    
    DEPENDENCIES            CustomLibrary
)

ASSERT_TARGET_EXISTS(test3 "target for executable must exist")

ASSERT_INCLUDES_CONTAIN(test3 "/customIncludeFolder" "executable must get include variable defined in find script because of dependency")
ASSERT_LINK_LIBRARIES_CONTAIN(test3 "customLibrary" "executable must get library-name defined in find script for linking")

END_TEST()
