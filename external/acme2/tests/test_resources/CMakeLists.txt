DEFINE_TEST("test resources in acme modules")

ACME_MODULE(

    NAME               TestSingleExactResource
    TYPE               BINARY
    ENABLE_INSTALL     OFF

    FILES_SOURCE       dummy.cpp

    FILES_RESOURCE     res/resource.txt
)

ASSERT_TARGET_EXISTS(TestSingleExactResource "target for binary must exist")
ASSERT_RESOURCES_FOR_TARGET(TestSingleExactResource "resource.txt")

#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------

ACME_MODULE(

    NAME               TestTwoExactResources
    TYPE               BINARY
    ENABLE_INSTALL     OFF

    FILES_SOURCE       dummy.cpp

    FILES_RESOURCE     res/*
)

ASSERT_TARGET_EXISTS(TestTwoExactResources "target for binary must exist")
ASSERT_RESOURCES_FOR_TARGET(TestTwoExactResources "resource.txt;resource2.txt")

ACME_MODULE(

    NAME               TestWithExactResourceInSubfolder
    TYPE               BINARY
    ENABLE_INSTALL     OFF

    FILES_SOURCE       dummy.cpp

    FILES_RESOURCE     res/subfolder/sub_resource.txt
)

ASSERT_TARGET_EXISTS(TestWithExactResourceInSubfolder "target for binary must exist")
ASSERT_RESOURCES_FOR_TARGET(TestWithExactResourceInSubfolder "sub_resource.txt")

#---------------------------------------------------------------------------------
ACME_MODULE(

    NAME               TestWithAllResourcesInSubfolder
    TYPE               BINARY
    ENABLE_INSTALL     OFF

    FILES_SOURCE       dummy.cpp

    FILES_RESOURCE     res/subfolder/*
)

ASSERT_TARGET_EXISTS(TestWithAllResourcesInSubfolder "target for binary must exist")
ASSERT_RESOURCES_FOR_TARGET(TestWithAllResourcesInSubfolder "sub_resource.txt")

#---------------------------------------------------------------------------------
ACME_MODULE(

    NAME               TestWithTwoResourcessInSubfolder
    TYPE               BINARY
    ENABLE_INSTALL     OFF

    FILES_SOURCE       dummy.cpp

    FILES_RESOURCE     res/subfolder2/*
)

ASSERT_TARGET_EXISTS(TestWithTwoResourcessInSubfolder "target for binary must exist")
ASSERT_RESOURCES_FOR_TARGET(TestWithTwoResourcessInSubfolder "sub_resource2a.txt;sub_resource2b.txt")

#---------------------------------------------------------------------------------
ACME_MODULE(

    NAME               TestWithAllResourcessAndSubfolder
    TYPE               BINARY
    ENABLE_INSTALL     ON

    FILES_SOURCE       dummy.cpp

    FILES_RESOURCE     res/*
                       res/subfolder/*
                       res/subfolder2/*
)

ASSERT_TARGET_EXISTS(TestWithAllResourcessAndSubfolder "target for binary must exist")
ASSERT_RESOURCES_FOR_TARGET(TestWithAllResourcessAndSubfolder "resource.txt;resource2.txt;sub_resource.txt;sub_resource2a.txt;sub_resource2b.txt")

#---------------------------------------------------------------------------------
ACME_MODULE(

    NAME               TestWithExactResourcesInMainFolder
    TYPE               BINARY
    ENABLE_INSTALL     OFF

    FILES_SOURCE       dummy.cpp

    FILES_RESOURCE     resource_mainfolder.txt
)

ASSERT_TARGET_EXISTS(TestWithExactResourcesInMainFolder "target for binary must exist")
ASSERT_RESOURCES_FOR_TARGET(TestWithExactResourcesInMainFolder "resource_mainfolder.txt")

#---------------------------------------------------------------------------------
ACME_MODULE(

    NAME               TestWithAllResourcesInSharedFolder
    TYPE               BINARY
    ENABLE_INSTALL     OFF

    FILES_SOURCE       dummy.cpp

    FILES_RESOURCE     ../shared_resources/*
)

ASSERT_TARGET_EXISTS(TestWithAllResourcesInSharedFolder "target for binary must exist")
ASSERT_RESOURCES_FOR_TARGET(TestWithAllResourcesInSharedFolder "shared_resource.txt")

END_TEST()
