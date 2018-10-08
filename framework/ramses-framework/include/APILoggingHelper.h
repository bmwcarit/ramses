//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_APILOGGINGHELPER_H
#define RAMSES_APILOGGINGHELPER_H

#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    class APILoggingHelper
    {
    public:
        static String MakeLoggingString(int32_t argc, const char* argv[])
        {
            StringOutputStream argumentsStream;
            argumentsStream << argc << " arguments: [ ";
            for (int32_t i = 0; i < argc; ++i)
            {
                argumentsStream << (i != 0 ? " ; " : "") << argv[i];
            }
            argumentsStream << " ]";
            String argString = argumentsStream.c_str();
            return argString;
        }
    };
}
#endif
