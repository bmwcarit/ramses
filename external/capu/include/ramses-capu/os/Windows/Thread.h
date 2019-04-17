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

#ifndef RAMSES_CAPU_WINDOWS_THREAD_H
#define RAMSES_CAPU_WINDOWS_THREAD_H

#include "ramses-capu/os/Windows/MinimalWindowsH.h"
#include "ramses-capu/os/Generic/Thread.h"
#include "ramses-capu/os/Thread.h"
#include "ramses-capu/Error.h"

namespace ramses_capu
{
    namespace os
    {
        class Thread : private generic::Thread
        {
        public:
            Thread(const std::string& name);
            ~Thread();
            status_t start(Runnable& runnable);
            status_t join();
            using ramses_capu::generic::Thread::cancel;
            using ramses_capu::generic::Thread::resetCancel;
            using ramses_capu::generic::Thread::getState;
            using ramses_capu::generic::Thread::getName;
            static status_t Sleep(uint32_t millis);
            static uint_t CurrentThreadId();
        private:

            DWORD  mThreadId;
            HANDLE mThreadHandle;
            static DWORD WINAPI run(LPVOID arg);
        };

        inline
        DWORD WINAPI
        Thread::run(LPVOID arg)
        {
            generic::ThreadRunnable* tr = reinterpret_cast<generic::ThreadRunnable*>(arg);
            tr->thread->setState(TS_RUNNING);
            if (tr->runnable != NULL)
            {
                tr->runnable->run();
            }
            tr->thread->setState(TS_TERMINATED);
            return 0;
        }

        inline
        Thread::Thread(const std::string& name)
            : generic::Thread(name)
            , mThreadHandle(0)
        {
        }

        inline
        Thread::~Thread()
        {
            join();
        }

        inline
        status_t
        Thread::start(Runnable& runnable)
        {
            if (mIsStarted)
            {
                // thread must have not been started or be joined before it can be started again
                return CAPU_ERROR;
            }

            mRunnable.runnable = &runnable;
            mRunnable.thread->setState(TS_STARTING);
            mIsStarted = true;
            mThreadHandle = CreateThread(NULL, 0, Thread::run, &mRunnable, 0, &mThreadId);
            if (mThreadHandle == NULL)
            {
                mRunnable.thread->setState(TS_NEW);
                mIsStarted = false;
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        Thread::join()
        {
            if (!mIsStarted)
            {
                return CAPU_ERROR;
            }
            if (WaitForSingleObject(mThreadHandle, INFINITE) == 0)
            {
                mIsStarted = false;
                return CAPU_OK;
            }

            return CAPU_ERROR;
        }

        inline
        status_t
        Thread::Sleep(uint32_t millis)
        {
            ::Sleep(millis);
            return CAPU_OK;
        }

        inline
        uint_t
        Thread::CurrentThreadId()
        {
            return GetCurrentThreadId();
        }
    }
}

#endif //RAMSES_CAPU_WINDOWS_THREAD_H
