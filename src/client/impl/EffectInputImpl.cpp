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
    void EffectInputImpl::initialize(const ResourceContentHash& effectHash, const EffectInputInformation& inputInfo, size_t index)
    {
        m_effectHash = effectHash;
        m_inputInfo = inputInfo;
        m_inputIndex = index;
    }

    ResourceContentHash EffectInputImpl::getEffectHash() const
    {
        return m_effectHash;
    }

    const std::string& EffectInputImpl::getName() const
    {
        return m_inputInfo.inputName;
    }

    ramses::EDataType EffectInputImpl::getDataType() const
    {
        return DataTypeUtils::ConvertDataTypeFromInternal(m_inputInfo.dataType);
    }

    ramses::internal::EDataType EffectInputImpl::getInternalDataType() const
    {
        return m_inputInfo.dataType;
    }

    EFixedSemantics EffectInputImpl::getSemantics() const
    {
        return m_inputInfo.semantics;
    }

    size_t EffectInputImpl::getElementCount() const
    {
        return m_inputInfo.elementCount;
    }

    size_t EffectInputImpl::getInputIndex() const
    {
        return m_inputIndex;
    }

    EEffectUniformSemantic EffectInputImpl::getUniformSemantics() const
    {
        return EffectInputSemanticUtils::GetEffectUniformSemanticFromInternal(m_inputInfo.semantics);
    }

    EEffectAttributeSemantic EffectInputImpl::getAttributeSemantics() const
    {
        return EffectInputSemanticUtils::GetEffectAttributeSemanticFromInternal(m_inputInfo.semantics);
    }

    UniformBufferBinding EffectInputImpl::getUniformBufferBinding() const
    {
        return m_inputInfo.uniformBufferBinding;
    }

    UniformBufferElementSize EffectInputImpl::getUniformBufferElementSize() const
    {
        return m_inputInfo.uniformBufferElementSize;
    }

    UniformBufferFieldOffset EffectInputImpl::getUniformBufferFieldOffset() const
    {
        return m_inputInfo.uniformBufferFieldOffset;
    }
}
