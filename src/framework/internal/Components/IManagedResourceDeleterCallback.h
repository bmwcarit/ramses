//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/Resource/IResource.h"

namespace ramses::internal
{
    class IManagedResourceDeleterCallback
    {
    public:
        virtual ~IManagedResourceDeleterCallback() = default;

        virtual void managedResourceDeleted(const IResource& resource) = 0;
    };

    class DefaultManagedResourceDeleterCallback final : public IManagedResourceDeleterCallback
    {
    public:
        void managedResourceDeleted(const IResource& resource) override
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
