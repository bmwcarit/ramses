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
#include <array>

namespace ramses_internal
{
    struct MessageEntry final
    {
        static const uint32_t MaxMessageLength = 128u;

        std::array<char, MaxMessageLength> m_text;
    };

    template <uint32_t MaxEntries, uint32_t SuccessMessageEntryID>
    class MessagePool final
    {
    public:
        MessagePool();

        uint32_t      addMessage(const char* message);
        [[nodiscard]] const char* getMessage(uint32_t id) const;
        [[nodiscard]] bool        isMessageCached(uint32_t id) const;

        static const char* getSuccessText();
        static const char* getUnknownText();

        static const uint32_t MaxMessageEntries = MaxEntries;
        static const uint32_t SuccessMessageID = SuccessMessageEntryID;

    private:
        uint32_t          m_nextIndex;
        std::vector<MessageEntry> m_messages;
    };

    template <uint32_t MaxEntries, uint32_t SuccessMessageEntryID>
    MessagePool<MaxEntries, SuccessMessageEntryID>::MessagePool()
        : m_nextIndex(SuccessMessageEntryID + 1u)
    {
        m_messages.resize(MaxEntries);
    }

    template <uint32_t MaxEntries, uint32_t SuccessMessageEntryID>
    uint32_t MessagePool<MaxEntries, SuccessMessageEntryID>::addMessage(const char* message)
    {
        const uint32_t arrayIndex = (m_nextIndex % MaxEntries);
        PlatformMemory::Set(m_messages[arrayIndex].m_text.data(), 0, MessageEntry::MaxMessageLength);
        PlatformStringUtils::Copy(m_messages[arrayIndex].m_text.data(), MessageEntry::MaxMessageLength, message);
        const uint32_t id = m_nextIndex;
        ++m_nextIndex;

        return id;
    }

    template <uint32_t MaxEntries, uint32_t SuccessMessageEntryID>
    const char* MessagePool<MaxEntries, SuccessMessageEntryID>::getMessage(uint32_t id) const
    {
        if (id == SuccessMessageEntryID)
        {
            return getSuccessText();
        }

        if (!isMessageCached(id))
        {
            return getUnknownText();
        }

        return m_messages[id % MaxEntries].m_text.data();
    }

    template <uint32_t MaxEntries, uint32_t SuccessMessageEntryID>
    bool MessagePool<MaxEntries, SuccessMessageEntryID>::isMessageCached(uint32_t id) const
    {
        const uint32_t oldestCachedIndex = (m_nextIndex < MaxEntries ? 0u : m_nextIndex - MaxEntries);
        return (id >= oldestCachedIndex && id < m_nextIndex);
    }

    template <uint32_t MaxEntries, uint32_t SuccessMessageEntryID>
    const char* MessagePool<MaxEntries, SuccessMessageEntryID>::getSuccessText()
    {
        return "OK";
    }

    template <uint32_t MaxEntries, uint32_t SuccessMessageEntryID>
    const char* MessagePool<MaxEntries, SuccessMessageEntryID>::getUnknownText()
    {
        return "Unknown";
    }
}

#endif
