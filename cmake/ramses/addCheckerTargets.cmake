#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF (ramses-sdk_PYTHON3 AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    ADD_CUSTOM_TARGET(CHECK_CODE_STYLE
        COMMAND ${ramses-sdk_PYTHON3} ${PROJECT_SOURCE_DIR}/scripts/code_style_checker/check_all_styles.py
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/scripts/code_style_checker
        )
    SET_PROPERTY(TARGET CHECK_CODE_STYLE PROPERTY FOLDER "CMakePredefinedTargets")
    ACME_INFO("+ CHECK_CODE_STYLE")

ELSE()
    ACME_INFO("- CHECK_CODE_STYLE [missing python]")
ENDIF()
