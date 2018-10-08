//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMCONDITIONVARIABLE_H
#define RAMSES_PLATFORMCONDITIONVARIABLE_H

#include <ramses-capu/os/CondVar.h>
#include <PlatformAbstraction/PlatformLock.h>
#include <PlatformAbstraction/PlatformError.h>


namespace ramses_internal
{
    class PlatformConditionVariable
    {
    public:
        EStatus signal();
        EStatus wait(PlatformLock* lock, UInt32 millisec = 0);
        EStatus wait(PlatformLightweightLock* lock, UInt32 millisec = 0);
        EStatus broadcast();

    private:
        ramses_capu::CondVar mCondVar;
    };

    inline EStatus PlatformConditionVariable::signal()
    {
        return static_cast<EStatus>(mCondVar.signal());
    }

    inline EStatus PlatformConditionVariable::wait(PlatformLock* lock, UInt32 millisec)
    {
        return static_cast<EStatus>(mCondVar.wait(lock->m_mutex, millisec));
    }

    inline EStatus PlatformConditionVariable::wait(PlatformLightweightLock* lock, UInt32 millisec)
    {
        return static_cast<EStatus>(mCondVar.wait(lock->m_mutex, millisec));
    }

    inline EStatus PlatformConditionVariable::broadcast()
    {
        return static_cast<EStatus>(mCondVar.broadcast());
    }
}

#endif
