/*
* Copyright (C) 2015 BMW Car IT GmbH
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

#ifndef RAMSES_CAPU_ALGORITHM_RAW_H
#define RAMSES_CAPU_ALGORITHM_RAW_H

#include "ramses-capu/os/Memory.h"
#include <iterator>
#include <type_traits>
#include <new>

namespace ramses_capu
{
    template<class InputIt, class OutputIt, typename T, bool>
    struct CopyRawHelper
    {
        static OutputIt copy(InputIt first, InputIt last, OutputIt dest)
        {
            while (first != last)
            {
                new(&(*dest))T(*first);
                ++first;
                ++dest;
            }
            return dest;
        }
    };

    template<class InputIt, class OutputIt, typename T>
    struct CopyRawHelper<InputIt, OutputIt, T, true>
    {
        static OutputIt copy(InputIt first, InputIt last, OutputIt dest)
        {
            const uint_t distance = (last - first);
            if (distance > 0)
            {
                Memory::Copy(&*dest, &*first, distance * sizeof(T));
            }
            return dest + distance;
        }
    };

    /**
     * Copy-construct range of elements in uninitialized memory
     * @param first begin of range to copy from
     * @param last non-inclusive end of range to copy from
     * @param dest iterator to raw (uninitialized memory) where elements are constructed
     * @return iterator one after the last element that was written
     */
    template <class InputIt, class OutputIt>
    OutputIt copy_to_raw(InputIt first, InputIt last, OutputIt dest)
    {
        typedef typename std::iterator_traits<InputIt>::value_type T;
        typedef typename std::iterator_traits<InputIt>::iterator_category InputItCategory;
        typedef typename std::iterator_traits<OutputIt>::iterator_category OutputItCategory;
        return CopyRawHelper<InputIt, OutputIt, T,
            std::is_same<InputItCategory, std::random_access_iterator_tag>::value &&
            std::is_same<OutputItCategory, std::random_access_iterator_tag>::value &&
            std::is_trivially_copyable<T>::value
            >::copy(first, last, dest);
    }

    template<class InputIt, class OutputIt, typename T, bool>
    struct MoveRawHelper
    {
        static OutputIt move(InputIt first, InputIt last, OutputIt dest)
        {
            while (first != last)
            {
                new(&(*dest))T(std::move(*first));
                ++first;
                ++dest;
            }
            return dest;
        }
    };

    template<class InputIt, class OutputIt, typename T>
    struct MoveRawHelper<InputIt, OutputIt, T, true>
    {
        static OutputIt move(InputIt first, InputIt last, OutputIt dest)
        {
            return CopyRawHelper<InputIt, OutputIt, T, true>::copy(first, last, dest);
        }
    };

   /**
     * Move-construct range of elements in uninitialized memory
     * @param first begin of range to move from
     * @param last non-inclusive end of range to move from
     * @param dest iterator to raw (uninitialized memory) where elements are constructed
     * @return iterator one after the last element that was written
     */
    template <class InputIt, class OutputIt>
    OutputIt move_to_raw(InputIt first, InputIt last, OutputIt dest)
    {
        typedef typename std::iterator_traits<InputIt>::value_type T;
        typedef typename std::iterator_traits<InputIt>::iterator_category InputItCategory;
        typedef typename std::iterator_traits<OutputIt>::iterator_category OutputItCategory;
        return MoveRawHelper<InputIt, OutputIt, T,
            std::is_same<InputItCategory, std::random_access_iterator_tag>::value &&
            std::is_same<OutputItCategory, std::random_access_iterator_tag>::value &&
            std::is_trivially_copyable<T>::value
            >::move(first, last, dest);
    }

    template<class OutputIt, class Size, typename T, bool>
    struct FillNRawHelper
    {
        static OutputIt fill(OutputIt first, Size count, const T& value)
        {
            for (Size i = 0; i < count; ++i)
            {
                new(&(*first))T(value);
                ++first;
            }
            return first;
        }

        static OutputIt fill(OutputIt first, Size count)
        {
            for (Size i = 0; i < count; ++i)
            {
                new(&(*first))T();
                ++first;
            }
            return first;
        }
    };

    template<class OutputIt, class Size, typename T>
        struct FillNRawHelper<OutputIt, Size, T, true>
    {
        static OutputIt fill(OutputIt first, Size count, const T& value)
        {
            for (Size i = 0; i < count; ++i)
            {
                first[i] = value;
            }
            return first + count;
        }

        static OutputIt fill(OutputIt first, Size count)
        {
            Memory::Set(&*first, 0, count * sizeof(T));
            return first + count;
        }
    };

    /**
     * Fill a number of elements in uninitialized memory with given value
     * @param first start of uninitialized memory range to fill
     * @param count number of consecutive elements to fill
     * @param value reference value used to set elements to
     * @return iterator one after the last element that was written
     */
    template <class OutputIt, class Size, class T>
        OutputIt fill_n_raw(OutputIt first, Size count, const T& value = T())
    {
        typedef typename std::iterator_traits<OutputIt>::iterator_category OutputItCategory;
        return FillNRawHelper<OutputIt, Size, T,
            std::is_same<OutputItCategory, std::random_access_iterator_tag>::value &&
            std::is_trivially_constructible<T>::value
            >::fill(first, count, value);
    }

    /**
    * Fill a number of elements in uninitialized memory with default value
    * @param first start of uninitialized memory range to fill
    * @param count number of consecutive elements to fill
    * @return iterator one after the last element that was written
    */
    template <class OutputIt, class Size>
    OutputIt fill_n_raw(OutputIt first, Size count)
    {
        typedef typename std::iterator_traits<OutputIt>::value_type T;
        typedef typename std::iterator_traits<OutputIt>::iterator_category OutputItCategory;
        return FillNRawHelper<OutputIt, Size, T,
            std::is_same<OutputItCategory, std::random_access_iterator_tag>::value &&
            std::is_trivially_default_constructible<T>::value
            >::fill(first, count);
    }

    template<class InputIt, typename T, bool>
    struct DestructRawHelper
    {
        static void destruct(InputIt first, InputIt last)
        {
            while (first != last)
            {
                first->~T();
                ++first;
            }
        }
    };

    template<class InputIt, typename T>
    struct DestructRawHelper<InputIt, T, true>
    {
        static void destruct(InputIt /*first*/, InputIt /*last*/)
        {
            // destruction of primitives is no-op
        }
    };

    /**
     * Destruct a range of elements in-place
     * @param first begin of range to destruct
     * @param last non-inclusive end of range to destruct
     */
    template <class InputIt>
    void destruct_raw(InputIt first, InputIt last)
    {
        typedef typename std::iterator_traits<InputIt>::value_type T;
        DestructRawHelper<InputIt, T,
            std::is_trivially_destructible<T>::value
            >::destruct(first, last);
    }
}

#endif // RAMSES_CAPU_ALGORITHM_RAW_H
