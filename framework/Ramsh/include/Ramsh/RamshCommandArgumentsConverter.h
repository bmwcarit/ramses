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
    using RamshArgumentData = std::string;
}

#define DEFINE_INT_CONVERTER(TYPE) \
    template<> \
    struct ArgumentConverter<TYPE> \
    { \
        static inline bool tryConvert(const RamshArgumentData& data, TYPE& value) \
        { \
            Int64 val = 0; \
            bool result = ArgumentConverter<Int64>::tryConvert(data,val); \
            value = static_cast<TYPE>(val); \
            return result; \
        } \
    };

namespace ramses_internal
{
    // converts raw binary data
    template<typename T>
    struct ArgumentConverter
    {
        static inline bool tryConvert(const RamshArgumentData& data, T& value)
        {
            if(data.size() < sizeof(T))
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
        static inline bool tryConvert(const RamshArgumentData& data, std::vector<T>& value)
        {
            String curr;

            bool result = true;

            for(UInt i = 0; i < data.size(); i++)
            {
                if(',' == data.at(i))
                {
                    T val = T();
                    result = result && ArgumentConverter<T>::tryConvert(curr,val);
                    value.push_back(val);

                    curr.clear();
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
        static inline bool tryConvert(const RamshArgumentData& data, std::pair<T1,T2>& value)
        {
            std::pair<T1,T2> val;

            bool result = false;

            String curr;
            for(UInt i = 0; i < data.size(); i++)
            {
                if(';' == data.at(i))
                {
                    result = ArgumentConverter<T1>::tryConvert(curr,val.first);
                    curr.clear();
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
        static inline bool tryConvert(const RamshArgumentData& data, Int64& value)
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
    struct ArgumentConverter<double>
    {
        static inline bool tryConvert(const RamshArgumentData& data, double& value)
        {
            Char* endptr;
            value = static_cast<double>(strtod(data.c_str(),&endptr));
            // conversion is only successful if anything was converted
            return endptr != data.c_str();
        }
    };

    template<>
    struct ArgumentConverter<Float>
    {
        static inline bool tryConvert(const RamshArgumentData& data, Float& value)
        {
            double val = 0.0;
            // reuse the double converter
            bool result = ArgumentConverter<double>::tryConvert(data,val);
            value = static_cast<Float>(val);
            return result;
        }
    };

    // initializes a String with the raw data
    template<>
    struct ArgumentConverter<String>
    {
        static inline bool tryConvert(const RamshArgumentData& data, String& value)
        {
            value = data;
            return true;
        }
    };

}// namespace ramses_internal

#endif
