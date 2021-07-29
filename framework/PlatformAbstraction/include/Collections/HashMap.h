//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COLLECTIONS_HASHMAP_H
#define RAMSES_COLLECTIONS_HASHMAP_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformError.h"
#include "PlatformAbstraction/Hash.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/Macros.h"
#include <cmath>
#include <functional>
#include <new>
#include <algorithm>

namespace ramses_internal
{
    /**
     * Table object container where keys are found and retrieved via hashs.
     */
    template <class Key, class T>
    class HashMap final
    {
    public:
        /// defines the threshold after which the list will get resized.
        static const double DefaultHashMapMaxLoadFactor;

        /// defines the capacity to use for hash tablesize
        static const size_t DefaultHashMapCapacity;

        class Pair final
        {
        public:
            Pair(const Key& key_, const T& value_)
                : key(key_)
                , value(value_)
            {
            }

            const Key key;
            T value;
        };

    private:
        /**
         * Data structure to hold a key/value pair inside the hash table
         */
        class HashMapEntry final
        {
        public:
            HashMapEntry()
                : next(nullptr)
                , previous(nullptr)
                , isChainElement(false)
            {
                // 'preconnect' free entries
                next = this + 1;
            }

            void constructKeyValue(const Key& key, const T& value)
            {
                // placement new
                new (keyValuePairMemory) Pair(key, value);
            }

            void destructKeyValue()
            {
                // inplace destruction of typed value
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) uses valid object in keyValuePairMemory
                reinterpret_cast<Pair*>(keyValuePairMemory)->~Pair();
            }

            Pair& getKeyValuePair()
            {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) uses valid object in keyValuePairMemory
                return *reinterpret_cast<Pair*>(keyValuePairMemory);
            }

