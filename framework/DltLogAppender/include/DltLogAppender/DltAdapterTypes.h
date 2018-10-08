//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DLTADAPTERTYPES_H
#define RAMSES_DLTADAPTERTYPES_H

namespace ramses_internal
{
    enum EDltError
    {
        EDltError_NO_ERROR = 0x0,
        EDltError_CONTEXT_INVALID,
        EDltError_MISSING_DLT_CONTEXT,
        EDltError_INIT,
        EDltError_DEINIT,
        EDltError_USER_LOG,
        EDltError_REGISTER_APPLICATION_FAILED,
        EDltError_REGISTER_CONTEXT_FAILED,
        EDltError_UNREGISTER_CONTEXT_FAILED,
        EDltError_UNREGISTER_APPLICATION_FAILED,
        EDltError_LOGLEVEL_CHANGED_CALLBACK_FAILURE,
        EDltError_INJECTION_CALLBACK_FAILURE,
        EDltError_FILETRANSFER_FAILURE
    };
}
#endif // RAMSES_DLTADAPTERTYPES_H
