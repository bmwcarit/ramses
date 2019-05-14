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
#include "Resource/EffectResource.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    struct TextureSlotInfo
    {
        TextureSlotInfo(TextureSlot slotInit = 0u)
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
        ~ShaderGPUResource_GL();

        GLInputLocation     getUniformLocation(DataFieldHandle) const;
        GLInputLocation     getAttributeLocation(DataFieldHandle) const;
        TextureSlotInfo     getTextureSlot(DataFieldHandle) const;

        bool                getBinaryInfo(UInt8Vector& binaryShader, UInt32& binaryShaderFormat) const;

    private:
        void                preloadVariableLocations(const EffectResource& effect);
        GLInputLocation     loadUniformLocation(const EffectResource& effect, const EffectInputInformation& input) const;
        GLInputLocation     loadAttributeLocation(const EffectResource& effect, const EffectInputInformation& input) const;

        ShaderProgramInfo m_shaderProgramInfo;

        typedef HashMap<DataFieldHandle, TextureSlotInfo> BufferSlotMap;
        typedef std::vector<GLInputLocation>                   InputLocationMap;

        BufferSlotMap    m_bufferSlots;
        InputLocationMap m_uniformLocationMap;
        InputLocationMap m_attributeLocationMap;
    };

    // inline implementation:
    // subclasses must use their custom OpenGL (ES) x.y headers and libraries

    inline
    ShaderGPUResource_GL::ShaderGPUResource_GL(const EffectResource& effect, ShaderProgramInfo shaderProgramInfo)
        : ShaderGPUResource(shaderProgramInfo.shaderProgramHandle)
        , m_shaderProgramInfo(shaderProgramInfo)
    {
        preloadVariableLocations(effect);
    }

    inline
    ShaderGPUResource_GL::~ShaderGPUResource_GL()
    {
        // To be safe, unbind the program so that it can be deleted
        // Generally, resources should not be deleted while rendering
        glUseProgram(0);

        if (0 != m_shaderProgramInfo.shaderProgramHandle)
        {
            glDeleteProgram(m_shaderProgramInfo.shaderProgramHandle);
        }

        if (0 != m_shaderProgramInfo.vertexShaderHandle)
        {
            glDeleteShader(m_shaderProgramInfo.vertexShaderHandle);
        }

        if (0 != m_shaderProgramInfo.fragmentShaderHandle)
        {
            glDeleteShader(m_shaderProgramInfo.fragmentShaderHandle);
        }
    }

    inline GLInputLocation ShaderGPUResource_GL::getUniformLocation(DataFieldHandle field) const
    {
        assert(field.asMemoryHandle() < m_uniformLocationMap.size());
        return m_uniformLocationMap[field.asMemoryHandle()];
    }

    inline GLInputLocation ShaderGPUResource_GL::getAttributeLocation(DataFieldHandle field) const
    {
        assert(field.asMemoryHandle() < m_attributeLocationMap.size());
        return m_attributeLocationMap[field.asMemoryHandle()];
    }

    inline TextureSlotInfo ShaderGPUResource_GL::getTextureSlot(DataFieldHandle field) const
    {
        TextureSlotInfo slot;
        const EStatus status = m_bufferSlots.get(field, slot);
        UNUSED(status);
        assert(status == EStatus_RAMSES_OK);
        return slot;
    }

    inline void ShaderGPUResource_GL::preloadVariableLocations(const EffectResource& effect)
    {
        const EffectInputInformationVector& uniformInputs = effect.getUniformInputs();
        const EffectInputInformationVector& attributeInputs = effect.getAttributeInputs();

        const UInt vertexInputCount = attributeInputs.size();
        const UInt globalInputCount = uniformInputs.size();

        m_attributeLocationMap.resize(vertexInputCount);
        m_uniformLocationMap.resize(globalInputCount);

        for (UInt32 i = 0u; i < vertexInputCount; ++i)
        {
            const GLInputLocation location = loadAttributeLocation(effect, attributeInputs[i]);
            m_attributeLocationMap[i] = location;
        }

        TextureSlot slotCounter = 0; // texture unit 0
        for (UInt32 i = 0; i < globalInputCount; ++i)
        {
            const EffectInputInformation& input = uniformInputs[i];
            if (EDataType_TextureSampler == input.dataType)
            {
                TextureSlotInfo bufferSlot;
                bufferSlot.slot = slotCounter++;
                bufferSlot.textureType = input.textureType;
                m_bufferSlots.put(DataFieldHandle(i), bufferSlot);
            }

            const GLInputLocation location = loadUniformLocation(effect, input);
            m_uniformLocationMap[i] = location;
        }
    }

    inline GLInputLocation ShaderGPUResource_GL::loadAttributeLocation(const EffectResource& effect, const EffectInputInformation& input) const
    {
        const Char* varName = input.inputName.c_str();
        const GLint address = glGetAttribLocation(m_shaderProgramInfo.shaderProgramHandle, varName);
        const GLInputLocation inputLocation(address);
        if (inputLocation == GLInputLocationInvalid)
        {
            LOG_WARN(CONTEXT_RENDERER, "ShaderGPUResource_GL::loadAttributeLocation:  glGetAttribLocation for effect '" << effect.getName() << "' for attribute '" << varName << "' failed");
        }

        return inputLocation;
    }

    inline GLInputLocation ShaderGPUResource_GL::loadUniformLocation(const EffectResource& effect, const EffectInputInformation& input) const
    {
        const Char* varName = input.inputName.c_str();
        const GLint address = glGetUniformLocation(m_shaderProgramInfo.shaderProgramHandle, varName);
        const GLInputLocation inputLocation(address);
        if (inputLocation == GLInputLocationInvalid)
        {
            LOG_WARN(CONTEXT_RENDERER, "ShaderGPUResource_GL::loadUniformLocation:  for effect '" << effect.getName() << "' for uniform '" << varName << "' failed");
        }

        return inputLocation;
    }

    inline bool ShaderGPUResource_GL::getBinaryInfo(UInt8Vector& binaryShader, UInt32& binaryShaderFormat) const
    {
        GLint length = -1;
        glGetProgramiv(m_shaderProgramInfo.shaderProgramHandle, GL_PROGRAM_BINARY_LENGTH, &length);

        if (length == 0)
        {
            return false;
        }

        GLenum binaryFormat;
        GLsizei binarySize;

        binaryShader.resize(length);
        glGetProgramBinary(m_shaderProgramInfo.shaderProgramHandle, length, &binarySize, &binaryFormat, &binaryShader.front());
        binaryShaderFormat = binaryFormat;
        return binarySize == length;
    }
}

#endif
