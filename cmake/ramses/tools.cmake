#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

function(ramses_move_targets_to_folder folder)
    foreach(tgt ${ARGN})
        if (TARGET ${tgt})
            set_property(TARGET ${tgt} PROPERTY FOLDER "external")
        endif()
    endforeach()
endfunction()
