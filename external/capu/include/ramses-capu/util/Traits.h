/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_TRAITS_H
#define RAMSES_CAPU_TRAITS_H

#include "ramses-capu/Config.h"

namespace ramses_capu
{
#define CAPU_TYPE_CLASS     6
#define CAPU_TYPE_VOID      5
#define CAPU_TYPE_ENUM      4
#define CAPU_TYPE_POINTER   3
#define CAPU_TYPE_REFERENCE 2
#define CAPU_TYPE_PRIMITIVE 1
#define CAPU_TYPE_NONE      0

    //is CAPU_TYPE_PRIMITIVE
    template<typename T> struct is_CAPU_PRIMITIVE
    {
        enum { Value = CAPU_TYPE_NONE };
    };
    template<> struct is_CAPU_PRIMITIVE<char  >
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    //template<> struct is_CAPU_PRIMITIVE<uchar_t > { enum { Value = CAPU_TYPE_PRIMITIVE }; }; // identical to uint8_t
    template<> struct is_CAPU_PRIMITIVE<int8_t  >
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<int16_t >
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<int32_t >
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<int64_t >
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<uint8_t >
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<uint16_t>
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<uint32_t>
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<uint64_t>
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<float >
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<double>
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };
    template<> struct is_CAPU_PRIMITIVE<bool  >
    {
        enum { Value = CAPU_TYPE_PRIMITIVE   };
    };

    //is CAPU_TYPE_POINTER
    template <typename T> struct is_CAPU_POINTER
    {
        enum { Value = CAPU_TYPE_NONE };
    };
    template <typename T> struct is_CAPU_POINTER<T*>
    {
        enum { Value = CAPU_TYPE_POINTER };
    };

    //is CAPU_TYPE_REFERENCE
    template <typename T> struct is_CAPU_REFERENCE
    {
        enum { Value = CAPU_TYPE_NONE };
    };
    template <typename T> struct is_CAPU_REFERENCE<T&>
    {
        enum { Value = CAPU_TYPE_REFERENCE };
    };

    //is CAPU_TYPE_ENUM
    template <typename T> struct is_CAPU_ENUM
    {
        // rely on compiler function __is_enum
        enum { Value = __is_enum(T) ? CAPU_TYPE_ENUM : CAPU_TYPE_NONE };
    };

    //is CAPU_CLASS
    template <typename T> struct is_CAPU_CLASS
    {
        enum { Value = (is_CAPU_ENUM<T>::Value == 0 && is_CAPU_PRIMITIVE<T>::Value == 0 && is_CAPU_REFERENCE<T>::Value == 0 && is_CAPU_POINTER<T>::Value == 0) ? CAPU_TYPE_CLASS : CAPU_TYPE_NONE };
    };

    // basic access
    template<typename T>
    struct Type
    {
        enum id
        {
            Identifier =
                ramses_capu::is_CAPU_PRIMITIVE<T>::Value
                | ramses_capu::is_CAPU_POINTER<T>::Value
                | ramses_capu::is_CAPU_REFERENCE<T>::Value
                | ramses_capu::is_CAPU_ENUM<T>::Value
                | ramses_capu::is_CAPU_CLASS<T>::Value
        };
    };

    template<>
    struct Type<void>
    {
        enum id
        {
            Identifier = CAPU_TYPE_VOID
        };
    };
}

#endif /* RAMSES_CAPU_TRAITS_H */
