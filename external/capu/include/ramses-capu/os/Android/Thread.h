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

#ifndef RAMSES_CAPU_ANDROID_THREAD_H
#define RAMSES_CAPU_ANDROID_THREAD_H

#include <ramses-capu/os/Posix/Thread.h>

namespace ramses_capu
{
    namespace os
    {
        class Thread: private ramses_capu::posix::Thread
        {

        public:
            Thread(const std::string& name);
            using ramses_capu::posix::Thread::start;
            using ramses_capu::posix::Thread::join;
            using ramses_capu::posix::Thread::cancel;
            using ramses_capu::posix::Thread::resetCancel;
            using ramses_capu::posix::Thread::getState;
            using ramses_capu::posix::Thread::Sleep;
            using ramses_capu::posix::Thread::getName;
            static uint_t CurrentThreadId();
        };

        inline
        Thread::Thread(const std::string& name)
            : ramses_capu::posix::Thread(name)
        {
        }

        inline
        uint_t
        Thread::CurrentThreadId()
        {
            return pthread_self();
        }
    }
}

#endif // RAMSES_CAPU_ANDROID_THREAD_H
