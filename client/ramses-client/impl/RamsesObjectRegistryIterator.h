//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTREGISTRYITERATOR_H
#define RAMSES_RAMSESOBJECTREGISTRYITERATOR_H

#include "ramses-client-api/RamsesObject.h"
#include "RamsesObjectRegistry.h"
#include "RamsesObjectTypeUtils.h"

namespace ramses
{
    class RamsesObjectRegistryIterator
    {
    public:
        RamsesObjectRegistryIterator(const RamsesObjectRegistry& registry, ERamsesObjectType type)
            : m_objects(registry.m_objects[type])
            , m_objectsTotalCount(m_objects.getTotalCount())
            , m_current(0u)
        {
            assert(RamsesObjectTypeUtils::IsConcreteType(type));
        }

        template <typename T = RamsesObject>
        const T* getNext()
        {
            assert((m_objectsTotalCount == m_objects.getTotalCount()) && "Container size changed while iterating!");
            for (; m_current < m_objectsTotalCount; ++m_current)
            {
                if (m_objects.isAllocated(m_current))
                {
                    const RamsesObject& obj = **m_objects.getMemory(m_current);
                    ++m_current;
                    return &RamsesObjectTypeUtils::ConvertTo<T>(obj);
                }
            }

            return nullptr;
        }

        template <typename T = RamsesObject>
        T* getNextNonConst()
        {
            // Non-const version of getNext cast to its const version to avoid duplicating code
            return const_cast<T*>(getNext<T>());
        }

    private:
        const RamsesObjectRegistry::RamsesObjectsPool& m_objects;
        const uint32_t                                 m_objectsTotalCount;
        RamsesObjectHandle                             m_current;
    };
}

#endif
