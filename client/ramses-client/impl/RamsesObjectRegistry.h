//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTREGISTRY_H
#define RAMSES_RAMSESOBJECTREGISTRY_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include "RamsesObjectHandle.h"
#include "RamsesObjectVector.h"
#include "IRamsesObjectRegistry.h"

#include "Collections/HashMap.h"
#include "Collections/String.h"
#include "Utils/MemoryPool.h"

namespace ramses
{
    class RamsesObject;
    class RamsesObjectImpl;
    class SceneObject;

    class RamsesObjectRegistry final : public IRamsesObjectRegistry
    {
    public:
        ~RamsesObjectRegistry() override;

        void     addObject(RamsesObject& object);
        void     removeObject(RamsesObject& object);
        void     reserveAdditionalGeneralCapacity(uint32_t additionalCount);
        void     reserveAdditionalObjectCapacity(ERamsesObjectType type, uint32_t additionalCount);
        [[nodiscard]] uint32_t getNumberOfObjects(ERamsesObjectType type) const;

        void     getObjectsOfType(RamsesObjectVector& objects, ERamsesObjectType ofType) const;

        // IRamsesObjectRegistry
        void updateName(RamsesObject& object, const ramses_internal::String& name) override;

        const RamsesObject* findObjectByName(const char* name) const;
        RamsesObject*       findObjectByName(const char* name);

        [[nodiscard]] const SceneObject* findObjectById(sceneObjectId_t id) const;
        SceneObject* findObjectById(sceneObjectId_t id);


        void setNodeDirty(NodeImpl& node, bool dirty);
        [[nodiscard]] bool isNodeDirty(const NodeImpl& node) const;

        [[nodiscard]] const NodeImplSet& getDirtyNodes() const;
        void               clearDirtyNodes();

    private:
        [[nodiscard]] bool containsObject(const RamsesObject& object) const;
        void trackSceneObjectById(RamsesObject& object);

        using ObjectNameMap = ramses_internal::HashMap<ramses_internal::String, RamsesObject *>;
        ObjectNameMap m_objectsByName;

        using ObjectIdMap = ramses_internal::HashMap<sceneObjectId_t, SceneObject *>;
        ObjectIdMap m_objectsById;

        using RamsesObjectsPool = ramses_internal::MemoryPool<RamsesObject *, RamsesObjectHandle>;
        RamsesObjectsPool m_objects[ERamsesObjectType_NUMBER_OF_TYPES];

        NodeImplSet m_dirtyNodes;

        friend class RamsesObjectRegistryIterator;
    };
}

#endif
