//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/ArrayResourceImpl.h"

namespace ramses::internal
{
    ArrayResourceImpl::ArrayResourceImpl(ResourceHashUsage arrayHash, SceneImpl& scene, std::string_view name)
        : ResourceImpl(ERamsesObjectType::ArrayResource, std::move(arrayHash), scene, name)
        , m_elementCount(0)
        , m_elementType(ramses::EDataType::UInt16)
    {
    }

    ArrayResourceImpl::~ArrayResourceImpl() = default;

    void ArrayResourceImpl::initializeFromFrameworkData(uint32_t elementCount, ramses::EDataType elementType)
    {
        m_elementCount = elementCount;
        m_elementType = elementType;
    }

    bool ArrayResourceImpl::serialize(IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!ResourceImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_elementCount;
        outStream << static_cast<uint32_t>(m_elementType);

        return true;
    }

    bool ArrayResourceImpl::deserialize(IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!ResourceImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_elementCount;
        uint32_t enumInt = 0u;
        inStream >> enumInt;
        m_elementType = static_cast<ramses::EDataType>(enumInt);

        return true;
    }

    uint32_t ArrayResourceImpl::getElementCount() const
    {
        return m_elementCount;
    }

    ramses::EDataType ArrayResourceImpl::getElementType() const
    {
        return m_elementType;
    }
}
