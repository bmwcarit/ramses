#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

function(ramses_glob_target_sources target bind_type expr)
    file(GLOB out ${expr} ${ARGN})
    target_sources(${target} ${bind_type} ${out})
endfunction()
