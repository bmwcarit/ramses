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

#ifndef RAMSES_CAPU_HASHTABLE_H
#define RAMSES_CAPU_HASHTABLE_H

#include <new>
#include <algorithm>
#include "ramses-capu/Error.h"
#include "ramses-capu/Config.h"
#include "ramses-capu/container/Hash.h"
#include "ramses-capu/os/Memory.h"
#include <cmath>
#include <functional>

namespace ramses_capu
{
    /**
     * Table object container where keys are found and retrieved via hashs.
     */
    template <class Key, class T, class C = std::equal_to<Key>, class H = Hash<Key>>
    class HashTable final
    {
    public:
        /// defines the threshold after which the list will get resized.
        static const double DefaultHashTableMaxLoadFactor;

        /// defines the capacity to use for hash tablesize
        static const uint_t DefaultHashTableCapacity;

        class Pair
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
        class HashTableEntry
        {
        public:
            HashTableEntry()
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
                reinterpret_cast<Pair*>(keyValuePairMemory)->~Pair();
            }

            Pair& getKeyValuePair()
            {
                return *reinterpret_cast<Pair*>(keyValuePairMemory);
            }

            const Pair& getKeyValuePair() const
            {
                return *reinterpret_cast<const Pair*>(keyValuePairMemory);
            }

            HashTableEntry(const HashTableEntry& other)  = delete;
            HashTableEntry& operator=(const HashTableEntry& other) = delete;

        private:
            HashTableEntry* next; // pointer to the next entry (chaining)
            HashTableEntry* previous; // pointer to the previous entry (chaining)
            alignas(Pair) char keyValuePairMemory[sizeof(Pair)];  // properly aligned memory for key and value
            bool isChainElement; // true if the element is not the first element in a chain

            friend class HashTable<Key, T, C, H>;
        };

    public:
        class ConstIterator
        {
        public:

            friend class HashTable;
            friend class Iterator;

            /**
             * Constructor.
             * @param begin Pointer to the entry on which iteration should start.
             * @param end Pointer to an entry on which iteration should end (if begin == end).
             */
            ConstIterator(HashTableEntry* begin)
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
            HashTableEntry* mCurrentHashMapEntry;
        };
        /**
         * Internal helper class to perform iterations over the map entries.
         */
        class Iterator
        {
        public:

            friend class HashTable;

            /**
             * Constructor.
             * @param begin Pointer to the entry on which iteration should start.
             * @param end Pointer to an entry on which iteration should end (if begin == end).
             */
            Iterator(HashTableEntry* begin)
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
            Iterator(const ConstIterator& iter)
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
            HashTableEntry* mCurrentHashMapEntry;
        };



        /**
         * Copy constructor
         */
        HashTable(const HashTable& other);

        HashTable(HashTable&& other);

        /**
         * Constructs HashTable.
         */
        HashTable();

        /**
         * Constructor.
         *                   return NO_MEMORY if too many items were added.
         */
        HashTable(const uint_t capcity);

        /**
         * Destructor.
         */
        ~HashTable();

        /**
         * overloading subscript operator to get read and write access to element referenced by given key.
         *
         * @param key Key value
         * @return value Value referenced by key. If no value is stored for given key, a default constructed object is added and returned
         */
        T& operator[](const Key& key);

        /**
         * put a new value to the hashtable.
         *
         * NOTE: Not STL compatible
         *
         * @param key               Key value
         * @param value             new value that will be put to hash table
         * @return CAPU_OK if put is successful
         *
         */
        Iterator put(const Key& key, const T& value);

        /**
         * Tries to find an element in the Hash Table.
         * If the element is found, it returns an iterator to HashTableEntry for the given key
         * If the element is not contained in the Hash Table, the end() iterator is returned.
         *
         * @param key       Key
         * @return iterator pointing to the Hash Table entry where the key got found
         *         iterator pointing to the end() element otherwise
         */
        Iterator find(const Key& key);

        /**
         * Tries to find an element in read only Hash Table.
         * If the element is found, it returns a ConstIterator to HashTableEntry for the given key
         * If the element is not contained in the Hash Table, the end() constIterator is returned.
         *
         * @param key       Key
         * @return ConstIterator pointing to the Hash Table entry where the key got found
         *         ConstIterator pointing to the end() element otherwise
         */
        ConstIterator find(const Key& key) const;

        /**
         * Checks weather the given key is present in the table.
         *
         * NOTE: Not STL compatible
         *
         * @param key The key.
         * @return True if the key is present, false otherwise.
         */
        bool contains(const Key& key) const;

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
        uint_t count() const;

        /**
         * Clears all keys and values of the hashtable.
         */
        void clear();

        /**
         * Returns an iterator for iterating over the key and values in the map.
         * @return Iterator
         */
        Iterator begin();

