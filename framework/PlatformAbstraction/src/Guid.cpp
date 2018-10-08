//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/Guid.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include <random>

namespace ramses_internal
{
    void Guid::createNew()
    {
        static std::mt19937 gen(std::random_device{}());
        static PlatformLightweightLock mutex;

        PlatformLightweightGuard g(mutex);

        std::uniform_int_distribution<uint32_t> dis32;
        std::uniform_int_distribution<uint16_t> dis16;
        // Note: std::uniform_int_distribution<uint8_t> is not allowed

        m_data.Data1 = dis32(gen);
        m_data.Data2 = dis16(gen);
        m_data.Data3 = 0x4000 | (dis16(gen) & 0x00FF); // a guid V4 starts with 0b0100 in data3
        m_data.Data4[0] = static_cast<uint8_t>(dis16(gen) & 0xFF);
        m_data.Data4[1] = static_cast<uint8_t>(dis16(gen) & 0xFF);
        m_data.Data4[2] = static_cast<uint8_t>(dis16(gen) & 0xFF);
        m_data.Data4[3] = static_cast<uint8_t>(dis16(gen) & 0xFF);
        m_data.Data4[4] = static_cast<uint8_t>(dis16(gen) & 0xFF);
        m_data.Data4[5] = static_cast<uint8_t>(dis16(gen) & 0xFF);
        m_data.Data4[6] = static_cast<uint8_t>(dis16(gen) & 0xFF);
        m_data.Data4[7] = static_cast<uint8_t>(dis16(gen) & 0xFF);
    }

    void Guid::createFromString(const char* guid, size_t len)
    {
        if (len != 36)
        {
            // invalid length, reset to zero.
            PlatformMemory::Set(&m_data, 0, sizeof(generic_uuid_t));
            return;
        }

        // because scanf need 4 byte space for scanning, we must use these temp variables
        uint32_t data1;
        uint32_t data2;
        uint32_t data3;
        uint32_t data40;
        uint32_t data41;
        uint32_t data42;
        uint32_t data43;
        uint32_t data44;
        uint32_t data45;
        uint32_t data46;
        uint32_t data47;

        // scanf cannot be wrapped
        Int retVal;
#ifdef OS_WINDOWS
        retVal = sscanf_s(guid, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                          &data1,
                          &data2,
                          &data3,
                          &data40,
                          &data41,
                          &data42,
                          &data43,
                          &data44,
                          &data45,
                          &data46,
                          &data47);
#else
        retVal = sscanf(guid, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        &data1,
                        &data2,
                        &data3,
                        &data40,
                        &data41,
                        &data42,
                        &data43,
                        &data44,
                        &data45,
                        &data46,
                        &data47);
#endif

        if (retVal == 11)
        {
            // successful scan
            m_data.Data1 = data1;
            m_data.Data2 = static_cast<uint16_t>(data2);
            m_data.Data3 = static_cast<uint16_t>(data3);
            m_data.Data4[0] = static_cast<uint8_t>(data40);
            m_data.Data4[1] = static_cast<uint8_t>(data41);
            m_data.Data4[2] = static_cast<uint8_t>(data42);
            m_data.Data4[3] = static_cast<uint8_t>(data43);
            m_data.Data4[4] = static_cast<uint8_t>(data44);
            m_data.Data4[5] = static_cast<uint8_t>(data45);
            m_data.Data4[6] = static_cast<uint8_t>(data46);
            m_data.Data4[7] = static_cast<uint8_t>(data47);
        }
        else
        {
            // error
            PlatformMemory::Set(&m_data, 0, sizeof(generic_uuid_t));
        }
    }
}
