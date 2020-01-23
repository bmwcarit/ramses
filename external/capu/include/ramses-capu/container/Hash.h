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

#ifndef RAMSES_CAPU_HASH_H
#define RAMSES_CAPU_HASH_H

#include "ramses-capu/Config.h"
#include "ramses-capu/util/Traits.h"
#include <type_traits>
#include <assert.h>
#include <memory>

namespace ramses_capu
{
    /*************************************************************************************/
    /****************************** Resizer **********************************************/
    /*************************************************************************************/

    struct Resizer
    {
        static uint_t Resize(uint_t hashValue, uint8_t bitcount)
        {
            return bitcount >= 8*sizeof(uint_t) ? hashValue : hashValue & ((static_cast<uint_t>(1) << bitcount) - 1);
        }
    };

    template <typename T>
    struct Hash;

    // HashCombine and HashVal(ue) (modeled according to standard proposal n3876)
    template <typename T>
    inline void HashCombine(uint_t& seed, const T& value)
    {
        seed ^= Hash<T>()(value)
            + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename T, typename... Types>
    inline void HashCombine(uint_t& seed, const T& value, const Types&... args)
    {
        HashCombine(seed, value);
        HashCombine(seed, args...);
    }

    template <typename... Types>
    inline uint_t HashValue(const Types&... args)
    {
        uint_t seed = 0;
        HashCombine(seed, args...);
        return seed;
    }

    namespace internal
    {
        template<typename T>
        struct HashCalculator;

        /**
         * Trait for Hash function which returns a uint32_t value for different types of keys
         */
        template<>
        struct HashCalculator<uint32_t>
        {
            //FNV Has prime numbers for 32 bit
            /**
             * A prime number for use in hash functions
             */
            static const uint32_t prime = 16777619UL;

            /**
             * Offset used in hash functions
             */
            static const uint32_t offset_base = 2166136261UL;

            //Car IT Hash
            /**
             * Compute a hashvalue for the given key.
             * @param key The key to hash
             * @return the computed hash value
             */
            static uint32_t Hash(const uint32_t key)
            {
                return key ^ (((key >> 16) ^ key) >> 8);
            }

            //Car IT Hash
            /**
             * Compute a hashvalue for the given key.
             * @param key The key to hash
             * @return the computed hash value
             */
            static uint32_t Hash(const int32_t key)
            {
                return key ^ (((key >> 16) ^ key) >> 8);
            }

            //FNV Hash
            /**
             * Compute a hashvalue for the given key.
             * @param key The key to hash
             * @return the computed hash value
             */
            static uint32_t Hash(const uint64_t key)
            {
                uint32_t result = offset_base;
                const void* keyPtr = &key;
                const uint32_t* ptr = static_cast<const uint32_t*>(keyPtr);
                result = (result ^ *ptr) * prime;
                ++ptr;
                return (result ^ *ptr) * prime;
            }

            //FNV Hash
            /**
             * Compute a hashvalue for the given key.
             * @param key The key to hash
             * @return the computed hash value
             */
            static uint32_t Hash(const int64_t key)
            {
                uint32_t result = offset_base;
                const void* keyPtr = &key;
                const int32_t* ptr = static_cast<const int32_t*>(keyPtr);
                result = (result ^ *ptr) * prime;
                ++ptr;
                return (result ^ *ptr) * prime;
            }

            //FNV Hash
            /**
             * Compute a hashvalue for the given key.
             * @param key The key to hash
             * @param len Number of bytes to hash from the given key pointer
             * @return the computed hash value
             */
            static uint32_t Hash(const void* key, uint_t len)
            {
                const Byte* ptr = static_cast<const Byte*>(key);
                uint32_t result = offset_base;
                for (uint_t i = 0; i < len; ++i)
                {
                    result = (result ^ ptr[i]) * prime;
                }
                return result;
            }
        };

        /**
         * Trait for Hash function which returns a uint64_t value for different types of keys
         */
        template<>
        struct HashCalculator<uint64_t>
        {
            //FNV Has prime numbers for 64 bit
            /**
             * A prime number for use in hash functions
             */
            static const uint64_t prime = 1099511628211ULL;

            /**
             * Offset used in hash functions
             */
            static const uint64_t offset_base = 14695981039346656037ULL;

            //Car IT Hash
            /**
             * Compute a hashvalue for the given key.
             * @param key The key to hash
             * @return the computed hash value
             */
            static uint64_t Hash(const uint64_t key)
            {
                return key ^ (((((key >> 32) ^ key) >> 16) ^ key) >> 8);
            }

            //Car IT Hash
            /**
             * Compute a hashvalue for the given key.
             * @param key The key to hash
             * @return the computed hash value
             */
            static uint64_t Hash(const int64_t key)
            {
                return key ^ (((((key >> 32) ^ key) >> 16) ^ key) >> 8);
            }

            static uint64_t Hash(const uint32_t key)
            {
                return Hash(static_cast<uint64_t>(key));
            }

            static uint64_t Hash(const int32_t key)
            {
                return Hash(static_cast<int64_t>(key));
            }

            //FNV Hash
            /**
             * Compute a hashvalue for the given key.
             * @param key Pointer to the key data to hash
             * @param len The length of the key data to hash
             * @return the computed hash value
             */
            static uint64_t Hash(const void* key, uint_t len)
            {
                const Byte* ptr = static_cast<const Byte*>(key);
                uint64_t result = offset_base;
                for (uint_t i = 0; i < len; ++i)
                {
                    result = (result ^ ptr[i]) * prime;
                }
                return result;
            }
        };


        // Hasher
        template<typename T, int TYPE>
        struct Hasher
        {
            static uint_t Hash(const T& key)
            {
                return std::hash<T>()(key);
            }
        };

        template<typename T>
        struct Hasher<T, CAPU_TYPE_PRIMITIVE>
        {
            static uint_t Hash(const T key)
            {
                // hasher for primitives
                return HashCalculator<uint_t>::Hash(key);
            }
        };

        template<typename T, uint_t N>
        struct Hasher<T[N], CAPU_TYPE_CLASS>
        {
            static uint_t Hash(const T key[N])
            {
                uint_t result = 0;
                for (uint_t i = 0; i < N; ++i)
                {
                    HashCombine(result, key[i]);
                }
                return result;
            }
        };

        template<typename T>
        struct Hasher<T, CAPU_TYPE_ENUM>
        {
            static uint_t Hash(const T key)
            {
                return HashCalculator<uint_t>::Hash(static_cast<typename std::underlying_type<T>::type>(key));
            }
        };

        template<typename T>
        struct Hasher<T, CAPU_TYPE_POINTER>
        {
            static uint_t Hash(const T key)
            {
                return HashCalculator<uint_t>::Hash(reinterpret_cast<std::uintptr_t>(key));
            }
        };
    }

    // Hash
    template <typename T>
    struct Hash
    {
        uint_t operator()(const T& key)
        {
            return internal::Hasher<T, Type<T>::Identifier>::Hash(key);
        }
    };

    // HashMemoryRange for hashing arbitrary data blob
    inline uint_t HashMemoryRange(const void* ptr, uint_t size)
    {
        return internal::HashCalculator<uint_t>::Hash(ptr, size);
    }
}

#endif /* RAMSES_CAPU_HASH_H */