            const Pair& getKeyValuePair() const
            {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) uses valid object in keyValuePairMemory
                return *reinterpret_cast<const Pair*>(keyValuePairMemory);
            }

            HashMapEntry(const HashMapEntry& other)  = delete;
            HashMapEntry& operator=(const HashMapEntry& other) = delete;

        private:
            HashMapEntry* next; // pointer to the next entry (chaining)
            HashMapEntry* previous; // pointer to the previous entry (chaining)
            alignas(Pair) char keyValuePairMemory[sizeof(Pair)];  // properly aligned memory for key and value
            bool isChainElement; // true if the element is not the first element in a chain

            friend class HashMap<Key, T>;
        };

    public:
        class ConstIterator final
        {
        public:

            friend class HashMap;
            friend class Iterator;

            /**
             * Constructor.
             * @param begin Pointer to the entry on which iteration should start.
             * @param end Pointer to an entry on which iteration should end (if begin == end).
             */
            explicit ConstIterator(HashMapEntry* begin)
                : mCurrentHashMapEntry(begin)
            {
            }

            /**
             * Copy Constructor.
             * @param iter non-const iterator to copy
             */
            ConstIterator(const ConstIterator& iter)
                : mCurrentHashMapEntry(iter.mCurrentHashMapEntry)
            {
            }

            ConstIterator& operator=(const ConstIterator&) = default;

            /**
             * Indirection
             * @return the current value referenced by the iterator
             */
            const Pair& operator*() const
            {
                return mCurrentHashMapEntry->getKeyValuePair();
            }

            /**
             * Dereference
             * @return a pointer to the current object the iterator points to
             */
            const Pair* operator->() const
            {
                return &mCurrentHashMapEntry->getKeyValuePair();
            }

            /**
             * Compares two iterators
             * @return true if the iterators point to the same position
             */
            bool operator==(const ConstIterator& iter) const
            {
                return (mCurrentHashMapEntry == iter.mCurrentHashMapEntry);
            }

            /**
             * Compares two iterators
             * @return true if the iterators do not point to the same position
             */
            bool operator!=(const ConstIterator& iter) const
            {
                return (mCurrentHashMapEntry != iter.mCurrentHashMapEntry);
            }

            /**
             * Step the iterator forward to the next element (prefix operator)
             * @return the next iterator
             */
            ConstIterator& operator++()
            {
                mCurrentHashMapEntry = mCurrentHashMapEntry->next;
                return *this;
            }

            /**
             * Step the iterator forward to the next element (postfix operator)
             * @return the next iterator
             */
            const ConstIterator operator++(int32_t)
            {
                ConstIterator oldValue(*this);
                ++(*this);
                return oldValue;
            }

        private:
            HashMapEntry* mCurrentHashMapEntry;
        };
        /**
         * Internal helper class to perform iterations over the map entries.
         */
        class Iterator final
        {
        public:

            friend class HashMap;

            /**
             * Constructor.
             * @param begin Pointer to the entry on which iteration should start.
             * @param end Pointer to an entry on which iteration should end (if begin == end).
             */
            explicit Iterator(HashMapEntry* begin)
                : mCurrentHashMapEntry(begin)
            {
            }

            /**
             * Copy Constructor.
             * @param iter non-const iterator to copy
             */
            Iterator(const Iterator& iter)
                : mCurrentHashMapEntry(iter.mCurrentHashMapEntry)
            {
            }

            /**
             * Convert Constructor
             * @param iter ConstIterator to convert from
             */
            Iterator(const ConstIterator& iter)  // NOLINT(google-explicit-constructor) const to non-const iterator should be implicit
                : mCurrentHashMapEntry(iter.mCurrentHashMapEntry)
            {
            }

            Iterator& operator=(const Iterator&) = default;

            /**
             * Indirection
             * @return the current value referenced by the iterator
             */
            Pair& operator*()
            {
                return mCurrentHashMapEntry->getKeyValuePair();
            }

            const Pair& operator*() const
            {
                return mCurrentHashMapEntry->getKeyValuePair();
            }

            /**
             * Dereference
             * @return a pointer to the current object the iterator points to
             */
            Pair* operator->()
            {
                return &mCurrentHashMapEntry->getKeyValuePair();
            }

            const Pair* operator->() const
            {
                return &mCurrentHashMapEntry->getKeyValuePair();
            }

            /**
             * Compares two iterators
             * @return true if the iterators point to the same position
             */
            bool operator==(const Iterator& iter) const
            {
                return (mCurrentHashMapEntry == iter.mCurrentHashMapEntry);
            }

            /**
             * Compares two iterators
             * @return true if the iterators do not point to the same position
             */
            bool operator!=(const Iterator& iter) const
            {
                return (mCurrentHashMapEntry != iter.mCurrentHashMapEntry);
            }

            /**
             * Step the iterator forward to the next element (prefix operator)
             * @return the next iterator
             */
            Iterator& operator++()
            {
                mCurrentHashMapEntry = mCurrentHashMapEntry->next;
                return *this;
            }

            /**
             * Step the iterator forward to the next element (postfix operator)
             * @return the next iterator
             */
            const Iterator operator++(int32_t)
            {
                Iterator oldValue(*this);
                ++(*this);
                return oldValue;
            }

        private:
            HashMapEntry* mCurrentHashMapEntry;
        };


        /**
         * Copy constructor
         */
        HashMap(const HashMap& other);

        HashMap(HashMap&& other) noexcept;

        /**
         * Constructs HashMap.
         */
        HashMap();

        /**
         * Constructor.
         * Always allocated at least DefaultHashMapCapacity even when less is requested.
         */
        explicit HashMap(const size_t minimumCapacity);

        /**
         * Destructor.
         */
        ~HashMap();

        /**
         * overloading subscript operator to get read and write access to element referenced by given key.
         *
         * @param key Key value
         * @return value Value referenced by key. If no value is stored for given key, a default constructed object is added and returned
         */
        T& operator[](const Key& key);

        Iterator put(const Key& key, const T& value);

        EStatus get(const Key& key, T& value) const;
        RNODISCARD T*      get(const Key& key) const;

        /**
         * Tries to find an element in the Hash Table.
         * If the element is found, it returns an iterator to HashMapEntry for the given key
         * If the element is not contained in the Hash Table, the end() iterator is returned.
         *
         * @param key       Key
         * @return iterator pointing to the Hash Table entry where the key got found
         *         iterator pointing to the end() element otherwise
         */
        RNODISCARD Iterator find(const Key& key);

        /**
         * Tries to find an element in read only Hash Table.
         * If the element is found, it returns a ConstIterator to HashMapEntry for the given key
         * If the element is not contained in the Hash Table, the end() constIterator is returned.
         *
         * @param key       Key
         * @return ConstIterator pointing to the Hash Table entry where the key got found
         *         ConstIterator pointing to the end() element otherwise
         */
        RNODISCARD ConstIterator find(const Key& key) const;

        /**
         * Checks weather the given key is present in the table.
         *
         * NOTE: Not STL compatible
         *
         * @param key The key.
         * @return True if the key is present, false otherwise.
         */
        RNODISCARD bool contains(const Key& key) const;

        /**
         * Removes the value associated with key in the hashtable.
         *
         * NOTE: Not STL compatible
         *
         * @param key               Key value.
         * @param value_old         Buffer which will be used to store value of removed element.
         *                          Default value is nullptr to indicate that it should be discarded.
         *
         * @return true if remove is successful
         *         false if the key was not found in the map.
         */
        bool remove(const Key& key, T* value_old = nullptr);

        /**
         * Remove the element where the iterator is pointing to
         * @param the iterator to the element to remove
         * @param out parameter to the removed element
         * @return iterator after removed element
         */
        Iterator remove(Iterator iter, T* value_old = nullptr);

        /**
         * Returns count of the hashtable.
         * @return number of elements in hash table
         */
        RNODISCARD size_t size() const;

        /**
         * Clears all keys and values of the hashtable.
         */
        void clear();

        /**
         * Returns an iterator for iterating over the key and values in the map.
         * @return Iterator
         */
        RNODISCARD Iterator begin();

        /**
         * Returns a ConstIterator for iterating over the key and values in the map.
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
         * Reserve space for given number of bits elements. Does nothing if the
         * HashMap is already bigger.
         */
        void reserve(size_t minimumCapacity);

        RNODISCARD size_t capacity() const;

        /**
         * Assignment operator for HashMap
         * @param HashMap to copy from
         * @return reference to HashMap with copied data
         */
        HashMap<Key, T>& operator=(const HashMap<Key, T>& other);

        HashMap<Key, T>& operator=(HashMap<Key, T>&& other) noexcept;

        /**
        * Swap this HashMap with another
        * @param other HashMap to copy from
        */
        void swap(HashMap<Key, T>& other);

    private:
        static uint8_t BitSizeFromElementCount(size_t count);

        uint8_t mBitCount; // bit size for the hash function
        size_t mSize; // the size of the data list
        size_t mThreshold; // defines when a rehashing may occur
        HashMapEntry** mBuckets; // bucket list
        HashMapEntry* mData; // placeholder for the data
        HashMapEntry* mLastHashMapEntry; // pointer to the last entry in the map
        HashMapEntry* mFirstFreeHashMapEntry; // start of pointer list of free entries
        size_t mCount; // the current entry count

        void initializeLastEntry();
        void destructAll();
        void rehash();
        size_t calcHashValue(const Key& key) const;
        HashMapEntry* internalGet(const Key& key) const;
        void internalPut(HashMapEntry* entry, size_t hashValue);
        void internalRemove(HashMapEntry* entry, const size_t hashValue, T* value_old = nullptr);
    };

    /**
    * swap specialization for HashMap<Key, T>
    * @param first first HashMap
    * @param second HashMap to swap with first
    */
    template <class Key, class T>
    inline void swap(HashMap<Key, T>& first, HashMap<Key, T>& second)
    {
        first.swap(second);
    }

    template <class Key, class T>
    const double HashMap<Key, T>::DefaultHashMapMaxLoadFactor = 0.8;
    template <class Key, class T>
    const size_t HashMap<Key, T>::DefaultHashMapCapacity =
        static_cast<size_t>(std::ceil(16 * HashMap<Key, T>::DefaultHashMapMaxLoadFactor));

    template <class Key, class T>
    inline HashMap<Key, T>::HashMap()
        : mBitCount(BitSizeFromElementCount(DefaultHashMapCapacity))
        , mSize(static_cast<size_t>(1) << mBitCount)
        , mThreshold(static_cast<size_t>(mSize * DefaultHashMapMaxLoadFactor))
        , mBuckets(new HashMapEntry*[mSize])
        , mData(new HashMapEntry[mThreshold + 1])  // One dummy for the end
        , mLastHashMapEntry(mData + mThreshold)
        , mFirstFreeHashMapEntry(mData)
        , mCount(0)
    {
        PlatformMemory::Set(mBuckets, 0, sizeof(HashMapEntry*) * mSize);  //NOLINT(bugprone-sizeof-expression) sizeof is really a pointer type here
        initializeLastEntry();
    }

    template <class Key, class T>
    inline HashMap<Key, T>::HashMap(const HashMap<Key, T>& other)
        : mBitCount(other.mBitCount)
        , mSize(other.mSize)
        , mThreshold(other.mThreshold)
        , mBuckets(new HashMapEntry*[mSize])
        , mData(new HashMapEntry[mThreshold + 1])  //One dummy for the end
        , mLastHashMapEntry(mData + mThreshold)
        , mFirstFreeHashMapEntry(mData)
        , mCount(0) // will get increased by internalPut
    {
        PlatformMemory::Set(mBuckets, 0, sizeof(HashMapEntry*) * mSize);  //NOLINT(bugprone-sizeof-expression) sizeof is really a pointer type here
        initializeLastEntry();

        // right here, we have an empty map with the exact same size as the given other table
        // now we put each entry of the other map into this map (this may be a lot less than the actual size)

        typename HashMap<Key, T>::ConstIterator iter = other.begin();
        while (iter != other.end())
        {
            mFirstFreeHashMapEntry->constructKeyValue(iter->key, iter->value);
            internalPut(mFirstFreeHashMapEntry, calcHashValue(iter->key));
            ++iter;
            ++mFirstFreeHashMapEntry;
        }
    }

    template <class Key, class T>
    inline HashMap<Key, T>::HashMap(HashMap<Key, T>&& other) noexcept
        // poor mans move because must HashMap cannot work with nullptr data
        : HashMap()
    {
        static_assert(std::is_nothrow_move_constructible<HashMap>::value, "HashMap must be movable");

        swap(other);
        other.clear();
    }

    template <class Key, class T>
    inline HashMap<Key, T>::HashMap(const size_t minimumCapacity)
        : mBitCount(BitSizeFromElementCount(minimumCapacity))
        , mSize(static_cast<size_t>(1) << mBitCount)
        , mThreshold(static_cast<size_t>(mSize * DefaultHashMapMaxLoadFactor))
        , mBuckets(new HashMapEntry*[mSize])
        , mData(new HashMapEntry[mThreshold + 1])  //One dummy for the end
        , mLastHashMapEntry(mData + mThreshold)
        , mFirstFreeHashMapEntry(mData)
        , mCount(0)
    {
        PlatformMemory::Set(mBuckets, 0, sizeof(HashMapEntry*) * mSize);  //NOLINT(bugprone-sizeof-expression) sizeof is really a pointer type here
        initializeLastEntry();
    }

    template <class Key, class T>
    inline HashMap<Key, T>& HashMap<Key, T>::operator=(const HashMap<Key, T>& other)
    {
        if (&other == this)
        {
            // self assignment
            return *this;
        }
        destructAll();

        delete[] mBuckets;
        delete[] mData;

        mCount = 0;
        mSize = other.mSize;
        mBitCount = other.mBitCount;
        mThreshold = other.mThreshold;
        mBuckets = new HashMapEntry*[mSize];
        mData    = new HashMapEntry[mThreshold + 1];

        PlatformMemory::Set(mBuckets, 0, sizeof(HashMapEntry*) * mSize);  //NOLINT(bugprone-sizeof-expression) sizeof is really a pointer type here
        mFirstFreeHashMapEntry = mData;
        mLastHashMapEntry = mData + mThreshold;
        initializeLastEntry();

        HashMap::ConstIterator iter = other.begin();
        while (iter != other.end())
        {
            put(iter->key, iter->value);
            ++iter;
        }

        return *this;
    }

    template <class Key, class T>
    inline HashMap<Key, T>& HashMap<Key, T>::operator=(HashMap<Key, T>&& other) noexcept
    {
        static_assert(std::is_nothrow_move_assignable<HashMap>::value, "HashMap must be movable");

        if (&other == this)
        {
            // self assignment
            return *this;
        }
        swap(other);
        other.clear();
        return *this;
    }

    template <class Key, class T>
    inline HashMap<Key, T>::~HashMap()
    {
        destructAll();
        delete[] mBuckets;
        delete[] mData;
    }

    template <class Key, class T>
    inline size_t HashMap<Key, T>::size() const
    {
        return mCount;
    }

    template <class Key, class T>
    inline bool HashMap<Key, T>::contains(const Key& key) const
    {
        return internalGet(key) != nullptr;
    }

    template <class Key, class T>
    inline size_t HashMap<Key, T>::calcHashValue(const Key& key) const
    {
        size_t hash = 0;
        // hash and shuffle bits around a bit to counteract effect of truncating upper bits only
        HashCombine(hash, key);
        if (mBitCount < 8*sizeof(size_t))
            hash &= ((static_cast<size_t>(1) << mBitCount) - 1);
        return hash;
    }

    template <class Key, class T>
    inline T& HashMap<Key, T>::operator[](const Key& key)
    {
        if (HashMapEntry* entry = internalGet(key))
            return entry->getKeyValuePair().value;
        //if key is not in hash table, add default constructed value to it
        return put(key, T())->value;
    }

    template <class Key, class T>
    inline typename HashMap<Key, T>::Iterator HashMap<Key, T>::put(const Key& key, const T& value)
    {
        size_t hashValue = calcHashValue(key);

        // check if we already have the key in the map, if so, just override the value
        HashMapEntry* current = mBuckets[hashValue];
        if (current)
        {
            do
            {
                if (current->getKeyValuePair().key == key)
                {
                    current->getKeyValuePair().value = value;
                    return Iterator(current);
                }
                current = current->next;
            }
            while (current->isChainElement);
        }

        // check if the next free entry is outside of the threshold (resizing would be necessary)
        if (mFirstFreeHashMapEntry == mData + mThreshold)
        {
            rehash();
            hashValue = calcHashValue(key); // calculate the new index (in the resized map)
        }

        // the entry holding the values is the first free entry
        HashMapEntry* newentry = mFirstFreeHashMapEntry;

        // adjust the pointer to the next free entry ('preconnected' through constructor)
        mFirstFreeHashMapEntry = mFirstFreeHashMapEntry->next;

        newentry->constructKeyValue(key, value);

        internalPut(newentry, hashValue);

        return Iterator(newentry);
    }

    template<class Key, class T>
    inline
    EStatus HashMap<Key, T>::get(const Key& key, T& value)  const
    {
        auto iter = find(key);
        if (iter == end())
            return EStatus::NotExist;
        value = iter->value;
        return EStatus::Ok;
    }

    template<class Key, class T>
    inline
    T* HashMap<Key, T>::get(const Key& key) const
    {
        auto iter = find(key);
        if (iter != end())
            return const_cast<T*>(&iter->value);   // TODO(tobias) constness is broken and has to be fixed
        return nullptr;
    }

    template <class Key, class T>
    typename HashMap<Key, T>::Iterator HashMap<Key, T>::find(const Key& key)
    {
        if (HashMapEntry* entry = internalGet(key))
        {
            // set new iterator to this position
            Iterator it(entry);
            return it;
        }

        // no entry found -> return end iterator
        return end();
    }

    template <class Key, class T>
    typename HashMap<Key, T>::ConstIterator HashMap<Key, T>::find(const Key& key) const
    {
        if (HashMapEntry* entry = internalGet(key))
        {
            // set new iterator to this position
            ConstIterator it(entry);
            return it;
        }

        // no entry found -> return end iterator
        return end();
    }

    template <class Key, class T>
    inline bool HashMap<Key, T>::remove(const Key& key, T* value_old)
    {
        const size_t hashValue = calcHashValue(key);
        HashMapEntry* current = mBuckets[hashValue];
        if (current)
        {
            do
            {
                if (current->getKeyValuePair().key == key)
                {
                    internalRemove(current, hashValue, value_old);

                    // done
                    return true;
                }
                current = current->next;
            }
            while (current->isChainElement);
        }

        // element was not found
        return false;
    }

    template <class Key, class T>
    inline
    typename HashMap<Key, T>::Iterator HashMap<Key, T>::remove(Iterator iter, T* value_old)
    {
        HashMapEntry* current = iter.mCurrentHashMapEntry;
        size_t hashValue = calcHashValue(current->getKeyValuePair().key);

        auto result = current->next;
        internalRemove(current, hashValue, value_old);
        return Iterator{result};
    }

    template <class Key, class T>
    inline void HashMap<Key, T>::internalRemove(HashMapEntry* entry, const size_t hashValue, T* value_old)
    {
        if (value_old)
        {
            // perform the copy operation into the old value
            *value_old = entry->getKeyValuePair().value;
        }

        // destruct the value
        entry->destructKeyValue();

        // change the pointer to point to the next element,
        // so that the current element is taken out of the chain
        entry->previous->next = entry->next;
        entry->next->previous = entry->previous;
        if (mBuckets[hashValue] == entry)
        {
            mBuckets[hashValue] = entry->next->isChainElement ? entry->next : nullptr;
            entry->next->isChainElement = false;
        }

        // connect the unused entries:
        // our next entry is the current first unused entry
        // and we are the new first free entry.
        entry->next          = mFirstFreeHashMapEntry;
        mFirstFreeHashMapEntry = entry;
        mFirstFreeHashMapEntry->previous = mLastHashMapEntry;

        // decrease map count and adjust begin pointer
        --mCount;
    }

    template <class Key, class T>
    inline void HashMap<Key, T>::clear()
    {
        // destruct all valid values
        destructAll();

        // reset all buckets and link entries to empty list
        PlatformMemory::Set(mBuckets, 0, sizeof(HashMapEntry*) * mSize);  //NOLINT(bugprone-sizeof-expression) sizeof is really a pointer type here
        HashMapEntry* entry = mData;
        for (size_t i = 0; i < mThreshold + 1; ++i)
        {
            entry->previous = nullptr;
            entry->next = entry + 1;
            entry->isChainElement = false;
            ++entry;
        }

        // init free and last
        mFirstFreeHashMapEntry = mData;
        initializeLastEntry();
        mCount = 0;
    }

    template <class Key, class T>
    inline typename HashMap<Key, T>::Iterator HashMap<Key, T>::begin()
    {
        return typename HashMap<Key, T>::Iterator(mLastHashMapEntry->next);
    }

    template <class Key, class T>
    inline typename HashMap<Key, T>::ConstIterator HashMap<Key, T>::begin() const
    {
        return typename HashMap<Key, T>::ConstIterator(mLastHashMapEntry->next);
    }

    template <class Key, class T>
    inline typename HashMap<Key, T>::Iterator HashMap<Key, T>::end()
    {
        return typename HashMap<Key, T>::Iterator(mLastHashMapEntry);
    }

    template <class Key, class T>
    inline typename HashMap<Key, T>::ConstIterator HashMap<Key, T>::end() const
    {
        return typename HashMap<Key, T>::ConstIterator(mLastHashMapEntry);
    }

    template <class Key, class T>
    inline typename HashMap<Key, T>::HashMapEntry* HashMap<Key, T>::internalGet(const Key& key) const
    {
        const size_t hashValue = calcHashValue(key);
        HashMapEntry* current = mBuckets[hashValue];
        if (current)
        {
            do
            {
                if (current->getKeyValuePair().key == key)
                {
                    return current;
                }
                current = current->next;
            }
            while (current->isChainElement);
        }

        return nullptr;
    }

    template <class Key, class T>
    inline void HashMap<Key, T>::internalPut(HashMapEntry* newentry, const size_t hashValue)
    {
        // chaining of entries
        HashMapEntry* entry = mBuckets[hashValue];
        if (entry)
        {
            // chaining
            entry->isChainElement = true;
        }
        else
        {
            // we hit a free bucket
            entry = mLastHashMapEntry->next;
        }

        mBuckets[hashValue]   = newentry;
        newentry->next        = entry;
        newentry->previous    = entry->previous;
        newentry->isChainElement = false;
        entry->previous->next = newentry;
        entry->previous       = newentry;
        ++mCount;
    }

    template <class Key, class T>
    inline void HashMap<Key, T>::reserve(size_t requestedCapacity)
    {
        if (requestedCapacity <= capacity())
        {
            return;
        }

        HashMap<Key, T> tmp(requestedCapacity);
        for (Iterator it = begin(); it != end(); ++it)
        {
            tmp.put(it->key, it->value);
        }
        swap(tmp);
    }

    template <class Key, class T>
    inline size_t HashMap<Key, T>::capacity() const
    {
        return mThreshold;
    }

    template <class Key, class T>
    inline void HashMap<Key, T>::rehash()
    {
        reserve(static_cast<size_t>(1) << (mBitCount + 1u));
    }

    template <class Key, class T>
    inline void HashMap<Key, T>::initializeLastEntry()
    {
        mLastHashMapEntry->previous = mLastHashMapEntry;
        mLastHashMapEntry->next = mLastHashMapEntry;
    }

    template <class Key, class T>
        inline void HashMap<Key, T>::destructAll()
    {
        // destruct regular values
        HashMapEntry* entry = mLastHashMapEntry->next;
        while (entry != mLastHashMapEntry)
        {
            entry->destructKeyValue();
            entry = entry->next;
        }
    }

    template <class Key, class T>
    inline void HashMap<Key, T>::swap(HashMap<Key, T>& other)
    {
        using std::swap;
        swap(mBitCount, other.mBitCount);
        swap(mSize, other.mSize);
        swap(mThreshold, other.mThreshold);
        swap(mBuckets, other.mBuckets);
        swap(mData, other.mData);
        swap(mLastHashMapEntry, other.mLastHashMapEntry);
        swap(mFirstFreeHashMapEntry, other.mFirstFreeHashMapEntry);
        swap(mCount, other.mCount);
    }

    template <class Key, class T>
    inline uint8_t HashMap<Key, T>::BitSizeFromElementCount(size_t count)
    {
        // HashMap always allocates memory even for 0 elements. To reduce allocation cost
        // never go below default capacity (also count==0 would fail in calculation)
        if (count < DefaultHashMapCapacity)
            count = DefaultHashMapCapacity;
        const size_t countRespectingLoadFactor = static_cast<size_t>(std::ceil(count / DefaultHashMapMaxLoadFactor));
        return static_cast<uint8_t>(std::ceil(std::log2(countRespectingLoadFactor)));
    }
}

#endif
