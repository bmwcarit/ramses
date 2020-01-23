//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "framework_common_gmock_header.h"
#include "Resource/EffectResource.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include <memory>

namespace ramses_internal
{
    class AEffectResource : public testing::Test
    {
    public:
        AEffectResource()
        {
            EffectInputInformation uniformA;
            uniformA.inputName = "uni A";
            uniformInputs.push_back(uniformA);
            EffectInputInformation uniformB;
            uniformB.inputName = "uni B";
            uniformInputs.push_back(uniformB);

            EffectInputInformation attributeA;
            attributeA.inputName = "attr A";
            attributeInputs.push_back(attributeA);
        }

        EffectResource* serializeDeserialize(const EffectResource& effectResource, const String& name)
        {
            BinaryOutputStream outStream;
            effectResource.serializeResourceMetadataToStream(outStream);
            BinaryInputStream inStream(outStream.getData());
            IResource* resource = EffectResource::CreateResourceFromMetadataStream(inStream, effectResource.getCacheFlag(), name);
            if (!resource)
            {
                return nullptr;
            }
            const auto& resourceData = effectResource.getResourceData();
            resource->setResourceData(ResourceBlob(resourceData.size(), resourceData.data()));
            return resource->convertTo<EffectResource>();
        }

        EffectInputInformationVector uniformInputs;
        EffectInputInformationVector attributeInputs;
    };

    TEST_F(AEffectResource, canBeCreatedWithName)
    {
        EffectResource effect("", "", EffectInputInformationVector(), EffectInputInformationVector(), "myname", ResourceCacheFlag(0u));
        EXPECT_EQ(String("myname"), effect.getName());
    }

    TEST_F(AEffectResource, canBeCreatedWithShaders)
    {
        EffectResource effect("verttext", "fragtext", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag(0u));
        EXPECT_STREQ("verttext", effect.getVertexShader());
        EXPECT_STREQ("fragtext", effect.getFragmentShader());
    }

    TEST_F(AEffectResource, canBeCreatedWithInputs)
    {
        EffectResource effect("", "", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EXPECT_EQ(uniformInputs, effect.getUniformInputs());
        EXPECT_EQ(attributeInputs, effect.getAttributeInputs());
    }

    TEST_F(AEffectResource, canGetInputsByName)
    {
        EffectResource effect("", "", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));

        EXPECT_EQ(DataFieldHandle(1), effect.getUniformDataFieldHandleByName("uni B"));
        EXPECT_EQ(DataFieldHandle::Invalid(), effect.getUniformDataFieldHandleByName("does not exist"));

        EXPECT_EQ(DataFieldHandle(0), effect.getAttributeDataFieldHandleByName("attr A"));
        EXPECT_EQ(DataFieldHandle::Invalid(), effect.getAttributeDataFieldHandleByName("also does not exist"));
    }

    TEST_F(AEffectResource, sameParametersGiveSameHash)
    {
        EffectResource effect1("asd", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EffectResource effect2("asd", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EXPECT_EQ(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentVertexShaderResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EffectResource effect2("XXX", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentFragmentShaderResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EffectResource effect2("asd", "XXX", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentUniformInputResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EffectResource effect2("asd", "XXX", EffectInputInformationVector(), attributeInputs, "", ResourceCacheFlag(0u));
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentAttributeInputResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        EffectResource effect2("asd", "XXX", uniformInputs, EffectInputInformationVector(), "", ResourceCacheFlag(0u));
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentNameDoesNotChangeHash)
    {
        EffectResource effect1("asd", "def", uniformInputs, attributeInputs, "some name", ResourceCacheFlag(0u));
        EffectResource effect2("asd", "def", uniformInputs, attributeInputs, "different name", ResourceCacheFlag(0u));
        EXPECT_EQ(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, hasCorrectTypeAfterSerializeAndDeserialize)
    {
        EffectResource effectBefore("asd", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(0u));
        std::unique_ptr<EffectResource> effectAfter(serializeDeserialize(effectBefore, ""));
        ASSERT_TRUE(effectAfter);

        EXPECT_EQ(EResourceType_Effect, effectAfter->getTypeID());
    }

    TEST_F(AEffectResource, isEqualAfterSerializeAndDeserialize)
    {
        EffectResource effectBefore("asd", "def", uniformInputs, attributeInputs, "", ResourceCacheFlag(15u));
        std::unique_ptr<EffectResource> effectAfter(serializeDeserialize(effectBefore, ""));
        ASSERT_TRUE(effectAfter);

        EXPECT_EQ(effectBefore.getHash(), effectAfter->getHash());
        EXPECT_STREQ(effectBefore.getVertexShader(), effectAfter->getVertexShader());
        EXPECT_STREQ(effectBefore.getFragmentShader(), effectAfter->getFragmentShader());
        EXPECT_EQ(effectBefore.getUniformInputs(), effectAfter->getUniformInputs());
        EXPECT_EQ(effectBefore.getAttributeInputs(), effectAfter->getAttributeInputs());
        EXPECT_EQ(effectBefore.getCacheFlag(), effectAfter->getCacheFlag());
    }

    TEST_F(AEffectResource, hasNameProvidedToSerializeAfterSerializeAndDeserialize)
    {
        EffectResource effectBefore("asd", "def", uniformInputs, attributeInputs, "some name", ResourceCacheFlag(0u));
        std::unique_ptr<EffectResource> effectAfter(serializeDeserialize(effectBefore, "different name"));
        ASSERT_TRUE(effectAfter);

        EXPECT_EQ(String("different name"), effectAfter->getName());
    }
}
