//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEIMPL_H
#define RAMSES_RESOURCEIMPL_H

// internal
#include "SceneImpl.h"

// ramses framework
#include "SceneAPI/ResourceContentHash.h"
#include "Components/ManagedResource.h"
#include "Components/ResourceHashUsage.h"

#include <string_view>
#include <string>

namespace ramses_internal
{
    class ClientApplicationLogic;
}

namespace ramses
{
    class ResourceImpl : public SceneObjectImpl
    {
    public:
        ResourceImpl(ERamsesObjectType type,
            ramses_internal::ResourceHashUsage hashUsage,
            SceneImpl& scene,
            std::string_view name);
        ~ResourceImpl() override;

        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        resourceId_t     getResourceId() const;
        ramses_internal::ResourceContentHash getLowlevelResourceHash() const;

        status_t validate() const override;
        status_t setName(RamsesObject& object, std::string_view name) override;

        static resourceId_t CreateResourceHash(ramses_internal::ResourceContentHash llhash, const std::string& name, ERamsesObjectType type);

    private:
        void updateResourceHash();

        ramses_internal::ResourceHashUsage m_hashUsage;
        resourceId_t m_resourceId;
    };
}

#endif
