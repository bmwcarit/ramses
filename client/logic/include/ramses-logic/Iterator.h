//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"

#include <type_traits>
#include <iterator>

namespace ramses
{
    /**
     * An STL-style iterator for various object types (T) with forward-semantics. See also:
     *
     * - #ramses::Collection
     * - #ramses::LogicEngine::getCollection
     *
     * Template parameters:
     * - T: the object type to iterate over, e.g. #ramses::LuaScript
     * - internal_container: the internally wrapped container type. User code should *NOT* depend on this type
     *   to avoid API incompatibilities!
     * - isConst: true for const-iterators, false for non-const iterators
     *
     * Dereferencing the iterator yields a pointer of type T* or const T*, depending if the iterator is a const iterator.
     * The class is STL-compatible, meaning that the public type declarations may safely be used.
     *
     * Additional note: the Iterator class does not support swap() functionality as well as mutable assignments, because
     * the internal data structures used to iterate over objects are not supposed to be changed by user code. The iterators
     * are supposed to be used only in read-only scenarios. It is still possible to exercise write access on the
     * pointers returned by the iterator, but it's not possible to assign a new value to the pointer itself or to swap the pointers.
     */
    template <typename T, typename internal_container, bool isConst>
    class Iterator
    {
        friend class Iterator<T, internal_container, true>;
        friend class Iterator<T, internal_container, false>;
    private:
        /**
         * Internal type trait (const iterator type)
         */
        using const_iter = typename internal_container::const_iterator;

        /**
         * Internal type trait (non-const iterator type)
         */
        using non_const_iter = typename internal_container::iterator;

        /**
         * Internal type trait (internally wrapped iterator type)
         */
        using internal_iterator = typename std::conditional<isConst, const_iter, non_const_iter>::type;

        /**
         * Type returned when dereferencing the iterator (conditionally metaprogrammed for const-correctness when used inside const iterator)
         */
        using maybe_const_T = typename std::conditional<isConst, const T*, T*>::type;
    public:
        /**
         * Type traits as mandated by STL for custom iterators.
         */
        using difference_type = typename internal_iterator::difference_type;

        /**
         * The iterator type after dereferencing
         */
        using value_type = typename internal_iterator::value_type;

        /**
         * Iterator value pointer type
         */
        using pointer = typename internal_iterator::pointer;

        /**
         * Iterator value reference type
         */
        using reference = typename internal_iterator::reference;

        /**
         * Iterator type (refer to STL documentation for more information)
         */
        using iterator_category = std::forward_iterator_tag;

        /**
         * Operator dereferencing. Returns const T* if template argument isConst == true, otherwise returns T*.
         * @return T* or const T* pointer to iterable object, depending on iterator constness
         */
        [[nodiscard]] maybe_const_T operator*() noexcept
        {
            return *m_iterator;
        }

        /**
         * Member forwarding operator. Translates to '(const T*)->method' if template argument isConst == true, otherwise to '(T*)->method'.
         * @return T* or const T* pointer to iterable object, depending on iterator constness
         */
        [[nodiscard]] maybe_const_T operator->() noexcept
        {
            return *m_iterator;
        }

        /**
         * Pre-increment operator.
         * @return reference to self
         */
        Iterator& operator++() noexcept
        {
            ++m_iterator;
            return *this;
        }

        /**
         * Post-increment operator.
         * @return a copy of self before the increment was performed
         */
        Iterator operator++(int) noexcept //NOLINT(cert-dcl21-cpp) false-positive clang tidy warning
        {
            Iterator retval = *this;
            ++(*this);
            return retval;
        }

        /**
         * Equality operator.
         * @param other the other iterator to compare to
         * @return true if the iterators point to the same object internally
        */
        template <bool otherIsConst>
        bool operator==(const Iterator<T, internal_container, otherIsConst>& other) const noexcept
        {
            return m_iterator == other.m_iterator;
        }

        /**
         * Inequality operator.
         * @param other the other iterator to compare to
         * @return true if the iterators point to different objects internally
         */
        template <bool otherIsConst>
        bool operator!=(const Iterator<T, internal_container, otherIsConst>& other) const noexcept
        {
            return !(*this == other);
        }

        /**
         * Default constructor.
         */
        Iterator() noexcept = default;

        /**
         * Constructor which allows const-iterator to be constructed from a non-const iterator, but not the other way around
         * @param other iterator to construct from
         */
        template<bool otherIsConst, typename = std::enable_if_t<isConst && !otherIsConst>>
        Iterator(const Iterator<T, internal_container, otherIsConst> & other) noexcept  // NOLINT(google-explicit-constructor) needs conversion to work
            : m_iterator(other.m_iterator)
        {
        }

        /**
         * Assignment operator which allows const-iterator to be assigned from a non-const iterator, but not the other way around
         * @param other iterator to be assign from
         */
        template<bool otherIsConst, class = std::enable_if_t<isConst && !otherIsConst>>
        Iterator& operator=(const Iterator<T, internal_container, otherIsConst> & other) noexcept
        {
            m_iterator = other.m_iterator;
            return *this;
        }

        /**
         * Default copy constructor. This is redundant to the template version above, but is considered good style.
         */
        Iterator(const Iterator&) noexcept = default;

        /**
         * Default assignment operator. This is redundant to the template version above, but is considered good style.
         */
        Iterator& operator=(const Iterator&) noexcept = default;

        /**
         * Default destructor.
         */
        ~Iterator() noexcept = default;

        /**
         * Internal constructor which should not be called by user code to avoid API dependencies. Use
         * #ramses::Collection::begin() and #ramses::Collection::end() or their const-counterparts to
         * obtain iterators to desired #ramses::LogicEngine objects.
         * @param iter internal iterator to be constructed from
         */
        explicit Iterator(internal_iterator iter) noexcept
            : m_iterator(iter)
        {
        }

    private:
        /**
         * Internal iterator. This implementation is not following the pimpl pattern for efficiency reasons.
         */
        internal_iterator m_iterator = {};
    };
}
