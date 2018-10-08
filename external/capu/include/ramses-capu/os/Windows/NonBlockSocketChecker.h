/*
* Copyright (C) 2014 BMW Car IT GmbH
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

#ifndef RAMSES_CAPU_WINDOWS_NONBLOCKSOCKETCHECKER_H
#define RAMSES_CAPU_WINDOWS_NONBLOCKSOCKETCHECKER_H

#include "ramses-capu/os/Generic/NonBlockSocketChecker.h"

namespace ramses_capu
{
    namespace os
    {
        class NonBlockSocketChecker : private ramses_capu::generic::NonBlockSocketChecker
        {
        public:
            using ramses_capu::generic::NonBlockSocketChecker::CheckSocketsForIncomingData;
        };
    }
}

#endif // RAMSES_CAPU_WINDOWS_NONBLOCKSOCKETCHECKER_H
