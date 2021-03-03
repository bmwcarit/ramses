//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COLLECTIONS_HASHSET_H
#define RAMSES_COLLECTIONS_HASHSET_H

#include "PlatformAbstraction/Hash.h"
#include "PlatformAbstraction/Macros.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{
    /**
     * Unordered set of objects with fast lookup and retrieval.
     */
    template <class T>
    class HashSet final
    {
    private:

        /**
         * Iterator for HashSet
         */
        template<typename HashMapIteratorType>
        class HashSetIterator final
        {
        public:

            /**
             * Copy Constructor
             * @param iter iterator to copy
             */
            HashSetIterator(const HashSetIterator<HashMapIteratorType>& iter)
                : m_iter(iter.m_iter)
            {
            }

            /*
            * Assignment Operator
            * @param iter r-value iterator for assignment
            */
            HashSetIterator& operator=(const HashSetIterator<HashMapIteratorType>& iter)
            {
                m_iter = iter.m_iter;
                return *this;
            }

            /**
             * Indirection
             * @return the current value referenced by the iterator
             */
            const T& operator*()
            {
                return (*m_iter).key;
            }

            /**
             * Dereference
             * @return a pointer to the current object the iterator points to
             */
            const T* operator->()
            {
                return &((*m_iter).key);
            }

            /**
             * Compares two iterators
             * @return true if the iterators point to the same position
             */
            bool operator==(const HashSetIterator<HashMapIteratorType>& iter) const
            {
                return (m_iter == iter.m_iter);
            }

            /**
             * Compares two iterators
             * @return true if the iterators do not point to the same position
             */
            bool operator!=(const HashSetIterator<HashMapIteratorType>& iter) const
            {
                return (m_iter != iter.m_iter);
            }

            /**
             * Step the iterator forward to the next element (prefix operator)
             * @return the next iterator
             */
            HashSetIterator<HashMapIteratorType>& operator++()
            {
                m_iter++;
                return *this;
            }

            /**
             * Step the iterator forward to the next element (postfix operator)
             * @return the next iterator
             */
            const HashSetIterator<HashMapIteratorType> operator++(int32_t)
            {
                HashSetIterator<HashMapIteratorType> oldValue(*this);
                ++(*this);
                return oldValue;
            }

        private:

            friend class HashSet<T>;

            /**
             * Internal constructor for HashSet
             *
             * @para iter iterator of the underlying hash table
             */
            explicit HashSetIterator(const HashMapIteratorType& iter)
                : m_iter(iter)
            {
            }

            HashMapIteratorType m_iter;
        };

    public:

        /**
         * Iterator for hashsets
         */
        using Iterator = HashSetIterator<typename HashMap<T, char>::Iterator>;
        using ConstIterator = HashSetIterator<typename HashMap<T, char>::ConstIterator>;

        using value_type = T;

        /**
         * Default Constructor
         */
        HashSet();

        /**
         * Parameterized Constructor
         */
        explicit HashSet(size_t initialCapacity);

        /**
         * Copy constructor.
         */
        HashSet(const HashSet& other);

        HashSet(HashSet&& other) noexcept;

        /**
         * Destructor
         */
        ~HashSet();

        /**
         * put a new value to the hash set.
         *
         * @param value             new value that will be put to hash set
         *
         * @return iterator to element
         *
         */
        Iterator put(const T& value);

        /**
         * Remove value associated with key in the hash set.
         *
         * @param value             value that will be removed
         *
         * @return true if remove is successful
         *         false if specified value does not exist in hash set
         *
         */
        bool remove(const T& value);

        /**
         * Remove iterator associated with key in the hash set.
         *
         * @param iterator iterator pointing to value that will be removed
         *
         */
        Iterator remove(Iterator iterator);

        /**
         * Checks if the provided value is already contained in the hash set.
         *
         * @param value             value that will be checked
         *
         * @return true if element is already contained in the hash set
         *         false otherwise
         *
         */
        RNODISCARD bool contains(const T& value) const;

        /**
         * Returns count of the hash set.
         * @return number of element in hash set
         */
        RNODISCARD size_t size() const;

        /**
         * Clear all values of the hash set.
         */
        void clear();

        /**
         * Return iterator for iterating key value tuples.
         * @return Iterator
         */
        RNODISCARD Iterator begin();

        /**
         * Return ConstIterator for iterating read only key value tuples.
         * @return ConstIterator
         */
        RNODISCARD ConstIterator begin() const;

        /**
         * returns an iterator pointing after the last element of the list
         * @return iterator
         */
        RNODISCARD Iterator end();

        /**
         * returns a ConstIterator pointing after the last element of the list
         * @return ConstIterator
         */
        RNODISCARD ConstIterator end() const;

        /**
        * returns an iterator pointing to given element in set, if not contained pass-the-end iterator (end) is returned
        * @return iterator
        */
        RNODISCARD Iterator find(const T& value);

        /**
        * returns a const iterator pointing to given element in set, if not contained pass-the-end iterator (end) is returned
        * @return ConstIterator
        */
        RNODISCARD ConstIterator find(const T& value) const;

        void insert(std::initializer_list<T> ilist);
        template <typename InputIt>

        void insert(InputIt first, InputIt last);

        /**
         * Reserve space for given number of bits elements. Does nothing if the
         * HashSet is already bigger.
         */
        void reserve(size_t requestedCapacity);

        RNODISCARD size_t capacity() const;

        /**
        * Swap this HashSet with another
        * @param other HashSet to copy from
        */
        void swap(HashSet<T>& other);

        HashSet& operator=(const HashSet& other);

        HashSet& operator=(HashSet&& other) noexcept;

    private:
        HashMap<T, char> m_map;
    };

    /**
     * swap specialization for HashSet<T>
     * @param first first HashSet
     * @param second HashSet to swap with first
     */
    template <class Key, class T, class C, class H>
    inline void swap(HashSet<T>& first, HashSet<T>& second)
    {
        first.swap(second);
    }

    template <class T>
    HashSet<T>::HashSet(const HashSet& other)
        : m_map(other.m_map) // just copy the inner hash table (which defines a copy constructor)
    {
    }

    template <class T>
    HashSet<T>::HashSet(HashSet&& other) noexcept
        : m_map(std::move(other.m_map))
    {
        static_assert(std::is_nothrow_move_constructible<HashSet>::value, "HashSet must be movable");
    }

    template <class T>
    HashSet<T>::HashSet()
        : m_map()
    {
    }

    template <class T>
    HashSet<T>::~HashSet()
    {
    }

    template <class T>
    HashSet<T>::HashSet(const size_t initialCapacity)
        : m_map(initialCapacity)
    {
    }

    template <class T>
    typename HashSet<T>::Iterator HashSet<T>::put(const T& value)
    {
        const auto it = m_map.find(value);
        if (it != m_map.end())
            return Iterator{it};
        return Iterator{m_map.put(value, 0)};
    }

    template <class T>
    bool HashSet<T>::remove(const T& value)
    {
        return m_map.remove(value);
    }

    template <class T>
    typename HashSet<T>::Iterator HashSet<T>::remove(Iterator it)
    {
        return Iterator{m_map.remove(it.m_iter)};
    }

    template <class T>
    bool HashSet<T>::contains(const T& value) const
    {
        return m_map.contains(value);
    }

    template <class T>
    size_t HashSet<T>::size() const
    {
        return m_map.size();
    }

    template <class T>
    void HashSet<T>::clear()
    {
        m_map.clear();
    }

    template <class T>
    typename HashSet<T>::Iterator HashSet<T>::begin()
    {
        return Iterator(m_map.begin());
    }

    template <class T>
    typename HashSet<T>::ConstIterator HashSet<T>::begin() const
    {
        return ConstIterator(m_map.begin());
    }

    template <class T>
    typename HashSet<T>::Iterator HashSet<T>::end()
    {
        return Iterator(m_map.end());
    }

    template <class T>
    typename HashSet<T>::ConstIterator HashSet<T>::end() const
    {
        return ConstIterator(m_map.end());
    }

    template <class T>
    typename HashSet<T>::Iterator HashSet<T>::find(const T& value)
    {
        return Iterator{m_map.find(value)};
    }

    template <class T>
    typename HashSet<T>::ConstIterator HashSet<T>::find(const T& value) const
    {
        return ConstIterator{m_map.find(value)};
    }

    template<class T>
    inline
    void HashSet<T>::insert(std::initializer_list<T> ilist)
    {
        reserve(size() + ilist.size());
        for (const auto& v : ilist)
        {
            put(v);
        }
    }

    template<class T>
    template <typename InputIt>
    inline
    void HashSet<T>::insert(InputIt first, InputIt last)
    {
        reserve(size() + std::distance(first, last));
        for (auto it = first; it != last; ++it)
        {
            put(*it);
        }
    }

    template <class T>
    void HashSet<T>::reserve(size_t requestedCapacity)
    {
        m_map.reserve(requestedCapacity);
    }

    template <class T>
    size_t HashSet<T>::capacity() const
    {
        return m_map.capacity();
    }

    template <class T>
    void HashSet<T>::swap(HashSet<T>& other)
    {
        m_map.swap(other.m_map);
    }

    template <class T>
    HashSet<T>& HashSet<T>::operator=(const HashSet<T>& other)
    {
        m_map = other.m_map;
        return *this;
    }

    template <class T>
    HashSet<T>& HashSet<T>::operator=(HashSet<T>&& other) noexcept
    {
        static_assert(std::is_nothrow_move_assignable<HashSet>::value, "HashSet must be movable");

        m_map = std::move(other.m_map);
        return *this;
    }
}

#endif
