//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMENVIRONMENTVARIABLES_H
#define RAMSES_PLATFORMENVIRONMENTVARIABLES_H

#include "Collections/String.h"
#include <stdlib.h>

namespace ramses_internal
{
    class PlatformEnvironmentVariables
    {
    public:
        static Bool get(const String& key, String& value);
        static bool HasEnvVar(const String& key);

        static void SetEnvVar(const String& key, const String& value);
        static inline void UnsetEnvVar(const String& key);
    };

    inline
    Bool PlatformEnvironmentVariables::get(const String& key, String& value)
    {
#ifdef _MSC_VER
        char* envValue = 0;
        errno_t err = _dupenv_s(&envValue, 0, key.c_str());
        bool found = (err == 0 && envValue != 0);
        value = envValue;
        free(envValue);
        return found;
#else
        char * env = getenv(key.c_str());
        value = env;
        return (0 != env);
#endif
    }

    inline bool PlatformEnvironmentVariables::HasEnvVar(const String& key)
    {
        return getenv(key.c_str()) != nullptr;
    }

    inline
    void PlatformEnvironmentVariables::SetEnvVar(const String& key, const String& value)
    {
#ifdef _WIN32
        _putenv_s(key.c_str(), value.c_str());
#else
        setenv(key.c_str(), value.c_str(), 1);
#endif
    }

    inline
    void PlatformEnvironmentVariables::UnsetEnvVar(const String& key)
    {
#ifdef _WIN32
        _putenv_s(key.c_str(), "");
#else
        unsetenv(key.c_str());
#endif
    }
}

#endif
