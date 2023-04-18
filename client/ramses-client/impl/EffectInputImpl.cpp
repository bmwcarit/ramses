//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EffectInputImpl.h"
#include "DataTypeUtils.h"
#include "EffectInputSemanticUtils.h"
#include "SceneAPI/IScene.h"

namespace ramses
{
    EffectInputImpl::EffectInputImpl()
        : StatusObjectImpl()
        , m_effectHash(ramses_internal::ResourceContentHash::Invalid())
        , m_dataType(ramses_internal::EDataType::Invalid)
        , m_semantics(ramses_internal::EFixedSemantics::Invalid)
        , m_elementCount(0u)
        , m_inputIndex(static_cast<uint32_t>(-1))
    {
    }

    EffectInputImpl::~EffectInputImpl()
    {
    }

    void EffectInputImpl::initialize(
        const ramses_internal::ResourceContentHash& effectHash,
        const ramses_internal::String&              name,
        ramses_internal::EDataType                  dataType,
        ramses_internal::EFixedSemantics            semantics,
        uint32_t                                    elementCount,
        uint32_t                                    index)
    {
        m_effectHash = effectHash;
        m_name = name;
        m_dataType = dataType;
        m_semantics = semantics;
        m_elementCount = elementCount;
        m_inputIndex = index;
    }

    bool EffectInputImpl::isValid() const
    {
        return m_effectHash.isValid();
    }

    ramses_internal::ResourceContentHash EffectInputImpl::getEffectHash() const
    {
        return m_effectHash;
    }

    const ramses_internal::String& EffectInputImpl::getName() const
    {
        return m_name;
    }

    std::optional<EDataType> EffectInputImpl::getDataType() const
    {
        if (!isValid())
            return std::nullopt;

        return DataTypeUtils::ConvertDataTypeFromInternal(m_dataType);
    }

    ramses_internal::EDataType EffectInputImpl::getInternalDataType() const
    {
        return m_dataType;
    }

    ramses_internal::EFixedSemantics EffectInputImpl::getSemantics() const
    {
        return m_semantics;
    }

    uint32_t EffectInputImpl::getElementCount() const
    {
        return m_elementCount;
    }

    uint32_t EffectInputImpl::getInputIndex() const
    {
        return m_inputIndex;
    }

    EEffectUniformSemantic EffectInputImpl::getUniformSemantics() const
    {
        return EffectInputSemanticUtils::GetEffectUniformSemanticFromInternal(m_semantics);
    }

    EEffectAttributeSemantic EffectInputImpl::getAttributeSemantics() const
    {
        return EffectInputSemanticUtils::GetEffectAttributeSemanticFromInternal(m_semantics);
    }

    ramses::status_t EffectInputImpl::serialize(ramses_internal::IOutputStream& outStream) const
    {
        outStream << m_effectHash;
        outStream << m_name;
        outStream << static_cast<uint32_t>(m_dataType);
        outStream << static_cast<uint32_t>(m_semantics);
        outStream << m_elementCount;
        outStream << m_inputIndex;

        return StatusOK;
    }

    ramses::status_t EffectInputImpl::deserialize(ramses_internal::IInputStream& inStream)
    {
        inStream >> m_effectHash;
        inStream >> m_name;

        uint32_t dataType;
        inStream >> dataType;

        m_dataType = ramses_internal::EDataType(dataType);

        uint32_t semantics;
        inStream >> semantics;

        m_semantics = ramses_internal::EFixedSemantics(semantics);

        inStream >> m_elementCount;
        inStream >> m_inputIndex;

        return StatusOK;
    }

    bool EffectInputImpl::operator==(const EffectInputImpl& other) const
    {
        return m_effectHash == other.m_effectHash &&
            m_name == other.m_name &&
            m_dataType == other.m_dataType &&
            m_semantics == other.m_semantics &&
            m_elementCount == other.m_elementCount &&
            m_inputIndex == other.m_inputIndex;
    }

    bool EffectInputImpl::operator!=(const EffectInputImpl& other) const
    {
        return !operator==(other);
    }
}
