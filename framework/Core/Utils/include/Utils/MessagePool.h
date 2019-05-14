//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MESSAGEPOOL_H
#define RAMSES_MESSAGEPOOL_H

#include "Collections/Vector.h"
#include "PlatformAbstraction/PlatformStringUtils.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
{
    struct MessageEntry
    {
        static const UInt32 MaxMessageLength = 128u;

        Char m_text[MaxMessageLength];
    };

    template <UInt32 MaxEntries, UInt32 SuccessMessageEntryID>
    class MessagePool
    {
    public:
        MessagePool();

        UInt32      addMessage(const Char* message);
        const Char* getMessage(UInt32 id) const;
        Bool        isMessageCached(UInt32 id) const;

        static const Char* getSuccessText();
        static const Char* getUnknownText();

        static const UInt32 MaxMessageEntries = MaxEntries;
        static const UInt32 SuccessMessageID = SuccessMessageEntryID;

    private:
        UInt32          m_nextIndex;
        std::vector<MessageEntry> m_messages;
    };

    template <UInt32 MaxEntries, UInt32 SuccessMessageEntryID>
    MessagePool<MaxEntries, SuccessMessageEntryID>::MessagePool()
        : m_nextIndex(SuccessMessageEntryID + 1u)
    {
        m_messages.resize(MaxEntries);
    }

    template <UInt32 MaxEntries, UInt32 SuccessMessageEntryID>
    UInt32 MessagePool<MaxEntries, SuccessMessageEntryID>::addMessage(const Char* message)
    {
        const UInt32 arrayIndex = (m_nextIndex % MaxEntries);
        PlatformMemory::Set(m_messages[arrayIndex].m_text, 0, MessageEntry::MaxMessageLength);
        PlatformStringUtils::Copy(m_messages[arrayIndex].m_text, MessageEntry::MaxMessageLength, message);
        const UInt32 id = m_nextIndex;
        ++m_nextIndex;

        return id;
    }

    template <UInt32 MaxEntries, UInt32 SuccessMessageEntryID>
    const Char* MessagePool<MaxEntries, SuccessMessageEntryID>::getMessage(UInt32 id) const
    {
        if (id == SuccessMessageEntryID)
        {
            return getSuccessText();
        }

        if (!isMessageCached(id))
        {
            return getUnknownText();
        }

        return m_messages[id % MaxEntries].m_text;
    }

    template <UInt32 MaxEntries, UInt32 SuccessMessageEntryID>
    Bool MessagePool<MaxEntries, SuccessMessageEntryID>::isMessageCached(UInt32 id) const
    {
        const UInt32 oldestCachedIndex = (m_nextIndex < MaxEntries ? 0u : m_nextIndex - MaxEntries);
        return (id >= oldestCachedIndex && id < m_nextIndex);
    }

    template <UInt32 MaxEntries, UInt32 SuccessMessageEntryID>
    const Char* MessagePool<MaxEntries, SuccessMessageEntryID>::getSuccessText()
    {
        return "OK";
    }

    template <UInt32 MaxEntries, UInt32 SuccessMessageEntryID>
    const Char* MessagePool<MaxEntries, SuccessMessageEntryID>::getUnknownText()
    {
        return "Unknown";
    }
}

#endif
