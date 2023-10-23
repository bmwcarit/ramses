//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "RamsesObjectResolverMock.h"
#include "RamsesTestUtils.h"
#include "SerializationTestUtils.h"

#include "impl/logic/LogicEngineImpl.h"
#include "impl/logic/AppearanceBindingImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkImpl.h"
#include "internal/logic/RamsesHelper.h"
#include "internal/logic/flatbuffers/generated/AppearanceBindingGen.h"

#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/AppearanceBinding.h"

#include "ramses/client/Effect.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Appearance.h"

#include "ramses/client/ramses-utils.h"

#include <string_view>

namespace ramses::internal
{
    class AAppearanceBinding : public ALogicEngine
    {
    protected:
    };

    TEST_F(AAppearanceBinding, HasANameAfterCreation)
    {
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
        EXPECT_EQ("AppearanceBinding", appearanceBinding.getName());
    }

    TEST_F(AAppearanceBinding, HasNoOutputsAfterCreation)
    {
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
        EXPECT_EQ(nullptr, appearanceBinding.getOutputs());
    }

    TEST_F(AAppearanceBinding, ProducesNoErrorsDuringUpdate_IfNoRamsesAppearanceIsAssigned)
    {
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
        EXPECT_EQ(std::nullopt, appearanceBinding.impl().update());
    }

    // This fixture only contains serialization unit tests, for higher order tests see `AAppearanceBinding_WithRamses_AndFiles`
    class AAppearanceBinding_SerializationLifecycle : public AAppearanceBinding
    {
    protected:
        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils {m_flatBufferBuilder};
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;
        ErrorReporting m_errorReporting;
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap{ m_scene->impl() };
    };

