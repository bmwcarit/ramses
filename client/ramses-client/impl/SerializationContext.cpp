//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SerializationContext.h"
#include "RamsesObjectImpl.h"
#include "ramses-client-api/RamsesObject.h"

namespace ramses
{
    SerializationContext::SerializationContext()
        : m_lastID(GetObjectIDNull() + 1u)
    {
    }

    ObjectIDType DeserializationContext::GetObjectIDNull()
    {
        return ObjectIDType(0u);
    }

    DeserializationContext::DeserializationContext()
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

    bool DeserializationContext::registerObjectImpl(RamsesObjectImpl* obj, ObjectIDType id)
    {
        if (m_objectImpls.size() <= id)
        {
            m_objectImpls.resize(id + 1);
        }
        m_objectImpls[id] = obj;
        return true;
    }

    void DeserializationContext::addForDependencyResolve(RamsesObjectImpl* obj)
    {
        m_dependingObjects.put(obj);
    }

    status_t DeserializationContext::resolveDependencies()
    {
        for (auto obj : m_dependingObjects)
        {
            CHECK_RETURN_ERR(obj->resolveDeserializationDependencies(*this));
        }

        return StatusOK;
    }

    void DeserializationContext::addNodeHandleToNodeImplMapping(ramses_internal::NodeHandle nodeHandle, NodeImpl* node)
    {
        assert(nodeHandle < m_nodeMap.size());
        m_nodeMap[nodeHandle.asMemoryHandle()] = node;
    }

    void DeserializationContext::resize(uint32_t totalObjects, uint32_t nodeCount)
    {
        m_objectImpls.resize(totalObjects);
        m_nodeMap.resize(nodeCount);
        m_dependingObjects.reserve(totalObjects);
    }

    NodeImpl* DeserializationContext::getNodeImplForHandle(ramses_internal::NodeHandle nodeHandle) const
    {
        assert(nodeHandle.asMemoryHandle() < m_nodeMap.size());
        return m_nodeMap[nodeHandle.asMemoryHandle()];
    }
}
