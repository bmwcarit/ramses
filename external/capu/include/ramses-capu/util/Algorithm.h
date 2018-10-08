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

#ifndef RAMSES_CAPU_ALGORITHM_H
#define RAMSES_CAPU_ALGORITHM_H

#include "ramses-capu/os/Memory.h"
#include <iterator>
#include <type_traits>

namespace ramses_capu
{
    template<class InputIt, class OutputIt, typename T, bool>
    struct CopyHelper
    {
        static OutputIt copy(InputIt first, InputIt last, OutputIt dest)
        {
            while (first != last)
            {
                *dest = *first;
                ++first;
                ++dest;
            }
            return dest;
        }
    };

    template<class InputIt, class OutputIt, typename T>
        struct CopyHelper<InputIt, OutputIt, T, true>
    {
        static OutputIt copy(InputIt first, InputIt last, OutputIt dest)
        {
            const uint_t distance = (last - first);
            if (distance > 0u)
            {
                Memory::Move(&*dest, &*first, distance * sizeof(T));
                return dest + distance;
            }
            return dest;
        }
    };

    /**
     * Copy range to destination iterator
     * @param first begin of source range
     * @param last non-inclusive end of source range
     * @param dest start of destination range
     * @return iterator one after the last element that was written
     */
    template <class InputIt, class OutputIt>
    OutputIt copy(InputIt first, InputIt last, OutputIt dest)
    {
        typedef typename std::iterator_traits<InputIt>::value_type T;
        typedef typename std::iterator_traits<InputIt>::iterator_category InputItCategory;
        typedef typename std::iterator_traits<OutputIt>::iterator_category OutputItCategory;
        return CopyHelper<InputIt, OutputIt, T,
            std::is_same<InputItCategory, std::random_access_iterator_tag>::value &&
            std::is_same<OutputItCategory, std::random_access_iterator_tag>::value &&
            std::is_trivially_copyable<T>::value
            >::copy(first, last, dest);
    }

    template<class InputIt, class OutputIt, typename T, bool>
    struct CopyBackwardHelper
    {
        static OutputIt copy_backward(InputIt first, InputIt last, OutputIt lastResult)
        {
            while (first != last)
            {
                --last;
                --lastResult;
                *lastResult = *last;
            }
            return lastResult;
        }
    };

    template<class InputIt, class OutputIt, typename T>
    struct CopyBackwardHelper<InputIt, OutputIt, T, true>
    {
        static OutputIt copy_backward(InputIt first, InputIt last, OutputIt lastResult)
        {
            const uint_t distance = (last - first);
            if (distance > 0u)
            {
                Memory::Move(&*(lastResult-distance), &*first, distance * sizeof(T));
                return lastResult - distance;
            }
            return lastResult;
        }
    };

    /**
    * Copy range to destination iterator
    * @param first begin of source range
    * @param last non-inclusive end of source range
    * @param lastResult non-inclusive end of destination range
    * @return iterator on the last element that was written
    */
    template <class InputIt, class OutputIt>
    OutputIt copy_backward(InputIt first, InputIt last, OutputIt lastResult)
    {
        typedef typename std::iterator_traits<InputIt>::value_type T;
        typedef typename std::iterator_traits<InputIt>::iterator_category InputItCategory;
        typedef typename std::iterator_traits<OutputIt>::iterator_category OutputItCategory;
        return CopyBackwardHelper<InputIt, OutputIt, T,
            std::is_same<InputItCategory, std::random_access_iterator_tag>::value &&
            std::is_same<OutputItCategory, std::random_access_iterator_tag>::value &&
            std::is_trivially_copyable<T>::value
            >::copy_backward(first, last, lastResult);
    }

    /**
     * Fill a number of elements with given value
     * @param first start of range to fill
     * @param count number of consecutive elements to fill
     * @param value reference value used set elements to
     * @return iterator one after the last element that was written
     */
    template <class OutputIt, class Size, class T>
    OutputIt fill_n(OutputIt first, Size count, const T& value)
    {
        for (Size i = 0; i < count; ++i)
        {
            *first = value;
            ++first;
        }
        return first;
    }

    /**
     * Compare two equal-length ranges for default equality
     * @param first1 begin of first range
     * @param last1 non-inclusive end of first range
     * @param first2 begin of second range to compare to
     * @return true if all elements from first and second range are equal, false if not
     */
    template <class InputIt1, class InputIt2>
    bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2)
    {
        while (first1 != last1)
        {
            if (*first1 != *first2)
            {
                return false;
            }
            ++first1;
            ++first2;
        }
        return true;
    }

    /**
     * Find a value in a given range
     * @param first begin of range to search value in
     * @param last non-inclusive end of range to search value in
     * @param value to search for
     * @return iterator pointing to the element if found or pointing to last if not
     */
    template <class InputIt, class T>
    InputIt find(InputIt first, InputIt last, const T& value)
    {
        while (first != last && *first != value)
        {
            ++first;
        }
        return first;
    }
}

#endif // RAMSES_CAPU_ALGORITHM_H
