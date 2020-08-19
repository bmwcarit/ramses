CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../acme2.cmake)

MACRO(TEST_MSG MSG DETAILS)
    MESSAGE("${TAB_SPACE} ${MSG}: ${DETAILS}")
ENDMACRO()

MACRO(DEFINE_TEST DESCRIPTION)
    MESSAGE("[----------]")
    MESSAGE("[ RUN      ] ${DESCRIPTION}")
    SET(TAB_SPACE "${TAB_SPACE}  ")
ENDMACRO()

MACRO(END_TEST)
    MESSAGE("[       OK ] ${DESCRIPTION}")
ENDMACRO()

MACRO(EXPECT_STR_CONTAINS THESTRING SUBSTRING)
    IF("${THESTRING}" MATCHES "${SUBSTRING}")
    ELSE()
        MESSAGE(FATAL_ERROR  "does not contain")
    ENDIF()
ENDMACRO()

MACRO(ASSERT_INCLUDES_CONTAIN THETARGET INCLUDETOLOOKFOR ERRORMSG)
    IF (NOT TARGET ${THETARGET})
        MESSAGE(FATAL_ERROR ERRORMSG "(was not target)" )
    ENDIF()
    GET_TARGET_PROPERTY(INC_DIRS "${THETARGET}" INCLUDE_DIRECTORIES)
    IF("${INC_DIRS}" MATCHES "${INCLUDETOLOOKFOR}")
    ELSE()
        TEST_MSG(${ERRORMSG} "(Include was contained)")
        MESSAGE(FATAL_ERROR "")
    ENDIF()
ENDMACRO()

MACRO(ASSERT_INCLUDES_DONT_CONTAIN THETARGET INCLUDETOLOOKFOR ERRORMSG)
    IF (NOT TARGET ${THETARGET})
        MESSAGE(FATAL_ERROR ERRORMSG "(was not target)" )
    ENDIF()
    GET_TARGET_PROPERTY(INC_DIRS "${THETARGET}" INCLUDE_DIRECTORIES)
    IF("${INC_DIRS}" MATCHES "${INCLUDETOLOOKFOR}")
        TEST_MSG(${ERRORMSG} "(Include was not contained)")
        MESSAGE(FATAL_ERROR "")
    ENDIF()
ENDMACRO()

MACRO(ASSERT_SOURCES_CONTAIN THETARGET SOURCETOLOOKFOR ERRORMSG)
IF (POLICY CMP0051)
    CMAKE_POLICY(SET CMP0051 NEW)
ENDIF()
    IF (NOT TARGET ${THETARGET})
        MESSAGE(FATAL_ERROR ERRORMSG "(was not target)" )
    ENDIF()
    GET_TARGET_PROPERTY(SRCS "${THETARGET}" SOURCES)
    IF(NOT "${SRCS}" MATCHES "${SOURCETOLOOKFOR}")
        MESSAGE("sources were: ${SRCS}")
        TEST_MSG(${ERRORMSG} "(source was not contained)")
        MESSAGE(FATAL_ERROR "")
    ENDIF()
ENDMACRO()

MACRO(ASSERT_SOURCES_DONT_CONTAIN THETARGET SOURCETOLOOKFOR ERRORMSG)
    IF (NOT TARGET ${THETARGET})
        MESSAGE(FATAL_ERROR ERRORMSG "(was not target)" )
    ENDIF()
    GET_TARGET_PROPERTY(SRCS "${THETARGET}" SOURCES)
    IF("${SRCS}" MATCHES "${SOURCETOLOOKFOR}")
        MESSAGE("sources were: ${SRCS}")
        TEST_MSG(${ERRORMSG} "(source was contained)")
        MESSAGE(FATAL_ERROR "")
    ENDIF()
ENDMACRO()

MACRO(ASSERT_DEFINES_CONTAIN THETARGET DEFINETOLOOKFOR ERRORMSG)
    IF (NOT TARGET ${THETARGET})
        MESSAGE(FATAL_ERROR ERRORMSG "(was not target)" )
    ENDIF()
    GET_TARGET_PROPERTY(TARGET_DEFINES "${THETARGET}" INTERFACE_COMPILE_DEFINITIONS)
    IF("${TARGET_DEFINES}" MATCHES "${DEFINETOLOOKFOR}")
    ELSE()
        MESSAGE("defines were: ${TARGET_DEFINES}")
        TEST_MSG(${ERRORMSG} "(define was not contained)")
        MESSAGE(FATAL_ERROR "")
    ENDIF()
