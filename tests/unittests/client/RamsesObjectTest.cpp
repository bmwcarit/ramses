//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/MeshNode.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/SceneReference.h"
#include "ramses/client/logic/LogicEngine.h"

#include "ramses/client/ramses-utils.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/ValidationReportImpl.h"
#include "RamsesObjectTestTypes.h"
#include "ClientTestUtils.h"

using namespace testing;

namespace ramses::internal
{
    template <typename ObjectType>
    class RamsesObjectTest : public LocalTestClientWithScene, public testing::Test
    {
    };

    TYPED_TEST_SUITE(RamsesObjectTest, RamsesObjectTypes);

    TYPED_TEST(RamsesObjectTest, getType)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");
        constexpr ERamsesObjectType type = TYPE_ID_OF_RAMSES_OBJECT<TypeParam>::ID;
        EXPECT_EQ(type, obj.getType());
        EXPECT_TRUE(obj.isOfType(ERamsesObjectType::RamsesObject));
        static_assert(RamsesObjectTypeUtils::IsConcreteType(type));
    }

    TYPED_TEST(RamsesObjectTest, isOfTypeForAllDefinedBaseClasses)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");
        constexpr ERamsesObjectType type = TYPE_ID_OF_RAMSES_OBJECT<TypeParam>::ID;

        auto baseType = type;
        while (baseType != ERamsesObjectType::Invalid)
        {
            EXPECT_TRUE(RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, baseType));
            EXPECT_TRUE(obj.isOfType(type));
            baseType = RamsesObjectTraits[static_cast<int>(baseType)].baseClassTypeID;
        }
    }

    TYPED_TEST(RamsesObjectTest, getSetName)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        EXPECT_EQ("object", obj.getName());
        EXPECT_TRUE(obj.setName("newName"));
        EXPECT_EQ("newName", obj.getName());

        if (obj.isOfType(ERamsesObjectType::SceneObject))
        {
            EXPECT_EQ(nullptr, this->m_scene.findObject("object"));
            EXPECT_EQ(&obj, this->m_scene.findObject("newName"));
        }
    }

    TYPED_TEST(RamsesObjectTest, getIdentificationString)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const std::string typeName = RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj.getType());
        if constexpr (std::is_base_of_v<SceneObject, TypeParam>)
        {
            EXPECT_EQ(fmt::format("object [{} ScnObjId={}]", typeName, obj.as<SceneObject>()->getSceneObjectId()), obj.impl().getIdentificationString());
        }
        else
        {
            EXPECT_EQ(fmt::format("object [{}]", typeName), obj.impl().getIdentificationString());
        }
    }

    TYPED_TEST(RamsesObjectTest, getSetUserID)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        EXPECT_EQ(0u, obj.getUserId().first);
        EXPECT_EQ(0u, obj.getUserId().second);

        EXPECT_TRUE(obj.setUserId(11u, 22u));
        EXPECT_EQ(11u, obj.getUserId().first);
        EXPECT_EQ(22u, obj.getUserId().second);

        const std::string typeName = RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj.getType());
        if constexpr (std::is_base_of_v<SceneObject, TypeParam>)
        {
            EXPECT_EQ(fmt::format("object [{} UserId=000000000000000B0000000000000016 ScnObjId={}]", typeName, obj.as<SceneObject>()->getSceneObjectId()), obj.impl().getIdentificationString());
        }
        else
        {
            EXPECT_EQ(fmt::format("object [{} UserId=000000000000000B0000000000000016]", typeName), obj.impl().getIdentificationString());
        }
    }

    TYPED_TEST(RamsesObjectTest, convertToTypes)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        EXPECT_TRUE(nullptr != obj.as<TypeParam>());
        EXPECT_TRUE(nullptr != constObj.as<TypeParam>());
        EXPECT_TRUE(nullptr != object_cast<TypeParam*>(&obj));
        EXPECT_TRUE(nullptr != object_cast<const TypeParam*>(&constObj));

        EXPECT_TRUE(nullptr != obj.as<RamsesObject>());
        EXPECT_TRUE(nullptr != constObj.as<RamsesObject>());
        EXPECT_TRUE(nullptr != object_cast<RamsesObject*>(&obj));
        EXPECT_TRUE(nullptr != object_cast<const RamsesObject*>(&constObj));
    }

    TYPED_TEST(RamsesObjectTest, convertToItsClosestBaseClass)
    {
        using BaseClassType = typename CLASS_OF_RAMSES_OBJECT_TYPE<CLASS_OF_RAMSES_OBJECT_TYPE<TYPE_ID_OF_RAMSES_OBJECT<TypeParam>::ID>::BaseTypeID>::ClassType;

        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        if (TYPE_ID_OF_RAMSES_OBJECT<BaseClassType>::ID != ERamsesObjectType::Invalid)
        {
            EXPECT_TRUE(nullptr != obj.as<BaseClassType>());
            EXPECT_TRUE(nullptr != constObj.as<BaseClassType>());
            EXPECT_TRUE(nullptr != object_cast<BaseClassType*>(&obj));
            EXPECT_TRUE(nullptr != object_cast<const BaseClassType*>(&constObj));
        }
    }

    TYPED_TEST(RamsesObjectTest, getRamsesObjectFromImpl)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        RamsesObjectImpl& objImpl = obj.impl();
        const RamsesObjectImpl& constObjImpl = constObj.impl();

        EXPECT_EQ(&obj, &objImpl.getRamsesObject());
        EXPECT_EQ(&constObj, &constObjImpl.getRamsesObject());
    }

    TYPED_TEST(RamsesObjectTest, convertToWrongType)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        if (constObj.getType() != ERamsesObjectType::PerspectiveCamera)
        {
            EXPECT_TRUE(nullptr == obj.as<PerspectiveCamera>());
            EXPECT_TRUE(nullptr == constObj.as<PerspectiveCamera>());
            EXPECT_TRUE(nullptr == object_cast<PerspectiveCamera*>(&obj));
            EXPECT_TRUE(nullptr == object_cast<const PerspectiveCamera*>(&constObj));
        }
        else
        {
            EXPECT_TRUE(nullptr == obj.as<OrthographicCamera>());
            EXPECT_TRUE(nullptr == constObj.as<OrthographicCamera>());
            EXPECT_TRUE(nullptr == object_cast<OrthographicCamera*>(&obj));
            EXPECT_TRUE(nullptr == object_cast<const OrthographicCamera*>(&constObj));
        }
    }

    TEST(RamsesObjectTypesTest, TypeHasName)
    {
        static_assert(static_cast<int>(ERamsesObjectType::TextureSamplerExternal) == 33, "add type name");

        EXPECT_STREQ("Invalid", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Invalid));
        EXPECT_STREQ("ClientObject", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::ClientObject));
        EXPECT_STREQ("RamsesObject", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::RamsesObject));
        EXPECT_STREQ("SceneObject", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::SceneObject));
        EXPECT_STREQ("Client", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Client));
        EXPECT_STREQ("Scene", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Scene));
        EXPECT_STREQ("LogicEngine", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::LogicEngine));
        EXPECT_STREQ("LogicObject", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::LogicObject));
        EXPECT_STREQ("Node", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Node));
        EXPECT_STREQ("MeshNode", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::MeshNode));
        EXPECT_STREQ("Camera", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Camera));
        EXPECT_STREQ("PerspectiveCamera", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::PerspectiveCamera));
        EXPECT_STREQ("OrthographicCamera", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::OrthographicCamera));
        EXPECT_STREQ("Effect", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Effect));
        EXPECT_STREQ("Appearance", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Appearance));
        EXPECT_STREQ("Geometry", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Geometry));
        EXPECT_STREQ("PickableObject", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::PickableObject));
        EXPECT_STREQ("Resource", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Resource));
        EXPECT_STREQ("Texture2D", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Texture2D));
        EXPECT_STREQ("Texture3D", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Texture3D));
        EXPECT_STREQ("TextureCube", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::TextureCube));
        EXPECT_STREQ("ArrayResource", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::ArrayResource));
        EXPECT_STREQ("RenderGroup", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::RenderGroup));
        EXPECT_STREQ("RenderPass", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::RenderPass));
        EXPECT_STREQ("BlitPass", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::BlitPass));
        EXPECT_STREQ("TextureSampler", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::TextureSampler));
        EXPECT_STREQ("TextureSamplerMS", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::TextureSamplerMS));
        EXPECT_STREQ("RenderBuffer", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::RenderBuffer));
        EXPECT_STREQ("RenderTarget", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::RenderTarget));
        EXPECT_STREQ("ArrayBuffer", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::ArrayBuffer));
        EXPECT_STREQ("Texture2DBuffer", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::Texture2DBuffer));
        EXPECT_STREQ("DataObject", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::DataObject));
        EXPECT_STREQ("SceneReference", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::SceneReference));
        EXPECT_STREQ("TextureSamplerExternal", RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType::TextureSamplerExternal));
    }
}
