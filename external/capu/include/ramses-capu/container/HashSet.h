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

#ifndef RAMSES_CAPU_HASHSET_H
#define RAMSES_CAPU_HASHSET_H

#include "ramses-capu/container/Hash.h"
#include "ramses-capu/container/HashTable.h"

namespace ramses_capu
{
    /**
     * Unordered set of objects with fast lookup and retrieval.
     */
    template <class T, class C = std::equal_to<T>, class H = Hash<T>>
    class HashSet final
    {
    private:

        /**
         * Iterator for HashSet
         */
        template<typename HashTableIteratorType>
        class HashSetIterator
        {
        public:

            /**
             * Copy Constructor
             * @param iter iterator to copy
             */
            HashSetIterator(const HashSetIterator<HashTableIteratorType>& iter)
                : m_iter(iter.m_iter)
            {
            }

            /*
            * Assignment Operator
            * @param iter r-value iterator for assignment
            */
            HashSetIterator& operator=(const HashSetIterator<HashTableIteratorType>& iter)
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
            bool operator==(const HashSetIterator<HashTableIteratorType>& iter) const
            {
                return (m_iter == iter.m_iter);
            }

            /**
             * Compares two iterators
             * @return true if the iterators do not point to the same position
             */
            bool operator!=(const HashSetIterator<HashTableIteratorType>& iter) const
            {
                return (m_iter != iter.m_iter);
            }

            /**
             * Step the iterator forward to the next element (prefix operator)
             * @return the next iterator
             */
            HashSetIterator<HashTableIteratorType>& operator++()
            {
                m_iter++;
                return *this;
            }

            /**
             * Step the iterator forward to the next element (postfix operator)
             * @return the next iterator
             */
            const HashSetIterator<HashTableIteratorType> operator++(int32_t)
            {
                HashSetIterator<HashTableIteratorType> oldValue(*this);
                ++(*this);
                return oldValue;
            }

        private:

            friend class HashSet<T, C, H>;

            /**
             * Internal constructor for HashSet
             *
             * @para iter iterator of the underlying hash table
             */
            HashSetIterator(const HashTableIteratorType& iter)
                : m_iter(iter)
            {
            }

            HashTableIteratorType m_iter;
        };

    public:

        /**
         * Iterator for hashsets
         */
        typedef HashSetIterator< typename HashTable<T, char, C, H>::Iterator >        Iterator;
        typedef HashSetIterator< typename HashTable<T, char, C, H>::ConstIterator >   ConstIterator;

        /**
         * Default Constructor
         */
        HashSet();

        /**
         * Parameterized Constructor
         */
        HashSet(uint_t initialCapacity);

        /**
         * Copy constructor.
         */
        HashSet(const HashSet& other);

        HashSet(HashSet&& other);

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
        bool hasElement(const T& value) const;

        /**
         * Returns count of the hash set.
         * @return number of element in hash set
         */
        uint_t count() const;

        /**
         * Clear all values of the hash set.
         */
        void clear();

        /**
         * Return iterator for iterating key value tuples.
         * @return Iterator
         */
        Iterator begin();

        /**
         * Return ConstIterator for iterating read only key value tuples.
         * @return ConstIterator
         */
        ConstIterator begin() const;

        /**
         * returns an iterator pointing after the last element of the list
         * @return iterator
         */
        Iterator end();

        /**
         * returns a ConstIterator pointing after the last element of the list
         * @return ConstIterator
         */
        ConstIterator end() const;

        /**
        * returns an iterator pointing to given element in set, if not contained pass-the-end iterator (end) is returned
        * @return iterator
        */
        Iterator find(const T& value);

        /**
        * returns a const iterator pointing to given element in set, if not contained pass-the-end iterator (end) is returned
        * @return ConstIterator
        */
        ConstIterator find(const T& value) const;

        /**
         * Reserve space for given number of bits elements. Does nothing if the
         * HashSet is already bigger.
         */
        void reserve(uint_t requestedCapacity);

        uint_t capacity() const;

        /**
        * Swap this HashSet with another
        * @param other HashSet to copy from
        */
        void swap(HashSet<T, C, H>& other);

        HashSet& operator=(const HashSet& other);

