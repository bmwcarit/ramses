//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTAINER_VECTOR_H
#define RAMSES_CONTAINER_VECTOR_H

#include <vector>
#include <algorithm>

namespace ramses_internal
{
    template <typename T>
    inline
    typename std::vector<T>::const_iterator find_c(const std::vector<T>& vec, const typename std::vector<T>::value_type& value)
    {
        return std::find(vec.cbegin(), vec.cend(), value);
    }

    template <typename T>
    inline
    typename std::vector<T>::iterator find_c(std::vector<T>& vec, const typename std::vector<T>::value_type& value)
    {
        return std::find(vec.begin(), vec.end(), value);
    }

    template <typename T>
    inline
    bool contains_c(const std::vector<T>& vec, const typename std::vector<T>::value_type& value)
    {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }
}

#endif
