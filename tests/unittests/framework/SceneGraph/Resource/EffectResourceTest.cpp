//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include <memory>

namespace ramses::internal
{
    class AEffectResource : public testing::Test
    {
    public:
        AEffectResource()
        {
            uniformInputs.emplace_back("uni A", 10u, EDataType::Bool, EFixedSemantics::CameraWorldPosition);
            uniformInputs.emplace_back("uni B", 1u, EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{11u}, UniformBufferElementSize{12u}, UniformBufferFieldOffset{13u} );

            attributeInputs.emplace_back("attr A", 1u, EDataType::ByteBlob, EFixedSemantics::Invalid);
        }

        static std::unique_ptr<EffectResource> SerializeDeserialize(const EffectResource& effectResource, std::string_view name)
        {
            BinaryOutputStream outStream;
            effectResource.serializeResourceMetadataToStream(outStream);
            BinaryInputStream inStream(outStream.getData());
            std::unique_ptr<IResource> resource = EffectResource::CreateResourceFromMetadataStream(inStream, name, EFeatureLevel_Latest);
            if (!resource)
            {
                return nullptr;
            }
            const auto& resourceData = effectResource.getResourceData();
            resource->setResourceData(ResourceBlob(resourceData.size(), resourceData.data()));
            return std::unique_ptr<EffectResource>(static_cast<EffectResource*>(resource.release()));
        }

        void checkSpirvShaders(const EffectResource& effectRes, bool withGeometryShader = true)
        {
            ASSERT_EQ(effectRes.getVertexShaderSPIRVSize(), dummySpirvShaders.m_vertexSPIRVBlob.size() * sizeof(uint32_t));
            ASSERT_EQ(effectRes.getFragmentShaderSPIRVSize(), dummySpirvShaders.m_fragmentSPIRVBlob.size() * sizeof(uint32_t));
            ASSERT_EQ(effectRes.getGeometryShaderSPIRVSize(), withGeometryShader ? dummySpirvShaders.m_geometrySPIRVBlob.size() * sizeof(uint32_t) : 0u);

            // check alignment
            EXPECT_EQ(0u, uintptr_t(effectRes.getVertexShaderSPIRV()) % sizeof(uint32_t));
            EXPECT_EQ(0u, uintptr_t(effectRes.getFragmentShaderSPIRV()) % sizeof(uint32_t));

            // check contents
            const SPIRVShaderBlob effectVertexSPIRV(effectRes.getVertexShaderSPIRV(), effectRes.getVertexShaderSPIRV() + effectRes.getVertexShaderSPIRVSize() / sizeof(uint32_t));
            const SPIRVShaderBlob effectFragmentSPIRV(effectRes.getFragmentShaderSPIRV(), effectRes.getFragmentShaderSPIRV() + effectRes.getFragmentShaderSPIRVSize() / sizeof(uint32_t));
            EXPECT_EQ(dummySpirvShaders.m_vertexSPIRVBlob, effectVertexSPIRV);
            EXPECT_EQ(dummySpirvShaders.m_fragmentSPIRVBlob, effectFragmentSPIRV);

            if(withGeometryShader)
            {
                EXPECT_EQ(0u, uintptr_t(effectRes.getGeometryShaderSPIRV()) % sizeof(uint32_t));
                const SPIRVShaderBlob effectGeometrySPIRV(effectRes.getGeometryShaderSPIRV(), effectRes.getGeometryShaderSPIRV() + effectRes.getGeometryShaderSPIRVSize() / sizeof(uint32_t));
                EXPECT_EQ(dummySpirvShaders.m_geometrySPIRVBlob, effectGeometrySPIRV);
            }
        }

        EffectInputInformationVector uniformInputs;
        EffectInputInformationVector attributeInputs;
        SPIRVShaders dummySpirvShaders{
            SPIRVShaderBlob{ 1u, 2u, 3u, 4u },
            SPIRVShaderBlob{ 5u, 6u, 7u, 8u, 9u },
            SPIRVShaderBlob{ 10u, 11u, 12u} };
    };

    TEST_F(AEffectResource, canBeCreatedWithName)
    {
        EffectResource effect("", "", "", {}, {}, EffectInputInformationVector(), EffectInputInformationVector(), "myname", EFeatureLevel_Latest);
        EXPECT_EQ(std::string{"myname"}, effect.getName());
        EXPECT_FALSE(effect.getGeometryShaderInputType().has_value());
    }

