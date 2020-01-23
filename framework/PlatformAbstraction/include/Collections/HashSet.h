//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_HASHSET_H
#define RAMSES_UTILS_HASHSET_H

#include <ramses-capu/container/HashSet.h>
#include <PlatformAbstraction/PlatformError.h>
#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/Macros.h"
#include <cmath>

namespace ramses_internal
{
    template<class T>
    class HashSet
    {
    public:
        typedef T value_type;
        typedef typename ramses_capu::HashSet<T>::Iterator Iterator;
        typedef typename ramses_capu::HashSet<T>::ConstIterator ConstIterator;

        HashSet();
        HashSet(const HashSet<T>& other);
        HashSet(HashSet<T>&& other) RNOEXCEPT;
        explicit HashSet(UInt initialCapacity);
        ~HashSet();

        void reserve(UInt requestedCapacity);
        UInt capacity() const;
        Iterator put(const T& value);
        bool remove(const T& value);
        Iterator remove(Iterator value);
        UInt size() const;
        void clear();
        ConstIterator begin() const;
        ConstIterator end() const;
        Iterator begin();
        Iterator end();
        bool contains(const T& element) const;

        HashSet& operator=(const HashSet& other);
        HashSet& operator=(HashSet&& other) RNOEXCEPT;

        void insert(std::initializer_list<value_type> ilist);
        template <typename InputIt>
        void insert(InputIt first, InputIt last);

    private:
        ramses_capu::HashSet<T> mHashSet;
    };

    template<class T>
    HashSet<T>::HashSet(const HashSet<T>& other)
        : mHashSet(other.mHashSet)
    {
    }

    template<class T>
    HashSet<T>::HashSet(HashSet<T>&& other) RNOEXCEPT
        : mHashSet(std::move(other.mHashSet))
    {
        static_assert(std::is_nothrow_move_constructible<HashSet>::value, "HashSet must be movable");
    }

    template<class T>
    HashSet<T>::HashSet()
    {
    }

    template<class T>
    HashSet<T>::HashSet(UInt initialCapacity)
        : mHashSet(initialCapacity)
    {
    }

    template<class T>
    HashSet<T>::~HashSet()
    {
    }

    template<class T>
    inline
    void HashSet<T>::reserve(UInt capacity)
    {
        mHashSet.reserve(capacity);
    }

    template<class T>
    inline
    UInt HashSet<T>::capacity() const
    {
        return mHashSet.capacity();
    }

    template<class T>
    inline
    typename HashSet<T>::Iterator HashSet<T>::put(const T& value)
    {
        return mHashSet.put(value);
    }

    template<class T>
    inline
    bool HashSet<T>::remove(const T& value)
    {
        return mHashSet.remove(value);
    }

    template<class T>
    inline
    typename HashSet<T>::Iterator HashSet<T>::remove(Iterator iterator)
    {
        return mHashSet.removeAt(iterator);
    }

    template<class T>
    inline
    UInt HashSet<T>::size() const
    {
        return static_cast<EStatus>(mHashSet.count());
    }

    template<class T>
    inline
    void HashSet<T>::clear()
    {
        mHashSet.clear();
    }

    template<class T>
    inline
    typename HashSet<T>::ConstIterator HashSet<T>::begin() const
    {
        return mHashSet.begin();
    }

    template<class T>
    inline
    typename HashSet<T>::ConstIterator HashSet<T>::end() const
    {
        return mHashSet.end();
    }

    template<class T>
    inline
    typename HashSet<T>::Iterator HashSet<T>::begin()
    {
        return mHashSet.begin();
    }

    template<class T>
    inline
    typename HashSet<T>::Iterator HashSet<T>::end()
    {
        return mHashSet.end();
    }

    template<class T>
    inline
    bool HashSet<T>::contains(const T& element) const
    {
        return mHashSet.hasElement(element);
    }

    template<class T>
    inline
    HashSet<T>& HashSet<T>::operator=(const HashSet& other)
    {
        mHashSet = other.mHashSet;
        return *this;
    }

    template<class T>
    inline
    HashSet<T>& HashSet<T>::operator=(HashSet&& other) RNOEXCEPT
    {
        static_assert(std::is_nothrow_move_assignable<HashSet>::value, "HashSet must be movable");

        mHashSet = std::move(other.mHashSet);
        return *this;
    }

    template<class T>
    inline
    void HashSet<T>::insert(std::initializer_list<value_type> ilist)
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
}

#endif
