//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesObjectRegistry.h"
#include "ramses-client-api/RamsesObject.h"
#include "ramses-client-api/Node.h"
#include "RamsesObjectImpl.h"
#include "ObjectIteratorImpl.h"
#include "NodeImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "PlatformAbstraction/PlatformStringUtils.h"

namespace ramses
{
    RamsesObjectRegistry::~RamsesObjectRegistry()
    {
    }

    void RamsesObjectRegistry::addObject(RamsesObject& object)
    {
        assert(!containsObject(object));

        const ERamsesObjectType type = object.impl.getType();
        const RamsesObjectHandle handle = m_objects[type].allocate();
        *m_objects[type].getMemory(handle) = &object;
        object.impl.setObjectRegistry(*this);
        object.impl.setObjectRegistryHandle(handle);

        updateName(object, object.impl.getName());
    }

    void RamsesObjectRegistry::removeObject(RamsesObject& object)
    {
        assert(containsObject(object));

        if (object.isOfType(ERamsesObjectType_Node))
        {
            setNodeDirty(RamsesObjectTypeUtils::ConvertTo<Node>(object).impl, false);
        }

        m_objectsByName.remove(object.impl.getName());
        const RamsesObjectHandle handle = object.impl.getObjectRegistryHandle();
        const ERamsesObjectType type = object.impl.getType();
        m_objects[type].release(handle);
    }

    void RamsesObjectRegistry::reserveAdditionalGeneralCapacity(uint32_t additionalCount)
    {
        // not every object has a name. this might reserve more than needed but never more than
        // num of current objects with name + additionalCapacity
        m_objectsByName.reserve(m_objectsByName.count() + additionalCount);
    }

    void RamsesObjectRegistry::reserveAdditionalObjectCapacity(ERamsesObjectType type, uint32_t additionalCount)
    {
        assert(RamsesObjectTypeUtils::IsConcreteType(type));
        m_objects[type].preallocateSize(m_objects[type].getActualCount() + additionalCount);
    }

    uint32_t RamsesObjectRegistry::getNumberOfObjects(ERamsesObjectType type) const
    {
        assert(RamsesObjectTypeUtils::IsConcreteType(type));
        return m_objects[type].getActualCount();
    }

    bool RamsesObjectRegistry::containsObject(const RamsesObject& object) const
    {
        const RamsesObjectHandle handle = object.impl.getObjectRegistryHandle();
        const ERamsesObjectType type = object.impl.getType();
        const RamsesObjectsPool& objectsPool = m_objects[type];
        return objectsPool.isAllocated(handle) && (*objectsPool.getMemory(handle) == &object);
    }

    void RamsesObjectRegistry::updateName(RamsesObject& object, const ramses_internal::String& name)
    {
        assert(containsObject(object));
        const ramses_internal::String& oldName = object.impl.getName();
        if (!oldName.empty())
        {
            m_objectsByName.remove(oldName);
        }
        if (name.getLength()> 0)
        {
            m_objectsByName.put(name, &object);
        }
    }

    RamsesObject* RamsesObjectRegistry::findObjectByName(const char* name)
    {
        RamsesObject* object(0);
        m_objectsByName.get(name, object);
        return object;
    }

    const RamsesObject* RamsesObjectRegistry::findObjectByName(const char* name) const
    {
        // const version of findObjectByName cast to its non-const version to avoid duplicating code
        return const_cast<RamsesObject*>((const_cast<RamsesObjectRegistry&>(*this)).findObjectByName(name));
    }

    void RamsesObjectRegistry::getObjectsOfType(RamsesObjectVector& objects, ERamsesObjectType ofType) const
    {
        assert(objects.empty());

        // preallocate memory in container
        uint32_t objectCount = 0u;
        for (uint32_t i = 0u; i < ERamsesObjectType_NUMBER_OF_TYPES; ++i)
        {
            const ERamsesObjectType type = ERamsesObjectType(i);
            if (RamsesObjectTypeUtils::IsConcreteType(type) && RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ofType))
            {
                objectCount += m_objects[type].getActualCount();
            }
        }
        objects.reserve(objectCount);

        for (uint32_t i = 0u; i < ERamsesObjectType_NUMBER_OF_TYPES; ++i)
        {
            const ERamsesObjectType type = ERamsesObjectType(i);
            if (RamsesObjectTypeUtils::IsConcreteType(type) && RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ofType))
            {
                const RamsesObjectsPool& objectsPool = m_objects[type];
                for (RamsesObjectHandle handle(0u); handle < objectsPool.getTotalCount(); ++handle)
                {
                    if (objectsPool.isAllocated(handle))
                    {
                        objects.push_back(*objectsPool.getMemory(handle));
                    }
                }
            }
        }
    }

    void RamsesObjectRegistry::setNodeDirty(NodeImpl& node, bool dirty)
    {
        if (dirty)
        {
            m_dirtyNodes.put(&node);
        }
        else
        {
            m_dirtyNodes.remove(&node);
        }
    }

    bool RamsesObjectRegistry::isNodeDirty(const NodeImpl& node) const
    {
        NodeImpl* nodeImplPtr = &const_cast<NodeImpl&>(node);
        return m_dirtyNodes.hasElement(nodeImplPtr);
    }

    const NodeImplSet& RamsesObjectRegistry::getDirtyNodes() const
    {
        return m_dirtyNodes;
    }

    void RamsesObjectRegistry::clearDirtyNodes()
    {
        m_dirtyNodes.clear();
    }
}
