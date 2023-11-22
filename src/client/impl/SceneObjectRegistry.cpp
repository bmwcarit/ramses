//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SceneObjectRegistry.h"
#include "ramses/framework/RamsesObject.h"
#include "ramses/client/Node.h"
#include "impl/RamsesObjectImpl.h"
#include "impl/ObjectIteratorImpl.h"
#include "impl/NodeImpl.h"
#include "impl/RamsesObjectTypeUtils.h"

namespace ramses::internal
{
    void SceneObjectRegistry::registerObjectInternal(SceneObject& object)
    {
        assert(!object.isOfType(ERamsesObjectType::LogicObject)); // logic objects have their own registry in corresponding LogicEngine
        assert(!containsObject(object));

        const ERamsesObjectType type = object.impl().getType();
        m_objects[static_cast<int>(type)].push_back(&object);

        trackSceneObjectById(object);
    }

    void SceneObjectRegistry::destroyAndUnregisterObject(SceneObject& object)
    {
        assert(containsObject(object));

        if (object.isOfType(ERamsesObjectType::Node))
            setNodeDirty(RamsesObjectTypeUtils::ConvertTo<Node>(object).m_impl, false);

        if (object.isOfType(ERamsesObjectType::SceneObject))
        {
            const sceneObjectId_t sceneObjectId = RamsesObjectTypeUtils::ConvertTo<SceneObject>(object).getSceneObjectId();
            assert(m_objectsById.count(sceneObjectId) != 0u);
            m_objectsById.erase(sceneObjectId);
        }

        auto& objects = m_objects[static_cast<int>(object.impl().getType())];
        auto it = std::find(objects.begin(), objects.end(), &object);
        assert(it != objects.end());
        objects.erase(it);

        auto it2 = std::find_if(m_objectsOwningContainer.begin(), m_objectsOwningContainer.end(), [&object](auto& ro) { return ro.get() == &object; });
        assert(it2 != m_objectsOwningContainer.end());
        m_objectsOwningContainer.erase(it2);
    }

    void SceneObjectRegistry::reserveAdditionalGeneralCapacity(size_t additionalCount)
    {
        m_objectsOwningContainer.reserve(m_objectsOwningContainer.size() + additionalCount);
    }

    void SceneObjectRegistry::reserveAdditionalObjectCapacity(ERamsesObjectType type, size_t additionalCount)
    {
        assert(RamsesObjectTypeUtils::IsConcreteType(type));
        const auto index = static_cast<int>(type);
        m_objects[index].reserve(m_objects[index].size() + additionalCount);
    }

    size_t SceneObjectRegistry::getNumberOfObjects(ERamsesObjectType type) const
    {
        assert(RamsesObjectTypeUtils::IsConcreteType(type));
        return m_objects[static_cast<int>(type)].size();
    }

    bool SceneObjectRegistry::containsObject(const SceneObject& object) const
    {
        const auto& objects = m_objects[static_cast<int>(object.impl().getType())];
        return std::find(objects.begin(), objects.end(), &object) != objects.end();
    }

    void SceneObjectRegistry::trackSceneObjectById(SceneObject& object)
    {
        if (object.isOfType(ERamsesObjectType::SceneObject))
        {
            auto& sceneObject = RamsesObjectTypeUtils::ConvertTo<SceneObject>(object);
            const sceneObjectId_t sceneObjectId = RamsesObjectTypeUtils::ConvertTo<SceneObject>(object).getSceneObjectId();
            assert(m_objectsById.count(sceneObjectId) == 0u);
            m_objectsById[sceneObjectId] = &sceneObject;
        }
    }

    SceneObject* SceneObjectRegistry::findObjectById(sceneObjectId_t id)
    {
        const auto it = m_objectsById.find(id);
        if (it != m_objectsById.cend())
            return it->second;

        auto& logicEngines = m_objects[static_cast<uint32_t>(ERamsesObjectType::LogicEngine)];
        for (auto* le : logicEngines)
        {
            auto obj = le->as<LogicEngine>()->findObject(id);
            if (obj)
                return obj;
        }

        return nullptr;
    }

    const SceneObject* SceneObjectRegistry::findObjectById(sceneObjectId_t id) const
    {
        // const version of findObjectById cast to its non-const version to avoid duplicating code
        return const_cast<SceneObject*>((const_cast<SceneObjectRegistry&>(*this)).findObjectById(id));
    }

    void SceneObjectRegistry::getObjectsOfType(SceneObjectVector& objects, ERamsesObjectType ofType) const
    {
        assert(objects.empty());

        // preallocate memory in container
        size_t objectCount = 0u;
        for (size_t i = 0u; i < RamsesObjectTypeCount; ++i)
        {
            const auto type = ERamsesObjectType(i);
            if (RamsesObjectTypeUtils::IsConcreteType(type) && RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ofType))
                objectCount += m_objects[i].size();
        }
        objects.reserve(objectCount);

        for (size_t i = 0u; i < RamsesObjectTypeCount; ++i)
        {
            const auto type = ERamsesObjectType(i);
            if (RamsesObjectTypeUtils::IsConcreteType(type) && RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ofType))
                objects.insert(objects.end(), m_objects[i].begin(), m_objects[i].end());
        }
    }

    void SceneObjectRegistry::setNodeDirty(NodeImpl& node, bool dirty)
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

    bool SceneObjectRegistry::isNodeDirty(const NodeImpl& node) const
    {
        NodeImpl* nodeImplPtr = &const_cast<NodeImpl&>(node);
        return m_dirtyNodes.contains(nodeImplPtr);
    }

    const NodeImplSet& SceneObjectRegistry::getDirtyNodes() const
    {
        return m_dirtyNodes;
    }

    void SceneObjectRegistry::clearDirtyNodes()
    {
        m_dirtyNodes.clear();
    }
}
