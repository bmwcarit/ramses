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
#include "Collections/Pair.h"
#include "Collections/StringOutputStream.h"
#include "PlatformAbstraction/PlatformMemory.h"

#include <string>
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
            int64_t val = 0; \
            bool result = ArgumentConverter<int64_t>::tryConvert(data,val); \
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
            std::string curr;

            bool result = true;

            for(size_t i = 0; i < data.size(); i++)
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

            std::string curr;
            for(size_t i = 0; i < data.size(); i++)
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
    struct ArgumentConverter<int64_t>
    {
        static inline bool tryConvert(const RamshArgumentData& data, int64_t& value)
        {
            char* endptr;
            value = static_cast<int64_t>(strtol(data.c_str(),&endptr,10));
            // conversion is only successful if anything was converted
            return endptr != data.c_str();
        }
    };

    // convertes which use the standard converter for int-types and just cast the value to the right type
    DEFINE_INT_CONVERTER(uint64_t)
    DEFINE_INT_CONVERTER(uint32_t)
    DEFINE_INT_CONVERTER(int32_t)
    DEFINE_INT_CONVERTER(uint16_t)
    DEFINE_INT_CONVERTER(int16_t)

    template<>
    struct ArgumentConverter<double>
    {
        static inline bool tryConvert(const RamshArgumentData& data, double& value)
        {
            char* endptr;
            value = static_cast<double>(strtod(data.c_str(),&endptr));
            // conversion is only successful if anything was converted
            return endptr != data.c_str();
        }
    };

    template<>
    struct ArgumentConverter<float>
    {
        static inline bool tryConvert(const RamshArgumentData& data, float& value)
        {
            double val = 0.0;
            // reuse the double converter
            bool result = ArgumentConverter<double>::tryConvert(data,val);
            value = static_cast<float>(val);
            return result;
        }
    };

    // initializes a std::string with the raw data
    template<>
    struct ArgumentConverter<std::string>
    {
        static inline bool tryConvert(const RamshArgumentData& data, std::string& value)
        {
            value = data;
            return true;
        }
    };

}// namespace ramses_internal

#endif