    // More unit tests with inputs/outputs declared in LogicNode (base class) serialization tests
    TEST_F(AAppearanceBinding_SerializationLifecycle, RemembersBaseClassData)
    {
        // Serialize
        {
            AppearanceBindingImpl binding(m_scene->impl(), *m_appearance, "name", sceneObjectId_t{ 1u });
            binding.createRootProperties();
            (void)AppearanceBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedBinding.base());
        ASSERT_TRUE(serializedBinding.base()->base());
        ASSERT_TRUE(serializedBinding.base()->base()->name());
        EXPECT_EQ(serializedBinding.base()->base()->name()->string_view(), "name");
        EXPECT_EQ(serializedBinding.base()->base()->id(), 1u);

        ASSERT_TRUE(serializedBinding.base()->rootInput());
        EXPECT_EQ(serializedBinding.base()->rootInput()->rootType(), rlogic_serialization::EPropertyRootType::Struct);
        ASSERT_TRUE(serializedBinding.base()->rootInput()->children());
        EXPECT_EQ(serializedBinding.base()->rootInput()->children()->size(), 1u);

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesAppearanceInScene(::testing::Eq("name"), m_appearance->getSceneObjectId())).WillOnce(::testing::Return(m_appearance));
            std::unique_ptr<AppearanceBindingImpl> deserializedBinding = AppearanceBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(deserializedBinding->getName(), "name");
            EXPECT_EQ(deserializedBinding->getSceneObjectId().getValue(), 1u);
            EXPECT_FALSE(m_errorReporting.getError().has_value());
        }
    }

    TEST_F(AAppearanceBinding_SerializationLifecycle, RemembersRamsesAppearanceIdAndType_AndEffectHash)
    {
        // Serialize
        {
            AppearanceBindingImpl binding(m_scene->impl(), *m_appearance, "name", sceneObjectId_t{ 1u });
            binding.createRootProperties();
            (void)AppearanceBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedBinding.base()->boundRamsesObject());
        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectId(), m_appearance->getSceneObjectId().getValue());
        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectType(), static_cast<uint32_t>(ramses::ERamsesObjectType::Appearance));
        EXPECT_EQ(serializedBinding.parentEffectId()->resourceIdLow(), m_appearance->getEffect().getResourceId().lowPart);
        EXPECT_EQ(serializedBinding.parentEffectId()->resourceIdHigh(), m_appearance->getEffect().getResourceId().highPart);

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesAppearanceInScene(::testing::Eq("name"), m_appearance->getSceneObjectId())).WillOnce(::testing::Return(m_appearance));
            std::unique_ptr<AppearanceBindingImpl> deserializedBinding = AppearanceBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(&deserializedBinding->getRamsesAppearance(), m_appearance);
            EXPECT_FALSE(m_errorReporting.getError().has_value());

            // Check that input was deserialized too
            EXPECT_EQ(deserializedBinding->getInputs()->getChild("floatUniform")->getType(), EPropertyType::Float);
        }
    }

    TEST_F(AAppearanceBinding_SerializationLifecycle, ErrorWhenNoBindingBaseData)
    {
        {
            auto binding = rlogic_serialization::CreateAppearanceBinding(
                m_flatBufferBuilder,
                0 // no base binding info
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<AppearanceBindingImpl> deserialized = AppearanceBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of AppearanceBinding from serialized data: missing base class info!");
    }

    TEST_F(AAppearanceBinding_SerializationLifecycle, ErrorWhenNoBindingName)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    0, // no name!
                    1u)
            );
            auto binding = rlogic_serialization::CreateAppearanceBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<AppearanceBindingImpl> deserialized = AppearanceBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(this->m_errorReporting.getError().has_value());
        EXPECT_EQ("Fatal error during loading of AppearanceBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getError()->message);
    }

    TEST_F(AAppearanceBinding_SerializationLifecycle, ErrorWhenNoBindingId)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    0) // no id
            );
            auto binding = rlogic_serialization::CreateAppearanceBinding(m_flatBufferBuilder, base);
            m_flatBufferBuilder.Finish(binding);
        }

        const auto&                                  serialized   = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<AppearanceBindingImpl> deserialized = AppearanceBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(this->m_errorReporting.getError().has_value());
        EXPECT_EQ("Fatal error during loading of AppearanceBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getError()->message);
    }

    TEST_F(AAppearanceBinding_SerializationLifecycle, ErrorWhenNoRootInput)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0 // no root input
            );
            auto binding = rlogic_serialization::CreateAppearanceBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<AppearanceBindingImpl> deserialized = AppearanceBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of AppearanceBinding from serialized data: missing root input!");
    }

    TEST_F(AAppearanceBinding_SerializationLifecycle, ErrorWhenBoundAppearanceCannotBeResolved)
    {
        const ramses::sceneObjectId_t mockObjectId {12};
        {
            auto ramsesRef = rlogic_serialization::CreateRamsesReference(
                m_flatBufferBuilder,
                mockObjectId.getValue()
            );
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                ramsesRef,
                m_testUtils.serializeTestProperty("")
            );
            auto binding = rlogic_serialization::CreateAppearanceBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        EXPECT_CALL(m_resolverMock, findRamsesAppearanceInScene(::testing::Eq("name"), mockObjectId)).WillOnce(::testing::Return(nullptr));

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<AppearanceBindingImpl> deserialized = AppearanceBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
    }

    TEST_F(AAppearanceBinding_SerializationLifecycle, ErrorWhenRootInputHasErrors)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0,
                m_testUtils.serializeTestProperty("", rlogic_serialization::EPropertyRootType::Struct, false, true) // rootInput with errors
            );
            auto binding = rlogic_serialization::CreateAppearanceBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<AppearanceBindingImpl> deserialized = AppearanceBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_F(AAppearanceBinding_SerializationLifecycle, ReportsErrorWhenDeserializedWithDifferentAppearanceThenDuringSerialization)
    {
        // Use different shaders than the "trivial ones" below to force different appearance inputs
        std::string_view differentVertShader = R"(
            #version 100

            uniform highp float differentFloatUniform;
            attribute vec3 a_position;

            void main()
            {
                gl_Position = differentFloatUniform * vec4(a_position, 1.0);
            })";

        std::string_view fragShader = R"(
            #version 100

            void main(void)
            {
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            })";

        ramses::Appearance& differentAppearance = RamsesTestSetup::CreateTestAppearance(*m_scene, differentVertShader, fragShader);

        // Serialize
        {
            AppearanceBindingImpl binding(m_scene->impl(), *m_appearance, "name", sceneObjectId_t{ 1u });
            binding.createRootProperties();
            (void)AppearanceBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::AppearanceBinding>(m_flatBufferBuilder.GetBufferPointer());

        // Deserialize with different appearance -> expect errors
        {
            EXPECT_CALL(m_resolverMock, findRamsesAppearanceInScene(::testing::Eq("name"), m_appearance->getSceneObjectId())).WillOnce(::testing::Return(&differentAppearance));
            std::unique_ptr<AppearanceBindingImpl> deserializedBinding = AppearanceBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            EXPECT_FALSE(deserializedBinding);
            ASSERT_TRUE(m_errorReporting.getError().has_value());
            EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of AppearanceBinding from serialized data: effect signature doesn't match after loading!");
        }
    }

    class AAppearanceBinding_WithRamses : public AAppearanceBinding
    {
    protected:
        const std::string_view m_vertShader_simple = R"(
            #version 300 es

            uniform highp float floatUniform;

            void main()
            {
                gl_Position = floatUniform * vec4(1.0);
            })";

        const std::string_view m_vertShader_twoUniforms = R"(
            #version 300 es

            uniform highp float floatUniform1;
            uniform highp float floatUniform2;

            void main()
            {
                gl_Position = floatUniform1 *  floatUniform2 * vec4(1.0);
            })";

        const std::string_view m_vertShader_allTypes = R"(
            #version 300 es

            uniform highp float floatUniform;
            uniform bool        boolUniform;
            uniform highp int   intUniform;
            uniform highp ivec2 ivec2Uniform;
            uniform highp ivec3 ivec3Uniform;
            uniform highp ivec4 ivec4Uniform;
            uniform highp vec2  vec2Uniform;
            uniform highp vec3  vec3Uniform;
            uniform highp vec4  vec4Uniform;
            uniform bool        boolArray[2];
            uniform highp ivec2 ivec2Array[2];
            uniform highp vec2  vec2Array[2];
            uniform highp ivec3 ivec3Array[2];
            uniform highp vec3  vec3Array[2];
            uniform highp ivec4 ivec4Array[2];
            uniform highp vec4  vec4Array[2];
            uniform highp vec4  vec4Uniform_shouldHaveDefaultValue;

            void main()
            {
                gl_Position = floatUniform * vec4(1.0);
            })";

        const std::string_view m_fragShader_trivial = R"(
            #version 300 es

            out lowp vec4 color;
            void main(void)
            {
                color = vec4(1.0, 0.0, 0.0, 1.0);
            })";

        ramses::Effect& createTestEffect(std::string_view vertShader, std::string_view fragShader)
        {
            ramses::EffectDescription effectDesc;
            effectDesc.setUniformSemantic("u_DisplayBufferResolution", ramses::EEffectUniformSemantic::DisplayBufferResolution);
            effectDesc.setVertexShader(vertShader.data());
            effectDesc.setFragmentShader(fragShader.data());
            return *m_scene->createEffect(effectDesc);
        }

        ramses::Appearance& createTestAppearance(ramses::Effect& effect, std::string_view name = "test appearance")
        {
            return *m_scene->createAppearance(effect, name);
        }

        static float GetUniformValueFloat(ramses::Appearance& appearance, std::string_view uniformName)
        {
            const auto optUniform = appearance.getEffect().findUniformInput(uniformName);
            assert(optUniform.has_value());
            float value = 0.f;
            appearance.getInputValue(*optUniform, value);
            return value;
        }

        static void SetUniformValueFloat(ramses::Appearance& appearance, std::string_view uniformName, float value)
        {
            const auto optUniform = appearance.getEffect().findUniformInput(uniformName);
            assert(optUniform.has_value());
            appearance.setInputValue(*optUniform, value);
        }
    };

    TEST_F(AAppearanceBinding_WithRamses, ReturnsReferenceToRamsesAppearance)
    {
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
        EXPECT_EQ(m_appearance, &appearanceBinding.getRamsesAppearance());
    }

    TEST_F(AAppearanceBinding_WithRamses, HasInputsAfterCreation)
    {
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");

        auto inputs = appearanceBinding.getInputs();

        ASSERT_EQ(1u, inputs->getChildCount());
        auto floatUniform = inputs->getChild(0);
        ASSERT_NE(nullptr, floatUniform);
        EXPECT_EQ("floatUniform", floatUniform->getName());
        EXPECT_EQ(EPropertyType::Float, floatUniform->getType());
    }

    TEST_F(AAppearanceBinding_WithRamses, GivesInputs_BindingInputSemantics)
    {
        auto&      appearanceBinding = *m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
        auto       inputs            = appearanceBinding.getInputs();
        const auto inputCount        = inputs->getChildCount();
        for (size_t i = 0; i < inputCount; ++i)
        {
            EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild(i)->impl().getPropertySemantics());
        }
    }

    TEST_F(AAppearanceBinding_WithRamses, CreatesOnlyInputsForSupportedUniformTypes)
    {

        const std::string_view fragShader_ManyUniformTypes = R"(
            #version 300 es

            // This is the same uniform like in the vertex shader - that's intended!
            uniform highp float floatUniform;
            // Other types, mixed up on purpose with some types which are not supported yet
            uniform highp vec2 u_vec2f;
            uniform highp sampler2D u_tex2d;
            //uniform highp samplerCube cubeTex;    // Not supported
            uniform highp vec4 u_vec4f;
            uniform highp sampler3D u_tex3d;        // Not supported
            uniform lowp int u_int;
            uniform highp samplerCube u_texCube;    // Not supported
            uniform mediump mat2 u_mat2;            // Not supported
            uniform mediump mat3 u_mat3;            // Not supported
            uniform mediump mat4 u_mat4;            // Not supported
            uniform mediump vec2 u_DisplayBufferResolution; // explicitly prohibited to set by ramses
            uniform highp ivec2 u_vec2i;
            // Arrays
            uniform mediump vec2 u_vec2Array[2];
            uniform mediump ivec2 u_ivec2Array[2];

            out lowp vec4 color;
            void main(void)
            {
                color = vec4(floatUniform, 0.0, 0.0, 1.0);
                color.xy += u_vec2f;
                color += texture(u_tex2d, u_vec2f);
                color += texture(u_tex3d, vec3(u_vec2f, 1.0));
                color += texture(u_texCube, vec3(u_vec2f, 1.0));
                color.xy += vec2(float(u_vec2i.x), float(u_vec2i.y));
            })";

        ramses::Appearance& appearance = createTestAppearance(createTestEffect(m_vertShader_simple, fragShader_ManyUniformTypes));
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(appearance, "AppearanceBinding");

        const auto inputs = appearanceBinding.getInputs();
        ASSERT_EQ(7u, inputs->getChildCount());
        EXPECT_EQ("floatUniform", inputs->getChild(0)->getName());
        EXPECT_EQ(EPropertyType::Float, inputs->getChild(0)->getType());
        EXPECT_EQ("u_vec2f", inputs->getChild(1)->getName());
        EXPECT_EQ(EPropertyType::Vec2f, inputs->getChild(1)->getType());
        EXPECT_EQ("u_vec4f", inputs->getChild(2)->getName());
        EXPECT_EQ(EPropertyType::Vec4f, inputs->getChild(2)->getType());
        EXPECT_EQ("u_int", inputs->getChild(3)->getName());
        EXPECT_EQ(EPropertyType::Int32, inputs->getChild(3)->getType());
        EXPECT_EQ("u_vec2i", inputs->getChild(4)->getName());
        EXPECT_EQ(EPropertyType::Vec2i, inputs->getChild(4)->getType());

        // Arrays, also check their children
        Property* vec2fArray = inputs->getChild(5);
        EXPECT_EQ("u_vec2Array", vec2fArray->getName());
        EXPECT_EQ(EPropertyType::Array, vec2fArray->getType());
        EXPECT_EQ(2u, vec2fArray->getChildCount());
        for (size_t i = 0; i < 2; ++i)
        {
            EXPECT_EQ("", vec2fArray->getChild(i)->getName());
            EXPECT_EQ(EPropertyType::Vec2f, vec2fArray->getChild(i)->getType());
        }

        Property* vec2iArray = inputs->getChild(6);
        EXPECT_EQ("u_ivec2Array", vec2iArray->getName());
        EXPECT_EQ(EPropertyType::Array, vec2iArray->getType());
        EXPECT_EQ(2u, vec2iArray->getChildCount());
        for (size_t i = 0; i < 2; ++i)
        {
            EXPECT_EQ("", vec2iArray->getChild(i)->getName());
            EXPECT_EQ(EPropertyType::Vec2i, vec2iArray->getChild(i)->getType());
        }
    }

    TEST_F(AAppearanceBinding_WithRamses, UpdatesAppearanceIfInputValuesWereSet)
    {
        ramses::Appearance& appearance = createTestAppearance(createTestEffect(m_vertShader_allTypes, m_fragShader_trivial));
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(appearance, "AppearanceBinding");
        auto inputs = appearanceBinding.getInputs();
        ASSERT_EQ(17u, inputs->getChildCount());
        EXPECT_TRUE(inputs->getChild("floatUniform")->set(42.42f));
        EXPECT_TRUE(inputs->getChild("boolUniform")->set(true));
        EXPECT_TRUE(inputs->getChild("intUniform")->set(42));
        EXPECT_TRUE(inputs->getChild("vec2Uniform")->set<vec2f>({ 0.1f, 0.2f }));
        EXPECT_TRUE(inputs->getChild("vec3Uniform")->set<vec3f>({ 1.1f, 1.2f, 1.3f }));
        EXPECT_TRUE(inputs->getChild("vec4Uniform")->set<vec4f>({ 2.1f, 2.2f, 2.3f, 2.4f }));
        EXPECT_TRUE(inputs->getChild("ivec2Uniform")->set<vec2i>({ 1, 2 }));
        EXPECT_TRUE(inputs->getChild("ivec3Uniform")->set<vec3i>({ 3, 4, 5 }));
        EXPECT_TRUE(inputs->getChild("ivec4Uniform")->set<vec4i>({6, 7, 8, 9}));
        EXPECT_TRUE(inputs->getChild("boolArray")->getChild(0)->set<bool>(false));
        EXPECT_TRUE(inputs->getChild("boolArray")->getChild(1)->set<bool>(true));
        EXPECT_TRUE(inputs->getChild("ivec2Array")->getChild(0)->set<vec2i>({ 11, 12 }));
        EXPECT_TRUE(inputs->getChild("ivec2Array")->getChild(1)->set<vec2i>({ 13, 14 }));
        EXPECT_TRUE(inputs->getChild("vec2Array")->getChild(0)->set<vec2f>({ .11f, .12f }));
        EXPECT_TRUE(inputs->getChild("vec2Array")->getChild(1)->set<vec2f>({ .13f, .14f }));
        EXPECT_TRUE(inputs->getChild("ivec3Array")->getChild(0)->set<vec3i>({ 31, 32, 33 }));
        EXPECT_TRUE(inputs->getChild("ivec3Array")->getChild(1)->set<vec3i>({ 34, 35, 36 }));
        EXPECT_TRUE(inputs->getChild("vec3Array")->getChild(0)->set<vec3f>({ .31f, .32f, .33f }));
        EXPECT_TRUE(inputs->getChild("vec3Array")->getChild(1)->set<vec3f>({ .34f, .35f, .36f }));
        EXPECT_TRUE(inputs->getChild("ivec4Array")->getChild(0)->set<vec4i>({ 41, 42, 43, 44 }));
        EXPECT_TRUE(inputs->getChild("ivec4Array")->getChild(1)->set<vec4i>({ 45, 46, 47, 48 }));
        EXPECT_TRUE(inputs->getChild("vec4Array")->getChild(0)->set<vec4f>({ .41f, .42f, .43f, .44f }));
        EXPECT_TRUE(inputs->getChild("vec4Array")->getChild(1)->set<vec4f>({ .45f, .46f, .47f, .48f }));

        EXPECT_EQ(std::nullopt, appearanceBinding.impl().update());

        auto&                               effect = appearance.getEffect();
        std::optional<ramses::UniformInput> optUniform;
        {
            float resultFloat = 0.f;
            optUniform        = effect.findUniformInput("floatUniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, resultFloat);
            EXPECT_FLOAT_EQ(42.42f, resultFloat);
        }
        {
            bool resultBool = true;
            optUniform      = effect.findUniformInput("boolUniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, resultBool);
            EXPECT_EQ(true, resultBool);
        }
        {
            int32_t resultInt32 = 0;
            optUniform          = effect.findUniformInput("intUniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, resultInt32);
            EXPECT_EQ(42, resultInt32);
        }
        {
            ramses::vec2f result{ 0.0f, 0.0f };
            optUniform = effect.findUniformInput("vec2Uniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, result);
            EXPECT_EQ(result, vec2f(0.1f, 0.2f));
        }
        {
            ramses::vec3f result{ 0.0f, 0.0f, 0.0f };
            optUniform = effect.findUniformInput("vec3Uniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, result);
            EXPECT_EQ(result, vec3f(1.1f, 1.2f, 1.3f));
        }
        {
            ramses::vec4f result{ 0.0f, 0.0f, 0.0f, 0.0f };
            optUniform = effect.findUniformInput("vec4Uniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, result);
            EXPECT_EQ(result, vec4f(2.1f, 2.2f, 2.3f, 2.4f));

            optUniform = effect.findUniformInput("vec4Uniform_shouldHaveDefaultValue");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, result);
            EXPECT_EQ(result, vec4f(0.0f, 0.0f, 0.0f, 0.0f));
        }
        {
            ramses::vec2i result{ 0, 0 };
            optUniform = effect.findUniformInput("ivec2Uniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, result);
            EXPECT_EQ(result, vec2i(1, 2));
        }
        {
            ramses::vec3i result{ 0, 0, 0 };
            optUniform = effect.findUniformInput("ivec3Uniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, result);
            EXPECT_EQ(result, vec3i(3, 4, 5));
        }
        {
            ramses::vec4i result{ 0, 0, 0, 0 };
            optUniform = effect.findUniformInput("ivec4Uniform");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, result);
            EXPECT_EQ(result, vec4i(6, 7, 8, 9));
        }
        // Arrays
        {
            std::array<bool, 2> result{false};
            optUniform = effect.findUniformInput("boolArray");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, 2, result.data());
            EXPECT_THAT(result, ::testing::ElementsAre(false, true));
        }
        {
            std::array<ramses::vec2i, 2> result{ ramses::vec2i{0, 0}, ramses::vec2i{0, 0} };
            optUniform = effect.findUniformInput("ivec2Array");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, 2, result.data());
            EXPECT_THAT(result, ::testing::ElementsAre(ramses::vec2i{ 11, 12 }, ramses::vec2i{ 13, 14 }));
        }
        {
            std::array<ramses::vec2f, 2> result{ ramses::vec2f{0.0f, 0.0f}, ramses::vec2f{0.0f, 0.0f} };
            optUniform = effect.findUniformInput("vec2Array");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, 2, result.data());
            EXPECT_THAT(result, ::testing::ElementsAre(ramses::vec2f{ .11f, .12f }, ramses::vec2f{ .13f, .14f }));
        }
        {
            std::array<ramses::vec3i, 2> result{ ramses::vec3i{0, 0, 0}, ramses::vec3i{0, 0, 0} };
            optUniform = effect.findUniformInput("ivec3Array");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, 2, result.data());
            EXPECT_THAT(result, ::testing::ElementsAre(ramses::vec3i{ 31, 32, 33 }, ramses::vec3i{ 34, 35, 36 }));
        }
        {
            std::array<ramses::vec3f, 2> result = { ramses::vec3f{0.0f, 0.0f, 0.0f}, ramses::vec3f{0.0f, 0.0f, 0.0f} };
            optUniform = effect.findUniformInput("vec3Array");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, 2, result.data());
            EXPECT_THAT(result, ::testing::ElementsAre(ramses::vec3f{ .31f, .32f, .33f }, ramses::vec3f{ .34f, .35f, .36f }));
        }
        {
            std::array<ramses::vec4i, 2> result = { ramses::vec4i{0, 0, 0, 0}, ramses::vec4i{0, 0, 0, 0} };
            optUniform = effect.findUniformInput("ivec4Array");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, 2, result.data());
            EXPECT_THAT(result, ::testing::ElementsAre(ramses::vec4i{ 41, 42, 43, 44 }, ramses::vec4i{ 45, 46, 47, 48 }));
        }
        {
            std::array<ramses::vec4f, 2> result = { ramses::vec4f{0.0f, 0.0f, 0.0f, 0.0f}, ramses::vec4f{0.0f, 0.0f, 0.0f, 0.0f} };
            optUniform = effect.findUniformInput("vec4Array");
            ASSERT_TRUE(optUniform.has_value());
            appearance.getInputValue(*optUniform, 2, result.data());
            EXPECT_THAT(result, ::testing::ElementsAre(ramses::vec4f{ .41f, .42f, .43f, .44f }, ramses::vec4f{ .45f, .46f, .47f, .48f }));
        }
    }

    TEST_F(AAppearanceBinding_WithRamses, PropagateItsInputsToRamsesAppearanceOnUpdate_OnlyWhenExplicitlySet)
    {
        ramses::Appearance& appearance = createTestAppearance(createTestEffect(m_vertShader_twoUniforms, m_fragShader_trivial));
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(appearance, "AppearanceBinding");

        // Set values directly to ramses appearance
        SetUniformValueFloat(appearance, "floatUniform1", 11.f);
        SetUniformValueFloat(appearance, "floatUniform2", 22.f);

        // Set only one of the inputs to the binding object, the other one (floatUniform2) not
        EXPECT_TRUE(appearanceBinding.getInputs()->getChild("floatUniform1")->set(100.f));

        EXPECT_TRUE(m_logicEngine->update());

        // Only propagate the value which was also set in the binding object
        EXPECT_FLOAT_EQ(100.f, GetUniformValueFloat(appearance, "floatUniform1"));
        EXPECT_FLOAT_EQ(22.f, GetUniformValueFloat(appearance, "floatUniform2"));
    }

    TEST_F(AAppearanceBinding_WithRamses, PropagatesItsInputsToRamsesAppearanceOnUpdate_WithLinksInsteadOfSetCall)
    {
        ramses::Appearance& appearance = createTestAppearance(createTestEffect(m_vertShader_twoUniforms, m_fragShader_trivial));
        auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(appearance, "AppearanceBinding");

        // Set values directly to ramses appearance
        SetUniformValueFloat(appearance, "floatUniform1", 11.f);
        SetUniformValueFloat(appearance, "floatUniform2", 22.f);

        // Link binding input to a script (binding is not set directly, but is linked)
        const std::string_view scriptSrc = R"(
                function interface(IN,OUT)
                    OUT.float = Type:Float()
                end
                function run(IN,OUT)
                    OUT.float = 42.42
                end
            )";
        LuaScript* script = m_logicEngine->createLuaScript(scriptSrc);
        ASSERT_TRUE(m_logicEngine->link(*script->getOutputs()->getChild("float"), *appearanceBinding.getInputs()->getChild("floatUniform1")));

        EXPECT_TRUE(m_logicEngine->update());

        // Only propagate the value which was also linked over the binding object's input to a script
        EXPECT_FLOAT_EQ(42.42f, GetUniformValueFloat(appearance, "floatUniform1"));
        EXPECT_FLOAT_EQ(22.f, GetUniformValueFloat(appearance, "floatUniform2"));
    }

    class AAppearanceBinding_WithRamses_AndFiles : public AAppearanceBinding_WithRamses
    {
    public:
        AAppearanceBinding_WithRamses_AndFiles()
        {
            withTempDirectory();
        }
    };

    TEST_F(AAppearanceBinding_WithRamses_AndFiles, KeepsItsPropertiesAfterDeserialization_WhenNoRamsesLinksAndSceneProvided)
    {
        {
            m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
            ASSERT_TRUE(saveToFileWithoutValidation("appearancebinding.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("appearancebinding.bin"));
            ASSERT_TRUE(m_logicEngine != nullptr);
            auto loadedAppearanceBinding = m_logicEngine->findObject<AppearanceBinding>("AppearanceBinding");
            EXPECT_EQ(&loadedAppearanceBinding->getRamsesAppearance(), m_appearance);
            EXPECT_EQ(loadedAppearanceBinding->getInputs()->getChildCount(), 1u);
            EXPECT_EQ(loadedAppearanceBinding->getOutputs(), nullptr);
            EXPECT_EQ(loadedAppearanceBinding->getName(), "AppearanceBinding");
        }
    }

    TEST_F(AAppearanceBinding_WithRamses_AndFiles, ContainsItsInputsAfterDeserialization_WithoutReorderingThem)
    {
        std::vector<std::string> inputOrderBeforeSaving;
        sceneObjectId_t appearanceId;
        {
            ramses::Appearance& appearance = createTestAppearance(createTestEffect(m_vertShader_allTypes, m_fragShader_trivial));
            appearanceId = appearance.getSceneObjectId();
            auto& rAppearanceBinding = *m_logicEngine->createAppearanceBinding(appearance, "AppearanceBinding");
            auto inputs = rAppearanceBinding.getInputs();
            inputOrderBeforeSaving.reserve(inputs->getChildCount());

            for (size_t i = 0; i < inputs->getChildCount(); ++i)
            {
                inputOrderBeforeSaving.emplace_back(std::string(inputs->getChild(i)->getName()));
            }

            inputs->getChild("floatUniform")->set(42.42f);
            inputs->getChild("boolUniform")->set(true);
            inputs->getChild("intUniform")->set(42);
            inputs->getChild("vec2Uniform")->set<vec2f>({ 0.1f, 0.2f });
            inputs->getChild("vec3Uniform")->set<vec3f>({ 1.1f, 1.2f, 1.3f });
            inputs->getChild("vec4Uniform")->set<vec4f>({ 2.1f, 2.2f, 2.3f, 2.4f });
            inputs->getChild("ivec2Uniform")->set<vec2i>({ 1, 2 });
            inputs->getChild("ivec3Uniform")->set<vec3i>({ 3, 4, 5 });
            inputs->getChild("ivec4Uniform")->set<vec4i>({6, 7, 8, 9});
            inputs->getChild("boolArray")->getChild(1)->set<bool>(true);
            inputs->getChild("boolArray")->getChild(0)->set<bool>(false);
            inputs->getChild("ivec2Array")->getChild(0)->set<vec2i>({ 11, 12 });
            inputs->getChild("ivec2Array")->getChild(1)->set<vec2i>({ 13, 14 });
            inputs->getChild("vec2Array")->getChild(0)->set<vec2f>({ .11f, .12f });
            inputs->getChild("vec2Array")->getChild(1)->set<vec2f>({ .13f, .14f });
            m_logicEngine->update();
            ASSERT_TRUE(saveToFileWithoutValidation("logic.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("logic.bin"));
            ASSERT_TRUE(m_logicEngine != nullptr);
            auto loadedAppearanceBinding = m_logicEngine->findObject<AppearanceBinding>("AppearanceBinding");
            EXPECT_EQ(loadedAppearanceBinding->getRamsesAppearance().getSceneObjectId(), appearanceId);

            const auto& inputs = loadedAppearanceBinding->getInputs();
            ASSERT_EQ(17u, inputs->getChildCount());

            // check order after deserialization
            for (size_t i = 0; i < inputOrderBeforeSaving.size(); ++i)
            {
                EXPECT_EQ(inputOrderBeforeSaving[i], inputs->getChild(i)->getName());
            }

            auto expectValues = [&inputs](){
                EXPECT_FLOAT_EQ(42.42f, *inputs->getChild("floatUniform")->get<float>());
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("floatUniform")->impl().getPropertySemantics());
                EXPECT_EQ(true, *inputs->getChild("boolUniform")->get<bool>());
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("boolUniform")->impl().getPropertySemantics());
                EXPECT_EQ(42, *inputs->getChild("intUniform")->get<int32_t>());
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("intUniform")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("vec2Uniform")->get<vec2f>(), vec2f(0.1f, 0.2f));
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("vec2Uniform")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("vec3Uniform")->get<vec3f>(), vec3f(1.1f, 1.2f, 1.3f));
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("vec3Uniform")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("vec4Uniform")->get<vec4f>(), vec4f(2.1f, 2.2f, 2.3f, 2.4f));
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("vec4Uniform")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("vec4Uniform_shouldHaveDefaultValue")->get<vec4f>(), vec4f(.0f, .0f, .0f, .0f));
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("vec4Uniform_shouldHaveDefaultValue")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("ivec2Uniform")->get<vec2i>(), vec2i(1, 2));
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("ivec2Uniform")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("ivec3Uniform")->get<vec3i>(), vec3i(3, 4, 5));
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("ivec3Uniform")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("ivec4Uniform")->get<vec4i>(), vec4i(6, 7, 8, 9));
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("ivec4Uniform")->impl().getPropertySemantics());

                // Arrays
                EXPECT_EQ(EPropertyType::Array, inputs->getChild("boolArray")->getType());
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("boolArray")->impl().getPropertySemantics());
                EXPECT_EQ(inputs->getChild("boolArray")->getChild(0)->get<bool>(), false);
                EXPECT_EQ(inputs->getChild("boolArray")->getChild(1)->get<bool>(), true);
                EXPECT_EQ(EPropertyType::Array, inputs->getChild("ivec2Array")->getType());
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("ivec2Array")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("ivec2Array")->getChild(0)->get<vec2i>(), vec2i(11, 12));
                EXPECT_EQ(*inputs->getChild("ivec2Array")->getChild(1)->get<vec2i>(), vec2i(13, 14));
                EXPECT_EQ(EPropertyType::Array, inputs->getChild("vec2Array")->getType());
                EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild("vec2Array")->impl().getPropertySemantics());
                EXPECT_EQ(*inputs->getChild("vec2Array")->getChild(0)->get<vec2f>(), vec2f(.11f, .12f));
                EXPECT_EQ(*inputs->getChild("vec2Array")->getChild(1)->get<vec2f>(), vec2f(.13f, .14f));
            };

            expectValues();

            EXPECT_TRUE(m_logicEngine->update());

            expectValues();
        }
    }

    TEST_F(AAppearanceBinding_WithRamses_AndFiles, ContainsItsInputsAfterDeserialization_WhenRamsesSceneIsRecreatedBetweenSaveAndLoad)
    {
        const ramses::sceneObjectId_t appearanceIdBeforeReload = m_appearance->getSceneObjectId();
        {
            auto& rAppearanceBinding = *m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
            rAppearanceBinding.getInputs()->getChild("floatUniform")->set(42.42f);
            m_logicEngine->update();
            ASSERT_TRUE(saveToFileWithoutValidation("logic.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("logic.bin"));
            ASSERT_TRUE(m_logicEngine != nullptr);
            auto loadedAppearanceBinding = m_logicEngine->findObject<AppearanceBinding>("AppearanceBinding");
            EXPECT_EQ(loadedAppearanceBinding->getRamsesAppearance().getSceneObjectId(), appearanceIdBeforeReload);

            const auto& inputs = loadedAppearanceBinding->getInputs();
            ASSERT_EQ(1u, inputs->getChildCount());
            EXPECT_FLOAT_EQ(42.42f, *inputs->getChild("floatUniform")->get<float>());
        }
    }

    TEST_F(AAppearanceBinding_WithRamses_AndFiles, DoesNotReapplyAppearanceUniformValuesToRamses_WhenLoadingFromFileAndCallingUpdate_UntilSetToANewValue)
    {
        {
            ramses::Appearance& appearance = createTestAppearance(createTestEffect(m_vertShader_simple, m_fragShader_trivial), "app01");
            auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(appearance, "AppearanceBinding");
            auto inputs = appearanceBinding.getInputs();

            // Set a different input over the binding object
            inputs->getChild("floatUniform")->set<float>(42.42f);
            m_logicEngine->update();

            // Set uniform to a different value than the one set on the ramses binding
            SetUniformValueFloat(appearance, "floatUniform", 100.0f);

            ASSERT_TRUE(saveToFileWithoutValidation("SomeValuesSet.bin"));
        }

        {
            EXPECT_TRUE(recreateFromFile("SomeValuesSet.bin"));
            ASSERT_TRUE(m_logicEngine != nullptr);
            auto appearance = m_scene->findObject<Appearance>("app01");
            ASSERT_TRUE(appearance != nullptr);
            EXPECT_TRUE(m_logicEngine->update());

            // loadFromFile and update should not set any values whatsoever ...
            EXPECT_FLOAT_EQ(100.f, GetUniformValueFloat(*appearance, "floatUniform"));

            // ... unless explicitly set again on the binding object + update() called
            m_logicEngine->findObject<AppearanceBinding>("AppearanceBinding")->getInputs()->getChild("floatUniform")->set<float>(42.42f);
            EXPECT_TRUE(m_logicEngine->update());
            EXPECT_FLOAT_EQ(42.42f, GetUniformValueFloat(*appearance, "floatUniform"));
        }
    }

    // This is sort of a confidence test, testing a combination of:
    // - bindings only propagating their values to ramses appearance if the value was set by an incoming link
    // - saving and loading files
    // - value only re-applied to ramses if changed. Otherwise not.
    // The general expectation is that after loading + update(), the logic scene would overwrite ramses
    // properties wrapped by a LogicBinding if they are linked to a script
    TEST_F(AAppearanceBinding_WithRamses_AndFiles, SetsOnlyAppearanceUniformsForWhichTheBindingInputIsLinked_AfterLoadingFromFile_AndCallingUpdate)
    {
        {
            ramses::Appearance& appearance = createTestAppearance(createTestEffect(m_vertShader_twoUniforms, m_fragShader_trivial), "app01");
            const std::string_view scriptSrc = R"(
                function interface(IN,OUT)
                    IN.float = Type:Float()
                    OUT.float = Type:Float()
                end
                function run(IN,OUT)
                    OUT.float = IN.float
                end
            )";

            LuaScript* script = m_logicEngine->createLuaScript(scriptSrc);
            auto& appearanceBinding = *m_logicEngine->createAppearanceBinding(appearance, "AppearanceBinding");

            script->getInputs()->getChild("float")->set<float>(42.42f);
            ASSERT_TRUE(m_logicEngine->link(*script->getOutputs()->getChild("float"), *appearanceBinding.getInputs()->getChild("floatUniform1")));
            ASSERT_TRUE(m_logicEngine->update());

            // Set uniform1 to a different value than the one set by the link
            SetUniformValueFloat(appearance, "floatUniform1", 100.0f);
            // Set uniform2 to custom value - it should not be overwritten by logic at all, because there is no link
            // or any set() calls to the corresponding AppearanceBinding input
            SetUniformValueFloat(appearance, "floatUniform2", 200.0f);

            ASSERT_TRUE(saveToFileWithoutValidation("SomeValuesLinked.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("SomeValuesLinked.bin"));
            ASSERT_TRUE(m_logicEngine != nullptr);

            auto appearance = m_scene->findObject<Appearance>("app01");
            ASSERT_TRUE(appearance != nullptr);
            // nothing happens before update()
            EXPECT_FLOAT_EQ(100.0f, GetUniformValueFloat(*appearance, "floatUniform1"));
            EXPECT_FLOAT_EQ(200.0f, GetUniformValueFloat(*appearance, "floatUniform2"));

            EXPECT_TRUE(m_logicEngine->update());

            // Script is executed -> link is activated -> binding is updated, only for the linked uniform
            EXPECT_FLOAT_EQ(42.42f, GetUniformValueFloat(*appearance, "floatUniform1"));
            EXPECT_FLOAT_EQ(200.0f, GetUniformValueFloat(*appearance, "floatUniform2"));

            // Reset uniform manually and call update does nothing (must set binding input explicitly to cause overwrite in ramses)
            SetUniformValueFloat(*appearance, "floatUniform1", 100.0f);
            EXPECT_TRUE(m_logicEngine->update());
            EXPECT_FLOAT_EQ(100.0f, GetUniformValueFloat(*appearance, "floatUniform1"));
            EXPECT_FLOAT_EQ(200.0f, GetUniformValueFloat(*appearance, "floatUniform2"));
        }
    }
}
