//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMUNICATIONCHANNELCONSOLE_H
#define RAMSES_RAMSHCOMMUNICATIONCHANNELCONSOLE_H

#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "PlatformAbstraction/PlatformTypes.h"

#include <vector>
#include <memory>
#include <string>

namespace ramses_internal
{
    class ConsoleInput;
    class Ramsh;

    class RamshCommunicationChannelConsole : public Runnable
    {
    public:
        static std::unique_ptr<RamshCommunicationChannelConsole> Construct(Ramsh& ramsh, const std::string& prompt, bool startThread = true);
        ~RamshCommunicationChannelConsole() override;

        // TODO(tobias) called from RamshCommunicationChannelConsoleSignalHandler from signal handler context
        void stopThread();

        // for testing only
        void processInput(char c);

    private:
        RamshCommunicationChannelConsole(Ramsh& ramsh, const std::string& prompt, std::unique_ptr<ConsoleInput> consoleInput, bool startThread);

        void cancel() override;
        void run() override;
        void afterSendCallback();

        Ramsh& m_ramsh;
        std::string m_prompt;
        mutable PlatformLock m_lock;
        std::string promptString() const;

        std::string m_input;
        std::atomic<bool> m_pausePrompt;
        PlatformThread m_checkInputThread;

        std::vector<std::string> m_commandHistory;
        uint32_t m_nextCommandFromHistory;

        bool m_interactiveMode;
        std::unique_ptr<ConsoleInput> m_console;
    };
}

#endif
