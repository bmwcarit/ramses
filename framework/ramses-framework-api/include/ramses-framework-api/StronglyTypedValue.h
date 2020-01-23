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
    template <typename _baseType, _baseType _invalid, typename _uniqueId>
    class StronglyTypedValue
    {
    public:
        /**
        * @brief The underlying type of the class
        */
        typedef _baseType BaseType;

        /**
        * @brief Static Getter for Invalid
        * @returns Invalid Value of the underlying type
        */
        static constexpr StronglyTypedValue Invalid()
        {
            return StronglyTypedValue(_invalid);
        }

        /**
        * @brief Constructor based on the underlying type
        * @param value The initial value of the underlying type in the object
        */
        explicit constexpr StronglyTypedValue(BaseType value)
            : m_value(value)
        {
        }

        /**
        * @brief Default constructor with invalid value
        */
        constexpr StronglyTypedValue()
            : m_value(_invalid)
        {
        }

        /**
        * @brief Copy constructor
        * @param other The object to be copied
        */
        constexpr StronglyTypedValue(const StronglyTypedValue& other) = default;

        /**
        * @brief Assignment operator
        * @param other The object to copy the value from
        * @returns A reference to this object.
        */
        StronglyTypedValue& operator=(const StronglyTypedValue& other) = default;

        /**
        * @brief Getter for retrieving the underlying value
        * @returns The underlying value of the object
        */
        constexpr BaseType getValue() const
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
        * @brief Predicate to check value is unequal to Invalid() value
        * @returns true when valid (i.e. not Invalid()), false otherwise
        */
        constexpr bool isValid() const
        {
            return m_value != _invalid;
        }

        /**
        * @brief Equals operator, compares the object to another
        * @param other The object to compare to
        * @returns True if the underlying value of the object equals to the underlying value of the other.
        */
        constexpr bool operator==(const StronglyTypedValue& other) const
        {
            return m_value == other.m_value;
        }

        /**
        * @brief Not equals operator, compares the object to another
        * @param other The object to compare to
        * @returns False if the underlying value of the object equals to the underlying value of the other.
        */
        constexpr bool operator!=(const StronglyTypedValue& other) const
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
    template <typename _baseType, _baseType _invalid, typename _uniqueId>
    struct hash<ramses::StronglyTypedValue<_baseType, _invalid, _uniqueId>>
    {
    public:
        /**
        * @brief Hasher implementation
        * @param v Value to be hashed
        * @returns Hash usable in STL hash maps.
        */
        size_t operator()(const ramses::StronglyTypedValue<_baseType, _invalid, _uniqueId>& v) const
        {
            return static_cast<size_t>(hash<_baseType>()(v.getValue()));
        }
    };
}

#endif
