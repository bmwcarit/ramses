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
            const char* name);
        virtual ~ResourceImpl();

        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        resourceId_t     getResourceId() const;
        ramses_internal::ResourceContentHash getLowlevelResourceHash() const;

        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;
        virtual status_t setName(RamsesObject& object, const char* name) override;

        static resourceId_t CreateResourceHash(ramses_internal::ResourceContentHash llhash, ramses_internal::String const& name, ERamsesObjectType type);

    private:
        void updateResourceHash();

        ramses_internal::ResourceHashUsage m_hashUsage;
        resourceId_t m_resourceId;
    };
}

#endif
