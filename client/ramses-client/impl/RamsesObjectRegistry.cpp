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
    RamsesObjectRegistry::~RamsesObjectRegistry() = default;

    void RamsesObjectRegistry::registerObjectInternal(RamsesObject& object)
    {
        assert(!containsObject(object));

        const ERamsesObjectType type = object.m_impl.getType();
        const RamsesObjectHandle handle = m_objects[static_cast<int>(type)].allocate();
        *m_objects[static_cast<int>(type)].getMemory(handle) = &object;
        object.m_impl.setObjectRegistry(*this);
        object.m_impl.setObjectRegistryHandle(handle);

        updateName(object, object.m_impl.getName());
        trackSceneObjectById(object);
    }

    void RamsesObjectRegistry::destroyAndUnregisterObject(RamsesObject& object)
    {
        assert(containsObject(object));

        if (object.isOfType(ERamsesObjectType::Node))
            setNodeDirty(RamsesObjectTypeUtils::ConvertTo<Node>(object).m_impl, false);

        if (object.isOfType(ERamsesObjectType::SceneObject))
        {
            const sceneObjectId_t sceneObjectId = RamsesObjectTypeUtils::ConvertTo<SceneObject>(object).getSceneObjectId();
            assert(m_objectsById.contains(sceneObjectId));
            m_objectsById.remove(sceneObjectId);
        }

        m_objectsByName.remove(object.m_impl.getName());

        const RamsesObjectHandle handle = object.m_impl.getObjectRegistryHandle();
        const auto type = static_cast<int>(object.m_impl.getType());
        m_objects[type].release(handle);

        auto it = std::find_if(m_objectsOwningContainer.begin(), m_objectsOwningContainer.end(), [&object](auto& ro) { return ro.get() == &object; });
        assert(it != m_objectsOwningContainer.end());
        m_objectsOwningContainer.erase(it);
    }

    void RamsesObjectRegistry::reserveAdditionalGeneralCapacity(uint32_t additionalCount)
    {
        // not every object has a name. this might reserve more than needed but never more than
        // num of current objects with name + additionalCapacity
        m_objectsByName.reserve(m_objectsByName.size() + additionalCount);
        m_objectsOwningContainer.reserve(m_objectsOwningContainer.size() + additionalCount);
    }

    void RamsesObjectRegistry::reserveAdditionalObjectCapacity(ERamsesObjectType type, uint32_t additionalCount)
    {
        assert(RamsesObjectTypeUtils::IsConcreteType(type));
        const auto index = static_cast<int>(type);
        m_objects[index].preallocateSize(m_objects[index].getActualCount() + additionalCount);
    }

    uint32_t RamsesObjectRegistry::getNumberOfObjects(ERamsesObjectType type) const
    {
        assert(RamsesObjectTypeUtils::IsConcreteType(type));
        return m_objects[static_cast<int>(type)].getActualCount();
    }

    bool RamsesObjectRegistry::containsObject(const RamsesObject& object) const
    {
        const RamsesObjectHandle handle = object.m_impl.getObjectRegistryHandle();
        const ERamsesObjectType type = object.m_impl.getType();
        const RamsesObjectsPool& objectsPool = m_objects[static_cast<int>(type)];
        return objectsPool.isAllocated(handle) && (*objectsPool.getMemory(handle) == &object);
    }

    void RamsesObjectRegistry::updateName(RamsesObject& object, const std::string& name)
    {
        assert(containsObject(object));
        const std::string& oldName = object.m_impl.getName();
        if (!oldName.empty())
        {
            m_objectsByName.remove(oldName);
        }
        if (name.size()> 0)
        {
            m_objectsByName.put(name, &object);
        }
    }

    void RamsesObjectRegistry::trackSceneObjectById(RamsesObject& object)
    {
        if (object.isOfType(ERamsesObjectType::SceneObject))
        {
            SceneObject& sceneObject = RamsesObjectTypeUtils::ConvertTo<SceneObject>(object);
            const sceneObjectId_t sceneObjectId = RamsesObjectTypeUtils::ConvertTo<SceneObject>(object).getSceneObjectId();
            assert(!m_objectsById.contains(sceneObjectId));
            m_objectsById.put(sceneObjectId, &sceneObject);
        }
    }

    RamsesObject* RamsesObjectRegistry::findObjectByName(std::string_view name)
    {
        RamsesObject* object(nullptr);
        m_objectsByName.get(std::string{name}, object);
        return object;
    }

    const RamsesObject* RamsesObjectRegistry::findObjectByName(std::string_view name) const
    {
        // const version of findObjectByName cast to its non-const version to avoid duplicating code
        return const_cast<RamsesObject*>((const_cast<RamsesObjectRegistry&>(*this)).findObjectByName(name));
    }

    SceneObject* RamsesObjectRegistry::findObjectById(sceneObjectId_t id)
    {
        SceneObject* object(nullptr);
        m_objectsById.get(id, object);

        return object;
    }

    const SceneObject* RamsesObjectRegistry::findObjectById(sceneObjectId_t id) const
    {
        // const version of findObjectById cast to its non-const version to avoid duplicating code
        return const_cast<SceneObject*>((const_cast<RamsesObjectRegistry&>(*this)).findObjectById(id));
    }

    void RamsesObjectRegistry::getObjectsOfType(RamsesObjectVector& objects, ERamsesObjectType ofType) const
    {
        assert(objects.empty());

        // preallocate memory in container
        uint32_t objectCount = 0u;
        for (uint32_t i = 0u; i < static_cast<uint32_t>(ERamsesObjectType::NUMBER_OF_TYPES); ++i)
        {
            const ERamsesObjectType type = ERamsesObjectType(i);
            if (RamsesObjectTypeUtils::IsConcreteType(type) && RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ofType))
            {
                objectCount += m_objects[i].getActualCount();
            }
        }
        objects.reserve(objectCount);

        for (uint32_t i = 0u; i < static_cast<uint32_t>(ERamsesObjectType::NUMBER_OF_TYPES); ++i)
        {
            const ERamsesObjectType type = ERamsesObjectType(i);
            if (RamsesObjectTypeUtils::IsConcreteType(type) && RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ofType))
            {
                const RamsesObjectsPool& objectsPool = m_objects[i];
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
        return m_dirtyNodes.contains(nodeImplPtr);
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
