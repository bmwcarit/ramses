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

#ifndef RAMSES_CAPU_GENERIC_THREAD_H
#define RAMSES_CAPU_GENERIC_THREAD_H

#include "ramses-capu/util/Runnable.h"
#include "ramses-capu/os/ThreadState.h"
#include <atomic>
#include <string>

namespace ramses_capu
{
    namespace generic
    {

        class Thread;
        class ThreadRunnable
        {
        public:
            ThreadRunnable();

            Thread* thread;
            Runnable* runnable;
        };

        class Thread
        {
        public:
            Thread(const std::string& name);
            void cancel();
            void resetCancel();
            ThreadState getState() const;
            void setState(ThreadState state);
            const char* getName();
        private:
            std::atomic<uint32_t> mState;
        protected:
            ThreadRunnable mRunnable;
            bool mIsStarted;
            std::string mName;
        };


        inline
        ThreadRunnable::ThreadRunnable()
            : thread(NULL)
            , runnable(NULL)
        {
        }

        inline
        Thread::Thread(const std::string& name)
            : mState(static_cast<uint32_t>(TS_NEW))
            , mIsStarted(false)
            , mName(name)
        {
            mRunnable.thread = this;
        }

        inline
        ThreadState
        Thread::getState() const
        {
            return static_cast<ThreadState>(mState.load());
        }

        inline
        void
        Thread::setState(ThreadState state)
        {
            mState = static_cast<uint32_t>(state);
        }

        inline
        const char* Thread::getName()
        {
            return mName.c_str();
        }

        inline
        void
        Thread::cancel()
        {
            if (mRunnable.runnable)
            {
                mRunnable.runnable->cancel();
            }
        }

        inline
        void
        Thread::resetCancel()
        {
            if (mRunnable.runnable)
            {
                mRunnable.runnable->resetCancel();
            }
        }
    }
}

#endif //RAMSES_CAPU_GENERIC_THREAD_H
