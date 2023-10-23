//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"

#include <string>
#include <type_traits>

namespace ramses::internal
{
    using RamshArgumentData = std::string;

    // converts raw binary data
    template<typename T, typename = void>
    struct ArgumentConverter
    {
    };

    // converts data and adds it to a Vector (raw data separated by ',')
    template<typename T>
    struct ArgumentConverter< std::vector<T> >
    {
        static inline bool tryConvert(const RamshArgumentData& data, std::vector<T>& value)
        {
            std::string curr;

            bool result = true;

            for(auto ch : data)
            {
                if(',' == ch)
                {
                    T val = T();
                    result = result && ArgumentConverter<T>::tryConvert(curr,val);
                    value.push_back(val);

                    curr.clear();
                }
                else
                {
                    curr += ch;
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
            for (auto ch : data)
            {
                if(';' == ch)
                {
                    result = ArgumentConverter<T1>::tryConvert(curr,val.first);
                    curr.clear();
                }
                else
                {
                    curr += ch;
                }
            }

            result = result && ArgumentConverter<T2>::tryConvert(curr,val.second);
            value = val;
            return result;
        };
    };

    // standard converter for int-types
    template<typename T>
    struct ArgumentConverter<T, typename std::enable_if_t< std::is_signed_v<T>>>
    {
        static inline bool tryConvert(const RamshArgumentData& data, T& value)
        {
            char* endptr = nullptr; // NOLINT(cppcoreguidelines-pro-type-vararg): false positive
            value = static_cast<T>(strtoll(data.c_str(),&endptr,10));
            // conversion is only successful if anything was converted
            return endptr != data.c_str();
        }
    };

    template<typename T>
    struct ArgumentConverter<T, typename std::enable_if_t< std::is_unsigned_v<T>>>
    {
        static inline bool tryConvert(const RamshArgumentData& data, T& value)
        {
            char* endptr = nullptr; // NOLINT(cppcoreguidelines-pro-type-vararg): false positive
            value = static_cast<T>(strtoull(data.c_str(), &endptr, 10));
            // conversion is only successful if anything was converted
            return endptr != data.c_str();
        }
    };

    template<>
    struct ArgumentConverter<double>
    {
        static inline bool tryConvert(const RamshArgumentData& data, double& value)
        {
            char* endptr = nullptr; // NOLINT(cppcoreguidelines-pro-type-vararg): false positive
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

}// namespace ramses::internal
