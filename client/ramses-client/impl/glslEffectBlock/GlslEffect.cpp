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
#include "Utils/ScopedPointer.h"
#include "GlslLimits.h"

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
        const std::vector<String>& compilerDefines,
        const HashMap<String, EFixedSemantics>& semanticInputs,
        const String& name)
        : m_vertexShader(vertexShader)
        , m_fragmentShader(fragmentShader)
        , m_compilerDefines(compilerDefines)
        , m_semanticInputs(semanticInputs)
        , m_name(name)
        , m_effectResource(NULL)
        , m_vertexShaderVersion(0)
        , m_fragmentShaderVersion(0)
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
        ScopedPointer<glslang::TShader> glslVertexShader(new glslang::TShader(EShLangVertex));
        ScopedPointer<glslang::TShader> glslFragmentShader(new glslang::TShader(EShLangFragment));

        if (!createShaderParts(vertexShaderParts, defineString, m_vertexShader) ||
            !createShaderParts(fragmentShaderParts, defineString, m_fragmentShader))
        {
            return NULL;
        }

        TBuiltInResource glslCompilationResources;
        // Use the GLSL version to determine the resource limits in the shader. The version used in vertex
        // and fragment shader must be the same (checked later), so we just pass one of them for now.
        GlslLimits::InitCompilationResources(glslCompilationResources, vertexShaderParts.version);

        if (!parseShader(*glslVertexShader, glslCompilationResources, vertexShaderParts, "vertex shader") ||
            !parseShader(*glslFragmentShader, glslCompilationResources, fragmentShaderParts, "fragment shader"))
        {
            return NULL;
        }

        ScopedPointer<glslang::TProgram> program(linkProgram(glslVertexShader.get(), glslFragmentShader.get()));
        if (!program.get())
        {
            return NULL;
        }

        GlslToEffectConverter glslToEffectConverter(m_semanticInputs);
        if (!glslToEffectConverter.parseShaderProgram(program.get()))
        {
            m_errorMessages << "[GLSL Input Parser] " << glslToEffectConverter.getStatusMessage();
            return NULL;
        }

        if (!extractAndCheckShaderVersions(program.get()))
        {
            return NULL;
        }

        if (!extractAndCheckExtensions(program.get()))
        {
            return NULL;
        }

        const EffectInputInformationVector& uniformInputs = glslToEffectConverter.getUniformInputs();
        const EffectInputInformationVector& attributeInputs = glslToEffectConverter.getAttributeInputs();

        m_effectResource = new EffectResource(mergeShaderParts(vertexShaderParts), mergeShaderParts(fragmentShaderParts), uniformInputs, attributeInputs, m_name, cacheFlag);
        return m_effectResource;
    }

    String GlslEffect::createDefineString() const
    {
        StringOutputStream result;
        for(const auto& defineString : m_compilerDefines)
        {
            result << "#define " << defineString << "\n";
        }
        return result.release();
    }

    bool GlslEffect::createShaderParts(ShaderParts& outParts, const String& defineString, const String& userShader) const
    {
        Int versionStringStart;
        String versionString;
        if ((versionStringStart = userShader.find("#version")) != -1)
        {
            Int versionStringEnd = userShader.indexOf('\n', versionStringStart);
            if (versionStringEnd == -1)
            {
                m_errorMessages << "[GLSL Compiler] " << m_name << " Shader contains #version without newline \n";
                return false;
            }

            outParts.version  = userShader.substr(versionStringStart, versionStringEnd + 1 - versionStringStart);
            outParts.userCode = userShader.substr(versionStringEnd + 1, userShader.getLength() - versionStringEnd - 1);
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
        return str.release();
    }

    bool GlslEffect::parseShader(glslang::TShader& tShader, const TBuiltInResource& glslCompilationResources, const ShaderParts& shaderParts, const String& shaderName)
    {
        const char* fragmentShaderCodeCString[] = { shaderParts.version.c_str(), shaderParts.defines.c_str(), shaderParts.userCode.c_str() };
        tShader.setStrings(fragmentShaderCodeCString, 3);
        bool parsingSuccessful = tShader.parse(&glslCompilationResources, 100, false, EShMsgDefault);
        if (!parsingSuccessful)
        {
            m_errorMessages << "[GLSL Compiler] " << shaderName << " Shader Parsing Error:\n" << tShader.getInfoLog() << "\n";
            return false;
        }
        return true;
    }

    glslang::TProgram* GlslEffect::linkProgram(glslang::TShader* vertexShader, glslang::TShader* fragmentShader) const
    {
        ScopedPointer<glslang::TProgram> program(new glslang::TProgram());
        program->addShader(vertexShader);
        program->addShader(fragmentShader);

        if (!program->link(EShMsgDefault))
        {
            m_errorMessages << "[GLSL Compiler] Shader Program Linker Error:\n" << program->getInfoLog() << "\n";
            return NULL;
        }

        if (!program->buildReflection())
        {
            m_errorMessages << "[GLSL Compiler] Shader program error in buildReflection\n";
            return NULL;
        }
        return program.release();
    }

    bool GlslEffect::extractAndCheckShaderVersions(const glslang::TProgram* program)
    {
        // profile check
        EProfile vertexShaderProfile = program->getIntermediate(EShLangVertex)->getProfile();
        EProfile fragmentShaderProfile = program->getIntermediate(EShLangFragment)->getProfile();

        if (vertexShaderProfile != EEsProfile || fragmentShaderProfile != EEsProfile)
        {
            m_errorMessages << "[GLSL Compiler] " << m_name << " unsupported profile (supported profile: es)\n";
            return false;
        }

        // version check: must be GLSL ES 2.0 (100) or GLSL ES 3.0 (300)
        UInt32 vertexShaderVersion = program->getIntermediate(EShLangVertex)->getVersion();
        UInt32 fragmentShaderVersion = program->getIntermediate(EShLangFragment)->getVersion();

        if ((vertexShaderVersion != 100 && vertexShaderVersion != 300) ||
            (fragmentShaderVersion != 100 && fragmentShaderVersion != 300))
        {
            m_errorMessages << "[GLSL Compiler] " << m_name << " unsupported version (supported version strings: 100, 300 es)\n";
            return false;
        }

        if (vertexShaderVersion != fragmentShaderVersion)
        {
            m_errorMessages << "[GLSL Compiler] " << m_name << " version of vertex and fragment shader must be same\n";
            return false;
        }

        m_vertexShaderVersion = vertexShaderVersion;
        m_fragmentShaderVersion = fragmentShaderVersion;
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

    UInt32 GlslEffect::getVertexShaderVersion() const
    {
        assert(m_effectResource);
        return m_vertexShaderVersion;
    }

    UInt32 GlslEffect::getFragmentShaderVersion() const
    {
        assert(m_effectResource);
        return m_fragmentShaderVersion;
    }

    ramses_internal::String GlslEffect::getErrorMessages() const
    {
        return ramses_internal::String(m_errorMessages.c_str());
    }

    bool GlslEffect::isSupportedExtension(const String& extension) const
    {
        UNUSED(extension);

        /*
        No extension supported at the moment. Add them here:

        if (String("GL_OES_{some_extension}") == extension)
        {
            return true;
        }
        */

        return false;
    }
}
