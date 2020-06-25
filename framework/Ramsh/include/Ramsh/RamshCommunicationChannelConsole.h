//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMUNICATIONCHANNELCONSOLE_H
#define RAMSES_RAMSHCOMMUNICATIONCHANNELCONSOLE_H

#include "Collections/String.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformLock.h"
#include <vector>
#include <memory>

namespace ramses_internal
{
    class ConsoleInput;
    class Ramsh;

    class RamshCommunicationChannelConsole : public Runnable
    {
    public:
        static std::unique_ptr<RamshCommunicationChannelConsole> Construct(Ramsh& ramsh, const String& prompt, bool startThread = true);
        virtual ~RamshCommunicationChannelConsole();

        // TODO(tobias) called from RamshCommunicationChannelConsoleSignalHandler from signal handler context
        void stopThread();

        // for testing only
        void processInput(Char c);

    private:
        RamshCommunicationChannelConsole(Ramsh& ramsh, const String& prompt, std::unique_ptr<ConsoleInput> consoleInput, bool startThread);

        virtual void cancel() override;
        void run() override;
        void afterSendCallback();

        Ramsh& m_ramsh;
        String m_prompt;
        mutable PlatformLock m_lock;
        String promptString() const;

        String m_input;
        std::atomic<bool> m_pausePrompt;
        PlatformThread m_checkInputThread;

        std::vector<String> m_commandHistory;
        uint32_t m_nextCommandFromHistory;

        bool m_interactiveMode;
        std::unique_ptr<ConsoleInput> m_console;
    };
}

#endif
