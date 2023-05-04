//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "GlslEffect.h"
#include "Resource/EffectResource.h"
#include "glslEffectBlock/GlslToEffectConverter.h"
#include "GlslLimits.h"
#include <memory>

namespace ramses_internal
{
    /*
      wrapper for glslang process wide initializer and finalizer.
      constructed once in this file. it is global and lives longer
      than main. will not work if global static objects start using
      effects but this should never happen.
    */
    class GlslangInitAndFinalizeOnceHelper
    {
    public:
        GlslangInitAndFinalizeOnceHelper()
        {
            glslang::InitializeProcess();
            glslang::InitProcess();
        }

        ~GlslangInitAndFinalizeOnceHelper()
        {
            glslang::DetachProcess();
            glslang::FinalizeProcess();
        }
    };
    static GlslangInitAndFinalizeOnceHelper glslangInitializer;


    GlslEffect::GlslEffect(const String& vertexShader,
        const String& fragmentShader,
        const String& geometryShader,
        const std::vector<String>& compilerDefines,
        const HashMap<String, EFixedSemantics>& semanticInputs,
        const String& name)
        : m_vertexShader(vertexShader)
        , m_fragmentShader(fragmentShader)
        , m_geometryShader(geometryShader)
        , m_compilerDefines(compilerDefines)
        , m_semanticInputs(semanticInputs)
        , m_name(name)
        , m_effectResource(nullptr)
        , m_shadingLanguageVersion(0)
    {
    }

    GlslEffect::~GlslEffect()
    {
    }

    EffectResource* GlslEffect::createEffectResource(ResourceCacheFlag cacheFlag)
    {
        if (m_effectResource)
        {
            return m_effectResource;
        }

        String defineString = createDefineString();

        ShaderParts vertexShaderParts;
        ShaderParts fragmentShaderParts;
        ShaderParts geometryShaderParts;
        std::unique_ptr<glslang::TShader> glslVertexShader(new glslang::TShader(EShLangVertex));
        std::unique_ptr<glslang::TShader> glslFragmentShader(new glslang::TShader(EShLangFragment));

        const bool hasGeometryShader = !m_geometryShader.empty();
        std::unique_ptr<glslang::TShader> glslGeometryShader(hasGeometryShader ? new glslang::TShader(EShLangGeometry) : nullptr);

        if (!createShaderParts(vertexShaderParts, defineString, m_vertexShader) ||
            !createShaderParts(fragmentShaderParts, defineString, m_fragmentShader) ||
            (hasGeometryShader && !createShaderParts(geometryShaderParts, defineString, m_geometryShader)))
        {
            return nullptr;
        }

        TBuiltInResource glslCompilationResources;
        // Use the GLSL version to determine the resource limits in the shader. The version used in vertex
        // and fragment shader must be the same (checked later), so we just pass one of them for now.
        GlslLimits::InitCompilationResources(glslCompilationResources, vertexShaderParts.version);
        if (!parseShader(*glslVertexShader, glslCompilationResources, vertexShaderParts, "vertex shader") ||
            !parseShader(*glslFragmentShader, glslCompilationResources, fragmentShaderParts, "fragment shader") ||
            (hasGeometryShader && !parseShader(*glslGeometryShader, glslCompilationResources, geometryShaderParts, "geometry shader")))
        {
            return nullptr;
        }

        std::unique_ptr<glslang::TProgram> program(linkProgram(glslVertexShader.get(),
                                                                glslFragmentShader.get(),
                                                                hasGeometryShader ? glslGeometryShader.get() : nullptr));
        if (!program)
        {
            return nullptr;
        }

        GlslToEffectConverter glslToEffectConverter(m_semanticInputs);
        if (!glslToEffectConverter.parseShaderProgram(program.get()))
        {
            m_errorMessages << "[GLSL Input Parser] " << glslToEffectConverter.getStatusMessage();
            return nullptr;
        }

        if (!extractAndCheckShaderVersions(program.get()))
        {
            return nullptr;
        }

        if (!extractAndCheckExtensions(program.get()))
        {
            return nullptr;
        }

        const EffectInputInformationVector& uniformInputs = glslToEffectConverter.getUniformInputs();
        const EffectInputInformationVector& attributeInputs = glslToEffectConverter.getAttributeInputs();
        const EDrawMode geomInputType = glslToEffectConverter.getGeometryShaderInputType();

        m_effectResource = new EffectResource(mergeShaderParts(vertexShaderParts), mergeShaderParts(fragmentShaderParts), mergeShaderParts(geometryShaderParts), geomInputType, uniformInputs, attributeInputs, m_name, cacheFlag);
        return m_effectResource;
    }

