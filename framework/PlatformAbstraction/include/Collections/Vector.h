//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTAINER_VECTOR_H
#define RAMSES_CONTAINER_VECTOR_H

#include <vector>
#include <assert.h>
#include <type_traits>

namespace ramses_internal
{
    template<typename T>
    class Vector
    {
    public:
        typedef T value_type;
        typedef typename std::vector<T>::iterator Iterator;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator ConstIterator;
        typedef typename std::vector<T>::const_iterator const_iterator;

        Vector()
        {
        }

        explicit Vector(const std::size_t initialSize)
            : m_vector(initialSize)
        {
        }

        Vector(const std::size_t initialSize, const T& value)
            : m_vector(initialSize, value)
        {
        }

        Vector(std::initializer_list<T> init)
            : m_vector(init)
        {
        }

        Vector(const Vector& other) = default;
        Vector& operator=(const Vector& other) = default;

        Vector(Vector&&) = default;
        Vector& operator=(Vector&&) = default;

        void push_back(const T& value)
        {
            m_vector.push_back(value);
        }

        void push_back(T&& value)
        {
            m_vector.push_back(std::move(value));
        }

        template <class... Args>
        void emplace_back(Args&&... args)
        {
            m_vector.emplace_back(std::forward<Args>(args)...);
        }

        void pop_back()
        {
            m_vector.pop_back();
        }

        T& front()
        {
            return m_vector.front();
        }

        const T& front() const
        {
            return m_vector.front();
        }

        T& back()
        {
            return m_vector.back();
        }

        const T& back() const
        {
            return m_vector.back();
        }

        T* data()
        {
            return m_vector.data();
        }

        const T* data() const
        {
            return m_vector.data();
        }

        void erase(const iterator& iter)
        {
            m_vector.erase(iter);
        }

        std::size_t size() const
        {
            return m_vector.size();
        }

        std::size_t capacity() const
        {
            return m_vector.capacity();
        }

        void shrink_to_fit()
        {
            m_vector.shrink_to_fit();
        }

        bool empty() const
        {
            return m_vector.empty();
        }

        void resize(const std::size_t size)
        {
            m_vector.resize(size);
        }

        void reserve(const std::size_t size)
        {
            m_vector.reserve(size);
        }

        void clear()
        {
            m_vector.clear();
        }

        typename std::vector<T>::const_reference operator[](const std::size_t index) const
        {
            return m_vector[index];
        }

        typename std::vector<T>::reference operator[](const std::size_t index)
        {
            return m_vector[index];
        }

        iterator begin()
        {
            return m_vector.begin();
        }

        const_iterator begin() const
        {
            return m_vector.begin();
        }

        const_iterator cbegin() const
        {
            return m_vector.cbegin();
        }

        Iterator end()
        {
            return m_vector.end();
        }

        const_iterator end() const
        {
            return m_vector.end();
        }

        const_iterator cend() const
        {
            return m_vector.cend();
        }

        void insert(const iterator& it, const T& value)
        {
            m_vector.insert(it, value);
        }

        template <typename InputIt>
        void insert(const iterator& it, InputIt first, InputIt last)
        {
            m_vector.insert(it, first, last);
        }

        iterator find(const T& value)
        {
            iterator it = m_vector.begin();
            for (; it != m_vector.end(); ++it)
            {
                if (*it == value)
                {
                    break;
                }
            }
            return it;
        }

        const_iterator find(const T& value) const
        {
            const_iterator it = m_vector.begin();
            for (; it != m_vector.end(); ++it)
            {
                if (*it == value)
                {
                    break;
                }
            }
            return it;
        }

        bool contains(const T& value) const
        {
            return find(value) != m_vector.end();
        }

        bool operator==(const Vector& other) const
        {
            return m_vector == other.m_vector;
        }

        bool operator!=(const Vector& other) const
        {
            return !(m_vector == other.m_vector);
        }

        void swap(Vector& other)
        {
            m_vector.swap(other.m_vector);
        }

        const std::vector<T>& stdRef() const
        {
            return m_vector;
        }

        std::vector<T>& stdRef()
        {
            return m_vector;
        }

    private:
        std::vector<T> m_vector;
    };

    // allow generic swap on ramses_internal::Vector
    template <typename T>
    inline
    void swap(ramses_internal::Vector<T>& first, ramses_internal::Vector<T>& second)
    {
        first.swap(second);
    }
}

#endif
