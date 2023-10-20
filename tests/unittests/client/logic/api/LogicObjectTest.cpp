//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"

#include "impl/logic/LuaModuleImpl.h"
#include "impl/logic/LuaScriptImpl.h"
#include "impl/logic/LuaInterfaceImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/CameraBindingImpl.h"
#include "impl/logic/AppearanceBindingImpl.h"
#include "impl/logic/RenderPassBindingImpl.h"
#include "impl/logic/RenderGroupBindingImpl.h"
#include "impl/logic/MeshNodeBindingImpl.h"
#include "impl/logic/SkinBindingImpl.h"
#include "impl/logic/DataArrayImpl.h"
#include "impl/logic/AnimationNodeImpl.h"
#include "impl/logic/TimerNodeImpl.h"
#include "impl/logic/AnchorPointImpl.h"

#include "impl/RamsesObjectTypeTraits.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/SceneImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkImpl.h"

#include "LogicEngineTestWithCreationHelper.h"

namespace ramses::internal
{
    using namespace testing;

    template <typename ObjectType>
    class LogicOwnershipTest : public ALogicEngineBaseWithCreationHelper, public ::testing::Test
    {
    };

    TYPED_TEST_SUITE(LogicOwnershipTest, LogicObjectTypes);

    TYPED_TEST(LogicOwnershipTest, logicObjectsAreOfTypeLogicObject)
    {
        const auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::LogicObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::SceneObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::RamsesObject));
    }

    TYPED_TEST(LogicOwnershipTest, sceneObjectsHaveReferenceToTheirSceneAndHL)
    {
        auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_EQ(obj, &obj->impl().getLogicObject());
        EXPECT_EQ(this->m_scene, &obj->getScene());
        EXPECT_EQ(&this->m_scene->impl(), &obj->impl().getSceneImpl());
        EXPECT_EQ(&this->m_scene->impl().getIScene(), &obj->impl().getIScene());
        EXPECT_EQ(&this->m_ramses.getClient().impl(), &obj->impl().getClientImpl());
        // also indirectly to framework
        EXPECT_EQ(&this->m_ramses.getFramework(), &obj->impl().getClientImpl().getFramework().getHLRamsesFramework());
    }

    TYPED_TEST(LogicOwnershipTest, sceneObjectsHaveReferenceToTheirScene_const)
    {
        const auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_EQ(obj, &obj->impl().getLogicObject());
        EXPECT_EQ(this->m_scene, &obj->getScene());
        EXPECT_EQ(&this->m_scene->impl(), &obj->impl().getSceneImpl());
        EXPECT_EQ(&this->m_scene->impl().getIScene(), &obj->impl().getIScene());
        EXPECT_EQ(&this->m_ramses.getClient().impl(), &obj->impl().getClientImpl());
        // also indirectly to framework
        EXPECT_EQ(&this->m_ramses.getFramework(), &obj->impl().getClientImpl().getFramework().getHLRamsesFramework());
    }

    TYPED_TEST(LogicOwnershipTest, canFindCreatedObjectInLogicEngineAndScene)
    {
        { // non-const
            const auto* obj = this->template createObjectOfType<TypeParam>("objectName");
            ASSERT_TRUE(obj);

            EXPECT_EQ(obj, this->m_logicEngine->template findObject<LogicObject>("objectName"));
            EXPECT_EQ(obj, this->m_logicEngine->template findObject<TypeParam>("objectName"));
            EXPECT_EQ(obj, this->m_scene->template findObject<LogicObject>("objectName"));
            EXPECT_EQ(obj, this->m_scene->template findObject<SceneObject>("objectName"));

            EXPECT_EQ(obj, this->m_logicEngine->template findObject<LogicObject>(obj->getSceneObjectId()));
            EXPECT_EQ(obj, this->m_logicEngine->template findObject<TypeParam>(obj->getSceneObjectId()));
            EXPECT_EQ(obj, this->m_scene->template findObject<LogicObject>(obj->getSceneObjectId()));
            EXPECT_EQ(obj, this->m_scene->template findObject<SceneObject>(obj->getSceneObjectId()));

            const auto coll = this->m_logicEngine->template getCollection<LogicObject>();
            EXPECT_TRUE(std::find(coll.cbegin(), coll.cend(), obj) != coll.cend());
            const auto collobj = this->m_logicEngine->template getCollection<TypeParam>();
            EXPECT_TRUE(std::find(collobj.cbegin(), collobj.cend(), obj) != collobj.cend());
        }

        { // const
            const auto* obj = this->template createObjectOfType<TypeParam>("objectName2");
            ASSERT_TRUE(obj);
            const auto* logicEngineConst = this->m_logicEngine;
            const auto* sceneConst = this->m_scene;

            EXPECT_EQ(obj, logicEngineConst->template findObject<LogicObject>("objectName2"));
            EXPECT_EQ(obj, logicEngineConst->template findObject<TypeParam>("objectName2"));
            EXPECT_EQ(obj, sceneConst->template findObject<LogicObject>("objectName2"));
            EXPECT_EQ(obj, sceneConst->template findObject<SceneObject>("objectName2"));

            EXPECT_EQ(obj, logicEngineConst->template findObject<LogicObject>(obj->getSceneObjectId()));
            EXPECT_EQ(obj, logicEngineConst->template findObject<TypeParam>(obj->getSceneObjectId()));
            EXPECT_EQ(obj, sceneConst->template findObject<LogicObject>(obj->getSceneObjectId()));
            EXPECT_EQ(obj, sceneConst->template findObject<SceneObject>(obj->getSceneObjectId()));

            const auto coll = logicEngineConst->template getCollection<LogicObject>();
            EXPECT_TRUE(std::find(coll.cbegin(), coll.cend(), obj) != coll.cend());
            const auto collobj = logicEngineConst->template getCollection<TypeParam>();
            EXPECT_TRUE(std::find(collobj.cbegin(), collobj.cend(), obj) != collobj.cend());
        }
    }

    TYPED_TEST(LogicOwnershipTest, objectCanUseUserID)
    {
        auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_TRUE(obj->setUserId(1u, 2u));
        EXPECT_EQ(1u, obj->getUserId().first);
        EXPECT_EQ(2u, obj->getUserId().second);
        EXPECT_TRUE(obj->setUserId(3u, 4u));
        EXPECT_EQ(3u, obj->getUserId().first);
        EXPECT_EQ(4u, obj->getUserId().second);
    }

    TYPED_TEST(LogicOwnershipTest, objectCanBeCast)
    {
        auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        ASSERT_TRUE(obj->template as<LogicObject>());
        EXPECT_TRUE(obj->template as<LogicObject>()->template as<TypeParam>());
        EXPECT_TRUE(object_cast<TypeParam*>(object_cast<LogicObject*>(obj)));

        const auto* objConst = obj;
        ASSERT_TRUE(objConst->template as<LogicObject>());
        EXPECT_TRUE(objConst->template as<LogicObject>()->template as<TypeParam>());
        EXPECT_TRUE(object_cast<const TypeParam*>(object_cast<const LogicObject*>(objConst)));
    }

    TYPED_TEST(LogicOwnershipTest, objectCanBeCastFromRamsesObject)
    {
        RamsesObject* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_TRUE(obj->as<LogicObject>());
        EXPECT_TRUE(obj->as<SceneObject>());
        EXPECT_TRUE(obj->as<TypeParam>());
        EXPECT_TRUE(object_cast<SceneObject*>(obj));
        EXPECT_TRUE(object_cast<LogicObject*>(obj));
        EXPECT_TRUE(object_cast<TypeParam*>(obj));

        const RamsesObject* objConst = obj;
        EXPECT_TRUE(objConst->as<SceneObject>());
        EXPECT_TRUE(objConst->as<LogicObject>());
        EXPECT_TRUE(objConst->as<TypeParam>());
        EXPECT_TRUE(object_cast<const SceneObject*>(objConst));
        EXPECT_TRUE(object_cast<const LogicObject*>(objConst));
        EXPECT_TRUE(object_cast<const TypeParam*>(objConst));
    }

    TYPED_TEST(LogicOwnershipTest, logicEngineDoesNotContainDestroyedObject)
    {
        auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_TRUE(this->m_logicEngine->destroy(*obj));
        EXPECT_EQ(nullptr, this->m_logicEngine->template findObject<LogicObject>("objectName"));
    }

    TYPED_TEST(LogicOwnershipTest, sceneReportsErrorIfTryingToDestroyLogicObjectInIt)
    {
        auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_FALSE(this->m_scene->destroy(*obj));
        EXPECT_EQ(obj, this->m_logicEngine->template findObject<LogicObject>("objectName"));
    }

    TYPED_TEST(LogicOwnershipTest, objectNameChanged)
    {
        auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_TRUE(obj->setName("otherObjectName"));
        EXPECT_EQ(nullptr, this->m_logicEngine->template findObject<LogicObject>("objectName"));
        EXPECT_EQ(obj, this->m_logicEngine->template findObject<LogicObject>("otherObjectName"));
    }

    TYPED_TEST(LogicOwnershipTest, objectFailsToBeDestroyedIfFromOtherLogicEngine)
    {
        auto otherEngine = this->m_scene->createLogicEngine();
        ASSERT_TRUE(otherEngine);

        auto* obj = this->template createObjectOfType<TypeParam>("objectName");
        ASSERT_TRUE(obj);
        EXPECT_FALSE(otherEngine->destroy(*obj));
    }

    TYPED_TEST(LogicOwnershipTest, hasAllPropertiesAfterDeserialization)
    {
        this->withTempDirectory();

        sceneObjectId_t id{};
        {
            // fill with some objects first
            this->template createObjectOfType<LuaModule>("objectName");
            this->template createObjectOfType<LuaScript>("objectName");
            this->template createObjectOfType<LuaInterface>("objectName");
            this->template createObjectOfType<NodeBinding>("objectName");
            this->template createObjectOfType<AppearanceBinding>("objectName");
            this->template createObjectOfType<CameraBinding>("objectName");
            this->template createObjectOfType<RenderPassBinding>("objectName");
            this->template createObjectOfType<RenderGroupBinding>("objectName");
            this->template createObjectOfType<MeshNodeBinding>("objectName");
            this->template createObjectOfType<SkinBinding>("objectName");
            this->template createObjectOfType<DataArray>("objectName");
            this->template createObjectOfType<AnimationNode>("objectName");
            this->template createObjectOfType<TimerNode>("objectName");
            this->template createObjectOfType<AnchorPoint>("objectName");

            auto* obj = this->template createObjectOfType<TypeParam>("objectName");
            ASSERT_TRUE(obj);

            // set/change some parameters
            id = obj->getSceneObjectId();
            EXPECT_TRUE(obj->setName("newName"));
            EXPECT_TRUE(obj->setUserId(1u, 2u));

            ASSERT_TRUE(this->saveToFileWithoutValidation("file.tmp"));
            ASSERT_TRUE(this->recreateFromFile("file.tmp"));
        }

        const auto* obj = this->m_logicEngine->template findObject<TypeParam>(id);
        ASSERT_TRUE(obj);

        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::LogicObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::SceneObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::RamsesObject));

        EXPECT_EQ(obj, &obj->impl().getLogicObject());
        EXPECT_EQ(this->m_scene, &obj->getScene());
        EXPECT_EQ(&this->m_scene->impl(), &obj->impl().getSceneImpl());
        EXPECT_EQ(&this->m_scene->impl().getIScene(), &obj->impl().getIScene());
        EXPECT_EQ(&this->m_ramses.getClient().impl(), &obj->impl().getClientImpl());
        EXPECT_EQ(&this->m_ramses.getFramework(), &obj->impl().getClientImpl().getFramework().getHLRamsesFramework());

        EXPECT_EQ(obj, this->m_logicEngine->template findObject<LogicObject>("newName"));
        EXPECT_EQ(obj, this->m_logicEngine->template findObject<TypeParam>("newName"));
        EXPECT_EQ(obj, this->m_scene->template findObject<LogicObject>("newName"));
        EXPECT_EQ(obj, this->m_scene->template findObject<SceneObject>("newName"));
        EXPECT_EQ(obj, this->m_logicEngine->template findObject<LogicObject>(obj->getSceneObjectId()));
        EXPECT_EQ(obj, this->m_logicEngine->template findObject<TypeParam>(obj->getSceneObjectId()));
        EXPECT_EQ(obj, this->m_logicEngine->findObject(obj->getSceneObjectId()));
        EXPECT_EQ(obj, this->m_scene->template findObject<LogicObject>(obj->getSceneObjectId()));
        EXPECT_EQ(obj, this->m_scene->template findObject<SceneObject>(obj->getSceneObjectId()));

        const auto coll = this->m_logicEngine->template getCollection<LogicObject>();
        EXPECT_TRUE(std::find(coll.cbegin(), coll.cend(), obj) != coll.cend());
        const auto collobj = this->m_logicEngine->template getCollection<TypeParam>();
        EXPECT_TRUE(std::find(collobj.cbegin(), collobj.cend(), obj) != collobj.cend());

        // check all ids unique
        std::unordered_set<sceneObjectId_t> ids;
        for (const auto* o : coll)
            ids.insert(o->getSceneObjectId());
        EXPECT_EQ(ids.size(), coll.size());

        ASSERT_TRUE(obj->template as<LogicObject>());
        EXPECT_TRUE(obj->template as<LogicObject>()->template as<TypeParam>());
        EXPECT_TRUE(object_cast<const TypeParam*>(object_cast<const LogicObject*>(obj)));

        EXPECT_EQ(1u, obj->getUserId().first);
        EXPECT_EQ(2u, obj->getUserId().second);
    }
}
