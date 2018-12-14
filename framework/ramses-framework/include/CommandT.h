//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMANDT_H
#define RAMSES_COMMANDT_H

#define DEFINE_COMMAND_TYPE(CommandClassType, CommandTypeValue) \
        CommandClassType() : Command< typename CommandClassType::CommandTypeInfo >(CommandTypeValue) \
        { \
        } \
        static const CommandClassType::CommandTypeInfo CommandType = CommandTypeValue;

#include <assert.h>

namespace ramses_internal
{
    template< typename COMMAND_TYPE_INFO >
    struct Command
    {
        typedef COMMAND_TYPE_INFO CommandTypeInfo;

        Command(COMMAND_TYPE_INFO commandType_)
            : commandType(commandType_)
        {
        }

        virtual ~Command()
        {
        }

        template <typename COMMAND_TYPE>
        const COMMAND_TYPE& convertTo() const
        {
            assert(COMMAND_TYPE::CommandType == this->commandType);
            return static_cast<const COMMAND_TYPE&>(*this);
        }

        template <typename COMMAND_TYPE>
        COMMAND_TYPE& convertTo()
        {
            assert(COMMAND_TYPE::CommandType == this->commandType);
            return static_cast<COMMAND_TYPE&>(*this);
        }

        const COMMAND_TYPE_INFO commandType;
    };
}

#endif // RAMSES_COMMANDT_H
