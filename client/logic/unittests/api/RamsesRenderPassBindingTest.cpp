//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "RamsesObjectResolverMock.h"
#include "RamsesTestUtils.h"
#include "SerializationTestUtils.h"
#include "WithTempDirectory.h"

#include "impl/LogicEngineImpl.h"
#include "impl/RamsesRenderPassBindingImpl.h"
#include "impl/PropertyImpl.h"
#include "internals/RamsesHelper.h"
#include "generated/RamsesRenderPassBindingGen.h"

#include "ramses-logic/RamsesRenderPassBinding.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LogicEngine.h"

#include "ramses-client-api/RenderPass.h"

#include "ramses-utils.h"

namespace ramses::internal
{
    class ARamsesRenderPassBinding : public ALogicEngine
    {
    };

    TEST_F(ARamsesRenderPassBinding, HasANameAfterCreation)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
        EXPECT_EQ("renderPass", renderPassBinding.getName());
    }

    TEST_F(ARamsesRenderPassBinding, RefersToGivenRenderPass)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
        EXPECT_EQ(m_renderPass, &renderPassBinding.getRamsesRenderPass());
        const auto& rpConst = renderPassBinding;
        EXPECT_EQ(m_renderPass, &rpConst.getRamsesRenderPass());
        const auto& rpImplConst = renderPassBinding.m_renderPassBinding;
        EXPECT_EQ(m_renderPass, &rpImplConst.getRamsesRenderPass());
    }

    TEST_F(ARamsesRenderPassBinding, HasInputsAfterCreation)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
        ASSERT_NE(nullptr, renderPassBinding.getInputs());
        ASSERT_EQ(std::size_t(RamsesRenderPassBindingImpl::EPropertyIndex_COUNT), renderPassBinding.getInputs()->getChildCount());
        EXPECT_EQ(renderPassBinding.getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_Enabled), renderPassBinding.getInputs()->getChild("enabled"));
        EXPECT_EQ(EPropertyType::Bool, renderPassBinding.getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_Enabled)->getType());
        EXPECT_EQ(renderPassBinding.getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_RenderOrder), renderPassBinding.getInputs()->getChild("renderOrder"));
        EXPECT_EQ(EPropertyType::Int32, renderPassBinding.getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_RenderOrder)->getType());
        EXPECT_EQ(renderPassBinding.getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_ClearColor), renderPassBinding.getInputs()->getChild("clearColor"));
        EXPECT_EQ(EPropertyType::Vec4f, renderPassBinding.getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_ClearColor)->getType());
        EXPECT_EQ(renderPassBinding.getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_RenderOnce), renderPassBinding.getInputs()->getChild("renderOnce"));
        EXPECT_EQ(EPropertyType::Bool, renderPassBinding.getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_RenderOnce)->getType());
    }

    TEST_F(ARamsesRenderPassBinding, HasNoOutputsAfterCreation)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
        EXPECT_EQ(nullptr, renderPassBinding.getOutputs());
    }

    // This fixture only contains serialization unit tests, for higher order tests see `ARamsesRenderPassBinding_WithRamses_AndFiles`
    class ARamsesRenderPassBinding_SerializationLifecycle : public ARamsesRenderPassBinding
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            RootNotStruct,
            BoundObjectReferenceMissing,
            BoundObjectTypeMismatch
        };

        std::unique_ptr<RamsesRenderPassBindingImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            {
                auto inputsType = MakeStruct("", {
                    TypeData{"enabled", EPropertyType::Bool},
                    TypeData{"renderOrder", EPropertyType::Int32},
                    TypeData{"clearColor", EPropertyType::Vec4f},
                    TypeData{"renderOnce", EPropertyType::Bool}
                });

                HierarchicalTypeData inputs = (issue == ESerializationIssue::RootNotStruct ? MakeType("", EPropertyType::Bool) : inputsType);
                auto inputsImpl = std::make_unique<PropertyImpl>(std::move(inputs), EPropertySemantics::BindingInput);

                auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(m_flatBufferBuilder,
                    rlogic_serialization::CreateLogicObject(m_flatBufferBuilder, m_flatBufferBuilder.CreateString("name"), 1u, 0u, 0u),
                    (issue == ESerializationIssue::BoundObjectReferenceMissing ? 0 : rlogic_serialization::CreateRamsesReference(m_flatBufferBuilder,
                        1u, (issue == ESerializationIssue::BoundObjectTypeMismatch ? 0 : static_cast<uint32_t>(ramses::ERamsesObjectType_RenderPass)))),
                    PropertyImpl::Serialize(*inputsImpl, m_flatBufferBuilder, m_serializationMap));

                auto fbRenderPassBinding = rlogic_serialization::CreateRamsesRenderPassBinding(m_flatBufferBuilder, fbRamsesBinding);
                m_flatBufferBuilder.Finish(fbRenderPassBinding);
            }

            switch (issue)
            {
            case ESerializationIssue::AllValid:
            case ESerializationIssue::BoundObjectTypeMismatch:
                EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 1u })).WillOnce(::testing::Return(m_renderPass));
                break;
            case ESerializationIssue::RootNotStruct:
            case ESerializationIssue::BoundObjectReferenceMissing:
                break;
            }

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
            return RamsesRenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils {m_flatBufferBuilder};
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;
        ErrorReporting m_errorReporting;
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap;
    };

    // More unit tests with inputs/outputs declared in LogicNode (base class) serialization tests
    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, StoresBaseClassData)
    {
        // Serialize
        {
            RamsesRenderPassBindingImpl binding(*m_renderPass, "name", 1u);
            binding.createRootProperties();
            (void)RamsesRenderPassBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedBinding.base());
        ASSERT_TRUE(serializedBinding.base()->base());
        ASSERT_TRUE(serializedBinding.base()->base()->name());
        EXPECT_EQ(serializedBinding.base()->base()->name()->string_view(), "name");
        EXPECT_EQ(serializedBinding.base()->base()->id(), 1u);

        ASSERT_TRUE(serializedBinding.base()->rootInput());
        EXPECT_EQ(serializedBinding.base()->rootInput()->rootType(), rlogic_serialization::EPropertyRootType::Struct);
        ASSERT_TRUE(serializedBinding.base()->rootInput()->children());
        EXPECT_EQ(serializedBinding.base()->rootInput()->children()->size(), size_t(RamsesRenderPassBindingImpl::EPropertyIndex_COUNT));

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("name"), m_renderPass->getSceneObjectId())).WillOnce(::testing::Return(m_renderPass));
            std::unique_ptr<RamsesRenderPassBindingImpl> deserializedBinding = RamsesRenderPassBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(deserializedBinding->getName(), "name");
            EXPECT_EQ(deserializedBinding->getId(), 1u);
            EXPECT_TRUE(m_errorReporting.getErrors().empty());
        }
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, StoresIdAndTypeAndRenderPassRef)
    {
        // Serialize
        {
            RamsesRenderPassBindingImpl binding(*m_renderPass, "name", 1u);
            binding.createRootProperties();
            (void)RamsesRenderPassBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedBinding.base()->boundRamsesObject());
        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectId(), m_renderPass->getSceneObjectId().getValue());
        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectType(), static_cast<uint32_t>(ramses::ERamsesObjectType_RenderPass));

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("name"), m_renderPass->getSceneObjectId())).WillOnce(::testing::Return(m_renderPass));
            std::unique_ptr<RamsesRenderPassBindingImpl> deserializedBinding = RamsesRenderPassBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(&deserializedBinding->getRamsesRenderPass(), m_renderPass);
            EXPECT_TRUE(m_errorReporting.getErrors().empty());

            // Check that input was deserialized too
            EXPECT_EQ(deserializedBinding->getInputs()->getChild("enabled")->getType(), EPropertyType::Bool);
        }
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ErrorWhenNoBindingBaseData)
    {
        {
            auto binding = rlogic_serialization::CreateRamsesRenderPassBinding(
                m_flatBufferBuilder,
                0 // no base binding info
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesRenderPassBindingImpl> deserialized = RamsesRenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderPassBinding from serialized data: missing base class info!");
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ErrorWhenNoBindingName)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    0, // no name!
                    1u)
            );
            auto binding = rlogic_serialization::CreateRamsesRenderPassBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesRenderPassBindingImpl> deserialized = RamsesRenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing name!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of RamsesRenderPassBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ErrorWhenNoBindingId)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    0) // no id
            );
            auto binding = rlogic_serialization::CreateRamsesRenderPassBinding(m_flatBufferBuilder, base);
            m_flatBufferBuilder.Finish(binding);
        }

        const auto&                                  serialized   = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesRenderPassBindingImpl> deserialized = RamsesRenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing or invalid ID!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of RamsesRenderPassBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ErrorWhenNoRootInput)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0 // no root input
            );
            auto binding = rlogic_serialization::CreateRamsesRenderPassBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesRenderPassBindingImpl> deserialized = RamsesRenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderPassBinding from serialized data: missing root input!");
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ErrorWhenBoundRenderPassCannotBeResolved)
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
            auto binding = rlogic_serialization::CreateRamsesRenderPassBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("name"), mockObjectId)).WillOnce(::testing::Return(nullptr));

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesRenderPassBindingImpl> deserialized = RamsesRenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ErrorWhenRootInputHasErrors)
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
            auto binding = rlogic_serialization::CreateRamsesRenderPassBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesRenderPassBindingImpl> deserialized = RamsesRenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, CanSerializeWithNoIssue)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ARamsesRenderPassBinding_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ReportsSerializationError_RootNotStructType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderPassBinding_SerializationLifecycle::ESerializationIssue::RootNotStruct));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderPassBinding from serialized data: root input has unexpected type!");
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ReportsSerializationError_BoundObjectReferenceMissing)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderPassBinding_SerializationLifecycle::ESerializationIssue::BoundObjectReferenceMissing));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderPassBinding from serialized data: missing ramses object reference!");
    }

    TEST_F(ARamsesRenderPassBinding_SerializationLifecycle, ReportsSerializationError_BoundObjectTypeMismatch)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderPassBinding_SerializationLifecycle::ESerializationIssue::BoundObjectTypeMismatch));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderPassBinding from serialized data: loaded object type does not match referenced object type!");
    }

    class ARamsesRenderPassBinding_WithRamses : public ARamsesRenderPassBinding
    {
    };

    TEST_F(ARamsesRenderPassBinding_WithRamses, ReturnsReferenceToRamsesRenderPass)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
        EXPECT_EQ(m_renderPass, &renderPassBinding.getRamsesRenderPass());
    }

    TEST_F(ARamsesRenderPassBinding_WithRamses, GivesInputs_BindingInputSemantics)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
        for (size_t i = 0; i < renderPassBinding.getInputs()->getChildCount(); ++i)
        {
            EXPECT_EQ(EPropertySemantics::BindingInput, renderPassBinding.getInputs()->getChild(i)->m_impl->getPropertySemantics());
        }
    }

    TEST_F(ARamsesRenderPassBinding_WithRamses, TakesInitialValuesFromRamsesRenderPass)
    {
        m_renderPass->setEnabled(false);
        m_renderPass->setRenderOrder(42);
        m_renderPass->setClearColor({0.1f, 0.2f, 0.3f, 0.4f});
        m_renderPass->setRenderOnce(true);

        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
        auto inputs = renderPassBinding.getInputs();
        EXPECT_FALSE(*inputs->getChild("enabled")->get<bool>());
        EXPECT_EQ(42, *inputs->getChild("renderOrder")->get<int32_t>());
        EXPECT_EQ(*inputs->getChild("clearColor")->get<vec4f>(), vec4f(0.1f, 0.2f, 0.3f, 0.4f));
        EXPECT_TRUE(inputs->getChild("renderOnce")->set(true));
    }

    TEST_F(ARamsesRenderPassBinding_WithRamses, UpdatesRenderPassIfInputValuesWereSet)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
        auto inputs = renderPassBinding.getInputs();
        ASSERT_EQ(std::size_t(RamsesRenderPassBindingImpl::EPropertyIndex_COUNT), inputs->getChildCount());
        EXPECT_TRUE(inputs->getChild("enabled")->set(false));
        EXPECT_TRUE(inputs->getChild("renderOrder")->set(42));
        EXPECT_TRUE(inputs->getChild("clearColor")->set<vec4f>({ 0.1f, 0.2f, 0.3f, 0.4f }));
        EXPECT_TRUE(inputs->getChild("renderOnce")->set(true));

        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_FALSE(m_renderPass->isEnabled());
        EXPECT_EQ(42, m_renderPass->getRenderOrder());
        vec4f clearColor = m_renderPass->getClearColor();
        EXPECT_EQ(clearColor, vec4f(0.1f, 0.2f, 0.3f, 0.4f));
        EXPECT_TRUE(m_renderPass->isRenderOnce());
    }

    TEST_F(ARamsesRenderPassBinding_WithRamses, PropagateItsInputsToRamsesRenderPassOnUpdate_OnlyWhenExplicitlySet)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");

        // Set values directly
        m_renderPass->setEnabled(false);
        m_renderPass->setRenderOrder(3);

        // Set only one of the inputs to the binding object, the other one (enabled) not
        EXPECT_TRUE(renderPassBinding.getInputs()->getChild("renderOrder")->set(99));

        EXPECT_TRUE(m_logicEngine.update());

        // Only propagate the value which was also set in the binding object
        EXPECT_FALSE(m_renderPass->isEnabled());
        EXPECT_EQ(99, m_renderPass->getRenderOrder());
    }

    TEST_F(ARamsesRenderPassBinding_WithRamses, PropagatesItsInputsToRamsesRenderPassOnUpdate_WithLinksInsteadOfSetCall)
    {
        auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");

        // Set values directly
        m_renderPass->setRenderOrder(13);
        m_renderPass->setEnabled(false);

        // Link binding input to a script (binding is not set directly, but is linked)
        const std::string_view scriptSrc = R"(
                function interface(IN,OUT)
                    OUT.val = Type:Int32()
                end
                function run(IN,OUT)
                    OUT.val = 42
                end
            )";
        const LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("val"), *renderPassBinding.getInputs()->getChild("renderOrder")));

        EXPECT_TRUE(m_logicEngine.update());

        // Only propagate the value which was also linked over the binding object's input to a script
        EXPECT_EQ(42, m_renderPass->getRenderOrder());
        EXPECT_FALSE(m_renderPass->isEnabled());
    }

    class ARamsesRenderPassBinding_WithRamses_AndFiles : public ARamsesRenderPassBinding_WithRamses
    {
    protected:
        WithTempDirectory tempFolder;
    };

    TEST_F(ARamsesRenderPassBinding_WithRamses_AndFiles, KeepsItsPropertiesAfterDeserialization_WhenNoRamsesLinksAndSceneProvided)
    {
        {
            m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
            ASSERT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "binding.bin"));
        }

        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("binding.bin", m_scene));
            auto loadedBinding = m_logicEngine.findByName<RamsesRenderPassBinding>("renderPass");
            EXPECT_EQ(&loadedBinding->getRamsesRenderPass(), m_renderPass);
            EXPECT_EQ(loadedBinding->getName(), "renderPass");

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(std::size_t(RamsesRenderPassBindingImpl::EPropertyIndex_COUNT), loadedBinding->getInputs()->getChildCount());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_Enabled), loadedBinding->getInputs()->getChild("enabled"));
            EXPECT_EQ(EPropertyType::Bool, loadedBinding->getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_Enabled)->getType());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_RenderOrder), loadedBinding->getInputs()->getChild("renderOrder"));
            EXPECT_EQ(EPropertyType::Int32, loadedBinding->getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_RenderOrder)->getType());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_ClearColor), loadedBinding->getInputs()->getChild("clearColor"));
            EXPECT_EQ(EPropertyType::Vec4f, loadedBinding->getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_ClearColor)->getType());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_RenderOnce), loadedBinding->getInputs()->getChild("renderOnce"));
            EXPECT_EQ(EPropertyType::Bool, loadedBinding->getInputs()->getChild(RamsesRenderPassBindingImpl::EPropertyIndex_RenderOnce)->getType());
        }
    }

    TEST_F(ARamsesRenderPassBinding_WithRamses_AndFiles, KeepsPropertyValueAfterDeserializationWithScene)
    {
        const ramses::sceneObjectId_t bindingIdBeforeReload = m_renderPass->getSceneObjectId();
        {
            auto& binding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
            binding.getInputs()->getChild("renderOrder")->set(42);
            m_logicEngine.update();
            EXPECT_EQ(42, m_renderPass->getRenderOrder());
            ASSERT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "logic.bin"));
        }

        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("logic.bin", m_scene));
            auto loadedBinding = m_logicEngine.findByName<RamsesRenderPassBinding>("renderPass");
            EXPECT_EQ(loadedBinding->getRamsesRenderPass().getSceneObjectId(), bindingIdBeforeReload);

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(std::size_t(RamsesRenderPassBindingImpl::EPropertyIndex_COUNT), loadedBinding->getInputs()->getChildCount());
            EXPECT_EQ(42, *loadedBinding->getInputs()->getChild("renderOrder")->get<int32_t>());
            EXPECT_EQ(42, m_renderPass->getRenderOrder());
        }
    }

    TEST_F(ARamsesRenderPassBinding_WithRamses_AndFiles, ProducesError_WhenHavingLinkToRenderPass_ButNoSceneWasProvided)
    {
        {
            LogicEngine tempEngineForSaving{ m_logicEngine.getFeatureLevel() };
            tempEngineForSaving.createRamsesRenderPassBinding(*m_renderPass, "AppBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(tempEngineForSaving, "WithRamsesRenderPass.bin"));
        }
        {
            EXPECT_FALSE(m_logicEngine.loadFromFile("WithRamsesRenderPass.bin"));
            const auto& errors = m_logicEngine.getErrors();
            ASSERT_EQ(errors.size(), 1u);
            EXPECT_EQ(errors[0].message, "Fatal error during loading from file! File contains references to Ramses objects but no Ramses scene was provided!");
        }
    }

    // This is sort of a confidence test, testing a combination of:
    // - bindings only propagating their values to ramses if the value was set by an incoming link
    // - saving and loading files
    // - value only re-applied to ramses if changed. Otherwise not.
    // The general expectation is that after loading + update(), the logic scene would overwrite ramses
    // properties wrapped by a LogicBinding if they are linked to a script
    TEST_F(ARamsesRenderPassBinding_WithRamses_AndFiles, SetsOnlyRenderPassValuesForWhichTheBindingInputIsLinked_AfterLoadingFromFile_AndCallingUpdate)
    {
        {
            const std::string_view scriptSrc = R"(
                function interface(IN,OUT)
                    OUT.val = Type:Int32()
                end
                function run(IN,OUT)
                    OUT.val = 42
                end
            )";

            LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);
            auto& renderPassBinding = *m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");

            ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("val"), *renderPassBinding.getInputs()->getChild("renderOrder")));
            ASSERT_TRUE(m_logicEngine.update());
            ASSERT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "SomeValuesLinked.bin"));
        }

        // Set renderOrder to a different value than the one set by the link
        m_renderPass->setRenderOrder(13);
        // Set renderOnce to custom value - it should not be overwritten by logic at all, because there is no link
        // or any set() calls to the corresponding RamsesRenderPassBinding input
        m_renderPass->setRenderOnce(true);

        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("SomeValuesLinked.bin", m_scene));

            // nothing happens before update()
            EXPECT_EQ(13, m_renderPass->getRenderOrder());
            EXPECT_TRUE(m_renderPass->isRenderOnce());

            EXPECT_TRUE(m_logicEngine.update());

            // Script is executed -> link is activated -> binding is updated, only for the linked property
            EXPECT_EQ(42, m_renderPass->getRenderOrder());
            EXPECT_TRUE(m_renderPass->isRenderOnce());

            // Reset uniform manually and call update does nothing (must set binding input explicitly to cause overwrite in ramses)
            m_renderPass->setRenderOrder(13);
            EXPECT_TRUE(m_logicEngine.update());
            EXPECT_EQ(13, m_renderPass->getRenderOrder());
            EXPECT_TRUE(m_renderPass->isRenderOnce());
        }
    }
}
