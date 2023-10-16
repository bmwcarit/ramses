//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses/framework/RamsesObject.h"
#include "impl/SceneObjectRegistry.h"
#include "impl/NodeImpl.h"
#include "ramses/client/Node.h"
#include "ClientTestUtils.h"
#include <unordered_set>

namespace ramses::internal
{
    using namespace testing;

    class ASceneObjectRegistry : public testing::Test
    {
    public:
        Node* createAndRegisterDummyObject()
        {
            return &m_registry.createAndRegisterObject<Node>(std::make_unique<NodeImpl>(m_dummyScene.getScene().impl(), ERamsesObjectType::Node, ""));
        }

    protected:
        LocalTestClientWithScene m_dummyScene;
        SceneObjectRegistry m_registry;
    };

    TEST_F(ASceneObjectRegistry, isEmptyUponCreation)
    {
        SceneObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType::RamsesObject);
        EXPECT_TRUE(objects.empty());
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType::Node));
    }

    TEST_F(ASceneObjectRegistry, canAddObject)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        EXPECT_EQ(dummyObject, m_registry.findObjectByName<SceneObject>("name"));
        EXPECT_EQ(dummyObject, m_registry.findObjectById(sceneObjectId_t{1u}));
        EXPECT_EQ(dummyObject, &dummyObject->impl().getRamsesObject());
        EXPECT_EQ(1u, m_registry.getNumberOfObjects(ERamsesObjectType::Node));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType::RenderBuffer));
        SceneObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType::RamsesObject);
        ASSERT_EQ(1u, objects.size());
        EXPECT_EQ(dummyObject, objects[0]);
    }

    TEST_F(ASceneObjectRegistry, sceneObjectsGetUniqueIds)
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

    TEST_F(ASceneObjectRegistry, canRemoveObject)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        m_registry.destroyAndUnregisterObject(*dummyObject);
        EXPECT_TRUE(nullptr == m_registry.findObjectByName<SceneObject>("name"));
        EXPECT_TRUE(nullptr == m_registry.findObjectById(sceneObjectId_t{1u}));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType::Node));
        EXPECT_EQ(0u, m_registry.getNumberOfObjects(ERamsesObjectType::RenderBuffer));
        SceneObjectVector objects;
        m_registry.getObjectsOfType(objects, ERamsesObjectType::RamsesObject);
        EXPECT_TRUE(objects.empty());
    }

    TEST_F(ASceneObjectRegistry, canAddAndRetrieveObjectInfo)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        EXPECT_EQ(dummyObject, m_registry.findObjectByName<SceneObject>("name"));
        EXPECT_EQ(dummyObject, m_registry.findObjectById(sceneObjectId_t{1u}));
    }

    TEST_F(ASceneObjectRegistry, canRenameObjectAndFindUnderNewName)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        EXPECT_EQ(dummyObject, m_registry.findObjectByName<SceneObject>("name"));
        EXPECT_EQ(dummyObject, m_registry.findObjectById(sceneObjectId_t{ 1u }));
        dummyObject->setName("newName");
        EXPECT_EQ(dummyObject, m_registry.findObjectByName<SceneObject>("newName"));
        EXPECT_EQ(dummyObject, m_registry.findObjectById(sceneObjectId_t{ 1u }));
        // cannot find under old name
        EXPECT_EQ(nullptr, m_registry.findObjectByName<SceneObject>("name"));
    }

    TEST_F(ASceneObjectRegistry, cannotRetrieveObjectInfoAfterObjectDeleted)
    {
        auto dummyObject = createAndRegisterDummyObject();
        dummyObject->setName("name");
        m_registry.destroyAndUnregisterObject(*dummyObject);
        EXPECT_TRUE(nullptr == m_registry.findObjectByName<SceneObject>("name"));
        EXPECT_TRUE(nullptr == m_registry.findObjectById(sceneObjectId_t{1u}));
    }

    TEST_F(ASceneObjectRegistry, canRetrieveObjectFromImpl)
    {
        auto dummyObject = createAndRegisterDummyObject();
        EXPECT_EQ(dummyObject, &dummyObject->impl().getRamsesObject());
    }

    TEST_F(ASceneObjectRegistry, hasNoDirtyNodesAfterInitialization)
    {
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
    }

    TEST_F(ASceneObjectRegistry, marksNodeDirty)
    {
        auto dummyObject = createAndRegisterDummyObject();
        m_registry.setNodeDirty(dummyObject->impl(), true);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_TRUE(m_registry.getDirtyNodes().contains(&dummyObject->impl()));
        EXPECT_TRUE(m_registry.isNodeDirty(dummyObject->impl()));
    }

    TEST_F(ASceneObjectRegistry, marksNodeClean)
    {
        auto object1(createAndRegisterDummyObject());
        m_registry.setNodeDirty(object1->impl(), true);

        auto object2(createAndRegisterDummyObject());
        m_registry.setNodeDirty(object2->impl(), true);

        EXPECT_EQ(2u, m_registry.getDirtyNodes().size());

        m_registry.setNodeDirty(object1->impl(), false);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_FALSE(m_registry.getDirtyNodes().contains(&object1->impl()));
        EXPECT_FALSE(m_registry.isNodeDirty(object1->impl()));
    }

    TEST_F(ASceneObjectRegistry, clearsDirtyNodes)
    {
        auto dummyObject = createAndRegisterDummyObject();
        m_registry.setNodeDirty(dummyObject->impl(), true);
        m_registry.clearDirtyNodes();
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
        EXPECT_FALSE(m_registry.isNodeDirty(dummyObject->impl()));
    }

    TEST_F(ASceneObjectRegistry, marksNodeCleanWhenDeleted)
    {
        auto dummyObject = createAndRegisterDummyObject();
        m_registry.setNodeDirty(dummyObject->impl(), true);
        m_registry.destroyAndUnregisterObject(*dummyObject);
        EXPECT_EQ(0u, m_registry.getDirtyNodes().size());
    }

    TEST_F(ASceneObjectRegistry, nodeAppearsInDirtyListAfterMarkedDirty)
    {
        auto dummyObject = createAndRegisterDummyObject();
        m_registry.setNodeDirty(dummyObject->impl(), true);
        EXPECT_EQ(1u, m_registry.getDirtyNodes().size());
        EXPECT_TRUE(m_registry.getDirtyNodes().contains(&dummyObject->impl()));
        EXPECT_TRUE(m_registry.isNodeDirty(dummyObject->impl()));
    }
}
