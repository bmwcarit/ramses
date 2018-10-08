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

#ifndef RAMSES_CAPU_PAIR_H
#define RAMSES_CAPU_PAIR_H

#include "ramses-capu/Config.h"

namespace ramses_capu
{
    /**
     * Pair class
     */
    template <class T1, class T2>
    class Pair
    {

    public:

        /**
         * Compare if equal to another pair
         */
        inline bool operator==(const Pair<T1, T2>& rhs) const
        {
            return ((first == rhs.first) && (second == rhs.second));
        }

        ~Pair()
        {

        }

        Pair()
            : first()
            , second()
        {

        }

        /**
         * Constructor with initial values
         * @param _first The first element of the pair
         * @param _second The second element of the pair
         */
        Pair(const T1& _first, const T2& _second)
            : first(_first), second(_second)
        {

        }

        /**
         * First value of the pair
         */
        T1 first;

        /**
         * Second value of the pair
         */
        T2 second;
    };
}


#endif /* RAMSES_CAPU_PAIR_H */