ENDMACRO()

MACRO(ASSERT_LINK_LIBRARIES_CONTAIN THETARGET LIBTOLOOKFOR ERRORMSG)
    IF (NOT TARGET ${THETARGET})
        MESSAGE(FATAL_ERROR ERRORMSG "(was not target)" )
    ENDIF()
    GET_TARGET_PROPERTY(LIBS "${THETARGET}" LINK_LIBRARIES)
    IF("${LIBS}" MATCHES "${LIBTOLOOKFOR}")
    ELSE()
        MESSAGE("libs were: ${LIBS}")
        TEST_MSG(${ERRORMSG} "(library for linking was not contained)")
        MESSAGE(FATAL_ERROR "")
    ENDIF()
ENDMACRO()

MACRO(ASSERT_LINK_LIBRARIES_DONT_CONTAIN THETARGET LIBTOLOOKFOR ERRORMSG)
    IF (NOT TARGET ${THETARGET})
        MESSAGE(FATAL_ERROR ERRORMSG "(was not target)" )
    ENDIF()
    GET_TARGET_PROPERTY(LIBS "${THETARGET}" LINK_LIBRARIES)
    IF("${LIBS}" MATCHES "${LIBTOLOOKFOR}")
        MESSAGE("libs were: ${LIBS}")
        TEST_MSG(${ERRORMSG} "(library for linking was contained)")
        MESSAGE(FATAL_ERROR "")
    ENDIF()
ENDMACRO()

MACRO(ASSERT_TARGET_EXISTS TARGETNAME ERRORMSG)
IF (NOT TARGET ${TARGETNAME})
    TEST_MSG(${ERRORMSG} "Target '${TARGETNAME}' does not exist")
    MESSAGE(FATAL_ERROR "")
ENDIF()
ENDMACRO()

MACRO(ASSERT_TARGET_NOT_EXISTS TARGETNAME ERRORMSG)
IF (TARGET ${TARGETNAME})
    TEST_MSG(${ERRORMSG} "Target '${TARGETNAME}' should not exist")
    MESSAGE(FATAL_ERROR "")
ENDIF()
ENDMACRO()

FUNCTION(ASSERT_RESOURCES_FOR_TARGET TARGETNAME RESOURCES)
  IF (NOT TARGET ${TARGETNAME})
    MESSAGE(FATAL_ERROR ERRORMSG "(was not target)" )
  ENDIF()

  IF (NOT TARGET RESCOPY_${TARGETNAME})
    MESSAGE(FATAL_ERROR ERRORMSG "(no resource target created)" )
  ENDIF()

  FOREACH(RESOURCE ${RESOURCES})
    LIST(FIND FILES_RESOURCE_BINARYDIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/${RESOURCE}" _index)
    IF(_index LESS 0)
      MESSAGE(FATAL_ERROR ERRORMSG "(resource ${RESOURCE} not added to FILES_RESOURCE_BINARYDIR)" )
    ENDIF()
  ENDFOREACH()

  list(LENGTH RESOURCES _expected_size)
  list(LENGTH FILES_RESOURCE_BINARYDIR _actual_size)
  IF(NOT _expected_size EQUAL _actual_size)
    MESSAGE(FATAL_ERROR ERRORMSG "(resource list FILES_RESOURCE_BINARYDIR size is not same as expected size)" )
  ENDIF()
ENDFUNCTION()

#test1 executable_justFromSources
#test2 executableDependingOnStaticLibrary
#test3 executableDependingOnFindScript
#test4 executableDependingOnObjectLibrary
#test5 executable_object_static
#test6 executable_static_object
#test7 exe_sharedLibrary_dependingOnChainOfObjectLibraries
#test8 executable_static_static

set(GIT_COMMIT_HASH "abcdef")
set(GIT_COMMIT_COUNT "123")

ACME2_PROJECT(
    #==========================================================================
    # project metadata
    #==========================================================================
    NAME                        acme2Tests
    CONTENT                     ON   test1
    CONTENT                     ON   test2
                                ON   test3
                                # ON   test8
                                ON   test10
                                ON   test11
)
