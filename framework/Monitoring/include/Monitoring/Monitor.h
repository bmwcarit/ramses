//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MONITOR_H
#define RAMSES_MONITOR_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "Collections/String.h"
#include "Collections/Vector.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/File.h"

namespace ramses_internal
{
    struct FrameInfo
    {
        UInt64 timestamp;
        Float framerate;
        UInt32 avgDrawCalls;
        UInt32 gpuMemoryUsed;
    };

    class Monitor
    {
    public:
        Monitor(const String& csvFilename);
        ~Monitor();

        void recordFrameInfo(const FrameInfo& frameInfo);

    private:

        struct Runner : public Runnable
        {
            Runner(const String& filename);
            void run() override;

            File file;
            PlatformLightweightLock lock;
            std::vector<FrameInfo> queue;
        };

        PlatformThread m_thread;
        Runner m_runner;
        bool m_running;
    };
}

#endif
