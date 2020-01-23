//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMANDLINEARGUMENT_H
#define RAMSES_COMMANDLINEARGUMENT_H

#include "Collections/String.h"

namespace ramses_internal
{
    class CommandLineArgument
    {
    public:
        CommandLineArgument()
            : m_hasBeenUsed(false)
        {
        }

        void setValue(const String& value)
        {
            m_value = value;
        }

        void setName(const String& name)
        {
            m_name = name;
        }

        const String& getName() const
        {
            return m_name;
        }

        void setUsed()
        {
            m_hasBeenUsed = true;
        }

        bool hasBeenUsed() const
        {
            return m_hasBeenUsed;
        }

        bool hasValue() const
        {
            return 0 != m_value.size();
        }

        const String& getValue() const
        {
            return m_value;
        }

    private:
        String m_name;
        String m_value;
        bool m_hasBeenUsed;
    };
}

#endif