        HashSet& operator=(HashSet&& other);

    private:
        HashTable<T, char, C, H> m_table;
    };

    /**
     * swap specialization for HashSet<T, C, H>
     * @param first first HashSet
     * @param second HashSet to swap with first
     */
    template <class Key, class T, class C, class H>
    inline void swap(HashSet<T, C, H>& first, HashSet<T, C, H>& second)
    {
        first.swap(second);
    }

    template <class T, class C, class H>
    HashSet<T, C, H>::HashSet(const HashSet& other)
        : m_table(other.m_table) // just copy the inner hash table (which defines a copy constructor)
    {
    }

    template <class T, class C, class H>
    HashSet<T, C, H>::HashSet(HashSet&& other)
        : m_table(std::move(other.m_table))
    {
    }

    template <class T, class C, class H>
    HashSet<T, C, H>::HashSet()
        : m_table()
    {
    }

    template <class T, class C, class H>
    HashSet<T, C, H>::~HashSet()
    {
    }

    template <class T, class C, class H>
    HashSet<T, C, H>::HashSet(const uint_t initialCapacity)
        : m_table(initialCapacity)
    {
    }

    template <class T, class C, class H>
    typename HashSet<T, C, H>::Iterator HashSet<T, C, H>::put(const T& value)
    {
        const auto it = m_table.find(value);
        if (it != m_table.end())
            return it;
        return m_table.put(value, 0);
    }

    template <class T, class C, class H>
    bool HashSet<T, C, H>::remove(const T& value)
    {
        return m_table.remove(value);
    }

    template <class T, class C, class H>
    typename HashSet<T, C, H>::Iterator HashSet<T, C, H>::remove(Iterator it)
    {
        return m_table.remove(it.m_iter);
    }

    template <class T, class C, class H>
    bool HashSet<T, C, H>::hasElement(const T& value) const
    {
        return m_table.contains(value);
    }

    template <class T, class C, class H>
    uint_t HashSet<T, C, H>::count() const
    {
        return m_table.count();
    }

    template <class T, class C, class H>
    void HashSet<T, C, H>::clear()
    {
        m_table.clear();
    }

    template <class T, class C, class H>
    typename HashSet<T, C, H>::Iterator HashSet<T, C, H>::begin()
    {
        return Iterator(m_table.begin());
    }

    template <class T, class C, class H>
    typename HashSet<T, C, H>::ConstIterator HashSet<T, C, H>::begin() const
    {
        return ConstIterator(m_table.begin());
    }

    template <class T, class C, class H>
    typename HashSet<T, C, H>::Iterator HashSet<T, C, H>::end()
    {
        return Iterator(m_table.end());
    }

    template <class T, class C, class H>
    typename HashSet<T, C, H>::ConstIterator HashSet<T, C, H>::end() const
    {
        return ConstIterator(m_table.end());
    }

    template <class T, class C, class H>
    typename HashSet<T, C, H>::Iterator HashSet<T, C, H>::find(const T& value)
    {
        return m_table.find(value);
    }

    template <class T, class C, class H>
    typename HashSet<T, C, H>::ConstIterator HashSet<T, C, H>::find(const T& value) const
    {
        return m_table.find(value);
    }

    template <class T, class C, class H>
    void HashSet<T, C, H>::reserve(uint_t requestedCapacity)
    {
        m_table.reserve(requestedCapacity);
    }

    template <class T, class C, class H>
    uint_t HashSet<T, C, H>::capacity() const
    {
        return m_table.capacity();
    }

    template <class T, class C, class H>
    void HashSet<T, C, H>::swap(HashSet<T, C, H>& other)
    {
        m_table.swap(other.m_table);
    }

    template <class T, class C, class H>
    HashSet<T, C, H>& HashSet<T, C, H>::operator=(const HashSet<T, C, H>& other)
    {
        m_table = other.m_table;
        return *this;
    }

    template <class T, class C, class H>
    HashSet<T, C, H>& HashSet<T, C, H>::operator=(HashSet<T, C, H>&& other)
    {
        m_table = std::move(other.m_table);
        return *this;
    }
}

#endif /* RAMSES_CAPU_HASHSET_H */
