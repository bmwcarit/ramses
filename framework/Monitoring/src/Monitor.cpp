//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Monitoring/Monitor.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses_internal
{
    Monitor::Monitor(const String& csvFilename)
        : m_thread("R_Monitor")
        , m_runner(csvFilename)
        , m_running(false)
    {
        if (m_runner.file.open(EFileMode_WriteNew) == EStatus_RAMSES_OK)
        {
            m_thread.start(m_runner);
        }
        else
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "Monitor::Monitor: Opening " << csvFilename << " failed");
        }
    }

    Monitor::~Monitor()
    {
        if (m_running)
        {
            m_thread.cancel();
            m_thread.join();
            m_runner.file.flush();
            m_runner.file.close();
        }
    }

    void Monitor::recordFrameInfo(const FrameInfo& frameInfo)
    {
        PlatformLightweightGuard g(m_runner.lock);
        m_runner.queue.push_back(frameInfo);
    }

    Monitor::Runner::Runner(const String& filename)
        : file(filename)
    {
    }

    void Monitor::Runner::run()
    {
        while (!isCancelRequested())
        {
            std::vector<FrameInfo> local;
            {
                PlatformLightweightGuard g(lock);
                local.swap(queue);
            }

            StringOutputStream stream;
            for (const auto& entry : local)
            {
                stream << entry.timestamp << ", "
                    << entry.framerate << ", "
                    << entry.avgDrawCalls << ", "
                    << entry.gpuMemoryUsed << "\n";
            }
            file.write(stream.c_str(), stream.length());
            file.flush();

            PlatformThread::Sleep(100);
        }
    }
}
