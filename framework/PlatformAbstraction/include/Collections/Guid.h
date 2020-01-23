//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GUID_H
#define RAMSES_GUID_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/String.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "ramses-capu/container/Hash.h"
#include "ramses-capu/os/StringUtils.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    class String;

    struct generic_uuid_t
    {
        uint32_t  Data1;
        uint16_t Data2;
        uint16_t Data3;
        uint8_t Data4[8];
    };


    class Guid
    {
    public:
        explicit Guid(bool valid = false);
        explicit Guid(const generic_uuid_t& data);
        explicit Guid(const char* guid);
        explicit Guid(const String& guid);
        Guid(uint64_t high, uint64_t low);

        Guid(const Guid&) = default;
        Guid& operator=(const Guid&) = default;

        String toString() const;

        uint64_t getLow64() const;

        const generic_uuid_t& getGuidData() const;

        bool isInvalid() const;
        bool isValid() const;

        bool operator==(const Guid& other) const;
        bool operator!=(const Guid& other) const;

    private:
        void createNew();
        void createFromString(const char* guid, size_t len);
        void initFromInt64(uint64_t low);

        generic_uuid_t m_data;
    };

    static_assert(sizeof(generic_uuid_t) == 2*sizeof(uint64_t), "generic_uuid_t not properly packed");

    inline Guid::Guid(bool valid)
    {
        if (valid)
            createNew();
        else
            initFromInt64(0);
    }

    inline Guid::Guid(const generic_uuid_t& data)
        : m_data(data)
    {
        m_data.Data1 = 0;
        m_data.Data2 = 0;
        m_data.Data3 = 0;
    }

    inline Guid::Guid(const char* guid)
    {
        createFromString(guid, std::strlen(guid));
    }

    inline Guid::Guid(const String& guid)
        : Guid(guid.c_str())
    {
        createFromString(guid.c_str(), guid.size());
    }

    inline Guid::Guid(uint64_t /*high*/, uint64_t low)
    {
        initFromInt64(low);
    }

    inline String Guid::toString() const
    {
        String res;
        res.resize(17);
        std::snprintf(res.data(), 18, "%02X%02X-%02X%02X%02X%02X%02X%02X",
                      m_data.Data4[0],
                      m_data.Data4[1],
                      m_data.Data4[2],
                      m_data.Data4[3],
                      m_data.Data4[4],
                      m_data.Data4[5],
                      m_data.Data4[6],
                      m_data.Data4[7]);
        return res;
    }

    inline uint64_t Guid::getLow64() const
    {
        return (static_cast<uint64_t>(m_data.Data4[0]) << 56) +
            (static_cast<uint64_t>(m_data.Data4[1]) << 48) +
            (static_cast<uint64_t>(m_data.Data4[2]) << 40) +
            (static_cast<uint64_t>(m_data.Data4[3]) << 32) +
            (static_cast<uint64_t>(m_data.Data4[4]) << 24) +
            (static_cast<uint64_t>(m_data.Data4[5]) << 16) +
            (static_cast<uint64_t>(m_data.Data4[6]) <<  8) +
            static_cast<uint64_t>(m_data.Data4[7]);
    }

    inline void Guid::initFromInt64(uint64_t low)
    {
        m_data.Data1 = 0u;
        m_data.Data2 = 0u;
        m_data.Data3 = 0u;
        m_data.Data4[0] = (low >> 56) & 0xFF;
        m_data.Data4[1] = (low >> 48) & 0xFF;
        m_data.Data4[2] = (low >> 40) & 0xFF;
        m_data.Data4[3] = (low >> 32) & 0xFF;
        m_data.Data4[4] = (low >> 24) & 0xFF;
        m_data.Data4[5] = (low >> 16) & 0xFF;
        m_data.Data4[6] = (low >>  8) & 0xFF;
        m_data.Data4[7] = low& 0xFF;
    }

    inline const generic_uuid_t& Guid::getGuidData() const
    {
        return m_data;
    }

    inline bool Guid::isInvalid() const
    {
        return *this == Guid(false);
    }

    inline bool Guid::isValid() const
    {
        return !isInvalid();
    }

    inline bool Guid::operator==(const Guid& other) const
    {
        return PlatformMemory::Compare(&m_data, &other.m_data, sizeof(generic_uuid_t)) == 0;
    }

    inline bool Guid::operator!=(const Guid& other) const
    {
        return !operator==(other);
    }

    inline IOutputStream& operator<<(IOutputStream& stream, const Guid& value)
    {
        return stream.write(&value.getGuidData(), sizeof(generic_uuid_t));
    }

    inline IInputStream& operator>>(IInputStream& stream, Guid& value)
    {
        generic_uuid_t uuid;
        stream.read(reinterpret_cast<char*>(&uuid), sizeof(generic_uuid_t));
        value = Guid(uuid);
        return stream;
    }

    inline StringOutputStream& operator<<(StringOutputStream& sos, const Guid& value)
    {
        const generic_uuid_t& data = value.getGuidData();
        char buffer[37] = {0};
        if (data.Data1 == 0 && data.Data2 == 0 && data.Data3 == 0 &&
            data.Data4[0] == 0 && data.Data4[1] == 0 && data.Data4[2] == 0 && data.Data4[3] == 0 &&
            data.Data4[4] == 0 && data.Data4[5] == 0 && data.Data4[6] == 0)
        {
            std::snprintf(buffer, sizeof(buffer), "00%02X", data.Data4[7]);
        }
        else
        {
            std::snprintf(buffer, sizeof(buffer), "%02X%02X-%02X%02X%02X%02X%02X%02X",
                          data.Data4[0], data.Data4[1], data.Data4[2], data.Data4[3],
                          data.Data4[4], data.Data4[5], data.Data4[6], data.Data4[7]);
        }
        return sos << buffer;
    }
}

namespace std
{
    template<>
    struct hash<ramses_internal::Guid>
    {
        size_t operator()(const ramses_internal::Guid& key) const
        {
            return ramses_capu::HashMemoryRange(&(key.getGuidData()), sizeof(ramses_internal::generic_uuid_t));
        }
    };
}

#endif
