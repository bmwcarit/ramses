//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IManagedResourceDeleterCallback.h"

namespace ramses::internal
{
    class ResourceDeleterCallingCallback
    {
    public:
        explicit ResourceDeleterCallingCallback(IManagedResourceDeleterCallback& callback = DefaultManagedResourceDeleterCallback::GetInstance())
            : m_callback(callback)
        {
        }

        void operator()(const IResource* p)
        {
            assert(p != nullptr);
            m_callback.managedResourceDeleted(*p);
        }

    private:
        IManagedResourceDeleterCallback& m_callback;
    };
}
