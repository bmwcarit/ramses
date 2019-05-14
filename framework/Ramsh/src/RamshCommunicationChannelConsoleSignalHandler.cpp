//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommunicationChannelConsoleSignalHandler.h"

#include "Ramsh/RamshCommunicationChannelConsole.h"
#include "PlatformAbstraction/PlatformSignal.h"

namespace ramses_internal
{
    static void handleSignalCallback(int sig)
    {
        RamshCommunicationChannelConsoleSignalHandler::getInstance().handleSignal(sig);
    }

    RamshCommunicationChannelConsoleSignalHandler::RamshCommunicationChannelConsoleSignalHandler()
    {
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGABRT, handleSignalCallback, true);
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGFPE, handleSignalCallback, true);
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGINT, handleSignalCallback, true);
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGTERM, handleSignalCallback, true);
    }

    RamshCommunicationChannelConsoleSignalHandler& RamshCommunicationChannelConsoleSignalHandler::getInstance()
    {
        static RamshCommunicationChannelConsoleSignalHandler handler;
        return handler;
    }

    void RamshCommunicationChannelConsoleSignalHandler::insert(RamshCommunicationChannelConsole* console)
    {
        m_consoles.push_back(console);
    }

    void RamshCommunicationChannelConsoleSignalHandler::remove(RamshCommunicationChannelConsole* console)
    {
        std::vector<RamshCommunicationChannelConsole*>::iterator i = find_c(m_consoles, console);
        if (i != m_consoles.end())
        {
            m_consoles.erase(i);
        }
    }

    void RamshCommunicationChannelConsoleSignalHandler::handleSignal(int sig)
    {
        ESignal enumSignal = static_cast<ESignal>(sig);
        String signal = Signal::SignalToString(enumSignal);

        LOG_WARN(CONTEXT_RAMSH, "Received signal " + signal);
        for (auto c : m_consoles)
        {
            c->stopThread();
        }
    }
}
