//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "OSDependent/osinclude.h"
#include "assert.h"
#include "stdio.h"

// arbitrary number of slots. should always be enough for anyone
#define MAX_TLS_SLOTS ((size_t)10)
static bool tls_slots_in_use[MAX_TLS_SLOTS] = {false};
static void* tls_slots_values[MAX_TLS_SLOTS] = {0};

namespace glslang {

void OS_CleanupThreadData(void)
{
}

OS_TLSIndex OS_AllocTLSIndex()
{
    for (size_t idx = 0; idx < MAX_TLS_SLOTS; ++idx)
        if (!tls_slots_in_use[idx]) {
            tls_slots_in_use[idx] = true;
            return (void*)(idx + 1);
        }
    return OS_INVALID_TLS_INDEX;
}

bool OS_SetTLSValue(OS_TLSIndex nIndex, void *lpvValue)
{
    size_t idx = (size_t)nIndex;
    if (nIndex == OS_INVALID_TLS_INDEX ||
        idx >= MAX_TLS_SLOTS ||
        !tls_slots_in_use[idx-1])
        return false;
    tls_slots_values[idx-1] = lpvValue;
    return true;
}

void* OS_GetTLSValue(OS_TLSIndex nIndex)
{
    size_t idx = (size_t)nIndex;
    if (nIndex == OS_INVALID_TLS_INDEX ||
        idx >= MAX_TLS_SLOTS ||
        !tls_slots_in_use[idx-1])
        return 0;
    return tls_slots_values[idx-1];
}

bool OS_FreeTLSIndex(OS_TLSIndex nIndex)
{
    size_t idx = (size_t)nIndex;
    if (nIndex == OS_INVALID_TLS_INDEX ||
        idx >= MAX_TLS_SLOTS ||
        !tls_slots_in_use[idx-1])
        return 0;
    tls_slots_in_use[idx-1] = false;
    return true;
}

void InitGlobalLock()
{
}

void GetGlobalLock()
{
}

void ReleaseGlobalLock()
{
}

void* OS_CreateThread(TThreadEntrypoint /*entry*/)
{
    return 0;
}

void OS_WaitForAllThreads(void* /*threads*/, int /*numThreads*/)
{
}

void OS_Sleep(int /*milliseconds*/)
{
}

void OS_DumpMemoryCounters()
{
}

} // end namespace glslang

