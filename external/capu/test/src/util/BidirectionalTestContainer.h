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

#ifndef RAMSES_CAPU_BIDIRECTIONALTESTCONTAINER_H
#define RAMSES_CAPU_BIDIRECTIONALTESTCONTAINER_H

#include "ramses-capu/Config.h"
#include <vector>
#include <cassert>

namespace ramses_capu
{
    template <typename T>
    class BidirectionalTestContainer
    {
        struct TWithPadding
        {
            T value;
            char padding[32];
        };
        std::vector<TWithPadding> vec;

    public:
        BidirectionalTestContainer(uint_t initial_size)
            : vec(initial_size)
        {
        }

        T& operator[](uint_t idx)
        {
            return vec[idx].value;
        }

        class Iterator : public std::iterator<std::bidirectional_iterator_tag, T>
        {
        public:
            Iterator(std::vector<TWithPadding>& v_, uint_t idx_)
                : v(v_), idx(idx_)
            {
            }
            Iterator& operator--()
            {
                assert(idx > 0);
                --idx;
                return *this;
            }
            Iterator& operator++()
            {
                assert(idx < v.size());
                ++idx;
                return *this;
            }

            bool operator!=(const Iterator& other) const
            {
                return other.idx != idx;
            }

            bool operator==(const Iterator& other) const
            {
                return other.idx == idx;
            }

            T& operator*()
            {
                return v[idx].value;
            }

        private:
            std::vector<TWithPadding>& v;
            uint_t idx;
        };

        Iterator begin()
        {
            return Iterator(vec, 0);
        }

        Iterator end()
        {
            return Iterator(vec, vec.size());
        }

        Iterator iteratorAt(uint_t index)
        {
            return Iterator(vec, index);
        }
    };
}

#endif
