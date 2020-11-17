//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/SceneObjectIterator.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/DataMatrix22f.h"
#include "ramses-client-api/DataMatrix33f.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector3i.h"
#include "ramses-client-api/DataVector4i.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/SceneReference.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-utils.h"

#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"
#include "RamsesObjectTypeTraits.h"

using namespace testing;

namespace ramses
{
    template <typename SceneObjectType>
    class SceneObjectIteratorTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        SceneObjectIteratorTest() : LocalTestClientWithScene()
        {

        }
        ramses::ERamsesObjectType getTypeIDForObjectType()
        {
            return ramses::TYPE_ID_OF_RAMSES_OBJECT<SceneObjectType>::ID;
        }
    };

    TYPED_TEST_SUITE(SceneObjectIteratorTest, SceneObjectTypes);

    TYPED_TEST(SceneObjectIteratorTest, ObjectIteratorReturnsMultipleObjects)
    {
        RamsesObject* ro1 = &this->template createObject<TypeParam>();
        RamsesObject* ro2 = &this->template createObject<TypeParam>();

        SceneObjectIterator iterator(this->m_scene, this->getTypeIDForObjectType());

        RamsesObject* iteratedObject1 = iterator.getNext();
        RamsesObject* iteratedObject2 = iterator.getNext();

        ASSERT_TRUE(nullptr != iteratedObject1);
        ASSERT_TRUE(nullptr != iteratedObject2);
        EXPECT_TRUE(ro1 == iteratedObject1);
        EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<TypeParam>(*iteratedObject1));
        EXPECT_TRUE(ro2 == iteratedObject2);
        EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<TypeParam>(*iteratedObject2));
        EXPECT_TRUE(nullptr == iterator.getNext());
    }

    TYPED_TEST(SceneObjectIteratorTest, ObjectIteratorDirectlyReturnsNullIfNoObjectsAvailable)
    {
        SceneObjectIterator iterator(this->m_scene, this->getTypeIDForObjectType());

        EXPECT_TRUE(nullptr == iterator.getNext());
    }
}
