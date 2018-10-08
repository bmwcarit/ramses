//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMTYPEINFO_H
#define RAMSES_PLATFORMTYPEINFO_H

#include "Collections/String.h"

namespace ramses_internal
{
    typedef const void* PlatformTypeInfo;

    template<typename T>
    struct PlatformStaticTypeId
    {
        operator PlatformTypeInfo()
        {
            static PlatformTypeInfo id;
            return id;
        }
    };

    template<typename T>
    struct PlatformStaticTypeId<const volatile T>
    {
        operator PlatformTypeInfo()
        {
            return PlatformStaticTypeId<T>();
        }
    };

    template<typename T>
    struct PlatformStaticTypeId<const T>
    {
        operator PlatformTypeInfo()
        {
            return PlatformStaticTypeId<T>();
        }
    };

    template<typename T>
    struct PlatformStaticTypeId<volatile T>
    {
        operator PlatformTypeInfo()
        {
            return PlatformStaticTypeId<T>();
        }
    };

    struct PlatformTypeId
    {
        template<typename T>
        static PlatformTypeInfo id()
        {
            return PlatformStaticTypeId<T>();
        }
    };

// must be specialized to provide more beautiful names
    template<typename T>
    struct TypeName
    {
    };

    template<>
    struct TypeName<Bool>
    {
        inline operator String() const
        {
            return "Bool";
        }
    };

    template<>
    struct TypeName<String>
    {
        inline operator String() const
        {
            return "String";
        }
    };

    template<>
    struct TypeName<Float>
    {
        inline operator String() const
        {
            return "float";
        }
    };

    template<>
    struct TypeName<Double>
    {
        inline operator String() const
        {
            return "double";
        }
    };

    template<>
    struct TypeName<Int8>
    {
        inline operator String() const
        {
            return "int8";
        }
    };

    template<>
    struct TypeName<Int16>
    {
        inline operator String() const
        {
            return "int16";
        }
    };

    template<>
    struct TypeName<Int32>
    {
        inline operator String() const
        {
            return "int32";
        }
    };

    template<>
    struct TypeName<Int64>
    {
        inline operator String() const
        {
            return "int64";
        }
    };

    template<>
    struct TypeName<UInt8>
    {
        inline operator String() const
        {
            return "uint8";
        }
    };

    template<>
    struct TypeName<UInt16>
    {
        inline operator String() const
        {
            return "uint16";
        }
    };

    template<>
    struct TypeName<UInt32>
    {
        inline operator String() const
        {
            return "uint32";
        }
    };

    template<>
    struct TypeName<UInt64>
    {
        inline operator String() const
        {
            return "uint64";
        }
    };

    template<>
    struct TypeName<Char>
    {
        inline operator String() const
        {
            return "char";
        }
    };

}

#endif
