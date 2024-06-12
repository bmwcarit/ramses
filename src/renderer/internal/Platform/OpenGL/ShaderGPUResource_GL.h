//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/ShaderGPUResource.h"
#include "internal/Platform/OpenGL/ShaderProgramInfo.h"
#include "internal/SceneGraph/Resource/EffectInputInformation.h"

namespace ramses::internal
{
    class EffectResource;

    struct TextureSlotInfo
    {
        explicit TextureSlotInfo(TextureSlot slotInit = 0u)
            : slot(slotInit)
        {
        }

        TextureSlot slot;
        EEffectInputTextureType textureType{EEffectInputTextureType_Invalid};
    };

    class ShaderGPUResource_GL final : public ShaderGPUResource
    {
    public:
        ShaderGPUResource_GL(const EffectResource& effect, ShaderProgramInfo shaderProgramInfo);
        ~ShaderGPUResource_GL() override;

        [[nodiscard]] GLInputLocation     getUniformLocation(DataFieldHandle field) const;
        [[nodiscard]] GLInputLocation     getAttributeLocation(DataFieldHandle field) const;
        [[nodiscard]] TextureSlotInfo     getTextureSlot(DataFieldHandle field) const;
        [[nodiscard]] UniformBufferBinding getUniformBufferBinding(DataFieldHandle field) const;

        [[nodiscard]] bool                getBinaryInfo(std::vector<std::byte>& binaryShader, BinaryShaderFormatID& binaryShaderFormat) const;

    private:
        void                              init(const EffectResource& effect);
        [[nodiscard]] GLInputLocation     loadUniformLocation(const EffectResource& effect, const EffectInputInformation& input) const;
        [[nodiscard]] GLInputLocation     loadAttributeLocation(const EffectResource& effect, const EffectInputInformation& input) const;

        ShaderProgramInfo m_shaderProgramInfo;

        using BufferSlotMap = HashMap<DataFieldHandle, TextureSlotInfo>;
        using InputLocationMap = std::vector<GLInputLocation>;

        BufferSlotMap    m_bufferSlots;
        InputLocationMap m_uniformLocationMap;
        InputLocationMap m_attributeLocationMap;
        std::vector<UniformBufferBinding> m_uniformBufferBindings;
    };
}
