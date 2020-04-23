//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommunicationChannelConsole.h"
#include "Ramsh/RamshCommunicationChannelConsoleSignalHandler.h"
#include "Ramsh/Ramsh.h"
#include "Ramsh/RamshTools.h"
#include "Utils/RamsesLogger.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/ConsoleInput.h"

namespace ramses_internal
{
    RamshCommunicationChannelConsole::RamshCommunicationChannelConsole()
        : m_pausePrompt(false)
        , m_checkInputThread("R_Ramsh_Console")
        , m_commandHistory()
        , m_nextCommandFromHistory(0)
        , m_interactiveMode(!PlatformEnvironmentVariables::HasEnvVar("DISABLE_RAMSH_INTERACTIVE_MODE"))
        , m_console(ConsoleInput::TryGetUniqueConsoleInput())
    {
        if (m_console)
        {
            // only register own signal handler when ConsoleInput might mess up settings
            RamshCommunicationChannelConsoleSignalHandler::getInstance().insert(this);
        }
        else
        {
            LOG_INFO(CONTEXT_RAMSH, "Could not open stdin: Ramsh console input disabled");
            // no stdin means no interaction possible
            m_interactiveMode = false;
        }

        if (m_interactiveMode)
        {
            // register callback to output prompt and unfinished command after each log message
            GetRamsesLogger().setAfterConsoleLogCallback([this]() { afterSendCallback(); });
            fmt::print("{}", promptString());
            std::fflush(stdout);
        }
    }

    RamshCommunicationChannelConsole::~RamshCommunicationChannelConsole()
    {
        if (m_console)
            RamshCommunicationChannelConsoleSignalHandler::getInstance().remove(this);
        GetRamsesLogger().removeAfterConsoleLogCallback();
    }

    void RamshCommunicationChannelConsole::registerRamsh(Ramsh& ramsh)
    {
        PlatformGuard g(m_lock);
        RamshCommunicationChannel::registerRamsh(ramsh);
    }

    void RamshCommunicationChannelConsole::startThread()
    {
        if (m_console)
            m_checkInputThread.start(*this);
    }

    void RamshCommunicationChannelConsole::stopThread()
    {
        m_checkInputThread.cancel();
        m_checkInputThread.join();
    }

    void RamshCommunicationChannelConsole::afterSendCallback()
    {
        if (!m_pausePrompt)
        {
            //new prompt
            fmt::print("{}", promptString());
            std::fflush(stdout);
        }
    }

    void RamshCommunicationChannelConsole::processInput(Char c)
    {
        switch (c)
        {
        case'\r':
        case'\n': // enter/return
            {
                m_lock.lock();
                if (m_interactiveMode)
                {
                    fmt::print("\n");
                }
                else
                {
                    // Print command once when in non-interactive mode, allows easier correlation
                    // between and command and reaction
                    fmt::print("{}\n", promptString());
                }
                if(nullptr != m_ramsh)
                {
                    m_pausePrompt = true;

                    RamshInput input = RamshTools::parseCommandString(m_input);
                    m_commandHistory.insert(m_commandHistory.begin(), m_input);
                    m_input.clear();

                    // Another thread calling RamsesLogger::log(), locks RamsesLogger::m_appenderLock and then
                    // RamshCommunicationChannelConsole::m_lock from afterSendCallback.
                    // So, here it is not allowed to keep the lock to m_lock, while calling execute(), because execute
                    // calls RamsesLogger::log().
                    m_lock.unlock();
                    m_ramsh->execute(input);
                    m_lock.lock();

                    m_pausePrompt = false;

                    //resize command history to 10
                    m_commandHistory.resize(std::min(static_cast<uint32_t>(m_commandHistory.size()), 10u));
                    m_nextCommandFromHistory = 0;
                }

                if (m_interactiveMode)
                {
                    //new prompt
                    fmt::print("{}", promptString());
                    std::fflush(stdout);
                }

                m_lock.unlock();
                break;
            }

        case '\b': // backspace
        case 127:
            {
                bool inputEmpty = true;
                {
                    PlatformGuard g(m_lock);
                    inputEmpty = m_input.size() == 0;

                    if(!inputEmpty)
                    {
                        m_input.resize(m_input.size()-1);
                    }
                }

                // only delete characters when there is something to delete (prevent messing up other output)
                if(m_interactiveMode && !inputEmpty)
                {
                    fmt::print("\b \b");
                    std::fflush(stdout);
                }
            }
            m_nextCommandFromHistory = 0;
            break;

        case '#': // get youngest/next oldest history command
        {
            PlatformGuard g(m_lock);
            UInt inputLength = m_input.size();
            for (UInt i = 0u; i < inputLength; i++)
            {
                fmt::print("\b \b");
                std::fflush(stdout);
            }
            m_input.clear();
            if (m_nextCommandFromHistory < m_commandHistory.size())
            {
                m_input = m_commandHistory[m_nextCommandFromHistory];
                fmt::print("{}", m_input);
                std::fflush(stdout);
                ++m_nextCommandFromHistory;
            }
            else
            {
                m_nextCommandFromHistory = 0;
            }
        }
            break;
        default:
            if (m_interactiveMode)
            {
                //echo input
                fmt::print("{}", c);
                std::fflush(stdout);
            }
            m_lock.lock();
            m_input.append(String(1,c));
            m_lock.unlock();
            m_nextCommandFromHistory = 0;
        }
    }

    String RamshCommunicationChannelConsole::promptString() const
    {
        PlatformGuard g(m_lock);
        if (m_ramsh!=nullptr)
        {
            String currentPrompt(m_ramsh->getPrompt());
            currentPrompt.append(m_input);
            return currentPrompt;
        }
        else
        {
            return String("Unknown>");
        }
    }

    void RamshCommunicationChannelConsole::run()
    {
        assert(m_console);
        while(!isCancelRequested())
        {
            //blocking read
            char c = '\0';
            if (m_console->readChar(c))
            {
                processInput(c);
            }
            else
            {
                LOG_WARN(CONTEXT_RAMSH, "Error while reading from stdin (e.g. stdin closed). No more input will be handled");
                break; //thread does not need to continue reading
            }
        }

        m_pausePrompt = true;
    }

    void RamshCommunicationChannelConsole::cancel()
    {
        Runnable::cancel();
        if (m_console)
            m_console->interruptReadChar();
    }
}
