//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <type_traits>
#include <iterator>
#include <cassert>

namespace ramses::internal
{
    template<typename memory_pool_t, typename iterator_element_type_t>
    class memory_pool_iterator_t
    {
    public:
        using value_type = iterator_element_type_t;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        memory_pool_iterator_t(const memory_pool_iterator_t&) = default;
        memory_pool_iterator_t& operator=(const memory_pool_iterator_t&) = default;

        bool operator==(const memory_pool_iterator_t& other) const
        {
            return this->m_memPool == other.m_memPool && m_current == other.m_current;
        }

        bool operator!=(const memory_pool_iterator_t& other) const
        {
            return !(*this == other);
        }

        const iterator_element_type_t& operator*() const
        {
            return m_current;
        }

        const iterator_element_type_t* operator->() const
        {
            return &m_current;
        }

        memory_pool_iterator_t& operator++()
        {
            seekNextAllocated();
            return *this;
        }

        // NOLINTNEXTLINE(readability-const-return-type): required by cert-dcl21-cpp
        const memory_pool_iterator_t operator++(int)
        {
            auto currentIt = *this;

            seekNextAllocated();
            return currentIt;
        }

    protected:
        explicit memory_pool_iterator_t(memory_pool_t* memPool, typename memory_pool_t::handle_type start)
            : m_memPool(memPool)
            , m_current(start, nullptr)
        {
            if (m_memPool && start < m_memPool->getTotalCount())
            {
                if (m_memPool->isAllocated(start))
                {
                    m_current.second = m_memPool->getMemory(start);
                }
                else
                {
                    seekNextAllocated();
                }
            }
        }

        void seekNextAllocated()
        {
            assert(m_memPool);
            const auto totalCount = m_memPool->getTotalCount();
            assert(m_current.first < totalCount);
            m_current.second = nullptr;

            while (m_current.first < totalCount)
            {
                ++m_current.first;
                if (m_current.first == totalCount)
                    return;

                if (m_memPool->isAllocated(m_current.first))
                {
                    m_current.second = m_memPool->getMemory(m_current.first);
                    return;
                }
            }
        }

        memory_pool_t* m_memPool = nullptr;
        iterator_element_type_t m_current;
    };

    template<typename memory_pool_t,
        typename element_type_t = std::pair<typename memory_pool_t::handle_type, const typename memory_pool_t::object_type*>>
    class const_memory_pool_iterator : public memory_pool_iterator_t<const memory_pool_t, element_type_t>
    {
        friend memory_pool_t;
    public:
        const_memory_pool_iterator()
            : memory_pool_iterator_t<const memory_pool_t, element_type_t>(nullptr, typename memory_pool_t::handle_type{})
        {
        }

    protected:
        explicit const_memory_pool_iterator(const memory_pool_t& memPool, typename memory_pool_t::handle_type start)
            : memory_pool_iterator_t<const memory_pool_t, element_type_t>(&memPool, start)
        {
        }
    };

    template<typename memory_pool_t
        , typename element_type_t = std::pair<typename memory_pool_t::handle_type, typename memory_pool_t::object_type*>>
    class memory_pool_iterator : public memory_pool_iterator_t<memory_pool_t, element_type_t>
    {
        friend memory_pool_t;
    public:
        memory_pool_iterator()
            : memory_pool_iterator_t<memory_pool_t, element_type_t>(nullptr, typename memory_pool_t::handle_type{})
        {
        }

    protected:
        explicit memory_pool_iterator(memory_pool_t& memPool, typename memory_pool_t::handle_type start)
            : memory_pool_iterator_t<memory_pool_t, element_type_t>(&memPool, start)
        {
        }
    };
}
