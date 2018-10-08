//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ShaderConverterTest.h"
#include "ShaderFileUtil.h"

#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"

#include "EffectDescriptionImpl.h"
#include "glslEffectBlock/GlslEffect.h"
#include "RamsesClientImpl.h"

#include "Platform_Base/PlatformFactory_Base.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IDevice.h"
#include "Resource/EffectResource.h"

//////////////////////////////////////////////////////////////////////////
// SIMPLE TEST CASE FOR SHADER CONVERSION                               //
//////////////////////////////////////////////////////////////////////////

namespace ramses_internal
{

    ShaderConverterTest::ShaderConverterTest(DisplayConfig& displayConfig, Bool noGpuUpload)
        : m_ramses("Shader End-to-end Test Client", m_framework)
        , m_displayConfig(displayConfig)
        , m_platformFactory(NULL)
        , m_renderTestBackend(NULL)
    {
        if (!noGpuUpload)
        {
            initializeRenderBackend();
        }
    }

    void ShaderConverterTest::initializeRenderBackend()
    {
        RendererConfig rendererConfig;
        m_platformFactory = PlatformFactory_Base::CreatePlatformFactory(rendererConfig);

        if (m_platformFactory)
        {
            m_displayConfig.setFullscreenState(false);
            m_displayConfig.setBorderlessState(true);
            m_displayConfig.setResizable(false);
            m_displayConfig.setWindowPositionX(0u);
            m_displayConfig.setWindowPositionY(0u);
            m_displayConfig.setDesiredWindowWidth(1u);
            m_displayConfig.setDesiredWindowHeight(1u);

            m_renderTestBackend = m_platformFactory->createRenderBackend(m_displayConfig, m_windowEventHandlerDummy);
        }
    }

    ShaderConverterTest::~ShaderConverterTest()
    {
        if (m_renderTestBackend)
        {
            // destroy render backend
            m_platformFactory->destroyRenderBackend(*m_renderTestBackend);
        }
    }

    Bool ShaderConverterTest::hasRenderBackend() const
    {
        return NULL != m_renderTestBackend;
    }

    Bool ShaderConverterTest::testShader(const ShaderProgramTestInfo& shaderProgramTestInfo)
    {
        m_errorStream.clear();

        // abuse EffectDescription to load the shader content from the according shader files.
        ramses::EffectDescription effectDesc;
        if (effectDesc.setVertexShaderFromFile(shaderProgramTestInfo.vertexShaderPath.c_str()) != ramses::StatusOK)
        {
            m_errorStream << "effectDesc.setVertexShaderFromFile failed!\n";
            return false;
        }
        if (effectDesc.setFragmentShaderFromFile(shaderProgramTestInfo.fragmentShaderPath.c_str()) != ramses::StatusOK)
        {
            m_errorStream << "effectDesc.setFragmentShaderFromFile failed!\n";
            return false;
        }

        const String vertexShaderSourceCode = effectDesc.getVertexShader();
        const String fragmentShaderSourceCode = effectDesc.getFragmentShader();

        GlslEffect glslEffect(vertexShaderSourceCode, fragmentShaderSourceCode, Vector<String>(), effectDesc.impl.getSemanticsMap(), shaderProgramTestInfo.verboseShaderName);

        // create effect resource
        const EffectResource* effectResource = glslEffect.createEffectResource(ResourceCacheFlag_DoNotCache);
        if (NULL == effectResource)
        {
            m_errorStream << "Failed to parse shaders with GLSLang for shader program '" << shaderProgramTestInfo.verboseShaderName << "'\n\n";
            m_errorStream << "Error message from GLSLang:\n";
            m_errorStream << "---------------------------\n";
            m_errorStream << glslEffect.getErrorMessages();

            m_errorStream << "Vertex shader source:\n";
            m_errorStream << "---------------------\n";
            appendShaderProgramToErrorMessage(vertexShaderSourceCode, "2:");
            m_errorStream << "\n\n";
            m_errorStream << "Fragment shader source:\n";
            m_errorStream << "-----------------------\n";
            appendShaderProgramToErrorMessage(fragmentShaderSourceCode, "2:");

            return false;
        }

        if (m_renderTestBackend)
        {
            // test shader upload
            IDevice& device = m_renderTestBackend->getDevice();
            const DeviceResourceHandle shaderHandle = device.uploadShader(*effectResource);
            if (!shaderHandle.isValid())
            {
                m_errorStream << "\nFailed to upload shader program '" << shaderProgramTestInfo.verboseShaderName << "'\n\n";
                return false;
            }
            // clean up  device properly
            device.deleteShader(shaderHandle);
        }

        // create proper effect on HL API
        const ramses::Effect* hlEffect = m_ramses.impl.createEffectFromResource(effectResource, "");
        assert(NULL != hlEffect);
        // clean up HL API properly
        m_ramses.destroy(*hlEffect);

        return true;
    }

    const Char* ShaderConverterTest::getLastErrorMessage() const
    {
        return m_errorStream.c_str();
    }

    void ShaderConverterTest::appendShaderProgramToErrorMessage(const String& sourceCode, const String& prefix)
    {
        Int startIdx = 0;
        Int endIdx = -1;
        UInt lineNum = 1;
        do
        {
            endIdx = sourceCode.indexOf('\n', startIdx);
            UInt lineLength = 0;
            if (endIdx < 0)
            {
                lineLength = sourceCode.getLength() - startIdx;
            }
            else
            {
                lineLength = endIdx - startIdx;
            }
            m_errorStream << prefix << lineNum << "  " << sourceCode.substr(startIdx, lineLength + 1);
            startIdx = endIdx + 1;
            ++lineNum;
        }
        while (endIdx >= 0);

        m_errorStream << "\n";
    }

}
