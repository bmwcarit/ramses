//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "GlslEffect.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/glslEffectBlock/GlslToEffectConverter.h"
#include "internal/glslEffectBlock/GlslParser.h"
#include "internal/glslEffectBlock/GlslLimits.h"

#include <memory>

namespace ramses::internal
{
    GlslEffect::GlslEffect(std::string_view vertexShader,
        std::string_view fragmentShader,
        std::string_view geometryShader,
        std::vector<std::string> compilerDefines,
        SemanticsMap semanticInputs,
        ERenderBackendCompatibility compatibility,
        std::string_view name)
        : m_vertexShader(vertexShader)
        , m_fragmentShader(fragmentShader)
        , m_geometryShader(geometryShader)
        , m_compilerDefines(std::move(compilerDefines))
        , m_semanticInputs(std::move(semanticInputs))
        , m_renderBackendCompatibility(compatibility)
        , m_name(name)
    {
    }

    std::unique_ptr<EffectResource> GlslEffect::createEffectResource(EFeatureLevel featureLevel)
    {
        GlslParser parser{m_vertexShader, m_fragmentShader, m_geometryShader, m_compilerDefines, m_renderBackendCompatibility};
        if (!parser.valid())
        {
            m_errorMessages << parser.getErrors();
            return nullptr;
        }

        GlslToEffectConverter glslToEffectConverter(m_semanticInputs);
        if (!glslToEffectConverter.parseShaderProgram(parser.getProgram()))
        {
            m_errorMessages << "[GLSL Input Parser] " << glslToEffectConverter.getStatusMessage();
            return nullptr;
        }

        if (!extractAndCheckShaderVersions(parser.getProgram()))
        {
            return nullptr;
        }

        if (!extractAndCheckExtensions(parser.getProgram()))
        {
            return nullptr;
        }

        const EffectInputInformationVector& uniformInputs = glslToEffectConverter.getUniformInputs();
        const EffectInputInformationVector& attributeInputs = glslToEffectConverter.getAttributeInputs();
        const auto geomInputType = glslToEffectConverter.getGeometryShaderInputType();

        // forbid effect creation with UBOs below FL02
        if (featureLevel < EFeatureLevel_02)
        {
            if (std::any_of(uniformInputs.cbegin(), uniformInputs.cend(), [](const auto& uniform) { return uniform.uniformBufferBinding.isValid(); }))
            {
                m_errorMessages << "Uniform buffer objects are supported only with feature level 02 or higher";
                return nullptr;
            }
        }

        return std::make_unique<EffectResource>(parser.getVertexShader(),
            parser.getFragmentShader(),
            parser.getGeometryShader(),
            parser.getSPIRVShaders(),
            geomInputType,
            uniformInputs,
            attributeInputs,
            m_name,
            featureLevel);
    }

    bool GlslEffect::extractAndCheckShaderVersions(const glslang::TProgram* program)
    {
        // profile check
        const EProfile vertexShaderProfile = program->getIntermediate(EShLangVertex)->getProfile();
        const EProfile fragmentShaderProfile = program->getIntermediate(EShLangFragment)->getProfile();
        const auto geometryShaderIntermediate = program->getIntermediate(EShLangGeometry);
        const bool hasGeometryShader = (nullptr != geometryShaderIntermediate);
        const EProfile geometryShaderProfile = hasGeometryShader ? geometryShaderIntermediate->getProfile() : ENoProfile;

        if (vertexShaderProfile != EEsProfile || fragmentShaderProfile != EEsProfile || (hasGeometryShader && geometryShaderProfile != EEsProfile))
        {
            m_errorMessages << "[GLSL Compiler] " << m_name << " unsupported profile (supported profile: es)\n";
            return false;
        }

        const uint32_t vertexShaderVersion = program->getIntermediate(EShLangVertex)->getVersion();
        const uint32_t fragmentShaderVersion = program->getIntermediate(EShLangFragment)->getVersion();
        const uint32_t geometryShaderVersion = hasGeometryShader ? geometryShaderIntermediate->getVersion() : 0u;

        const uint32_t maximumSupportedVersion = 320u;

        if (vertexShaderVersion > maximumSupportedVersion ||
            fragmentShaderVersion > maximumSupportedVersion ||
            (hasGeometryShader && geometryShaderVersion > maximumSupportedVersion))
        {
            m_errorMessages << "[GLSL Compiler] " << m_name << " unsupported version (maximum supported version: 320 es)\n";
            return false;
        }

        if (vertexShaderVersion != fragmentShaderVersion ||
            (hasGeometryShader && vertexShaderVersion != geometryShaderVersion))
        {
            m_errorMessages << "[GLSL Compiler] " << m_name << " version of vertex, fragment and geometry shaders must be same\n";
            return false;
        }

        m_shadingLanguageVersion = vertexShaderVersion;
        return true;
    }

    bool GlslEffect::extractAndCheckExtensions(const glslang::TProgram* program)
    {
        bool success = true;
        const std::set<std::string>& vertexExtensions = program->getIntermediate(EShLangVertex)->getRequestedExtensions();
        for (const auto& it : vertexExtensions)
        {
            const char* extensionName = it.c_str();
            if (!IsSupportedExtension(extensionName))
            {
                m_errorMessages << "[GLSL Compiler] " << m_name << " extension not supported in vertex shader: " << extensionName << "\n";
                success = false;
            }
        }

        const std::set<std::string>& fragmentExtensions = program->getIntermediate(EShLangFragment)->getRequestedExtensions();
        for (const auto& it : fragmentExtensions)
        {
            const char* extensionName = it.c_str();
            if (!IsSupportedExtension(extensionName))
            {
                m_errorMessages << "[GLSL Compiler] " << m_name << " extension not supported in fragment shader: " << extensionName << "\n";
                success = false;
            }
        }

        return success;
    }

    uint32_t GlslEffect::getShadingLanguageVersion() const
    {
        return m_shadingLanguageVersion;
    }

    std::string GlslEffect::getEffectErrorMessages() const
    {
        return m_errorMessages.data();
    }

    bool GlslEffect::IsSupportedExtension(const std::string& extension)
    {
        if (extension == "GL_OES_EGL_image_external"
            || extension == "GL_OES_EGL_image_external_essl3")
        {
            return true;
        }

        return false;
    }
}
