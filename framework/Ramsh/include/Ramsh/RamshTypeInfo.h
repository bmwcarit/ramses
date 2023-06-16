//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHTYPEINFO_H
#define RAMSES_RAMSHTYPEINFO_H

namespace ramses_internal
{
    using RamshTypeInfo = const void *;

    template<typename T>
    struct RamshStaticTypeId
    {
        explicit operator RamshTypeInfo()
        {
            static RamshTypeInfo id;
            return id;
        }
    };

    template<typename T>
    struct RamshStaticTypeId<const T>
    {
        explicit operator RamshTypeInfo()
        {
            return RamshStaticTypeId<T>();
        }
    };

    struct RamshTypeId
    {
        template<typename T>
        static RamshTypeInfo id()
        {
            return static_cast<RamshTypeInfo>(RamshStaticTypeId<T>());
        }
    };

    // must be specialized to provide more beautiful names
    template<typename T>
    struct TypeName
    {
    };

    template<>
    struct TypeName<bool>
    {
        inline explicit operator std::string() const
        {
            return "bool";
        }
    };

    template<>
    struct TypeName<std::string>
    {
        inline explicit operator std::string() const
        {
            return "string";
        }
    };

    template<>
    struct TypeName<float>
    {
        inline explicit operator std::string() const
        {
            return "float";
        }
    };

    template<>
    struct TypeName<double>
    {
        inline explicit operator std::string() const
        {
            return "double";
        }
    };

    template<>
    struct TypeName<int8_t>
    {
        inline explicit operator std::string() const
        {
            return "int8";
        }
    };

    template<>
    struct TypeName<int16_t>
    {
        inline explicit operator std::string() const
        {
            return "int16";
        }
    };

    template<>
    struct TypeName<int32_t>
    {
        inline explicit operator std::string() const
        {
            return "int32";
        }
    };

    template<>
    struct TypeName<int64_t>
    {
        inline explicit operator std::string() const
        {
            return "int64";
        }
    };

    template<>
    struct TypeName<uint8_t>
    {
        inline explicit operator std::string() const
        {
            return "uint8";
        }
    };

    template<>
    struct TypeName<uint16_t>
    {
        inline explicit operator std::string() const
        {
            return "uint16";
        }
    };

    template<>
    struct TypeName<uint32_t>
    {
        inline explicit operator std::string() const
        {
            return "uint32";
        }
    };

    template<>
    struct TypeName<uint64_t>
    {
        inline explicit operator std::string() const
        {
            return "uint64";
        }
    };

    template<>
    struct TypeName<char>
    {
        inline explicit operator std::string() const
        {
            return "char";
        }
    };

#ifdef __APPLE__
    template<>
    struct TypeName<Int>
    {
        inline explicit operator std::string() const
        {
            return "int";
        }
    };
#endif

}

#endif
