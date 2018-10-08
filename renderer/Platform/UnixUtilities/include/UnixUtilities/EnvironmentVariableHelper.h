//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ENVIRONMENTVARIABLEHELPER_H
#define RAMSES_ENVIRONMENTVARIABLEHELPER_H

#include "Collections/String.h"
#include <array>

namespace ramses_internal
{
    enum EnvironmentVariableName : UInt8
    {
        XDGRuntimeDir = 0,
        WaylandSocket,
        WaylandDisplay,
        NUMBER_OF_ELEMENTS
    };


    class EnvironmentVariableHelper
    {
    public:

        EnvironmentVariableHelper();
        ~EnvironmentVariableHelper();

        void setVariable(EnvironmentVariableName variableName, const String& value);
        void unsetVariable(EnvironmentVariableName variableName);
        String getVariable(EnvironmentVariableName variableName) const;
        String getCachedVariable(EnvironmentVariableName variableName) const;

    private:
        std::array<String, EnvironmentVariableName::NUMBER_OF_ELEMENTS> m_environmentVariables;
    };
}

#endif
