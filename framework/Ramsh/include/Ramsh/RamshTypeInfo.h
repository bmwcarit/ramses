//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHTYPEINFO_H
#define RAMSES_RAMSHTYPEINFO_H

#include "Collections/String.h"

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
    struct TypeName<String>
    {
        inline explicit operator std::string() const
        {
            return "string";
        }
    };

    template<>
    struct TypeName<Float>
    {
        inline explicit operator std::string() const
        {
            return "float";
        }
    };

    template<>
    struct TypeName<Double>
    {
        inline explicit operator std::string() const
        {
            return "double";
        }
    };

    template<>
    struct TypeName<Int8>
    {
        inline explicit operator std::string() const
        {
            return "int8";
        }
    };

    template<>
    struct TypeName<Int16>
    {
        inline explicit operator std::string() const
        {
            return "int16";
        }
    };

    template<>
    struct TypeName<Int32>
    {
        inline explicit operator std::string() const
        {
            return "int32";
        }
    };

    template<>
    struct TypeName<Int64>
    {
        inline explicit operator std::string() const
        {
            return "int64";
        }
    };

    template<>
    struct TypeName<UInt8>
    {
        inline explicit operator std::string() const
        {
            return "uint8";
        }
    };

    template<>
    struct TypeName<UInt16>
    {
        inline explicit operator std::string() const
        {
            return "uint16";
        }
    };

    template<>
    struct TypeName<UInt32>
    {
        inline explicit operator std::string() const
        {
            return "uint32";
        }
    };

    template<>
    struct TypeName<UInt64>
    {
        inline explicit operator std::string() const
        {
            return "uint64";
        }
    };

    template<>
    struct TypeName<Char>
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
            return "int64";
        }
    };
#endif

}

#endif
