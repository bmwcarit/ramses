//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMANDCONTAINER_H
#define RAMSES_COMMANDCONTAINER_H

#include "Collections/Vector.h"
#include <cstdint>
#include <cassert>

namespace ramses_internal
{

    template< typename COMMAND_TYPE_INFO, typename COMMAND_BASE_TYPE >
    class CommandContainer
    {
    public:
        CommandContainer()
        {
        }

        ~CommandContainer()
        {
            clear();
        }

        template <typename COMMAND_TYPE>
        void addCommand(COMMAND_TYPE_INFO commandType, const COMMAND_TYPE& commandData)
        {
            Command cmd;
            cmd.m_commandType = commandType;
            cmd.m_commandData = new COMMAND_TYPE(commandData);
            m_commands.push_back(cmd);
        }

        template <typename COMMAND_TYPE>
        void addCommand(COMMAND_TYPE_INFO commandType, COMMAND_TYPE&& commandData)
        {
            Command cmd;
            cmd.m_commandType = commandType;
            cmd.m_commandData = new typename std::remove_reference<COMMAND_TYPE>::type(std::forward<COMMAND_TYPE>(commandData));
            m_commands.push_back(cmd);
        }

        template <typename COMMAND_TYPE>
        const COMMAND_TYPE& getCommandData(uint32_t index) const
        {
            assert(index < m_commands.size());
            assert(COMMAND_TYPE::CommandType == m_commands[index].m_commandData->commandType);
            return m_commands[index].m_commandData->template convertTo<COMMAND_TYPE>();
        }

        template <typename COMMAND_TYPE>
        COMMAND_TYPE& getCommandData(uint32_t index)
        {
            assert(index < m_commands.size());
            assert(COMMAND_TYPE::CommandType == m_commands[index].m_commandData->commandType);
            return m_commands[index].m_commandData->template convertTo<COMMAND_TYPE>();
        }

        COMMAND_TYPE_INFO getCommandType(uint32_t index) const
        {
            assert(index < m_commands.size());
            return m_commands[index].m_commandType;
        }

        uint32_t getTotalCommandCount() const
        {
            return static_cast<uint32_t>(m_commands.size());
        }

        void clear()
        {
            for(const auto& cmd : m_commands)
            {
                delete cmd.m_commandData;
            }
            m_commands.clear();
        }

        void swap(CommandContainer& commandContainer)
        {
            m_commands.swap(commandContainer.m_commands);
        }

    private:
        CommandContainer(const CommandContainer&);
        CommandContainer operator=(const CommandContainer&);

        struct Command
        {
            COMMAND_TYPE_INFO m_commandType;
            COMMAND_BASE_TYPE* m_commandData;
        };

        std::vector<Command> m_commands;
    };
}

#endif
