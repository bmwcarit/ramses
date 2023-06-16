//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ITERATORIMPL_H
#define RAMSES_ITERATORIMPL_H

#include <vector>

namespace ramses
{
    template <typename T>
    class IteratorImpl
    {
    public:
        using ObjectVector = std::vector<T>;

        IteratorImpl() = default;

        explicit IteratorImpl(ObjectVector&& objects)
            : m_objects{ std::move(objects) }
            , m_objectIterator{ m_objects.begin() }
        {
        }

        explicit IteratorImpl(const ObjectVector& objects)
            : m_objects{ objects }
            , m_objectIterator{ m_objects.begin() }
        {
        }

        T getNext()
        {
            if (m_objectIterator != m_objects.end())
            {
                T ret = *m_objectIterator;
                ++m_objectIterator;
                return ret;
            }

            return 0;
        }

    protected:
        ObjectVector                         m_objects;
        typename ObjectVector::iterator      m_objectIterator;
    };
}

#endif
