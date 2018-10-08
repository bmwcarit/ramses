/*
 * Copyright (C) 2016 BMW Car IT GmbH
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

#include "ramses-capu/Error.h"

namespace ramses_capu
{
    const char* StatusConversion::m_statusTexts[] =
    {
        "OK",
        "Unimplemented",
        "Out of range",
        "Input value",
        "Error",
        "Socket bind error",
        "Socket error",
        "Socket connections error",
        "Socket listen error",
        "Socket closing error",
        "Socket address error",
        "No memory",
        "Timeout",
        "Does not exist",
        "Is not supported",
        "IO error",
        "EOF",
        "INTERRUPTED"
    };

    const char* StatusConversion::GetStatusText(status_t status)
    {
        return StatusConversion::m_statusTexts[status];
    }
}
