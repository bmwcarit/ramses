//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/ThreadLocalLog.h"

namespace ramses_internal
{
    namespace TLSPrefix
    {
        // Some platforms require explicit compile flag to enable TLS,
        // isolate TLS in this compilation unit so that this flag
        // needs to be enabled only for this single file.
        thread_local static int ID = -1;
    }

    void ThreadLocalLog::SetPrefix(int prefixId)
    {
        assert(prefixId != -1);
        TLSPrefix::ID = prefixId;
    }

    int ThreadLocalLog::GetPrefix()
    {
        assert(TLSPrefix::ID != -1 && "Thread local log PrefixID must be set");
        return TLSPrefix::ID;
    }

    int ThreadLocalLog::GetPrefixUnchecked()
    {
        return TLSPrefix::ID;
    }
}
