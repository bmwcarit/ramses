//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDARGUMENTSCONVERTER_H
#define RAMSES_RAMSHCOMMANDARGUMENTSCONVERTER_H

#include "Collections/Vector.h"
#include "Collections/String.h"
#include "Collections/Pair.h"
#include "Collections/StringOutputStream.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
{
    typedef String RamshArgumentData;
}

#define DEFINE_INT_CONVERTER(TYPE) \
    template<> \
    struct ArgumentConverter<TYPE> \
    { \
        static inline Bool tryConvert(const RamshArgumentData& data, TYPE& value) \
        { \
            Int64 val = 0; \
            Bool result = ArgumentConverter<Int64>::tryConvert(data,val); \
            value = static_cast<TYPE>(val); \
            return result; \
        } \
    };

namespace ramses_internal
{

    template<typename T>
    inline StringOutputStream& operator<<(StringOutputStream& lhs, const std::vector<T>& rhs)
    {
        typename std::vector<T>::ConstIterator it = rhs.begin();
        const typename std::vector<T>::ConstIterator end = rhs.end();

        for(;it!=end;++it)
        {
            lhs << "," << *it;
        }
        return lhs;
    }

    template<typename A, typename B>
    inline StringOutputStream& operator<<(StringOutputStream& lhs, const std::pair<A,B>& rhs)
    {
        lhs << rhs.first;
        lhs << ";";
        lhs << rhs.second;
        return lhs;
    }

    // converts raw binary data
    template<typename T>
    struct ArgumentConverter
    {
        static inline Bool tryConvert(const RamshArgumentData& data, T& value)
        {
            if(data.getLength() < sizeof(T))
            {
                return false;
            }

            PlatformMemory::Copy(&value,data.c_str(),sizeof(T));
            return true;
        }
    };

    // converts data and adds it to a Vector (raw data separated by ',')
    template<typename T>
    struct ArgumentConverter< std::vector<T> >
    {
        static inline Bool tryConvert(const RamshArgumentData& data, std::vector<T>& value)
        {
            String curr;

            Bool result = true;

            for(UInt i = 0; i < data.getLength(); i++)
            {
                if(',' == data.at(i))
                {
                    T val = T();
                    result = result && ArgumentConverter<T>::tryConvert(curr,val);
                    value.push_back(val);

                    curr.truncate(0);
                }
                else
                {
                    curr+=data.at(i);
                }
            }

            T val = T();
            result = result && ArgumentConverter<T>::tryConvert(curr,val);
            value.push_back(val);

            return result;
        }
    };

    // converts data and initializes a std::pair (raw data separated by ';')
    template<typename T1, typename T2>
    struct ArgumentConverter< std::pair<T1,T2> >
    {
        static inline Bool tryConvert(const RamshArgumentData& data, std::pair<T1,T2>& value)
        {
            std::pair<T1,T2> val;

            Bool result = false;

            String curr;
            for(UInt i = 0; i < data.getLength(); i++)
            {
                if(';' == data.at(i))
                {
                    result = ArgumentConverter<T1>::tryConvert(curr,val.first);
                    curr.truncate(0);
                }
                else
                {
                    curr+=data.at(i);
                }
            }

            result = result && ArgumentConverter<T2>::tryConvert(curr,val.second);
            value = val;
            return result;
        };
    };

    // standard converter for int-types
    template<>
    struct ArgumentConverter<Int64>
    {
        static inline Bool tryConvert(const RamshArgumentData& data, Int64& value)
        {
            Char* endptr;
            value = static_cast<Int64>(strtol(data.c_str(),&endptr,10));
            // conversion is only successful if anything was converted
            return endptr != data.c_str();
        }
    };

    // convertes which use the standard converter for int-types and just cast the value to the right type
    DEFINE_INT_CONVERTER(UInt64)
    DEFINE_INT_CONVERTER(UInt32)
    DEFINE_INT_CONVERTER(Int32)
    DEFINE_INT_CONVERTER(UInt16)
    DEFINE_INT_CONVERTER(Int16)

    template<>
    struct ArgumentConverter<Double>
    {
        static inline Bool tryConvert(const RamshArgumentData& data, Double& value)
        {
            Char* endptr;
            value = static_cast<Double>(strtod(data.c_str(),&endptr));
            // conversion is only successful if anything was converted
            return endptr != data.c_str();
        }
    };

    template<>
    struct ArgumentConverter<Float>
    {
        static inline Bool tryConvert(const RamshArgumentData& data, Float& value)
        {
            Double val = 0.0;
            // reuse the Double converter
            Bool result = ArgumentConverter<Double>::tryConvert(data,val);
            value = static_cast<Float>(val);
            return result;
        }
    };

    // initializes a String with the raw data
    template<>
    struct ArgumentConverter<String>
    {
        static inline Bool tryConvert(const RamshArgumentData& data, String& value)
        {
            value = data;
            return true;
        }
    };

}// namespace ramses_internal

#endif
