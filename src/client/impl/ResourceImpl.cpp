//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// internal
#include "impl/ResourceImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkTypesImpl.h"

#include "ramses/client/Resource.h"

// framework
#include "internal/ClientApplicationLogic.h"
#include "internal/Components/ManagedResource.h"

#include "internal/Core/Utils/BinaryOutputStream.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "city.h"
#include "internal/Core/Utils/LogMacros.h"

#include <string_view>

namespace ramses::internal
{
    ResourceImpl::ResourceImpl(ERamsesObjectType type,
        ramses::internal::ResourceHashUsage hashUsage,
        SceneImpl& scene,
        std::string_view name)
        : SceneObjectImpl(scene, type, name)
        , m_hashUsage(std::move(hashUsage))
    {
        if (m_hashUsage.isValid())
        {
            updateResourceHash();
        }
    }

    ResourceImpl::~ResourceImpl()
    {
        LOG_DEBUG(CONTEXT_FRAMEWORK, "Destroy resource type {}, hl {}, ll {}", getType(), m_resourceId, getLowlevelResourceHash());
    }

    resourceId_t ResourceImpl::getResourceId() const
    {
        assert(m_resourceId.isValid());
        return m_resourceId;
    }

    resourceId_t ResourceImpl::CreateResourceHash(ramses::internal::ResourceContentHash llhash, const std::string& name, ERamsesObjectType type)
    {
        resourceId_t hash;

        ramses::internal::BinaryOutputStream metaDataStream(1024);
        metaDataStream << llhash;
        metaDataStream << name;
        metaDataStream << static_cast<uint32_t>(type);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) external API expects char* to binary data
        const cityhash::uint128 cityHashMetadataAndBlob = cityhash::CityHash128(reinterpret_cast<const char*>(metaDataStream.getData()), metaDataStream.getSize());
        hash.highPart = cityhash::Uint128High64(cityHashMetadataAndBlob);
        hash.lowPart = cityhash::Uint128Low64(cityHashMetadataAndBlob);
        return hash;
    }

    void ResourceImpl::updateResourceHash()
    {
        const auto id = m_resourceId;
        m_resourceId = CreateResourceHash(getLowlevelResourceHash(), getName(), getType());

        if (id.isValid() && id != m_resourceId)
            getSceneImpl().updateResourceId(id, RamsesObjectTypeUtils::ConvertTo<Resource>(getRamsesObject()));
    }

    void ResourceImpl::deinitializeFrameworkData()
    {
    }

    bool ResourceImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << getLowlevelResourceHash();
        outStream << m_resourceId.highPart;
        outStream << m_resourceId.lowPart;

        return true;
    }

    bool ResourceImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        ramses::internal::ResourceContentHash llhash;
        inStream >> llhash;
        m_hashUsage = getClientImpl().getHashUsage_ThreadSafe(llhash);

        inStream >> m_resourceId.highPart;
        inStream >> m_resourceId.lowPart;

        return true;
    }

    ramses::internal::ResourceContentHash ResourceImpl::getLowlevelResourceHash() const
    {
        return m_hashUsage.getHash();
    }

    bool ResourceImpl::setName(std::string_view name)
    {
        const auto status = SceneObjectImpl::setName(name);

        // name is also included in resource hash
        updateResourceHash();

        return status;
    }
}
