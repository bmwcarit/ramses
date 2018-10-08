//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADERCONVERTERTEST_H
#define RAMSES_SHADERCONVERTERTEST_H

#include "ramses-client.h"
#include "Collections/StringOutputStream.h"
#include "RendererAPI/IWindowEventHandler.h"


namespace ramses_internal
{
    class String;
    class IPlatformFactory;
    class IRenderBackend;
    class DisplayConfig;
    struct ShaderProgramTestInfo;

    class ShaderConverterTest
    {
    public:
        ShaderConverterTest(DisplayConfig& displayConfig, Bool noGpuUpload);
        ~ShaderConverterTest();

        Bool testShader(const ShaderProgramTestInfo& shaderProgramTestInfo);
        const Char* getLastErrorMessage() const;
        Bool hasRenderBackend() const;

    private:
        void initializeRenderBackend();
        void appendShaderProgramToErrorMessage(const String& sourceCode, const String& prefix);

        ramses::RamsesFramework m_framework;
        ramses::RamsesClient    m_ramses;
        DisplayConfig&          m_displayConfig;
        IPlatformFactory*       m_platformFactory;
        IRenderBackend*         m_renderTestBackend;

        StringOutputStream      m_errorStream;

        class WindowEventHandlerDummy : public IWindowEventHandler
        {
        public:
            virtual void onKeyEvent(EKeyEventType, UInt32, EKeyCode) override {}
            virtual void onMouseEvent(EMouseEventType, Int32, Int32) override {}
            virtual void onClose() override {}
        };
        WindowEventHandlerDummy m_windowEventHandlerDummy;
    };
}

#endif // RAMSES_SHADERCONVERTER_TEST_H
