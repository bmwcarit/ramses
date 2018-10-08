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

#ifndef RAMSES_CAPU_MEMORY_H
#define RAMSES_CAPU_MEMORY_H

#include "ramses-capu/Config.h"
#include <cstring>
#include <algorithm>

namespace ramses_capu
{
    /**
     * Memory operations
     */
    class Memory
    {
    public:
        /**
         * Set size number of bytes in dst to value val
         * @param dst Pointer to the data to be set
         * @param val value to set to memory
         * @param size number of bytes to set
         */
        static void Set(void* dst, int32_t val, uint_t size);

        /**
         * Move size number of bytes from src to dst
         * @param dst Pointer where the data should be moved to
         * @param src Pointer where the data comes from
         * @param size number of bytes to set
         */
        static void Move(void* dst, const void* src, uint_t size);

        /**
         * Compare num bytes from ptr1 with ptr2
         * @param ptr1 Data to compare
         * @param ptr2 Other data to compare with
         * @param num number of bytes to compare
         */
        static int32_t Compare(const void* ptr1, const void* ptr2, uint_t num);

        /**
         * Copy size number of bytes from src to dst
         * @param dst Pointer where data should be copied to
         * @param src Pointer to the data to be copied
         * @param size number of bytes to copy
         */
        static void Copy(void* dst, const void* src, const uint_t size);
    };

    inline
    void
    Memory::Set(void* dst, int32_t val, uint_t size)
    {
        if(size > 0)
        {
            std::memset(dst, val, size);
        }
    }

    inline
    void
    Memory::Move(void* dst, const void* src, uint_t size)
    {
        if(size > 0)
        {
            std::memmove(dst, src, size);
        }
    }

    inline
    int32_t
    Memory::Compare(const void* ptr1, const void* ptr2, uint_t num)
    {
        if(num > 0)
        {
            return std::memcmp(ptr1, ptr2, num);
        }
        return 0;
    }

    inline
    void
    Memory::Copy(void* dst, const void* src, const uint_t size)
    {
        if(size > 0)
        {
            std::memcpy(dst, src, size);
        }
    }
}
#endif // RAMSES_CAPU_MEMORY_H
