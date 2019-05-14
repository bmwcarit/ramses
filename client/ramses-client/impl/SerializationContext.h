//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SERIALIZATIONCONTEXT_H
#define RAMSES_SERIALIZATIONCONTEXT_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Collections/HashMap.h"
#include "Collections/HashSet.h"
#include "Collections/IInputStream.h"
#include "Components/ManagedResource.h"
#include "SceneAPI/Handles.h"

namespace ramses
{
    class RamsesObjectImpl;
    class NodeImpl;

    typedef uint32_t ObjectIDType;

    class DeserializationContext
    {
        typedef ramses_internal::HashSet<RamsesObjectImpl*> RamsesObjectImplSet;

    public:
        explicit DeserializationContext();

        void resize(uint32_t totalObjects, uint32_t nodeCount);
        static ObjectIDType GetObjectIDNull();

        // phase 1: deserialize
        bool registerObjectImpl(RamsesObjectImpl* obj, ObjectIDType id);
        void addNodeHandleToNodeImplMapping(ramses_internal::NodeHandle nodeHandle, NodeImpl* node);

        template <typename PTR_TYPE>
        static void ReadDependentPointerAndStoreAsID(ramses_internal::IInputStream& inStream, PTR_TYPE*& ptr);

        void addForDependencyResolve(RamsesObjectImpl* obj);

        // phase 2: resolve dependencies
        status_t  resolveDependencies();

        template <typename OBJECT_TYPE>
        void      resolveDependencyIDImplAndStoreAsPointer(OBJECT_TYPE*& ptrId) const;
        NodeImpl* getNodeImplForHandle(ramses_internal::NodeHandle) const;

    private:
        std::vector<RamsesObjectImpl*> m_objectImpls;
        std::vector<NodeImpl*>         m_nodeMap;
        RamsesObjectImplSet m_dependingObjects;
    };

    class SerializationContext
    {
        typedef ramses_internal::HashMap<const RamsesObjectImpl*, ObjectIDType> IdMap;

    public:
        SerializationContext();
        ObjectIDType        getIDForObject(const RamsesObjectImpl* obj);

        static ObjectIDType GetObjectIDNull();

    private:
        IdMap        m_ids;
        ObjectIDType m_lastID;
    };

    template <typename OBJECT_TYPE>
    void DeserializationContext::resolveDependencyIDImplAndStoreAsPointer(OBJECT_TYPE*& ptrId) const
    {
        const ObjectIDType id = static_cast<ObjectIDType>(reinterpret_cast<std::uintptr_t>(ptrId));
        assert(id < m_objectImpls.size());
        ptrId = static_cast<OBJECT_TYPE*>(m_objectImpls[id]);
    }

    template <typename PTR_TYPE>
    void DeserializationContext::ReadDependentPointerAndStoreAsID(ramses_internal::IInputStream& inStream, PTR_TYPE*& ptr)
    {
        ObjectIDType objID = GetObjectIDNull();
        inStream >> objID;
        ptr = reinterpret_cast<PTR_TYPE*>(size_t(objID));
    }
}

#endif