    String GlslEffect::createDefineString() const
    {
        StringOutputStream result;
        for(const auto& defineString : m_compilerDefines)
        {
            result << "#define " << defineString << "\n";
        }
        return String(result.release());
    }

    bool GlslEffect::createShaderParts(ShaderParts& outParts, const String& defineString, const String& userShader) const
    {
        size_t versionStringStart;
        String versionString;
        if ((versionStringStart = userShader.find("#version")) != String::npos)
        {
            size_t versionStringEnd = userShader.find('\n', versionStringStart);
            if (versionStringEnd == String::npos)
            {
                m_errorMessages << "[GLSL Compiler] " << m_name << " Shader contains #version without newline \n";
                return false;
            }

            outParts.version  = userShader.substr(versionStringStart, versionStringEnd + 1 - versionStringStart);
            outParts.userCode = userShader.substr(versionStringEnd + 1, userShader.size() - versionStringEnd - 1);
        }
        else
        {
            outParts.version = "#version 100\n";
            outParts.userCode = userShader;
        }

        outParts.defines = defineString;
        return true;
    }

    String GlslEffect::mergeShaderParts(const ShaderParts& shaderParts) const
    {
        StringOutputStream str;
        str << shaderParts.version << shaderParts.defines << shaderParts.userCode;
        return String(str.release());
    }

    bool GlslEffect::parseShader(glslang::TShader& tShader, const TBuiltInResource& glslCompilationResources, const ShaderParts& shaderParts, const String& shaderName)
    {
        const std::array fragmentShaderCodeCString = { shaderParts.version.c_str(), shaderParts.defines.c_str(), shaderParts.userCode.c_str() };
        tShader.setStrings(fragmentShaderCodeCString.data(), 3);
        bool parsingSuccessful = tShader.parse(&glslCompilationResources, 100, false, EShMsgDefault);
        if (!parsingSuccessful)
        {
            m_errorMessages << "[GLSL Compiler] " << shaderName << " Shader Parsing Error:\n" << tShader.getInfoLog() << "\n";
            return false;
        }
        return true;
    }

    glslang::TProgram* GlslEffect::linkProgram(glslang::TShader* vertexShader, glslang::TShader* fragmentShader, glslang::TShader* geometryShader) const
    {
        std::unique_ptr<glslang::TProgram> program(new glslang::TProgram());
        program->addShader(vertexShader);
        program->addShader(fragmentShader);
        if (geometryShader)
            program->addShader(geometryShader);

        if (!program->link(EShMsgDefault))
        {
            m_errorMessages << "[GLSL Compiler] Shader Program Linker Error:\n" << program->getInfoLog() << "\n";
            return nullptr;
        }

        if (!program->buildReflection())
        {
            m_errorMessages << "[GLSL Compiler] Shader program error in buildReflection\n";
            return nullptr;
        }
        return program.release();
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

        const UInt32 vertexShaderVersion = program->getIntermediate(EShLangVertex)->getVersion();
        const UInt32 fragmentShaderVersion = program->getIntermediate(EShLangFragment)->getVersion();
        const UInt32 geometryShaderVersion = hasGeometryShader ? geometryShaderIntermediate->getVersion() : 0u;

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
        for (std::set<std::string>::const_iterator it = vertexExtensions.begin(); it != vertexExtensions.end(); it++)
        {
            const char* extensionName = it->c_str();
            if (!isSupportedExtension(extensionName))
            {
                m_errorMessages << "[GLSL Compiler] " << m_name << " extension not supported in vertex shader: " << extensionName << "\n";
                success = false;
            }
        }

        const std::set<std::string>& fragmentExtensions = program->getIntermediate(EShLangFragment)->getRequestedExtensions();
        for (std::set<std::string>::const_iterator it = fragmentExtensions.begin(); it != fragmentExtensions.end(); it++)
        {
            const char* extensionName = it->c_str();
            if (!isSupportedExtension(extensionName))
            {
                m_errorMessages << "[GLSL Compiler] " << m_name << " extension not supported in fragment shader: " << extensionName << "\n";
                success = false;
            }
        }

        return success;
    }

    UInt32 GlslEffect::getShadingLanguageVersion() const
    {
        assert(m_effectResource);
        return m_shadingLanguageVersion;
    }

    ramses_internal::String GlslEffect::getEffectErrorMessages() const
    {
        return ramses_internal::String(m_errorMessages.c_str());
    }

    bool GlslEffect::isSupportedExtension(const String& extension) const
    {
        if (String("GL_OES_EGL_image_external") == extension
            || String("GL_OES_EGL_image_external_essl3") == extension)
        {
            return true;
        }

        return false;
    }
}
