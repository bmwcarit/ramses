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

    class ARamsesObjectRegistry : public testing::Test
    {
    public:
        Node* createAndRegisterDummyObject()
        {
            return &m_registry.createAndRegisterObject<Node>(std::make_unique<NodeImpl>(m_dummyScene.getScene().m_impl, ERamsesObjectType::Node, ""));
        }

    protected:
        LocalTestClientWithScene m_dummyScene;
        RamsesObjectRegistry m_registry;
    };

    TEST_F(ARamsesObjectRegistry, isEmptyUponCreation)
    {
        RamsesObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType::RamsesObject);
        EXPECT_TRUE(objects.empty());
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType::Node));
    }

    TEST_F(ARamsesObjectRegistry, canAddObject)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        EXPECT_EQ(dummyObject, m_registry.findObjectByName("name"));
        EXPECT_EQ(dummyObject, m_registry.findObjectById(sceneObjectId_t{1u}));
        EXPECT_EQ(dummyObject, &dummyObject->m_impl.getRamsesObject());
        EXPECT_EQ(1u, m_registry.getNumberOfObjects(ERamsesObjectType::Node));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType::RenderBuffer));
        RamsesObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType::RamsesObject);
        ASSERT_EQ(1u, objects.size());
        EXPECT_EQ(dummyObject, objects[0]);
    }

    TEST_F(ARamsesObjectRegistry, sceneObjectsGetUniqueIds)
    {
        const auto dummy1(createAndRegisterDummyObject());
        const auto dummy2(createAndRegisterDummyObject());
        const auto dummy3(createAndRegisterDummyObject());
        const auto dummy4(createAndRegisterDummyObject());
        const auto dummy5(createAndRegisterDummyObject());

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
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        m_registry.destroyAndUnregisterObject(*dummyObject);
        EXPECT_TRUE(nullptr == m_registry.findObjectByName("name"));
        EXPECT_TRUE(nullptr == m_registry.findObjectById(sceneObjectId_t{1u}));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType::Node));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType::RenderBuffer));
        RamsesObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType::RamsesObject);
        EXPECT_TRUE(objects.empty());
    }

    TEST_F(ARamsesObjectRegistry, canAddAndRetrieveObjectInfo)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        EXPECT_EQ(dummyObject, m_registry.findObjectByName("name"));
        EXPECT_EQ(dummyObject, m_registry.findObjectById(sceneObjectId_t{1u}));
    }

    TEST_F(ARamsesObjectRegistry, canRenameObjectAndFindUnderNewName)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        EXPECT_EQ(dummyObject, m_registry.findObjectByName("name"));
        EXPECT_EQ(dummyObject, m_registry.findObjectById(sceneObjectId_t{ 1u }));
        dummyObject->setName("newName");
        EXPECT_EQ(dummyObject, m_registry.findObjectByName("newName"));
        EXPECT_EQ(dummyObject, m_registry.findObjectById(sceneObjectId_t{ 1u }));
        // cannot find under old name
        EXPECT_EQ(nullptr, m_registry.findObjectByName("name"));
    }

    TEST_F(ARamsesObjectRegistry, cannotRetrieveObjectInfoAfterObjectDeleted)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        m_registry.destroyAndUnregisterObject(*dummyObject);
        EXPECT_TRUE(nullptr == m_registry.findObjectByName("name"));
        EXPECT_TRUE(nullptr == m_registry.findObjectById(sceneObjectId_t{1u}));
    }

    TEST_F(ARamsesObjectRegistry, canRetrieveObjectFromImpl)
    {
        auto dummyObject = createAndRegisterDummyObject();
        EXPECT_EQ(dummyObject, &dummyObject->m_impl.getRamsesObject());
    }

    TEST_F(ARamsesObjectRegistry, hasNoDirtyNodesAfterInitialization)
    {
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
    }

    TEST_F(ARamsesObjectRegistry, marksNodeDirty)
    {
        auto dummyObject = createAndRegisterDummyObject();
        m_registry.setNodeDirty(dummyObject->m_impl, true);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_TRUE(m_registry.getDirtyNodes().contains(&dummyObject->m_impl));
        EXPECT_TRUE(m_registry.isNodeDirty(dummyObject->m_impl));
    }

    TEST_F(ARamsesObjectRegistry, marksNodeClean)
    {
        auto object1(createAndRegisterDummyObject());
        m_registry.setNodeDirty(object1->m_impl, true);

        auto object2(createAndRegisterDummyObject());
        m_registry.setNodeDirty(object2->m_impl, true);

        EXPECT_EQ(2u, m_registry.getDirtyNodes().size());

        m_registry.setNodeDirty(object1->m_impl, false);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_FALSE(m_registry.getDirtyNodes().contains(&object1->m_impl));
        EXPECT_FALSE(m_registry.isNodeDirty(object1->m_impl));
    }

    TEST_F(ARamsesObjectRegistry, clearsDirtyNodes)
    {
        auto dummyObject = createAndRegisterDummyObject();
        m_registry.setNodeDirty(dummyObject->m_impl, true);
        m_registry.clearDirtyNodes();
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
        EXPECT_FALSE(m_registry.isNodeDirty(dummyObject->m_impl));
    }

    TEST_F(ARamsesObjectRegistry, marksNodeCleanWhenDeleted)
    {
        auto dummyObject = createAndRegisterDummyObject();
        m_registry.setNodeDirty(dummyObject->m_impl, true);
        m_registry.destroyAndUnregisterObject(*dummyObject);
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
    }

    TEST_F(ARamsesObjectRegistry, nodeAppearsInDirtyListAfterMarkedDirty)
    {
        auto dummyObject = createAndRegisterDummyObject();
        m_registry.setNodeDirty(dummyObject->m_impl, true);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_TRUE(m_registry.getDirtyNodes().contains(&dummyObject->m_impl));
        EXPECT_TRUE(m_registry.isNodeDirty(dummyObject->m_impl));
    }
}