    TEST_F(AEffectResource, canBeCreatedWithShaders)
    {
        EffectResource effect("verttext", "fragtext", "geomtext", {}, EDrawMode::Lines, EffectInputInformationVector(), EffectInputInformationVector(), "", EFeatureLevel_Latest);
        EXPECT_STREQ("verttext", effect.getVertexShader());
        EXPECT_STREQ("fragtext", effect.getFragmentShader());
        EXPECT_STREQ("geomtext", effect.getGeometryShader());
        EXPECT_EQ(EDrawMode::Lines, effect.getGeometryShaderInputType());
    }

    TEST_F(AEffectResource, canBeCreatedWithSPIRVShaders)
    {
        EffectResource effect("verttext", "fragtext", "geomtext", dummySpirvShaders, EDrawMode::Lines, EffectInputInformationVector(), EffectInputInformationVector(), "", EFeatureLevel_Latest);
        EXPECT_STREQ("verttext", effect.getVertexShader());
        EXPECT_STREQ("fragtext", effect.getFragmentShader());
        EXPECT_STREQ("geomtext", effect.getGeometryShader());
        EXPECT_EQ(EDrawMode::Lines, effect.getGeometryShaderInputType());

        checkSpirvShaders(effect);
    }

    TEST_F(AEffectResource, canBeCreatedWithoutGeometryWithSPIRVShaders)
    {
        SPIRVShaders dummySpirvWithGeom{ dummySpirvShaders.m_vertexSPIRVBlob, dummySpirvShaders.m_fragmentSPIRVBlob, SPIRVShaderBlob{} };
        EffectResource effect("verttext", "fragtext", "", dummySpirvWithGeom, std::nullopt, EffectInputInformationVector(), EffectInputInformationVector(), "", EFeatureLevel_Latest);
        EXPECT_STREQ("verttext", effect.getVertexShader());
        EXPECT_STREQ("fragtext", effect.getFragmentShader());
        EXPECT_STREQ("", effect.getGeometryShader());
        EXPECT_FALSE(effect.getGeometryShaderInputType().has_value());

        checkSpirvShaders(effect, false);
    }

