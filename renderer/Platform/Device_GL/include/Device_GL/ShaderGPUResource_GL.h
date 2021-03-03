//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADERGPURESOURCE_GL_H
#define RAMSES_SHADERGPURESOURCE_GL_H

#include "Platform_Base/ShaderGPUResource.h"
#include "Device_GL/ShaderProgramInfo.h"
#include "Resource/EffectInputInformation.h"

namespace ramses_internal
{
    class EffectResource;

    struct TextureSlotInfo
    {
        explicit TextureSlotInfo(TextureSlot slotInit = 0u)
            : slot(slotInit)
            , textureType(EEffectInputTextureType_Invalid)
        {
        }

        TextureSlot slot;
        EEffectInputTextureType textureType;
    };

    class ShaderGPUResource_GL : public ShaderGPUResource
    {
    public:
        ShaderGPUResource_GL(const EffectResource& effect, ShaderProgramInfo shaderProgramInfo);
        virtual ~ShaderGPUResource_GL() override;

        GLInputLocation     getUniformLocation(DataFieldHandle) const;
        GLInputLocation     getAttributeLocation(DataFieldHandle) const;
        TextureSlotInfo     getTextureSlot(DataFieldHandle) const;

        bool                getBinaryInfo(UInt8Vector& binaryShader, BinaryShaderFormatID& binaryShaderFormat) const;

    private:
        void                preloadVariableLocations(const EffectResource& effect);
        GLInputLocation     loadUniformLocation(const EffectResource& effect, const EffectInputInformation& input) const;
        GLInputLocation     loadAttributeLocation(const EffectResource& effect, const EffectInputInformation& input) const;

        ShaderProgramInfo m_shaderProgramInfo;

        using BufferSlotMap = HashMap<DataFieldHandle, TextureSlotInfo>;
        using InputLocationMap = std::vector<GLInputLocation>;

        BufferSlotMap    m_bufferSlots;
        InputLocationMap m_uniformLocationMap;
        InputLocationMap m_attributeLocationMap;
    };
}

#endif
