//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SerializationContext.h"
#include "RamsesObjectImpl.h"
#include "ramses/framework/RamsesObject.h"
#include "ramses/framework/RamsesObjectTypes.h"
#include <vector>

namespace ramses::internal
{
    SerializationContext::SerializationContext(const SaveFileConfigImpl& saveConfig)
        : m_lastID(GetObjectIDNull() + 1u)
        , m_saveConfig(saveConfig)
    {
    }

    const SaveFileConfigImpl& SerializationContext::getSaveConfig() const
    {
        return m_saveConfig;
    }

    ObjectIDType DeserializationContext::GetObjectIDNull()
    {
        return ObjectIDType(0u);
    }

    DeserializationContext::DeserializationContext(const SceneConfigImpl& loadConfig, SceneMergeHandleMapping* mapping)
        : m_loadConfig(loadConfig)
        , m_mapping(mapping)
    {
    }

    ObjectIDType SerializationContext::getIDForObject(const RamsesObjectImpl* obj)
    {
        IdMap::Iterator it = m_ids.find(obj);
        if (it != m_ids.end())
        {
            return it->value;
        }

        const ObjectIDType id = m_lastID;
        m_ids.put(obj, id);
        m_lastID++;

        return id;
    }

    ObjectIDType SerializationContext::GetObjectIDNull()
    {
        return ObjectIDType(0u);
    }


    void SerializationContext::serializeSceneObjectIds(bool flag)
    {
        m_serializeSceneObjectIds = flag;
    }

    bool SerializationContext::getSerializeSceneObjectIds() const
    {
        return m_serializeSceneObjectIds;
    }

    void DeserializationContext::registerObjectImpl(RamsesObjectImpl* obj, ObjectIDType id)
    {
        if (m_objectImpls.size() <= id)
            m_objectImpls.resize(id + 1);

        m_objectImpls[id] = obj;
    }

    void DeserializationContext::addForDependencyResolve(RamsesObjectImpl* obj)
    {
        m_dependingObjects.put(obj);
    }

    bool DeserializationContext::resolveDependencies()
    {
        // resolve dependencies for all object types BEFORE LogicEngine
        std::vector<RamsesObjectImpl*> logicEngines;
        for (auto* obj: m_dependingObjects)
        {
            if (obj->getType() != ERamsesObjectType::LogicEngine)
            {
                if (!obj->resolveDeserializationDependencies(*this))
                {
                    return false;
                }
            }
            else
            {
                logicEngines.push_back(obj);
            }
        }

        for (auto* obj: logicEngines)
        {
            if (!obj->resolveDeserializationDependencies(*this))
            {
                return false;
            }
        }

        return true;
    }

    void DeserializationContext::addNodeHandleToNodeImplMapping(NodeHandle nodeHandle, NodeImpl* node)
    {
        assert(nodeHandle.isValid());
        assert(nodeHandle < m_nodeMap.size());
        m_nodeMap[nodeHandle.asMemoryHandle()] = node;
    }

    void DeserializationContext::resize(uint32_t totalObjects, uint32_t nodeCount)
    {
        m_objectImpls.resize(totalObjects);
        m_nodeMap.resize(nodeCount);
        m_dependingObjects.reserve(totalObjects);
    }

    NodeImpl* DeserializationContext::getNodeImplForHandle(NodeHandle nodeHandle) const
    {
        assert(nodeHandle.asMemoryHandle() < m_nodeMap.size());
        return m_nodeMap[nodeHandle.asMemoryHandle()];
    }

    const SceneConfigImpl& DeserializationContext::getLoadConfig() const
    {
        return m_loadConfig;
    }

    SceneMergeHandleMapping* DeserializationContext::getSceneMergeHandleMapping()
    {
        return m_mapping;
    }

    const SceneMergeHandleMapping* DeserializationContext::getSceneMergeHandleMapping() const
    {
        return m_mapping;
    }
}
