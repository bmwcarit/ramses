//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/OpenGL/ShaderGPUResource_GL.h"
#include "internal/Platform/OpenGL/Device_GL_platform.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    ShaderGPUResource_GL::ShaderGPUResource_GL(const EffectResource& effect, ShaderProgramInfo shaderProgramInfo)
        : ShaderGPUResource(shaderProgramInfo.shaderProgramHandle)
        , m_shaderProgramInfo(std::move(shaderProgramInfo))
    {
        preloadVariableLocations(effect);
    }

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

    GLInputLocation ShaderGPUResource_GL::getUniformLocation(DataFieldHandle field) const
    {
        assert(field.asMemoryHandle() < m_uniformLocationMap.size());
        return m_uniformLocationMap[field.asMemoryHandle()];
    }

    GLInputLocation ShaderGPUResource_GL::getAttributeLocation(DataFieldHandle field) const
    {
        assert(field.asMemoryHandle() < m_attributeLocationMap.size());
        return m_attributeLocationMap[field.asMemoryHandle()];
    }

    TextureSlotInfo ShaderGPUResource_GL::getTextureSlot(DataFieldHandle field) const
    {
        TextureSlotInfo slot;
        [[maybe_unused]] const EStatus status = m_bufferSlots.get(field, slot);
        assert(status == EStatus::Ok);
        return slot;
    }

    void ShaderGPUResource_GL::preloadVariableLocations(const EffectResource& effect)
    {
        const EffectInputInformationVector& uniformInputs = effect.getUniformInputs();
        const EffectInputInformationVector& attributeInputs = effect.getAttributeInputs();

        const size_t vertexInputCount = attributeInputs.size();
        const size_t globalInputCount = uniformInputs.size();

        m_attributeLocationMap.resize(vertexInputCount);
        m_uniformLocationMap.resize(globalInputCount);

        for (uint32_t i = 0u; i < vertexInputCount; ++i)
        {
            const GLInputLocation location = loadAttributeLocation(effect, attributeInputs[i]);
            m_attributeLocationMap[i] = location;
        }

        TextureSlot slotCounter = 0; // texture unit 0
        for (uint32_t i = 0; i < globalInputCount; ++i)
        {
            const EffectInputInformation& input = uniformInputs[i];
            if (IsTextureSamplerType(input.dataType))
            {
                TextureSlotInfo bufferSlot;
                bufferSlot.slot = slotCounter++;
                bufferSlot.textureType = GetTextureTypeFromDataType(input.dataType);
                m_bufferSlots.put(DataFieldHandle(i), bufferSlot);
            }

            const GLInputLocation location = loadUniformLocation(effect, input);
            m_uniformLocationMap[i] = location;
        }
    }

    GLInputLocation ShaderGPUResource_GL::loadAttributeLocation(const EffectResource& effect, const EffectInputInformation& input) const
    {
        const char* varName = input.inputName.c_str();
        const GLint address = glGetAttribLocation(m_shaderProgramInfo.shaderProgramHandle, varName);
        const GLInputLocation inputLocation(address);
        if (!inputLocation.isValid())
        {
            LOG_WARN(CONTEXT_RENDERER, "ShaderGPUResource_GL::loadAttributeLocation:  glGetAttribLocation for effect '{}' for attribute '{}' failed", effect.getName(), varName);
        }

        return inputLocation;
    }

    GLInputLocation ShaderGPUResource_GL::loadUniformLocation(const EffectResource& effect, const EffectInputInformation& input) const
    {
        const char* varName = input.inputName.c_str();
        const GLint address = glGetUniformLocation(m_shaderProgramInfo.shaderProgramHandle, varName);
        const GLInputLocation inputLocation(address);
        if (!inputLocation.isValid())
        {
            LOG_WARN(CONTEXT_RENDERER, "ShaderGPUResource_GL::loadUniformLocation:  for effect '{}' for uniform '{}' failed", effect.getName(), varName);
        }

        return inputLocation;
    }

    bool ShaderGPUResource_GL::getBinaryInfo(std::vector<std::byte>& binaryShader, BinaryShaderFormatID& binaryShaderFormat) const
    {
        GLint sizeInBytes = -1;
        glGetProgramiv(m_shaderProgramInfo.shaderProgramHandle, GL_PROGRAM_BINARY_LENGTH, &sizeInBytes);

        if (sizeInBytes <= 0)
        {
            LOG_WARN(CONTEXT_RENDERER, "ShaderGPUResource_GL::getBinaryInfo: invalid binary shader size ({}) retrieved from device.", sizeInBytes);
            return false;
        }

        GLenum binaryFormat = 0;
        GLsizei actualSize = 0;

        binaryShader.resize(static_cast<size_t>(sizeInBytes));
        glGetProgramBinary(m_shaderProgramInfo.shaderProgramHandle, sizeInBytes, &actualSize, &binaryFormat, &binaryShader.front());
        binaryShaderFormat = BinaryShaderFormatID{ binaryFormat };
        return actualSize == sizeInBytes;
    }
}
