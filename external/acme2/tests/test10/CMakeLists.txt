DEFINE_TEST("CTest disabled test")

SET(${PROJECT_NAME}_BUILD_TESTS OFF)

ACME_MODULE(
    NAME                    test10
    TYPE                    TEST

    FILES_SOURCE            test10.cpp
)

ASSERT_TARGET_NOT_EXISTS(test10 "target for test executable must not exist if tests are disabled")

END_TEST()
