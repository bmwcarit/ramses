//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMUNICATIONCHANNELCONSOLE_H
#define RAMSES_RAMSHCOMMUNICATIONCHANNELCONSOLE_H

#include "Ramsh/RamshCommunicationChannel.h"
#include "Collections/StringOutputStream.h"
#include "RamshInput.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformLock.h"

namespace ramses_internal
{

class RamshCommunicationChannelConsole : public RamshCommunicationChannel, public Runnable
{
    public:
        RamshCommunicationChannelConsole();
        ~RamshCommunicationChannelConsole();
        void startThread();

        virtual void registerRamsh(Ramsh& ramsh) override;

        void processInput(Char c);
        void stopThread();
        virtual void cancel() override;

    private:
        void run() override;
        void afterSendCallback();

        mutable PlatformLock m_lock;
        String promptString() const;
        String m_input;
        std::atomic<bool> m_pausePrompt;
        PlatformThread m_checkInputThread;

        std::vector<String> m_commandHistory;
        uint32_t m_nextCommandFromHistory;

        const bool m_interactiveMode;
};

}// namespace ramses_internal

#endif
