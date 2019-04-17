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

#ifndef RAMSES_CAPU_UNIXBASED_THREAD_H
#define RAMSES_CAPU_UNIXBASED_THREAD_H

#include <pthread.h>
#include <unistd.h>
#include "ramses-capu/os/Generic/Thread.h"

namespace ramses_capu
{
    namespace posix
    {
        class Thread : private generic::Thread
        {
        public:
            Thread(const std::string& name);
            ~Thread();
            status_t start(Runnable& runnable);
            status_t join();
            static status_t Sleep(uint32_t millis);
            using ramses_capu::generic::Thread::cancel;
            using ramses_capu::generic::Thread::resetCancel;
            using ramses_capu::generic::Thread::getState;
            using ramses_capu::generic::Thread::getName;

        protected:
            pthread_t mThread;
            pthread_attr_t mAttr;

            static void* run(void* arg);
        };


        inline
        Thread::Thread(const std::string& name)
            : generic::Thread(name)
            , mThread(0)
        {
            pthread_attr_init(&mAttr);
            pthread_attr_setdetachstate(&mAttr, PTHREAD_CREATE_JOINABLE);
        }

        inline
        Thread::~Thread()
        {
            join();
            pthread_attr_destroy(&mAttr);
        }

        inline
        void*
        Thread::run(void* arg)
        {
            generic::ThreadRunnable* tr = static_cast<generic::ThreadRunnable*>(arg);
#ifdef OS_MACOSX
// OSX can only set name from within running thread
            pthread_setname_np(tr->thread->getName());
#elif !defined(OS_INTEGRITY)
            pthread_setname_np(pthread_self(), tr->thread->getName());
#endif
            tr->thread->setState(TS_RUNNING);
            if (tr->runnable != NULL)
            {
                tr->runnable->run();
            }
            tr->thread->setState(TS_TERMINATED);
            pthread_exit(NULL);
            return NULL;
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
            int32_t result = pthread_create(&mThread, &mAttr, Thread::run, &mRunnable);
            if (result != 0)
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
            if (pthread_join(mThread, NULL) == 0)
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
            if (usleep(millis * 1000) == 0)
            {
                return CAPU_OK;
            }
            return CAPU_ERROR;
        }
    }
}
#endif // RAMSES_CAPU_UNIXBASED_THREAD_H
