#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF (NOT PYTHONINTERP_FOUND)
    FIND_PACKAGE(PythonInterp 2.7)
ENDIF()

IF (PYTHONINTERP_FOUND)
    ADD_CUSTOM_TARGET(CHECK_CODE_STYLE
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/code_style_checker/check_all_styles.py ${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/code_style_checker
        )
    SET_PROPERTY(TARGET CHECK_CODE_STYLE PROPERTY FOLDER "CMakePredefinedTargets")
    ACME_INFO("+ CHECK_CODE_STYLE")

    ADD_CUSTOM_TARGET(CHECK_COMMIT_RULES
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/commit-checks/check_all_commit_rules.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    SET_PROPERTY(TARGET CHECK_COMMIT_RULES PROPERTY FOLDER "CMakePredefinedTargets")
    ACME_INFO("+ CHECK_COMMIT_RULES")

ELSE()
    ACME_INFO("- CHECK_CODE_STYLE [missing python]")
    ACME_INFO("- CHECK_COMMIT_RULES [missing python]")
ENDIF()
