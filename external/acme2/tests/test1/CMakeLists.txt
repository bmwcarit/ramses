DEFINE_TEST("Simple binary from sources")

ACME_MODULE(
    NAME                    test1
    TYPE                    BINARY
    
    FILES_SOURCE            test1.cpp
)

ASSERT_TARGET_EXISTS(test1 "target for executable must exist")
ASSERT_SOURCES_CONTAIN(test1 test1.cpp "must contain source file directly given in module")

END_TEST()