        /**
         * Returns a ConstIterator for iterating over the key and values in the map.
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
         * Reserve space for given number of bits elements. Does nothing if the
         * HashTable is already bigger.
         */
        void reserve(uint_t capacity);

        uint_t capacity() const;

        /**
         * Assignment operator for HashTable
         * @param HashTable to copy from
         * @return reference to HashTable with copied data
         */
        HashTable<Key, T, C, H>& operator=(const HashTable<Key, T, C, H>& other);

        HashTable<Key, T, C, H>& operator=(HashTable<Key, T, C, H>&& other);

        /**
        * Swap this HashTable with another
        * @param other HashTable to copy from
        */
       void swap(HashTable<Key, T, C, H>& other);

    private:
        static uint8_t BitSizeFromElementCount(uint_t count);

        uint8_t mBitCount; // bit size for the hash function
        uint_t mSize; // the size of the data list
        uint_t mThreshold; // defines when a rehashing may occur
        HashTableEntry** mBuckets; // bucket list
        HashTableEntry* mData; // placeholder for the data
        HashTableEntry* mLastHashMapEntry; // pointer to the last entry in the map
        HashTableEntry* mFirstFreeHashMapEntry; // start of pointer list of free entries
        uint_t mCount; // the current entry count
        C mComparator; // compares keys

