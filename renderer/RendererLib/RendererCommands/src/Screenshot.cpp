//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/Screenshot.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    Screenshot::Screenshot(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "save a screenshot";
        registerKeyword("p");
        registerKeyword("screenshot");
    }

    Bool Screenshot::executeInput(const RamshInput& input)
    {
        enum EOption
        {
            EOption_None = 0,
            EOption_Filename,
            EOption_Display
        };

        EOption lastOption = EOption_None;

        String              filename = "unnamed.png";
        DisplayHandle       display = DisplayHandle(0);
        Bool                sendViaDLT = false;

        const UInt32 numArgs = static_cast<UInt32>(input.size());
        for (UInt argStrIdx = 0u; argStrIdx < numArgs; ++argStrIdx)
        {
            const String argStr(input[argStrIdx]);

            if (argStr == String("-filename"))
            {
                lastOption = EOption_Filename;
            }
            else if (argStr == String("-displayId"))
            {
                lastOption = EOption_Display;
            }
            else if (argStr == String("-sendViaDLT"))
            {
                sendViaDLT = true;
            }
            else
            {
                switch( lastOption )
                {
                case EOption_Display:
                    display    = DisplayHandle(atoi(argStr.c_str()));
                    lastOption = EOption_None;
                    break;
                case EOption_Filename:
                    filename   = argStr;
                    lastOption = EOption_None;
                    break;

                case EOption_None:
                    if( contains_c(m_keywords, argStr) ) // check whether a keyword is the current argument
                    {
                        continue;
                    }
                    return false;

                default:
                    return false;
                }
            }
        }

        m_rendererCommandBuffer.readPixels(display, filename, true, 0u, 0u, 0u, 0u, sendViaDLT);

        return true;
    }
}
