//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STRONGLYTYPEDVALUE_H
#define RAMSES_STRONGLYTYPEDVALUE_H

#include "ramses-framework-api/APIExport.h"
#include <functional>
#include <type_traits>

namespace ramses
{

    /**
    * @brief Helper class to create strongly typed values out of various types.
    */
    template <typename _BaseType, typename _UniqueId>
    class StronglyTypedValue
    {
    public:
        /**
        * @brief The underlying type of the class
        */
        typedef _BaseType BaseType;

        /**
        * @brief Constructor based on the underlying type
        * @param value The initial value of the underlying type in the object
        */
        explicit StronglyTypedValue(BaseType value)
            : m_value(value)
        {
        }

        /**
        * @brief Copy constructor
        * @param other The object to be copied
        */
        StronglyTypedValue(const StronglyTypedValue<BaseType, _UniqueId>& other)
            : m_value(other.m_value)
        {
        }

        /**
        * @brief Assignment operator
        * @param other The object to copy the value from
        * @returns A reference to this object.
        */
        StronglyTypedValue<BaseType, _UniqueId>& operator=(const StronglyTypedValue<BaseType, _UniqueId>& other)
        {
            m_value = other.m_value;
            return *this;
        }

        /**
        * @brief Getter for retrieving the underlying value
        * @returns The underlying value of the object
        */
        BaseType getValue() const
        {
            return m_value;
        }

        /**
        * @brief Getter for a reference of the underlying value
        * @returns Reference of the underlying value of the object
        */
        BaseType& getReference()
        {
            return m_value;
        }

        /**
        * @brief Equals operator, compares the object to another
        * @param other The object to compare to
        * @returns True if the underlying value of the object equals to the underlying value of the other.
        */
        bool operator==(const StronglyTypedValue<BaseType, _UniqueId>& other) const
        {
            return m_value == other.m_value;
        }

        /**
        * @brief Not equals operator, compares the object to another
        * @param other The object to compare to
        * @returns False if the underlying value of the object equals to the underlying value of the other.
        */
        bool operator!=(const StronglyTypedValue<BaseType, _UniqueId>& other) const
        {
            return m_value != other.m_value;
        }

        static_assert(std::is_arithmetic<BaseType>::value || std::is_pointer<BaseType>::value, "expected arithmetic or pointer basetype");

    private:
        BaseType m_value;
    };
}

namespace std
{
    /// Hasher for StronglyTypedValue for use in STL hash maps
    template <typename _BaseType, typename _UniqueId>
    struct hash<ramses::StronglyTypedValue<_BaseType, _UniqueId>>
    {
    public:
        /**
        * @brief Hasher implementation
        * @param v Value to be hashed
        * @returns Hash usable in STL hash maps.
        */
        size_t operator()(const ramses::StronglyTypedValue<_BaseType, _UniqueId>& v) const
        {
            return static_cast<size_t>(hash<_BaseType>()(v.getValue()));
        }
    };
}

#endif
