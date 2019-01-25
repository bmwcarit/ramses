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
        RamsesClientImpl& client,
        const char* node)
        : ClientObjectImpl(client, type, node)
        , m_hashUsage(hashUsage)
        , m_resourceId(InvalidResourceId)
    {
        if (m_hashUsage.isValid())
        {
            updateResourceHash();
        }
    }

    ResourceImpl::~ResourceImpl()
    {
    }

    resourceId_t ResourceImpl::getResourceId() const
    {
        assert(m_resourceId != InvalidResourceId);
        return m_resourceId;
    }

    void ResourceImpl::updateResourceHash()
    {
        ramses_internal::BinaryOutputStream metaDataStream(1024);
        metaDataStream << getLowlevelResourceHash();
        metaDataStream << getName();
        metaDataStream << static_cast<uint32_t>(getType());
        const cityhash::uint128 cityHashMetadataAndBlob = cityhash::CityHash128(metaDataStream.getData(), metaDataStream.getSize());
        m_resourceId.highPart = cityhash::Uint128High64(cityHashMetadataAndBlob);
        m_resourceId.lowPart = cityhash::Uint128Low64(cityHashMetadataAndBlob);
    }

    void ResourceImpl::deinitializeFrameworkData()
    {
    }

    ramses::status_t ResourceImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(ClientObjectImpl::serialize(outStream, serializationContext));

        outStream << getLowlevelResourceHash();
        outStream << m_resourceId.highPart;
        outStream << m_resourceId.lowPart;

        return StatusOK;
    }

    status_t ResourceImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(ClientObjectImpl::deserialize(inStream, serializationContext));

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

    status_t ResourceImpl::validate(uint32_t indent) const
    {
        const status_t status = ClientObjectImpl::validate(indent);
        indent += IndentationStep;
        ramses_internal::StringOutputStream stringStream;
        stringStream << "Resource ID: " << ramses_internal::StringUtils::HexFromResourceContentHash({ m_resourceId.lowPart, m_resourceId.highPart });
        stringStream << "  Resource Hash: " << ramses_internal::StringUtils::HexFromResourceContentHash(m_hashUsage.getHash());
        addValidationMessage(EValidationSeverity_Info, indent, stringStream.c_str());
        return status;
    }

    status_t ResourceImpl::setName(RamsesObject& object, const char* name)
    {
        const status_t status = ClientObjectImpl::setName(object, name);

        // name is also included in resource hash
        updateResourceHash();

        return status;
    }
}
