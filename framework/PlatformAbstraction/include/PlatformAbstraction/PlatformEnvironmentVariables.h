//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMENVIRONMENTVARIABLES_H
#define RAMSES_PLATFORMENVIRONMENTVARIABLES_H

#include <string>
#include <cstdlib>

namespace ramses_internal
{
    namespace PlatformEnvironmentVariables
    {
        inline
        bool get(const std::string& key, std::string& value)
        {
#ifdef _MSC_VER
            char* envValue = nullptr;
            errno_t err = _dupenv_s(&envValue, 0, key.c_str());
            bool found = (err == 0 && envValue != 0);
            if (found)
            {
                value = envValue;
            }
            free(envValue);
            return found;
#else
            char * env = getenv(key.c_str());
            if (nullptr != env)
            {
                value = env;
            }
            return (nullptr != env);
#endif
        }

        inline bool HasEnvVar(const std::string& key)
        {
            return getenv(key.c_str()) != nullptr;
        }

        inline
        void SetEnvVar(const std::string& key, const std::string& value)
        {
#ifdef _WIN32
            _putenv_s(key.c_str(), value.c_str());
#else
            setenv(key.c_str(), value.c_str(), 1);
#endif
        }

        inline
        void UnsetEnvVar(const std::string& key)
        {
#ifdef _WIN32
            _putenv_s(key.c_str(), "");
#else
            unsetenv(key.c_str());
#endif
        }
    };
}

#endif
