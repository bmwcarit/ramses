//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/EFixedSemantics.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"

#include <string>
#include <string_view>
#include <optional>

namespace ramses::internal
{
    enum EEffectInputTextureType
    {
        EEffectInputTextureType_Texture2D = 0,
        EEffectInputTextureType_Texture2DMS,
        EEffectInputTextureType_Texture3D,
        EEffectInputTextureType_TextureCube,
        EEffectInputTextureType_TextureExternal,

        EEffectInputTextureType_Invalid
    };

    inline EEffectInputTextureType GetTextureTypeFromDataType(EDataType dataType)
    {
        switch (dataType)
        {
            case EDataType::TextureSampler2D: return EEffectInputTextureType_Texture2D;
            case EDataType::TextureSampler2DMS: return EEffectInputTextureType_Texture2DMS;
            case EDataType::TextureSamplerExternal: return EEffectInputTextureType_TextureExternal;
            case EDataType::TextureSampler3D: return EEffectInputTextureType_Texture3D;
            case EDataType::TextureSamplerCube: return EEffectInputTextureType_TextureCube;
            default: break;
        }
        return EEffectInputTextureType_Invalid;
    }

    struct EffectInputInformation
    {
        EffectInputInformation() = default;
        inline EffectInputInformation(std::string_view inputName_,
            uint32_t elementCount_,
            EDataType dataType_,
            EFixedSemantics semantics_,
            UniformBufferBinding uniformBufferBinding_ = {},
            UniformBufferElementSize uniformBufferElementSize_= {},
            UniformBufferFieldOffset uniformBufferFieldOffset_ = {}
        );

        inline friend bool operator==(const EffectInputInformation& a, const EffectInputInformation& b);
        inline friend bool operator!=(const EffectInputInformation& a, const EffectInputInformation& b);
        static inline bool IsUniformBuffer(const EffectInputInformation& input);
        static inline bool IsUniformBufferField(const EffectInputInformation& input);

        std::string             inputName;
        uint32_t                elementCount{ std::numeric_limits<uint32_t>::max() };
        EDataType               dataType{EDataType::Invalid};
        EFixedSemantics         semantics{EFixedSemantics::Invalid};

        UniformBufferBinding     uniformBufferBinding;
        UniformBufferElementSize uniformBufferElementSize;
        UniformBufferFieldOffset uniformBufferFieldOffset;

        DataFieldHandle dataFieldHandle{};
    };

    using EffectInputInformationVector = std::vector<EffectInputInformation>;

    EffectInputInformation::EffectInputInformation(std::string_view inputName_,
        uint32_t elementCount_,
        EDataType dataType_,
        EFixedSemantics semantics_,
        UniformBufferBinding uniformBufferBinding_,
        UniformBufferElementSize uniformBufferElementSize_,
        UniformBufferFieldOffset uniformBufferFieldOffset_ )
        : inputName(inputName_)
        , elementCount(elementCount_)
        , dataType(dataType_)
        , semantics(semantics_)
        , uniformBufferBinding(uniformBufferBinding_)
        , uniformBufferElementSize(uniformBufferElementSize_)
        , uniformBufferFieldOffset(uniformBufferFieldOffset_)
    {
    }

    bool operator==(const EffectInputInformation& a, const EffectInputInformation& b)
    {
        return a.inputName == b.inputName &&
            a.elementCount == b.elementCount &&
            a.dataType == b.dataType &&
            a.semantics == b.semantics &&
            a.uniformBufferBinding == b.uniformBufferBinding &&
            a.uniformBufferElementSize == b.uniformBufferElementSize &&
            a.uniformBufferFieldOffset == b.uniformBufferFieldOffset;
    }

    inline bool operator!=(const EffectInputInformation& a, const EffectInputInformation& b)
    {
        return !(a == b);
    }

    inline bool EffectInputInformation::IsUniformBuffer(const EffectInputInformation& input)
    {
        return input.dataType == EDataType::UniformBuffer;
    }

    inline bool EffectInputInformation::IsUniformBufferField(const EffectInputInformation& input)
    {
        return input.uniformBufferBinding.isValid() && input.dataType != EDataType::UniformBuffer;
    }
}
