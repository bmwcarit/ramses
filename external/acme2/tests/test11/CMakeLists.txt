DEFINE_TEST("CTest enabled test")

SET(${PROJECT_NAME}_BUILD_TESTS ON)

ACME_MODULE(
    NAME                    test11
    TYPE                    TEST

    FILES_SOURCE            test11.cpp
)

ASSERT_TARGET_EXISTS(test11 "target for test executable must exist if tests are enabled")
ASSERT_SOURCES_CONTAIN(test11 test11.cpp "must contain source file directly given in module")

END_TEST()
