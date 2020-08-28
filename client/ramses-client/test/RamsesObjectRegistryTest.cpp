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
#include <unordered_set>

namespace ramses
{
    using namespace testing;

    using DummyObjectImpl = NodeImpl;
    class DummyObject : public Node
    {
    public:
        explicit DummyObject(SceneImpl& scene)
            : Node(*new DummyObjectImpl(scene, ERamsesObjectType_Node, ""))
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
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType_Node));
    }

    TEST_F(ARamsesObjectRegistry, canAddObject)
    {
        m_dummyObject.setName("name");
        m_registry.addObject(m_dummyObject);
        EXPECT_EQ(&m_dummyObject, m_registry.findObjectByName("name"));
        EXPECT_EQ(&m_dummyObject, m_registry.findObjectById(sceneObjectId_t{1u}));
        EXPECT_EQ(&m_dummyObject, &m_dummyObject.impl.getRamsesObject());
        EXPECT_EQ(1u, m_registry.getNumberOfObjects(ERamsesObjectType_Node));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType_RenderBuffer));
        RamsesObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType_RamsesObject);
        ASSERT_EQ(1u, objects.size());
        EXPECT_EQ(&m_dummyObject, objects[0]);
    }

    TEST_F(ARamsesObjectRegistry, sceneObjectsGetUniqueIds)
    {
        const std::unique_ptr<DummyObject> dummy1(createDummyObject());
        const std::unique_ptr<DummyObject> dummy2(createDummyObject());
        const std::unique_ptr<DummyObject> dummy3(createDummyObject());
        const std::unique_ptr<DummyObject> dummy4(createDummyObject());
        const std::unique_ptr<DummyObject> dummy5(createDummyObject());

        const std::unordered_set<sceneObjectId_t> sceneObjectIds
        {
            dummy1->getSceneObjectId(),
            dummy2->getSceneObjectId(),
            dummy3->getSceneObjectId(),
            dummy4->getSceneObjectId(),
            dummy5->getSceneObjectId()
        };

        EXPECT_EQ(sceneObjectIds.size(), 5u);
    }

    TEST_F(ARamsesObjectRegistry, canRemoveObject)
    {
        m_dummyObject.setName("name");
        m_registry.addObject(m_dummyObject);
        m_registry.removeObject(m_dummyObject);
        EXPECT_TRUE(nullptr == m_registry.findObjectByName("name"));
        EXPECT_TRUE(nullptr == m_registry.findObjectById(sceneObjectId_t{1u}));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType_Node));
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
        EXPECT_EQ(&m_dummyObject, m_registry.findObjectById(sceneObjectId_t{1u}));
    }

    TEST_F(ARamsesObjectRegistry, cannotRetrieveObjectInfoAfterObjectDeleted)
    {
        m_dummyObject.setName("name");
        m_registry.addObject(m_dummyObject);
        m_registry.removeObject(m_dummyObject);
        EXPECT_TRUE(nullptr == m_registry.findObjectByName("name"));
        EXPECT_TRUE(nullptr == m_registry.findObjectById(sceneObjectId_t{1u}));
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
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
    }

    TEST_F(ARamsesObjectRegistry, marksNodeDirty)
    {
        m_registry.addObject(m_dummyObject);
        m_registry.setNodeDirty(m_dummyObject.impl, true);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_TRUE(m_registry.getDirtyNodes().contains(&m_dummyObject.impl));
        EXPECT_TRUE(m_registry.isNodeDirty(m_dummyObject.impl));
    }

    TEST_F(ARamsesObjectRegistry, marksNodeClean)
    {
        std::unique_ptr<DummyObject> object1(createDummyObject());
        m_registry.addObject(*object1);
        m_registry.setNodeDirty(object1->impl, true);

        std::unique_ptr<DummyObject> object2(createDummyObject());
        m_registry.addObject(*object2);
        m_registry.setNodeDirty(object2->impl, true);

        EXPECT_EQ(2u, m_registry.getDirtyNodes().size());

        m_registry.setNodeDirty(object1->impl, false);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_FALSE(m_registry.getDirtyNodes().contains(&object1->impl));
        EXPECT_FALSE(m_registry.isNodeDirty(object1->impl));
    }

    TEST_F(ARamsesObjectRegistry, clearsDirtyNodes)
    {
        m_registry.addObject(m_dummyObject);
        m_registry.setNodeDirty(m_dummyObject.impl, true);
        m_registry.clearDirtyNodes();
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
        EXPECT_FALSE(m_registry.isNodeDirty(m_dummyObject.impl));
    }

    TEST_F(ARamsesObjectRegistry, marksNodeCleanWhenDeleted)
    {
        m_registry.addObject(m_dummyObject);
        m_registry.setNodeDirty(m_dummyObject.impl, true);
        m_registry.removeObject(m_dummyObject);
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
    }

    TEST_F(ARamsesObjectRegistry, nodeAppearsInDirtyListAfterMarkedDirty)
    {
        m_registry.addObject(m_dummyObject);
        m_registry.setNodeDirty(m_dummyObject.impl, true);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_TRUE(m_registry.getDirtyNodes().contains(&m_dummyObject.impl));
        EXPECT_TRUE(m_registry.isNodeDirty(m_dummyObject.impl));
    }
}
