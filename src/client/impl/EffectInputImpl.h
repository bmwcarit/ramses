//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// client api
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/EDataType.h"
#include "ramses/client/EffectInputSemantic.h"

// framework
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/Resource/EffectInputInformation.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"

#include <string_view>
#include <string>

namespace ramses::internal
{
    class EffectInputImpl final
    {
    public:
        EffectInputImpl() = default;

        void initialize(const ResourceContentHash& effectHash, const EffectInputInformation& inputInfo, size_t index);

        [[nodiscard]] ResourceContentHash         getEffectHash() const;
        [[nodiscard]] const std::string&          getName() const;
        [[nodiscard]] ramses::internal::EDataType getInternalDataType() const;
        [[nodiscard]] EFixedSemantics             getSemantics() const;
        [[nodiscard]] size_t                      getElementCount() const;
        [[nodiscard]] size_t                      getInputIndex() const;
        [[nodiscard]] UniformBufferBinding        getUniformBufferBinding() const;
        [[nodiscard]] UniformBufferFieldOffset    getUniformBufferFieldOffset() const;
        [[nodiscard]] UniformBufferElementSize    getUniformBufferElementSize() const;

        [[nodiscard]] ramses::EDataType           getDataType() const;
        [[nodiscard]] EEffectUniformSemantic      getUniformSemantics() const;
        [[nodiscard]] EEffectAttributeSemantic    getAttributeSemantics() const;

        [[nodiscard]] DataFieldHandle getDataFieldHandle() const { return m_inputInfo.dataFieldHandle; }

    private:
        ResourceContentHash         m_effectHash{};
        EffectInputInformation      m_inputInfo;
        size_t                      m_inputIndex{ std::numeric_limits<uint32_t>::max() };
    };
}
