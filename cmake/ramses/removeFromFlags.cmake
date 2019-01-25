#  -------------------------------------------------------------------------
#  Copyright (C) 2019 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


FUNCTION(REMOVE_FROM_FLAGS flags toRemoveList outVar)
    string(REGEX REPLACE " +" ";" flags_LIST "${flags}")   # to list
    list(REMOVE_ITEM flags_LIST ${toRemoveList})           # filter list
    string(REPLACE ";" " " flags_filtered "${flags_LIST}") # to string
    set(${outVar} "${flags_filtered}" PARENT_SCOPE)
ENDFUNCTION()
