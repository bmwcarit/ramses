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
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/EFixedSemantics.h"

#include <string_view>
#include <string>

namespace ramses::internal
{
    class EffectInputImpl final
    {
    public:
        EffectInputImpl() = default;

        void initialize(
            const ResourceContentHash&  effectHash,
            std::string_view            name,
            ramses::internal::EDataType dataType,
            EFixedSemantics             semantics,
            size_t                      elementCount,
            size_t                      index);

        [[nodiscard]] ResourceContentHash         getEffectHash() const;
        [[nodiscard]] const std::string&          getName() const;
        [[nodiscard]] ramses::internal::EDataType getInternalDataType() const;
        [[nodiscard]] EFixedSemantics             getSemantics() const;
        [[nodiscard]] size_t                      getElementCount() const;
        [[nodiscard]] size_t                      getInputIndex() const;

        [[nodiscard]] ramses::EDataType           getDataType() const;
        [[nodiscard]] EEffectUniformSemantic      getUniformSemantics() const;
        [[nodiscard]] EEffectAttributeSemantic    getAttributeSemantics() const;

    private:
        ResourceContentHash         m_effectHash{};
        std::string                 m_name;
        ramses::internal::EDataType m_dataType{ramses::internal::EDataType::Invalid};
        EFixedSemantics             m_semantics{EFixedSemantics::Invalid};
        size_t                      m_elementCount{0u};
        size_t                      m_inputIndex{std::numeric_limits<uint32_t>::max()};
    };
}
