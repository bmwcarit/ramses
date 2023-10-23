//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/Components/ManagedResource.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

namespace ramses::internal
{
    class RamsesObjectImpl;
    class NodeImpl;
    class SaveFileConfigImpl;
    class SceneConfigImpl;

    using ObjectIDType = uint32_t;

    class DeserializationContext
    {
        using RamsesObjectImplSet = HashSet<RamsesObjectImpl *>;

    public:
        explicit DeserializationContext(const SceneConfigImpl& loadConfig);

        void resize(uint32_t totalObjects, uint32_t nodeCount);
        static ObjectIDType GetObjectIDNull();

        // phase 1: deserialize
        void registerObjectImpl(RamsesObjectImpl* obj, ObjectIDType id);
        void addNodeHandleToNodeImplMapping(NodeHandle nodeHandle, NodeImpl* node);

        template <typename PTR_TYPE>
        static void ReadDependentPointerAndStoreAsID(IInputStream& inStream, PTR_TYPE*& ptr);

        void addForDependencyResolve(RamsesObjectImpl* obj);

        // phase 2: resolve dependencies
        bool resolveDependencies();

        template <typename OBJECT_TYPE>
        void      resolveDependencyIDImplAndStoreAsPointer(OBJECT_TYPE*& ptrId) const;
        [[nodiscard]] NodeImpl* getNodeImplForHandle(NodeHandle /*nodeHandle*/) const;

        [[nodiscard]] const SceneConfigImpl& getLoadConfig() const;

    private:
        std::vector<RamsesObjectImpl*> m_objectImpls;
        std::vector<NodeImpl*>         m_nodeMap;
        RamsesObjectImplSet m_dependingObjects;
        const SceneConfigImpl&     m_loadConfig;
    };

    class SerializationContext
    {
        using IdMap = HashMap<const RamsesObjectImpl *, ObjectIDType>;

    public:
        explicit SerializationContext(const SaveFileConfigImpl& saveConfig);
        ObjectIDType        getIDForObject(const RamsesObjectImpl* obj);

        static ObjectIDType GetObjectIDNull();

        void serializeSceneObjectIds(bool flag);
        [[nodiscard]] bool getSerializeSceneObjectIds() const;

        [[nodiscard]] const SaveFileConfigImpl& getSaveConfig() const;

    private:
        bool         m_serializeSceneObjectIds = true;
        IdMap        m_ids;
        ObjectIDType m_lastID;
        const SaveFileConfigImpl& m_saveConfig;
    };

    template <typename OBJECT_TYPE>
    void DeserializationContext::resolveDependencyIDImplAndStoreAsPointer(OBJECT_TYPE*& ptrId) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) ptr really stores an id here
        const auto id = static_cast<ObjectIDType>(reinterpret_cast<std::uintptr_t>(ptrId));
        assert(id < m_objectImpls.size());
        ptrId = static_cast<OBJECT_TYPE*>(m_objectImpls[id]);
    }

    template <typename PTR_TYPE>
    void DeserializationContext::ReadDependentPointerAndStoreAsID(IInputStream& inStream, PTR_TYPE*& ptr)
    {
        ObjectIDType objID = GetObjectIDNull();
        inStream >> objID;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) ptr really stores an id here
        ptr = reinterpret_cast<PTR_TYPE*>(size_t(objID));
    }
}
