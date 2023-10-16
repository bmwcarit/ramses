//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/logic/LogicEngine.h"

#include "impl/RamsesClientImpl.h"
#include "impl/SceneImpl.h"
#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"
#include "impl/NodeImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/CameraNodeImpl.h"
#include "impl/GeometryImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/RenderPassImpl.h"
#include "impl/BlitPassImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/RenderTargetImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/DataObjectImpl.h"
#include "impl/ArrayBufferImpl.h"
#include "impl/Texture2DBufferImpl.h"
#include "impl/PickableObjectImpl.h"
#include "impl/SceneReferenceImpl.h"
#include "impl/Texture2DImpl.h"
#include "impl/Texture3DImpl.h"
#include "impl/TextureCubeImpl.h"
#include "impl/EffectImpl.h"
#include "impl/logic/LogicEngineImpl.h"

#include "impl/RamsesObjectTypeTraits.h"
#include "impl/RamsesObjectTypeUtils.h"

namespace ramses::internal
{
    using namespace testing;

    template <typename ObjectType>
    class SceneOwnershipTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        SceneOwnershipTest() : LocalTestClientWithScene()
        {
        }

        void expectNoFrameworkObjectsAllocated()
        {
            m_creationHelper.destroyAdditionalAllocatedSceneObjects();
            const ramses::internal::IScene& scene = this->m_scene.impl().getIScene();

            for (ramses::internal::NodeHandle i(0); i < scene.getNodeCount(); ++i)
            {
                EXPECT_FALSE(scene.isNodeAllocated(i));
            }

            for (ramses::internal::CameraHandle i(0); i < scene.getCameraCount(); ++i)
            {
                EXPECT_FALSE(scene.isCameraAllocated(i));
            }

            for (ramses::internal::TransformHandle i(0); i < scene.getTransformCount(); ++i)
            {
                EXPECT_FALSE(scene.isTransformAllocated(i));
            }

            for (ramses::internal::RenderableHandle i(0); i < scene.getRenderableCount(); ++i)
            {
                EXPECT_FALSE(scene.isRenderableAllocated(i));
            }

            for (ramses::internal::RenderStateHandle i(0); i < scene.getRenderStateCount(); ++i)
            {
                EXPECT_FALSE(scene.isRenderStateAllocated(i));
            }

            for (ramses::internal::DataLayoutHandle i(0); i < scene.getDataLayoutCount(); ++i)
            {
                EXPECT_FALSE(scene.isDataLayoutAllocated(i));
            }

            for (ramses::internal::DataInstanceHandle i(0); i < scene.getDataInstanceCount(); ++i)
            {
                EXPECT_FALSE(scene.isDataInstanceAllocated(i));
            }

            for (ramses::internal::RenderPassHandle i(0); i < scene.getRenderPassCount(); ++i)
            {
                EXPECT_FALSE(scene.isRenderPassAllocated(i));
            }

            for (ramses::internal::RenderTargetHandle i(0); i < scene.getRenderTargetCount(); ++i)
            {
                EXPECT_FALSE(scene.isRenderTargetAllocated(i));
            }

            for (ramses::internal::TextureSamplerHandle i(0); i < scene.getTextureSamplerCount(); ++i)
            {
                EXPECT_FALSE(scene.isTextureSamplerAllocated(i));
            }
        }
    };

    template <typename ResourceType>
    class ClientOwnershipTest : public LocalTestClientWithScene, public testing::Test{};

    TYPED_TEST_SUITE(SceneOwnershipTest, SceneObjectTypes);
    TYPED_TEST_SUITE(ClientOwnershipTest, ClientObjectTypes);

    TYPED_TEST(SceneOwnershipTest, sceneObjectsAreOfTypeSceneObject)
    {
        auto obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::SceneObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::ClientObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::RamsesObject));
    }

    TYPED_TEST(SceneOwnershipTest, sceneObjectsHaveReferenceToTheirScene)
    {
        auto* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_EQ(&this->m_scene, &obj->getScene());
        EXPECT_EQ(&this->m_scene.impl(), &obj->impl().getSceneImpl());
        EXPECT_EQ(&this->m_scene.impl().getIScene(), &obj->impl().getIScene());
        EXPECT_EQ(&this->client.impl(), &obj->impl().getClientImpl());
        // also indirectly to framework
        EXPECT_EQ(&this->framework, &obj->impl().getClientImpl().getFramework().getHLRamsesFramework());
    }

    TYPED_TEST(SceneOwnershipTest, sceneObjectsHaveReferenceToTheirScene_const)
    {
        const auto* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_EQ(&this->m_scene, &obj->getScene());
        EXPECT_EQ(&this->m_scene.impl(), &obj->impl().getSceneImpl());
        EXPECT_EQ(&this->m_scene.impl().getIScene(), &obj->impl().getIScene());
        EXPECT_EQ(&this->client.impl(), &obj->impl().getClientImpl());
        // also indirectly to framework
        EXPECT_EQ(&this->framework, &obj->impl().getClientImpl().getFramework().getHLRamsesFramework());
    }

    TYPED_TEST(SceneOwnershipTest, sceneContainsCreatedObject)
    {
        const RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        const RamsesObject* sn = this->m_scene.findObject("objectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
        EXPECT_EQ(obj, this->m_constRefToScene.findObject("objectName"));
    }

    TYPED_TEST(SceneOwnershipTest, sceneDoesNotContainDestroyedObject)
    {
        auto obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(this->m_scene.destroy(*obj));
        const RamsesObject* sn = this->m_scene.findObject("objectName");
        ASSERT_TRUE(nullptr == sn);

        this->expectNoFrameworkObjectsAllocated();
    }

    TYPED_TEST(SceneOwnershipTest, creatingAndDestroyingObjectsUpdatesStatisticCounter)
    {
        this->m_scene.impl().getStatisticCollection().nextTimeInterval(); //object number is updated by nextTimeInterval()
        uint32_t initialNumber = this->m_scene.impl().getStatisticCollection().statObjectsCount.getCounterValue();
        EXPECT_EQ(0u, this->m_scene.impl().getStatisticCollection().statObjectsCreated.getCounterValue());
        EXPECT_EQ(0u, this->m_scene.impl().getStatisticCollection().statObjectsDestroyed.getCounterValue());

        auto obj = &this->template createObject<TypeParam>("objectName");
        uint32_t numberCreated = this->m_scene.impl().getStatisticCollection().statObjectsCreated.getCounterValue();
        EXPECT_LE(1u, numberCreated); //some types create multiple scene objects (e.g. RenderTarget)
        EXPECT_EQ(0u, this->m_scene.impl().getStatisticCollection().statObjectsDestroyed.getCounterValue());

        this->m_scene.impl().getStatisticCollection().nextTimeInterval();
        EXPECT_EQ(initialNumber + numberCreated, this->m_scene.impl().getStatisticCollection().statObjectsCount.getCounterValue());

        EXPECT_TRUE(this->m_scene.destroy(*obj));

        EXPECT_LE(1u, this->m_scene.impl().getStatisticCollection().statObjectsDestroyed.getCounterValue());

        this->m_scene.impl().getStatisticCollection().nextTimeInterval();
        EXPECT_GT(initialNumber + numberCreated, this->m_scene.impl().getStatisticCollection().statObjectsCount.getCounterValue());
        EXPECT_LE(initialNumber, this->m_scene.impl().getStatisticCollection().statObjectsCount.getCounterValue());
    }

    TYPED_TEST(SceneOwnershipTest, sceneObjectsCanBeFound)
    {
        auto* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        const auto id = obj->getSceneObjectId();
        const auto& constScene = this->m_scene;
        EXPECT_EQ(this->m_scene.template findObject<TypeParam>("objectName"), obj);
        EXPECT_EQ(constScene.template findObject<TypeParam>("objectName"), obj);
        EXPECT_EQ(this->m_scene.template findObject<TypeParam>(id), obj);
        EXPECT_EQ(constScene.template findObject<TypeParam>(id), obj);
        // base type
        EXPECT_EQ(this->m_scene.template findObject<SceneObject>("objectName"), obj);
        EXPECT_EQ(constScene.template findObject<SceneObject>("objectName"), obj);
        EXPECT_EQ(this->m_scene.template findObject<SceneObject>(id), obj);
        EXPECT_EQ(constScene.template findObject<SceneObject>(id), obj);
        // wrong type
        if constexpr (std::is_same_v<TypeParam, MeshNode>)
        {
            EXPECT_EQ(this->m_scene.template findObject<Appearance>("objectName"), nullptr);
            EXPECT_EQ(constScene.template findObject<Appearance>("objectName"), nullptr);
            EXPECT_EQ(this->m_scene.template findObject<Appearance>(id), nullptr);
            EXPECT_EQ(constScene.template findObject<Appearance>(id), nullptr);
        }
        else
        {
            EXPECT_EQ(this->m_scene.template findObject<MeshNode>("objectName"), nullptr);
            EXPECT_EQ(constScene.template findObject<MeshNode>("objectName"), nullptr);
            EXPECT_EQ(this->m_scene.template findObject<MeshNode>(id), nullptr);
            EXPECT_EQ(constScene.template findObject<MeshNode>(id), nullptr);
        }
    }

    TYPED_TEST(ClientOwnershipTest, clientContainsCreatedObject)
    {
        const RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        const RamsesObject* sn = this->client.findSceneByName("objectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
    }

    TYPED_TEST(ClientOwnershipTest, clientDoesNotContainDestroyedObject)
    {
        RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(this->client.destroy(static_cast<ramses::Scene&>(*obj)));
        const RamsesObject* sn = this->client.findSceneByName("objectName");
        ASSERT_TRUE(nullptr == sn);
    }

    TYPED_TEST(SceneOwnershipTest, sceneObjectNameChanged)
    {
        RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(obj->setName("otherObjectName"));
        RamsesObject* sn = this->m_scene.findObject("otherObjectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
    }

    TYPED_TEST(ClientOwnershipTest, clientObjectsAreOfTypeClientObject)
    {
        const ClientObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::ClientObject));
    }

    TYPED_TEST(ClientOwnershipTest, clientObjectsHaveReferenceToTheirClient)
    {
        const ClientObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_EQ(&this->client.impl(), &obj->impl().getClientImpl());
        // also indirectly to framework
        EXPECT_EQ(&this->framework, &obj->impl().getClientImpl().getFramework().getHLRamsesFramework());
    }

    TYPED_TEST(ClientOwnershipTest, clientObjectNameChanged)
    {
        RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(obj->setName("otherObjectName"));
        RamsesObject* sn = this->client.findSceneByName("otherObjectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
    }

    TYPED_TEST(SceneOwnershipTest, objectFailsToBeDestroyedIfFromOtherScene)
    {
        auto& otherScene = this->template createObject<ramses::Scene>("otherScene");

        RamsesObject& obj = this->template createObject<TypeParam>("objectName");
        auto& objTyped = static_cast<TypeParam&>(obj);

        EXPECT_FALSE(otherScene.destroy(objTyped));
        EXPECT_TRUE(this->framework.getLastError().has_value());
    }
}