    TEST_F(AEffectResource, canBeCreatedWithInputs)
    {
        EffectResource effect("", "", "", {}, {}, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_EQ(uniformInputs, effect.getUniformInputs());
        EXPECT_EQ(attributeInputs, effect.getAttributeInputs());
        EXPECT_FALSE(effect.getGeometryShaderInputType().has_value());
    }

    TEST_F(AEffectResource, canGetInputsByName)
    {
        EffectResource effect("", "", "", {}, {}, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);

        EXPECT_EQ(DataFieldHandle(1), effect.getUniformDataFieldHandleByName("uni B"));
        EXPECT_EQ(DataFieldHandle::Invalid(), effect.getUniformDataFieldHandleByName("does not exist"));

        EXPECT_EQ(DataFieldHandle(0), effect.getAttributeDataFieldHandleByName("attr A"));
        EXPECT_EQ(DataFieldHandle::Invalid(), effect.getAttributeDataFieldHandleByName("also does not exist"));
    }

    TEST_F(AEffectResource, sameParametersGiveSameHash)
    {
        EffectResource effect1("asd", "def", "xyz", dummySpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("asd", "def", "xyz", dummySpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_EQ(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentVertexShaderResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("XXX", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentFragmentShaderResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("asd", "XXX", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentGeometryShaderResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("asd", "def", "XXX", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentUniformInputResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("asd", "def", "xyz", {}, EDrawMode::Lines, EffectInputInformationVector(), attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentAttributeInputResultsInDifferentHash)
    {
        EffectResource effect1("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, EffectInputInformationVector(), "", EFeatureLevel_Latest);
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentVertexShaderSPIRVResultsInDifferentHash)
    {
        SPIRVShaders otherSpirvShaders = dummySpirvShaders;
        otherSpirvShaders.m_vertexSPIRVBlob = SPIRVShaderBlob(9u, 9u);
        EffectResource effect1("asd", "def", "xyz", dummySpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("asd", "def", "xyz", otherSpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentFragmentShaderSPIRVResultsInDifferentHash)
    {
        SPIRVShaders otherSpirvShaders = dummySpirvShaders;
        otherSpirvShaders.m_fragmentSPIRVBlob = SPIRVShaderBlob(9u, 9u);
        EffectResource effect1("asd", "def", "xyz", dummySpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("asd", "def", "xyz", otherSpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentGeometryShaderSPIRVResultsInDifferentHash)
    {
        SPIRVShaders otherSpirvShaders = dummySpirvShaders;
        otherSpirvShaders.m_geometrySPIRVBlob = SPIRVShaderBlob(9u, 9u);
        EffectResource effect1("asd", "def", "xyz", dummySpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EffectResource effect2("asd", "def", "xyz", otherSpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        EXPECT_NE(effect1.getHash(), effect2.getHash());
    }

    TEST_F(AEffectResource, differentNameDoesNotChangeHash)
    {
        EffectResource effect1("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "some name", EFeatureLevel_Latest);
        EffectResource effect2("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "different name", EFeatureLevel_Latest);
        EXPECT_EQ(effect1.getHash(), effect2.getHash());
    }

    // TODO (backported) once 27 merged to master, add check that effect resource serialization also works with expected geometry input type (using nullopt everywhere here)

    TEST_F(AEffectResource, hasCorrectTypeAfterSerializeAndDeserialize)
    {
        EffectResource effectBefore("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        std::unique_ptr<EffectResource> effectAfter(SerializeDeserialize(effectBefore, ""));
        ASSERT_TRUE(effectAfter);

        EXPECT_EQ(EResourceType::Effect, effectAfter->getTypeID());
    }

    TEST_F(AEffectResource, isEqualAfterSerializeAndDeserialize)
    {
        EffectResource effectBefore("asd", "def", "xyz", dummySpirvShaders, EDrawMode::Lines, uniformInputs, attributeInputs, "", EFeatureLevel_Latest);
        std::unique_ptr<EffectResource> effectAfter(SerializeDeserialize(effectBefore, ""));
        ASSERT_TRUE(effectAfter);

        EXPECT_EQ(effectBefore.getHash(), effectAfter->getHash());
        EXPECT_STREQ(effectBefore.getVertexShader(), effectAfter->getVertexShader());
        EXPECT_STREQ(effectBefore.getFragmentShader(), effectAfter->getFragmentShader());
        EXPECT_STREQ(effectBefore.getGeometryShader(), effectAfter->getGeometryShader());
        EXPECT_EQ(effectBefore.getGeometryShaderInputType(), effectAfter->getGeometryShaderInputType());
        EXPECT_EQ(effectBefore.getUniformInputs(), effectAfter->getUniformInputs());
        EXPECT_EQ(effectBefore.getAttributeInputs(), effectAfter->getAttributeInputs());

        checkSpirvShaders(*effectAfter);
    }

    TEST_F(AEffectResource, isEqualAfterSerializeAndDeserializeNoGeometryShaderOrSPIRV)
    {
        EffectResource effectBefore("asd", "def", "", {}, {}, uniformInputs, attributeInputs, {}, EFeatureLevel_Latest);
        std::unique_ptr<EffectResource> effectAfter(SerializeDeserialize(effectBefore, ""));
        ASSERT_TRUE(effectAfter);

        EXPECT_EQ(effectBefore.getHash(), effectAfter->getHash());
        EXPECT_STREQ(effectBefore.getVertexShader(), effectAfter->getVertexShader());
        EXPECT_STREQ(effectBefore.getFragmentShader(), effectAfter->getFragmentShader());
        EXPECT_STREQ(effectBefore.getGeometryShader(), effectAfter->getGeometryShader());
        EXPECT_EQ(effectBefore.getGeometryShaderInputType(), effectAfter->getGeometryShaderInputType());
        EXPECT_EQ(effectBefore.getUniformInputs(), effectAfter->getUniformInputs());
        EXPECT_EQ(effectBefore.getAttributeInputs(), effectAfter->getAttributeInputs());
    }

    TEST_F(AEffectResource, hasNameProvidedToSerializeAfterSerializeAndDeserialize)
    {
        EffectResource effectBefore("asd", "def", "xyz", {}, EDrawMode::Lines, uniformInputs, attributeInputs, "some name", EFeatureLevel_Latest);
        std::unique_ptr<EffectResource> effectAfter(SerializeDeserialize(effectBefore, "different name"));
        ASSERT_TRUE(effectAfter);

        EXPECT_EQ(std::string{"different name"}, effectAfter->getName());
    }
}
