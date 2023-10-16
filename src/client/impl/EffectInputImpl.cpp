//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/EffectInputImpl.h"
#include "impl/DataTypeUtils.h"
#include "impl/EffectInputSemanticUtils.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"

namespace ramses::internal
{
    void EffectInputImpl::initialize(
        const ResourceContentHash&  effectHash,
        std::string_view            name,
        ramses::internal::EDataType dataType,
        EFixedSemantics             semantics,
        size_t                      elementCount,
        size_t                      index)
    {
        m_effectHash = effectHash;
        m_name = name;
        m_dataType = dataType;
        m_semantics = semantics;
        m_elementCount = elementCount;
        m_inputIndex = index;
    }

    ResourceContentHash EffectInputImpl::getEffectHash() const
    {
        return m_effectHash;
    }

    const std::string& EffectInputImpl::getName() const
    {
        return m_name;
    }

    ramses::EDataType EffectInputImpl::getDataType() const
    {
        return DataTypeUtils::ConvertDataTypeFromInternal(m_dataType);
    }

    ramses::internal::EDataType EffectInputImpl::getInternalDataType() const
    {
        return m_dataType;
    }

    EFixedSemantics EffectInputImpl::getSemantics() const
    {
        return m_semantics;
    }

    size_t EffectInputImpl::getElementCount() const
    {
        return m_elementCount;
    }

    size_t EffectInputImpl::getInputIndex() const
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
}
