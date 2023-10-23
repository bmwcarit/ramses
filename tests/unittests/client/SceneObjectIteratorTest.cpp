//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/SceneObjectIterator.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/SceneReference.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/logic/LogicEngine.h"

#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"
#include "impl/RamsesObjectTypeTraits.h"

using namespace testing;

namespace ramses::internal
{
    template <typename SceneObjectType>
    class SceneObjectIteratorTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        SceneObjectIteratorTest() : LocalTestClientWithScene()
        {

        }
        ERamsesObjectType getTypeIDForObjectType()
        {
            return TYPE_ID_OF_RAMSES_OBJECT<SceneObjectType>::ID;
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
        EXPECT_TRUE(nullptr != iteratedObject1->as<TypeParam>());
        EXPECT_TRUE(ro2 == iteratedObject2);
        EXPECT_TRUE(nullptr != iteratedObject2->as<TypeParam>());
        EXPECT_TRUE(nullptr == iterator.getNext());
    }

    TYPED_TEST(SceneObjectIteratorTest, ObjectIteratorDirectlyReturnsNullIfNoObjectsAvailable)
    {
        SceneObjectIterator iterator(this->m_scene, this->getTypeIDForObjectType());

        EXPECT_TRUE(nullptr == iterator.getNext());
    }
}
