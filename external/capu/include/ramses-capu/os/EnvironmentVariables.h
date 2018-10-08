/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_ENVIRONMENTVARIABLES_H
#define RAMSES_CAPU_ENVIRONMENTVARIABLES_H

#include "ramses-capu/os/StringUtils.h"
#include "ramses-capu/container/String.h"
#include "ramses-capu/container/Pair.h"
#include "ramses-capu/container/HashTable.h"
#include <ramses-capu/os/PlatformInclude.h>

#include RAMSES_CAPU_PLATFORM_INCLUDE(EnvironmentVariables)

namespace ramses_capu
{
    /**
     * Class for accessing environment variables of the operating system.
     */
    class EnvironmentVariables : private ramses_capu::os::EnvironmentVariables
    {
    public:
        /**
         * Obtain the value of an environment variable
         * @param key the key of the environment variable to query
         * @param value Reference to string which will contain the requested value
         * @return CAPU_OK if variable was successfully queried
         */
        static bool get(const String& key, String& value);
    };

    inline bool EnvironmentVariables::get(const String& key, String& value)
    {
        return ramses_capu::os::EnvironmentVariables::get(key, value);
    }
}

#endif // RAMSES_CAPU_ENVIRONMENTVARIABLES_H
