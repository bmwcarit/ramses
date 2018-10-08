//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "UnixUtilities/EnvironmentVariableHelper.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    namespace
    {
        const char* EnvironmentVariableNames[] =
        {
            "XDG_RUNTIME_DIR",
            "WAYLAND_SOCKET",
            "WAYLAND_DISPLAY"
        };

        ENUM_TO_STRING(EnvironmentVariableName, EnvironmentVariableNames, EnvironmentVariableName::NUMBER_OF_ELEMENTS);


        void setEnvironmentVariable(const String& name, const String& value)
        {
            if(value.getLength() > 0)
            {
                PlatformEnvironmentVariables::SetEnvVar(name, value);
            }
            else
            {
                PlatformEnvironmentVariables::UnsetEnvVar(name);
            }
        }

    }

    EnvironmentVariableHelper::EnvironmentVariableHelper()
    {
        for (UInt8 i = 0; i < EnvironmentVariableName::NUMBER_OF_ELEMENTS; ++i)
        {
            const EnvironmentVariableName variableName = static_cast<EnvironmentVariableName>(i);

            m_environmentVariables[i] = getVariable(variableName);
        }
    }

    EnvironmentVariableHelper::~EnvironmentVariableHelper()
    {
        for (UInt8 i = 0; i < EnvironmentVariableName::NUMBER_OF_ELEMENTS; ++i)
        {
            setVariable( static_cast<EnvironmentVariableName>(i), m_environmentVariables[i]);
        }
    }

    void EnvironmentVariableHelper::setVariable(EnvironmentVariableName variableName, const String& value)
    {
        setEnvironmentVariable( EnumToString(variableName), value);
    }

    void EnvironmentVariableHelper::unsetVariable(EnvironmentVariableName variableName)
    {
        setEnvironmentVariable( EnumToString(variableName), "");
    }

    String EnvironmentVariableHelper::getVariable(EnvironmentVariableName variableName) const
    {
        String variable;
        PlatformEnvironmentVariables::get(EnumToString(variableName), variable);
        return variable;
    }

    String EnvironmentVariableHelper::getCachedVariable(EnvironmentVariableName variableName) const
    {
        return m_environmentVariables[variableName];
    }
}
