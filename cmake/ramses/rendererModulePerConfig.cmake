#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

SET(RENDERER_CONFIG_LIST
    "x11-egl-es-3-0"

    "wayland-ivi-egl-es-3-0"
    "wayland-shell-egl-es-3-0"

    "windows-wgl-es-3-0"
    "windows-wgl-4-2-core"
    "windows-wgl-4-5"

    "android-egl-es-3-0"
    )

#helper macro
FUNCTION(RENDERER_MODULE_PER_CONFIG MODULE_PREFIX_NAME RENDERER_SPECIFIC_DEPENDENCIES)

    SET(ADDITIONAL_MODULE_SETTINGS ${ARGN})

    # always allow disabling when dependency missing (similar to ACME_PROJECT option AUTO)
    SET(ACME_ENABLE_DEPENDENCY_CHECK ON)

    FOREACH(PLATFORM_NAME ${RENDERER_CONFIG_LIST})

        if (TARGET platform-${PLATFORM_NAME})
            SET(MYMODULE_NAME "${MODULE_PREFIX_NAME}-${PLATFORM_NAME}")

            # build acme module for this configuration
            ACME_MODULE(NAME             ${MYMODULE_NAME}
                        DEPENDENCIES     ${RENDERER_SPECIFIC_DEPENDENCIES}
                        ${ADDITIONAL_MODULE_SETTINGS})

            IF("${ACME_TYPE}" STREQUAL "SHARED_LIBRARY" AND TARGET ${MYMODULE_NAME})
                SET_TARGET_PROPERTIES(${MYMODULE_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
                GET_TARGET_PROPERTY(MYMODULE_SOVERSION ${MYMODULE_NAME} SOVERSION)
                ACME_INFO("    setting shared library version to ${MYMODULE_SOVERSION}")
            ENDIF()
        endif()

    ENDFOREACH()

ENDFUNCTION()

FUNCTION(RENDERER_MODULE_PER_CONFIG_STATIC MODULE_PREFIX_NAME)

    SET(ADDITIONAL_MODULE_SETTINGS ${ARGN})

    RENDERER_MODULE_PER_CONFIG(${MODULE_PREFIX_NAME}
                    "ramses-renderer-lib;platform-\${PLATFORM_NAME}"
                    ${ADDITIONAL_MODULE_SETTINGS})

ENDFUNCTION()

FUNCTION(RENDERER_MODULE_PER_CONFIG_DYNAMIC MODULE_PREFIX_NAME)

    SET(ADDITIONAL_MODULE_SETTINGS ${ARGN})

    RENDERER_MODULE_PER_CONFIG(${MODULE_PREFIX_NAME}
                    "ramses-shared-lib-\${PLATFORM_NAME}"
                    ${ADDITIONAL_MODULE_SETTINGS})

ENDFUNCTION()

FUNCTION(MODULE_WITH_SHARED_LIBRARY)
    FOREACH(PLATFORM_NAME ${RENDERER_CONFIG_LIST})
        SET(RAMSES_PLATFORM_SHLIB_NAME ramses-shared-lib-${PLATFORM_NAME})
        IF (TARGET ${RAMSES_PLATFORM_SHLIB_NAME})
            ACME_MODULE(
            DEPENDENCIES               ${RAMSES_PLATFORM_SHLIB_NAME}
            ${ARGN})
            BREAK()
        ENDIF()
    ENDFOREACH()
ENDFUNCTION()
