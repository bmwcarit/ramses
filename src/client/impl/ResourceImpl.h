//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// internal
#include "impl/SceneImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/Components/ManagedResource.h"
#include "internal/Components/ResourceHashUsage.h"

#include <string_view>
#include <string>

namespace ramses::internal
{
    class ClientApplicationLogic;
}

namespace ramses::internal
{
    class ResourceImpl : public SceneObjectImpl
    {
    public:
        ResourceImpl(ERamsesObjectType type,
            ramses::internal::ResourceHashUsage hashUsage,
            SceneImpl& scene,
            std::string_view name);
        ~ResourceImpl() override;

        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        [[nodiscard]] resourceId_t     getResourceId() const;
        [[nodiscard]] ramses::internal::ResourceContentHash getLowlevelResourceHash() const;

        bool setName(std::string_view name) override;

        [[nodiscard]] static resourceId_t CreateResourceHash(ramses::internal::ResourceContentHash llhash, const std::string& name, ERamsesObjectType type);

    private:
        void updateResourceHash();

        ramses::internal::ResourceHashUsage m_hashUsage;
        resourceId_t m_resourceId;
    };
}
