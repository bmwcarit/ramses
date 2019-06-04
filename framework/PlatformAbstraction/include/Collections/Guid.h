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

        Guid(const Guid&) = default;
        Guid& operator=(const Guid&) = default;

        String toString() const;

        const generic_uuid_t& getGuidData() const;

        bool isInvalid() const;

        bool operator==(const Guid& other) const;
        bool operator!=(const Guid& other) const;

    private:
        void createNew();
        void createFromString(const char* guid, size_t len);

        generic_uuid_t m_data;

        friend IInputStream& operator>>(IInputStream& stream, Guid& value);
    };

    inline Guid::Guid(bool valid)
    {
        if (valid)
            createNew();
        else
            PlatformMemory::Set(&m_data, 0, sizeof(generic_uuid_t));
    }

    inline Guid::Guid(const generic_uuid_t& data)
        : m_data(data)
    {
    }

    inline Guid::Guid(const char* guid)
    {
        createFromString(guid, ramses_capu::StringUtils::Strlen(guid));
    }

    inline Guid::Guid(const String& guid)
        : Guid(guid.c_str())
    {
        createFromString(guid.c_str(), guid.getLength());
    }

    inline String Guid::toString() const
    {
        String res;
        res.resize(36);
        ramses_capu::StringUtils::Sprintf(res.data(), 37, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                             m_data.Data1,
                             m_data.Data2,
                             m_data.Data3,
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

    inline const generic_uuid_t& Guid::getGuidData() const
    {
        return m_data;
    }

    inline bool Guid::isInvalid() const
    {
        return *this == Guid(false);
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
        return stream.read(reinterpret_cast<char*>(&value.m_data), sizeof(generic_uuid_t));
    }
}

namespace ramses_capu
{
    /**
     * Hash code generation for a Guid instance. Necessary e. g. for using a Guid as a key in a hash map.
     */
    template<>
    struct Hash<ramses_internal::Guid>
    {
        uint_t operator()(const ramses_internal::Guid& key)
        {
            return HashMemoryRange(&(key.getGuidData()), sizeof(ramses_internal::generic_uuid_t));
        }
    };
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
