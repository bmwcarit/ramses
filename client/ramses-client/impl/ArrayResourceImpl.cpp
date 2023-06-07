//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ArrayResourceImpl.h"

namespace ramses
{
    ArrayResourceImpl::ArrayResourceImpl(ramses_internal::ResourceHashUsage arrayHash, SceneImpl& scene, std::string_view name)
        : ResourceImpl(ERamsesObjectType::ArrayResource, std::move(arrayHash), scene, name)
        , m_elementCount(0)
        , m_elementType(EDataType::UInt16)
    {
    }

    ArrayResourceImpl::~ArrayResourceImpl()
    {
    }

    void ArrayResourceImpl::initializeFromFrameworkData(uint32_t elementCount, EDataType elementType)
    {
        m_elementCount = elementCount;
        m_elementType = elementType;
    }

    status_t ArrayResourceImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(ResourceImpl::serialize(outStream, serializationContext));

        outStream << m_elementCount;
        outStream << static_cast<uint32_t>(m_elementType);

        return StatusOK;
    }

    status_t ArrayResourceImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(ResourceImpl::deserialize(inStream, serializationContext));

        inStream >> m_elementCount;
        uint32_t enumInt = 0u;
        inStream >> enumInt;
        m_elementType = static_cast<EDataType>(enumInt);

        return StatusOK;
    }

    uint32_t ArrayResourceImpl::getElementCount() const
    {
        return m_elementCount;
    }

    EDataType ArrayResourceImpl::getElementType() const
    {
        return m_elementType;
    }
}
