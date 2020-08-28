//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// internal
#include "ResourceImpl.h"
#include "RamsesClientImpl.h"
#include "RamsesFrameworkTypesImpl.h"

// framework
#include "ClientApplicationLogic.h"
#include "Components/ManagedResource.h"

#include "Collections/StringOutputStream.h"
#include "Utils/StringUtils.h"
#include "Utils/BinaryOutputStream.h"
#include "city.h"

namespace ramses
{
    ResourceImpl::ResourceImpl(ERamsesObjectType type,
        ramses_internal::ResourceHashUsage hashUsage,
        SceneImpl& scene,
        const char* name)
        : SceneObjectImpl(scene, type, name)
        , m_hashUsage(std::move(hashUsage))
    {
        if (m_hashUsage.isValid())
        {
            updateResourceHash();
        }
        getSceneImpl().getStatisticCollection().statResourceObjectsCreated.incCounter(1);
    }

    ResourceImpl::~ResourceImpl()
    {
        getSceneImpl().getStatisticCollection().statResourceObjectsDestroyed.incCounter(1);
    }

    resourceId_t ResourceImpl::getResourceId() const
    {
        assert(m_resourceId.isValid());
        return m_resourceId;
    }

    resourceId_t ResourceImpl::CreateResourceHash(ramses_internal::ResourceContentHash llhash, ramses_internal::String const& name, ERamsesObjectType type)
    {
        resourceId_t hash;

        ramses_internal::BinaryOutputStream metaDataStream(1024);
        metaDataStream << llhash;
        metaDataStream << name;
        metaDataStream << static_cast<uint32_t>(type);
        const cityhash::uint128 cityHashMetadataAndBlob = cityhash::CityHash128(reinterpret_cast<const char*>(metaDataStream.getData()), metaDataStream.getSize());
        hash.highPart = cityhash::Uint128High64(cityHashMetadataAndBlob);
        hash.lowPart = cityhash::Uint128Low64(cityHashMetadataAndBlob);
        return hash;
    }

    void ResourceImpl::updateResourceHash()
    {
        m_resourceId = CreateResourceHash(getLowlevelResourceHash(), getName(), getType());
    }

    void ResourceImpl::deinitializeFrameworkData()
    {
    }

    ramses::status_t ResourceImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << getLowlevelResourceHash();
        outStream << m_resourceId.highPart;
        outStream << m_resourceId.lowPart;

        return StatusOK;
    }

    status_t ResourceImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        ramses_internal::ResourceContentHash llhash;
        inStream >> llhash;
        m_hashUsage = getClientImpl().getHashUsage_ThreadSafe(llhash);

        inStream >> m_resourceId.highPart;
        inStream >> m_resourceId.lowPart;

        return StatusOK;
    }

    ramses_internal::ResourceContentHash ResourceImpl::getLowlevelResourceHash() const
    {
        return m_hashUsage.getHash();
    }

    status_t ResourceImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        const status_t status = SceneObjectImpl::validate(indent, visitedObjects);
        indent += IndentationStep;
        ramses_internal::StringOutputStream stringStream;
        stringStream << "Resource ID: " << m_resourceId;
        stringStream << "  Resource Hash: " << m_hashUsage.getHash();
        stringStream << "  Resource Type: " << getType();
        stringStream << "  Name: " << getName();
        addValidationMessage(EValidationSeverity_Info, indent, stringStream.c_str());
        return status;
    }

    status_t ResourceImpl::setName(RamsesObject& object, const char* name)
    {
        const status_t status = SceneObjectImpl::setName(object, name);

        // name is also included in resource hash
        updateResourceHash();

        return status;
    }
}
