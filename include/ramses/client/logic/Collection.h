//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/client/logic/Iterator.h"

#include <functional>
#include <memory>
#include <vector>

namespace ramses
{
    /**
     * A Collection which allows STL-style algorithm with forward semantics to be executed (e.g. find_if, for_each etc.).
     * A Collection cannot be instantiated directly, it can only be obtained using #ramses::LogicEngine::getCollection.
     *
     * See also #ramses::Iterator for more info on the iterator objects returned by #begin and #end.
     *
     * Template parameters:
     * - T: the object type to iterate over, e.g. #ramses::LuaScript
     */
    template <typename T>
    class Collection
    {
    private:
        /**
         * Internal container type. Not supposed to be used by user code in any way!
         */
        using internal_container_type = std::vector<T*>;
    public:
        /**
         * The iterator type returned by #begin() and #end()
         */
        using iterator = Iterator<T, internal_container_type, false>;

        /**
         * The iterator type returned by #cbegin() and #cend()
         */
        using const_iterator = Iterator<T, internal_container_type, true>;

        /**
         * The iterator type after dereferencing
         */
        using value_type = typename iterator::value_type;

        /**
         * Iterator value pointer type
         */
        using pointer = typename iterator::pointer;

        /**
         * Iterator value reference type
         */
        using reference = typename iterator::reference;

        /**
        * Return number of elements in the collection
        * @return number of elements in the collection
        */
        [[nodiscard]] size_t size() const
        {
            return m_container.get().size();
        }

        /**
         * Return an iterator to the start of the collection
         * @return iterator to the start of the collection
         */
        [[nodiscard]] iterator begin()
        {
            return iterator(m_container.get().begin());
        }

        /**
         * Return an const iterator to the start of the collection
         * @return const iterator to the start of the collection
         */
        [[nodiscard]] const_iterator begin() const
        {
            return iterator(m_container.get().begin());
        }

        /**
         * Return an iterator to the end of the collection
         * @return iterator to the end of the collection
         */
        [[nodiscard]] iterator end()
        {
            return iterator(m_container.get().end());
        }

        /**
         * Return a const iterator to the end of the collection
         * @return const iterator to the end of the collection
         */
        [[nodiscard]] const_iterator end() const
        {
            return const_iterator(m_container.get().cend());
        }

        /**
         * Return a const iterator to the start of the collection
         * @return const iterator to the  of the collection
         */
        [[nodiscard]] const_iterator cbegin() const
        {
            return const_iterator(m_container.get().cbegin());
        }

        /**
         * Return a const iterator to the end of the collection
         * @return const iterator to the end of the collection
         */
        [[nodiscard]] const_iterator cend() const
        {
            return const_iterator(m_container.get().cend());
        }

        /**
         * Default constructor is deleted because a collection is supposed to provide read-only access to
         * internal data of the #ramses::LogicEngine class, therefore it can only be obtained by calling
         * #ramses::LogicEngine::getCollection
         */
        Collection() = delete;

        /**
         * Default destructor
         */
        ~Collection() noexcept = default;

        /**
         * Default copy constructor
         * @param other collection to copy
         */
        Collection(const Collection& other) noexcept = default;

        /**
         * Default assignment operator
         * @param other collection to assign from
         */
        Collection& operator=(const Collection& other) noexcept = default;

        /**
         * Internal constructor. Not supposed to be called from user code!
         * @param container internal container
         */
        explicit Collection(internal_container_type& container)
            : m_container(container)
        {
        }

    private:
        /**
         * Internal reference to the container holding the actual data held by the collection.
         */
        std::reference_wrapper<internal_container_type> m_container;
    };

}
