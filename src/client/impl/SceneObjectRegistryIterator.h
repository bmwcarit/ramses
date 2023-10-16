//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesObject.h"
#include "impl/SceneObjectRegistry.h"
#include "impl/RamsesObjectTypeUtils.h"

namespace ramses::internal
{
    class SceneObjectRegistryIterator
    {
    public:
        SceneObjectRegistryIterator(const SceneObjectRegistry& registry, ERamsesObjectType type)
            : m_objects(registry.m_objects[static_cast<int>(type)])
            , m_objectsTotalCount(m_objects.getTotalCount())
            , m_current(0u)
        {
            assert(RamsesObjectTypeUtils::IsConcreteType(type));
        }

        template <typename T = SceneObject>
        const T* getNext()
        {
            static_assert(std::is_base_of_v<SceneObject, T>);
            assert((m_objectsTotalCount == m_objects.getTotalCount()) && "Container size changed while iterating!");
            for (; m_current < m_objectsTotalCount; ++m_current)
            {
                if (m_objects.isAllocated(m_current))
                {
                    const SceneObject& obj = **m_objects.getMemory(m_current);
                    ++m_current;
                    return &RamsesObjectTypeUtils::ConvertTo<T>(obj);
                }
            }

            return nullptr;
        }

        template <typename T = SceneObject>
        T* getNextNonConst()
        {
            // Non-const version of getNext cast to its const version to avoid duplicating code
            return const_cast<T*>(getNext<T>());
        }

    private:
        const SceneObjectRegistry::SceneObjectsPool&  m_objects;
        const uint32_t                                m_objectsTotalCount;
        SceneObjectRegistryHandle                     m_current;
    };
}
