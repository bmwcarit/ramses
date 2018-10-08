//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IMANAGEDRESOURCEDELETERCALLBACK_H
#define RAMSES_IMANAGEDRESOURCEDELETERCALLBACK_H

#include "SceneAPI/ResourceContentHash.h"

namespace ramses_internal
{
    class IManagedResourceDeleterCallback
    {
    public:
        virtual ~IManagedResourceDeleterCallback(){}

        virtual void managedResourceDeleted(const IResource& resource) = 0;
    };

    class DefaultManagedResourceDeleterCallback final : public IManagedResourceDeleterCallback
    {
    public:
        virtual void managedResourceDeleted(const IResource& resource) override
        {
            delete &resource;
        }

        static IManagedResourceDeleterCallback& GetInstance()
        {
            static DefaultManagedResourceDeleterCallback DefaultDeleter;
            return DefaultDeleter;
        }
    };
}

#endif
