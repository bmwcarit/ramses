//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PAIR_H
#define RAMSES_PAIR_H

#include "ramses-capu/container/Pair.h"
#include "ramses-capu/container/Hash.h"
#include <algorithm>

namespace ramses_internal
{
    template<typename A, typename B>
    class Pair: public ramses_capu::Pair<A, B>
    {
    public:
        Pair()
        {

        }

        Pair(const A& first_, const B& second_)
            : ramses_capu::Pair<A, B>(first_, second_)
        {

        }
    };

    template <typename A, typename B>
    Pair<A, B> MakePair(const A& a, const B& b)
    {
        return Pair<A, B>(a, b);
    }

    template<typename A, typename B>
    inline void swap(Pair<A, B>& a, Pair<A, B>& b)
    {
        using std::swap;
        swap(a.first, b.first);
        swap(b.second, b.second);
    }
}

namespace ramses_capu
{
    /**
     * Hash code generation for a Pair instance. Necessary e. g. for using a Pair as a key in a hash map.
     */
    template<typename A, typename B>
    struct Hash<ramses_internal::Pair<A, B>>
    {
        uint_t operator()(const ramses_internal::Pair<A, B>& data)
        {
            return HashValue(data.first, data.second);
        }
    };
}

#endif
