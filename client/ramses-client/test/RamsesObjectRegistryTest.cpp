//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-client-api/RamsesObject.h"
#include "RamsesObjectRegistry.h"
#include "NodeImpl.h"
#include "ramses-client-api/Node.h"
#include "ClientTestUtils.h"

namespace ramses
{
    using namespace testing;

    using DummyObjectImpl = NodeImpl;
    class DummyObject : public Node
    {
    public:
        DummyObject(SceneImpl& scene)
            : Node(*new DummyObjectImpl(scene, ERamsesObjectType_GroupNode, ""))
        {
        }
    };

    class ARamsesObjectRegistry : public testing::Test
    {
    public:
        ARamsesObjectRegistry()
            : m_dummyObject(*createDummyObject())
        {
        }

        ~ARamsesObjectRegistry()
        {
            delete &m_dummyObject;
        }

        DummyObject* createDummyObject()
        {
            return new DummyObject(m_dummyScene.getScene().impl);
        }

    protected:
        LocalTestClientWithScene m_dummyScene;
        RamsesObjectRegistry m_registry;
        DummyObject& m_dummyObject;
    };

    TEST_F(ARamsesObjectRegistry, isEmptyUponCreation)
    {
        RamsesObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType_RamsesObject);
        EXPECT_TRUE(objects.empty());
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType_GroupNode));
    }

    TEST_F(ARamsesObjectRegistry, canAddObject)
    {
        m_dummyObject.setName("name");
        m_registry.addObject(m_dummyObject);
        EXPECT_EQ(&m_dummyObject, m_registry.findObjectByName("name"));
        EXPECT_EQ(&m_dummyObject, &m_dummyObject.impl.getRamsesObject());
        EXPECT_EQ(1u, m_registry.getNumberOfObjects(ERamsesObjectType_GroupNode));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType_RenderBuffer));
        RamsesObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType_RamsesObject);
        ASSERT_EQ(1u, objects.size());
        EXPECT_EQ(&m_dummyObject, objects[0]);
    }

    TEST_F(ARamsesObjectRegistry, canRemoveObject)
    {
        m_dummyObject.setName("name");
        m_registry.addObject(m_dummyObject);
        m_registry.removeObject(m_dummyObject);
        EXPECT_TRUE(NULL == m_registry.findObjectByName("name"));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType_GroupNode));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType_RenderBuffer));
        RamsesObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType_RamsesObject);
        EXPECT_TRUE(objects.empty());
    }

    TEST_F(ARamsesObjectRegistry, canAddAndRetrieveObjectInfo)
    {
        m_dummyObject.setName("name");
        m_registry.addObject(m_dummyObject);
        EXPECT_EQ(&m_dummyObject, m_registry.findObjectByName("name"));
    }

    TEST_F(ARamsesObjectRegistry, cannotRetrieveObjectInfoAfterObjectDeleted)
    {
        m_dummyObject.setName("name");
        m_registry.addObject(m_dummyObject);
        m_registry.removeObject(m_dummyObject);
        EXPECT_TRUE(NULL == m_registry.findObjectByName("name"));
    }

    TEST_F(ARamsesObjectRegistry, cannotFindObjectByOldNameWhenNameUpdatedAfterAdding)
    {
        m_dummyObject.setName("name_old");
        m_registry.addObject(m_dummyObject);
        m_dummyObject.setName("name_new");
        EXPECT_TRUE(nullptr == m_registry.findObjectByName("name_old"));
    }

    TEST_F(ARamsesObjectRegistry, canAddAndRetrieveObjectInfoWhenUpdatedAfterAdding)
    {
        m_registry.addObject(m_dummyObject);
        m_dummyObject.setName("name");
        EXPECT_EQ(&m_dummyObject, m_registry.findObjectByName("name"));
    }

    TEST_F(ARamsesObjectRegistry, canRetrieveObjectFromImpl)
    {
        m_registry.addObject(m_dummyObject);
        EXPECT_EQ(&m_dummyObject, &m_dummyObject.impl.getRamsesObject());
    }

    TEST_F(ARamsesObjectRegistry, hasNoDirtyNodesAfterInitialization)
    {
        EXPECT_EQ(0u, m_registry.getDirtyNodes().count());
    }

    TEST_F(ARamsesObjectRegistry, marksNodeDirty)
    {
        m_registry.addObject(m_dummyObject);
        m_registry.setNodeDirty(m_dummyObject.impl, true);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().count());
        EXPECT_TRUE(m_registry.getDirtyNodes().hasElement(&m_dummyObject.impl));
        EXPECT_TRUE(m_registry.isNodeDirty(m_dummyObject.impl));
    }

    TEST_F(ARamsesObjectRegistry, marksNodeClean)
    {
        ramses_internal::ScopedPointer<DummyObject> object1(createDummyObject());
        m_registry.addObject(*object1);
        m_registry.setNodeDirty(object1->impl, true);

        ramses_internal::ScopedPointer<DummyObject> object2(createDummyObject());
        m_registry.addObject(*object2);
        m_registry.setNodeDirty(object2->impl, true);

        EXPECT_EQ(2u, m_registry.getDirtyNodes().count());

        m_registry.setNodeDirty(object1->impl, false);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().count());
        EXPECT_FALSE(m_registry.getDirtyNodes().hasElement(&object1->impl));
        EXPECT_FALSE(m_registry.isNodeDirty(object1->impl));
    }

    TEST_F(ARamsesObjectRegistry, clearsDirtyNodes)
    {
        m_registry.addObject(m_dummyObject);
        m_registry.setNodeDirty(m_dummyObject.impl, true);
        m_registry.clearDirtyNodes();
        EXPECT_EQ(0u, m_registry.getDirtyNodes().count());
        EXPECT_FALSE(m_registry.isNodeDirty(m_dummyObject.impl));
    }

    TEST_F(ARamsesObjectRegistry, marksNodeCleanWhenDeleted)
    {
        m_registry.addObject(m_dummyObject);
        m_registry.setNodeDirty(m_dummyObject.impl, true);
        m_registry.removeObject(m_dummyObject);
        EXPECT_EQ(0u, m_registry.getDirtyNodes().count());
    }

    TEST_F(ARamsesObjectRegistry, nodeAppearsInDirtyListAfterMarkedDirty)
    {
        m_registry.addObject(m_dummyObject);
        m_registry.setNodeDirty(m_dummyObject.impl, true);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().count());
        EXPECT_TRUE(m_registry.getDirtyNodes().hasElement(&m_dummyObject.impl));
        EXPECT_TRUE(m_registry.isNodeDirty(m_dummyObject.impl));
    }
}
