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
#include "PlatformAbstraction/PlatformGuard.h"
#include "Utils/RamsesLogger.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"

#include "ramses-capu/os/Console.h"

namespace ramses_internal
{
    RamshCommunicationChannelConsole::RamshCommunicationChannelConsole()
        : m_pausePrompt(false)
        , m_checkInputThread("R_Ramsh_Console")
        , m_commandHistory()
        , m_nextCommandFromHistory(0)
        , m_interactiveMode(!PlatformEnvironmentVariables::HasEnvVar("DISABLE_RAMSH_INTERACTIVE_MODE"))
    {
        RamshCommunicationChannelConsoleSignalHandler::getInstance().insert(this);
        if (m_interactiveMode)
        {
            // register callback to output prompt and unfinished command after each log message
            GetRamsesLogger().setAfterConsoleLogCallback([this]() { afterSendCallback(); });
            ramses_capu::Console::Print("%s", promptString().c_str());
            ramses_capu::Console::Flush();
        }
    }

    RamshCommunicationChannelConsole::~RamshCommunicationChannelConsole()
    {
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
            ramses_capu::Console::Print("%s", promptString().c_str());
            ramses_capu::Console::Flush();
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
                    ramses_capu::Console::Print("\n");
                }
                else
                {
                    // Print command once when in non-interactive mode, allows easier correlation
                    // between and command and reaction
                    ramses_capu::Console::Print("%s\n", promptString().c_str());
                }
                if(0 != m_ramsh)
                {
                    m_pausePrompt = true;

                    RamshInput input = RamshTools::parseCommandString(m_input);
                    m_commandHistory.insert(m_commandHistory.begin(), m_input);
                    m_input.truncate(0);

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
                    ramses_capu::Console::Print("%s", promptString().c_str());
                    ramses_capu::Console::Flush();
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
                    inputEmpty = m_input.getLength() == 0;

                    if(!inputEmpty)
                    {
                        m_input.truncate(m_input.getLength()-1);
                    }
                }

                // only delete characters when there is something to delete (prevent messing up other output)
                if(m_interactiveMode && !inputEmpty)
                {
                    ramses_capu::Console::Print("\b \b");
                    ramses_capu::Console::Flush();
                }
            }
            m_nextCommandFromHistory = 0;
            break;

        case '#': // get youngest/next oldest history command
        {
            PlatformGuard g(m_lock);
            UInt inputLength = m_input.getLength();
            for (UInt i = 0u; i < inputLength; i++)
            {
                ramses_capu::Console::Print("\b \b");
                ramses_capu::Console::Flush();
            }
            m_input.truncate(0);
            if (m_nextCommandFromHistory < m_commandHistory.size())
            {
                m_input = m_commandHistory[m_nextCommandFromHistory];
                ramses_capu::Console::Print("%s", m_input.c_str());
                ramses_capu::Console::Flush();
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
                ramses_capu::Console::Print("%c",c);
                ramses_capu::Console::Flush();
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
        if (m_ramsh!=0)
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
        while(!isCancelRequested())
        {
            //blocking read
            char c = '\0';
            ramses_capu::status_t status = ramses_capu::Console::ReadChar(c);
            if (status == ramses_capu::CAPU_OK)
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
        ramses_capu::Console::InterruptReadChar();
    }
}