        void initializeLastEntry();
        void destructAll();
        void rehash();
        uint_t calcHashValue(const Key& key) const;
        HashTableEntry* internalGet(const Key& key) const;
        void internalPut(HashTableEntry* entry, uint_t hashValue);
        void internalRemove(HashTableEntry* entry, const uint_t hashValue, T* value_old = nullptr);
    };

    /**
    * swap specialization for HashTable<Key, T, C, H>
    * @param first first HashTable
    * @param second HashTable to swap with first
    */
    template <class Key, class T, class C, class H>
    inline void swap(HashTable<Key, T, C, H>& first, HashTable<Key, T, C, H>& second)
    {
        first.swap(second);
    }

    template <class Key, class T, class C, class H>
    const double HashTable<Key, T, C, H>::DefaultHashTableMaxLoadFactor = 0.8;
    template <class Key, class T, class C, class H>
    const uint_t HashTable<Key, T, C, H>::DefaultHashTableCapacity =
        static_cast<uint_t>(std::ceil(16 * HashTable<Key, T, C, H>::DefaultHashTableMaxLoadFactor));

    template <class Key, class T, class C, class H>
    inline HashTable<Key, T, C, H>::HashTable()
        : mBitCount(BitSizeFromElementCount(DefaultHashTableCapacity))
        , mSize(static_cast<uint_t>(1) << mBitCount)
        , mThreshold(static_cast<uint_t>(mSize * DefaultHashTableMaxLoadFactor))
        , mBuckets(new HashTableEntry*[mSize])
        , mData(new HashTableEntry[mThreshold + 1])  // One dummy for the end
        , mLastHashMapEntry(mData + mThreshold)
        , mFirstFreeHashMapEntry(mData)
        , mCount(0)
        , mComparator()
    {
        Memory::Set(mBuckets, 0, sizeof(HashTableEntry*) * mSize);
        initializeLastEntry();
    }

    template <class Key, class T, class C, class H>
    inline HashTable<Key, T, C, H>::HashTable(const HashTable<Key, T, C, H>& other)
        : mBitCount(other.mBitCount)
        , mSize(other.mSize)
        , mThreshold(other.mThreshold)
        , mBuckets(new HashTableEntry*[mSize])
        , mData(new HashTableEntry[mThreshold + 1])  //One dummy for the end
        , mLastHashMapEntry(mData + mThreshold)
        , mFirstFreeHashMapEntry(mData)
        , mCount(0) // will get increased by internalPut
        , mComparator()
    {
        Memory::Set(mBuckets, 0, sizeof(HashTableEntry*) * mSize);
        initializeLastEntry();

        // right here, we have an empty map with the exact same size as the given other table
        // now we put each entry of the other map into this map (this may be a lot less than the actual size)

        typename HashTable<Key, T, C, H>::ConstIterator iter = other.begin();
        while (iter != other.end())
        {
            mFirstFreeHashMapEntry->constructKeyValue(iter->key, iter->value);
            internalPut(mFirstFreeHashMapEntry, calcHashValue(iter->key));
            ++iter;
            ++mFirstFreeHashMapEntry;
        }
    }

    template <class Key, class T, class C, class H>
    inline HashTable<Key, T, C, H>::HashTable(HashTable<Key, T, C, H>&& other)
        // poor mans move because must HashTable cannot work with nullptr data
        : HashTable()
    {
        swap(other);
        other.clear();
    }

    template <class Key, class T, class C, class H>
    inline HashTable<Key, T, C, H>::HashTable(const uint_t capacity)
        : mBitCount(BitSizeFromElementCount(capacity))
        , mSize(static_cast<uint_t>(1) << mBitCount)
        , mThreshold(static_cast<uint_t>(mSize * DefaultHashTableMaxLoadFactor))
        , mBuckets(new HashTableEntry*[mSize])
        , mData(new HashTableEntry[mThreshold + 1])  //One dummy for the end
        , mLastHashMapEntry(mData + mThreshold)
        , mFirstFreeHashMapEntry(mData)
        , mCount(0)
        , mComparator()
    {
        Memory::Set(mBuckets, 0, sizeof(HashTableEntry*) * mSize);
        initializeLastEntry();
    }

    template <class Key, class T, class C, class H>
    inline HashTable<Key, T, C, H>& HashTable<Key, T, C, H>::operator=(const HashTable<Key, T, C, H>& other)
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
        mBuckets = new HashTableEntry*[mSize];
        mData    = new HashTableEntry[mThreshold + 1];

        Memory::Set(mBuckets, 0, sizeof(HashTableEntry*) * mSize);
        mFirstFreeHashMapEntry = mData;
        mLastHashMapEntry = mData + mThreshold;
        initializeLastEntry();

        HashTable::ConstIterator iter = other.begin();
        while (iter != other.end())
        {
            put(iter->key, iter->value);
            ++iter;
        }

        return *this;
    }

    template <class Key, class T, class C, class H>
    inline HashTable<Key, T, C, H>& HashTable<Key, T, C, H>::operator=(HashTable<Key, T, C, H>&& other)
    {
        if (&other == this)
        {
            // self assignment
            return *this;
        }
        swap(other);
        other.clear();
        return *this;
    }

    template <class Key, class T, class C, class H>
    inline HashTable<Key, T, C, H>::~HashTable()
    {
        destructAll();
        delete[] mBuckets;
        delete[] mData;
    }

    template <class Key, class T, class C, class H>
    inline uint_t HashTable<Key, T, C, H>::count() const
    {
        return mCount;
    }

    template <class Key, class T, class C, class H>
    inline bool HashTable<Key, T, C, H>::contains(const Key& key) const
    {
        return internalGet(key) != nullptr;
    }

    template <class Key, class T, class C, class H>
    inline uint_t HashTable<Key, T, C, H>::calcHashValue(const Key& key) const
    {
        const uint_t hash = H()(key);
        return Resizer::Resize(hash, mBitCount);
    }

    template <class Key, class T, class C, class H>
    inline T& HashTable<Key, T, C, H>::operator[](const Key& key)
    {
        if (HashTableEntry* entry = internalGet(key))
            return entry->getKeyValuePair().value;
        //if key is not in hash table, add default constructed value to it
        return put(key, T())->value;
    }

    template <class Key, class T, class C, class H>
    inline typename HashTable<Key, T, C, H>::Iterator HashTable<Key, T, C, H>::put(const Key& key, const T& value)
    {
        uint_t hashValue = calcHashValue(key);

        // check if we already have the key in the map, if so, just override the value
        HashTableEntry* current = mBuckets[hashValue];
        if (current)
        {
            do
            {
                if (mComparator(current->getKeyValuePair().key, key))
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
        HashTableEntry* newentry = mFirstFreeHashMapEntry;

        // adjust the pointer to the next free entry ('preconnected' through constructor)
        mFirstFreeHashMapEntry = mFirstFreeHashMapEntry->next;

        newentry->constructKeyValue(key, value);

        internalPut(newentry, hashValue);

        return Iterator(newentry);
    }

    template <class Key, class T, class C, class H>
    typename HashTable<Key, T, C, H>::Iterator HashTable<Key, T, C, H>::find(const Key& key)
    {
        if (HashTableEntry* entry = internalGet(key))
        {
            // set new iterator to this position
            Iterator it(entry);
            return it;
        }

        // no entry found -> return end iterator
        return end();
    }

    template <class Key, class T, class C, class H>
    typename HashTable<Key, T, C, H>::ConstIterator HashTable<Key, T, C, H>::find(const Key& key) const
    {
        if (HashTableEntry* entry = internalGet(key))
        {
            // set new iterator to this position
            ConstIterator it(entry);
            return it;
        }

        // no entry found -> return end iterator
        return end();
    }

    template <class Key, class T, class C, class H>
    inline bool HashTable<Key, T, C, H>::remove(const Key& key, T* value_old)
    {
        const uint_t hashValue = calcHashValue(key);
        HashTableEntry* current = mBuckets[hashValue];
        if (current)
        {
            do
            {
                if (mComparator(current->getKeyValuePair().key, key))
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

    template <class Key, class T, class C, class H>
    inline
    typename HashTable<Key, T, C, H>::Iterator HashTable<Key, T, C, H>::remove(Iterator iter, T* value_old)
    {
        HashTableEntry* current = iter.mCurrentHashMapEntry;
        uint_t hashValue = calcHashValue(current->getKeyValuePair().key);

        auto result = current->next;
        internalRemove(current, hashValue, value_old);
        return result;
    }

    template <class Key, class T, class C, class H>
    inline void HashTable<Key, T, C, H>::internalRemove(HashTableEntry* entry, const uint_t hashValue, T* value_old)
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

    template <class Key, class T, class C, class H>
    inline void HashTable<Key, T, C, H>::clear()
    {
        // destruct all valid values
        destructAll();

        // reset all buckets and link entries to empty list
        Memory::Set(mBuckets, 0, sizeof(HashTableEntry*) * mSize);
        HashTableEntry* entry = mData;
        for (uint32_t i = 0; i < mThreshold + 1; ++i)
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

    template <class Key, class T, class C, class H>
    inline typename HashTable<Key, T, C, H>::Iterator HashTable<Key, T, C, H>::begin()
    {
        return typename HashTable<Key, T, C, H>::Iterator(mLastHashMapEntry->next);
    }

    template <class Key, class T, class C, class H>
    inline typename HashTable<Key, T, C, H>::ConstIterator HashTable<Key, T, C, H>::begin() const
    {
        return typename HashTable<Key, T, C, H>::ConstIterator(mLastHashMapEntry->next);
    }

    template <class Key, class T, class C, class H>
    inline typename HashTable<Key, T, C, H>::Iterator HashTable<Key, T, C, H>::end()
    {
        return typename HashTable<Key, T, C, H>::Iterator(mLastHashMapEntry);
    }

    template <class Key, class T, class C, class H>
    inline typename HashTable<Key, T, C, H>::ConstIterator HashTable<Key, T, C, H>::end() const
    {
        return typename HashTable<Key, T, C, H>::ConstIterator(mLastHashMapEntry);
    }

    template <class Key, class T, class C, class H>
    inline typename HashTable<Key, T, C, H>::HashTableEntry* HashTable<Key, T, C, H>::internalGet(const Key& key) const
    {
        const uint_t hashValue = calcHashValue(key);
        HashTableEntry* current = mBuckets[hashValue];
        if (current)
        {
            do
            {
                if (mComparator(current->getKeyValuePair().key, key))
                {
                    return current;
                }
                current = current->next;
            }
            while (current->isChainElement);
        }

        return nullptr;
    }

    template <class Key, class T, class C, class H>
    inline void HashTable<Key, T, C, H>::internalPut(HashTableEntry* newentry, const uint_t hashValue)
    {
        // chaining of entries
        HashTableEntry* entry = mBuckets[hashValue];
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

    template <class Key, class T, class C, class H>
    inline void HashTable<Key, T, C, H>::reserve(uint_t requestedCapacity)
    {
        if (requestedCapacity <= capacity())
        {
            return;
        }

        HashTable<Key, T, C, H> tmp(requestedCapacity);
        for (Iterator it = begin(); it != end(); ++it)
        {
            tmp.put(it->key, it->value);
        }
        swap(tmp);
    }

    template <class Key, class T, class C, class H>
    inline uint_t HashTable<Key, T, C, H>::capacity() const
    {
        return mThreshold;
    }

    template <class Key, class T, class C, class H>
    inline void HashTable<Key, T, C, H>::rehash()
    {
        reserve(static_cast<uint_t>(1) << (mBitCount + 1));
    }

    template <class Key, class T, class C, class H>
    inline void HashTable<Key, T, C, H>::initializeLastEntry()
    {
        mLastHashMapEntry->previous = mLastHashMapEntry;
        mLastHashMapEntry->next = mLastHashMapEntry;
    }

    template <class Key, class T, class C, class H>
        inline void HashTable<Key, T, C, H>::destructAll()
    {
        // destruct regular values
        HashTableEntry* entry = mLastHashMapEntry->next;
        while (entry != mLastHashMapEntry)
        {
            entry->destructKeyValue();
            entry = entry->next;
        }
    }

    template <class Key, class T, class C, class H>
    inline void HashTable<Key, T, C, H>::swap(HashTable<Key, T, C, H>& other)
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
        // no need to swap mComparator, it must be the same
    }

    template <class Key, class T, class C, class H>
    inline uint8_t HashTable<Key, T, C, H>::BitSizeFromElementCount(uint_t count)
    {
        const uint_t countRespectingLoadFactor = static_cast<uint_t>(std::ceil(count / DefaultHashTableMaxLoadFactor));
        return static_cast<uint8_t>(std::ceil(std::log2(countRespectingLoadFactor)));
    }
}

#endif // RAMSES_CAPU_HASHTABLE_H
