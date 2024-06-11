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

#include "impl/logic/LogicEngineImpl.h"
#include "impl/logic/RenderPassBindingImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/RamsesHelper.h"
#include "internal/logic/flatbuffers/generated/RenderPassBindingGen.h"

#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/ramses-utils.h"

namespace ramses::internal
{
    class ARenderPassBinding : public ALogicEngine
    {
    };

    TEST_F(ARenderPassBinding, HasANameAfterCreation)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        EXPECT_EQ("renderPass", renderPassBinding.getName());
    }

    TEST_F(ARenderPassBinding, RefersToGivenRenderPass)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        EXPECT_EQ(m_renderPass, &renderPassBinding.getRamsesRenderPass());
        const auto& rpConst = renderPassBinding;
        EXPECT_EQ(m_renderPass, &rpConst.getRamsesRenderPass());
        const auto& rpImplConst = renderPassBinding.impl();
        EXPECT_EQ(m_renderPass, &rpImplConst.getRamsesRenderPass());
        EXPECT_EQ(m_renderPass, &rpImplConst.getBoundObject());
    }

    TEST_F(ARenderPassBinding, HasInputsAfterCreation)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        ASSERT_NE(nullptr, renderPassBinding.getInputs());
        ASSERT_EQ(std::size_t(RenderPassBindingImpl::EPropertyIndex_COUNT), renderPassBinding.getInputs()->getChildCount());
        EXPECT_EQ(renderPassBinding.getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_Enabled), renderPassBinding.getInputs()->getChild("enabled"));
        EXPECT_EQ(EPropertyType::Bool, renderPassBinding.getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_Enabled)->getType());
        EXPECT_EQ(renderPassBinding.getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_RenderOrder), renderPassBinding.getInputs()->getChild("renderOrder"));
        EXPECT_EQ(EPropertyType::Int32, renderPassBinding.getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_RenderOrder)->getType());
        EXPECT_EQ(renderPassBinding.getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_ClearColor), renderPassBinding.getInputs()->getChild("clearColor"));
        EXPECT_EQ(EPropertyType::Vec4f, renderPassBinding.getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_ClearColor)->getType());
        EXPECT_EQ(renderPassBinding.getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_RenderOnce), renderPassBinding.getInputs()->getChild("renderOnce"));
        EXPECT_EQ(EPropertyType::Bool, renderPassBinding.getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_RenderOnce)->getType());
    }

    TEST_F(ARenderPassBinding, HasNoOutputsAfterCreation)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        EXPECT_EQ(nullptr, renderPassBinding.getOutputs());
    }

    // This fixture only contains serialization unit tests, for higher order tests see `ARenderPassBinding_WithRamses_AndFiles`
    class ARenderPassBinding_SerializationLifecycle : public ARenderPassBinding
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            RootNotStruct,
            BoundObjectReferenceMissing,
            BoundObjectTypeMismatch
        };

        std::unique_ptr<RenderPassBindingImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
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
                    rlogic_serialization::CreateLogicObject(m_flatBufferBuilder, m_flatBufferBuilder.CreateString("name"), 1u),
                    (issue == ESerializationIssue::BoundObjectReferenceMissing ? 0 : rlogic_serialization::CreateRamsesReference(m_flatBufferBuilder,
                        1u, (issue == ESerializationIssue::BoundObjectTypeMismatch ? 0 : static_cast<uint32_t>(ERamsesObjectType::RenderPass)))),
                    PropertyImpl::Serialize(*inputsImpl, m_flatBufferBuilder, m_serializationMap));

                auto fbRenderPassBinding = rlogic_serialization::CreateRenderPassBinding(m_flatBufferBuilder, fbRamsesBinding);
                m_flatBufferBuilder.Finish(fbRenderPassBinding);
            }

            switch (issue)
            {
            case ESerializationIssue::AllValid:
            case ESerializationIssue::BoundObjectTypeMismatch:
                EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("name"), sceneObjectId_t{ 1u })).WillOnce(::testing::Return(m_renderPass));
                break;
            case ESerializationIssue::RootNotStruct:
            case ESerializationIssue::BoundObjectReferenceMissing:
                break;
            }

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
            return RenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils {m_flatBufferBuilder};
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;
        ErrorReporting m_errorReporting;
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap{ m_scene->impl() };
    };

    // More unit tests with inputs/outputs declared in LogicNode (base class) serialization tests
    TEST_F(ARenderPassBinding_SerializationLifecycle, StoresBaseClassData)
    {
        // Serialize
        {
            RenderPassBindingImpl binding(m_scene->impl(), *m_renderPass, "name", sceneObjectId_t{ 1u });
            binding.createRootProperties();
            (void)RenderPassBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedBinding.base());
        ASSERT_TRUE(serializedBinding.base()->base());
        ASSERT_TRUE(serializedBinding.base()->base()->name());
        EXPECT_EQ(serializedBinding.base()->base()->name()->string_view(), "name");
        EXPECT_EQ(serializedBinding.base()->base()->id(), 1u);

        ASSERT_TRUE(serializedBinding.base()->rootInput());
        EXPECT_EQ(serializedBinding.base()->rootInput()->rootType(), rlogic_serialization::EPropertyRootType::Struct);
        ASSERT_TRUE(serializedBinding.base()->rootInput()->children());
        EXPECT_EQ(serializedBinding.base()->rootInput()->children()->size(), size_t(RenderPassBindingImpl::EPropertyIndex_COUNT));

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("name"), m_renderPass->getSceneObjectId())).WillOnce(::testing::Return(m_renderPass));
            std::unique_ptr<RenderPassBindingImpl> deserializedBinding = RenderPassBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(deserializedBinding->getName(), "name");
            EXPECT_EQ(deserializedBinding->getSceneObjectId().getValue(), 1u);
            EXPECT_FALSE(m_errorReporting.getError().has_value());
        }
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, StoresIdAndTypeAndRenderPassRef)
    {
        // Serialize
        {
            RenderPassBindingImpl binding(m_scene->impl(), *m_renderPass, "name", sceneObjectId_t{ 1u });
            binding.createRootProperties();
            (void)RenderPassBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedBinding.base()->boundRamsesObject());
        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectId(), m_renderPass->getSceneObjectId().getValue());
        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectType(), static_cast<uint32_t>(ERamsesObjectType::RenderPass));

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("name"), m_renderPass->getSceneObjectId())).WillOnce(::testing::Return(m_renderPass));
            std::unique_ptr<RenderPassBindingImpl> deserializedBinding = RenderPassBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(&deserializedBinding->getRamsesRenderPass(), m_renderPass);
            EXPECT_FALSE(m_errorReporting.getError().has_value());

            // Check that input was deserialized too
            EXPECT_EQ(deserializedBinding->getInputs()->getChild("enabled")->getType(), EPropertyType::Bool);
        }
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ErrorWhenNoBindingBaseData)
    {
        {
            auto binding = rlogic_serialization::CreateRenderPassBinding(
                m_flatBufferBuilder,
                0 // no base binding info
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RenderPassBindingImpl> deserialized = RenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderPassBinding from serialized data: missing base class info!");
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ErrorWhenNoBindingName)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    0, // no name!
                    1u)
            );
            auto binding = rlogic_serialization::CreateRenderPassBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RenderPassBindingImpl> deserialized = RenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(this->m_errorReporting.getError().has_value());
        EXPECT_EQ("Fatal error during loading of RenderPassBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getError()->message);
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ErrorWhenNoBindingId)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    0) // no id
            );
            auto binding = rlogic_serialization::CreateRenderPassBinding(m_flatBufferBuilder, base);
            m_flatBufferBuilder.Finish(binding);
        }

        const auto&                                  serialized   = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RenderPassBindingImpl> deserialized = RenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(this->m_errorReporting.getError().has_value());
        EXPECT_EQ("Fatal error during loading of RenderPassBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getError()->message);
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ErrorWhenNoRootInput)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0 // no root input
            );
            auto binding = rlogic_serialization::CreateRenderPassBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RenderPassBindingImpl> deserialized = RenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderPassBinding from serialized data: missing root input!");
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ErrorWhenBoundRenderPassCannotBeResolved)
    {
        const sceneObjectId_t mockObjectId {12};
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
            auto binding = rlogic_serialization::CreateRenderPassBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("name"), mockObjectId)).WillOnce(::testing::Return(nullptr));

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RenderPassBindingImpl> deserialized = RenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ErrorWhenRootInputHasErrors)
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
            auto binding = rlogic_serialization::CreateRenderPassBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RenderPassBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RenderPassBindingImpl> deserialized = RenderPassBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, CanSerializeWithNoIssue)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ARenderPassBinding_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ReportsSerializationError_RootNotStructType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderPassBinding_SerializationLifecycle::ESerializationIssue::RootNotStruct));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderPassBinding from serialized data: root input has unexpected type!");
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ReportsSerializationError_BoundObjectReferenceMissing)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderPassBinding_SerializationLifecycle::ESerializationIssue::BoundObjectReferenceMissing));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderPassBinding from serialized data: missing ramses object reference!");
    }

    TEST_F(ARenderPassBinding_SerializationLifecycle, ReportsSerializationError_BoundObjectTypeMismatch)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderPassBinding_SerializationLifecycle::ESerializationIssue::BoundObjectTypeMismatch));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderPassBinding from serialized data: loaded object type does not match referenced object type!");
    }

    class ARenderPassBinding_WithRamses : public ARenderPassBinding
    {
    };

    TEST_F(ARenderPassBinding_WithRamses, ReturnsReferenceToRamsesRenderPass)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        EXPECT_EQ(m_renderPass, &renderPassBinding.getRamsesRenderPass());
    }

    TEST_F(ARenderPassBinding_WithRamses, GivesInputs_BindingInputSemantics)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        for (size_t i = 0; i < renderPassBinding.getInputs()->getChildCount(); ++i)
        {
            EXPECT_EQ(EPropertySemantics::BindingInput, renderPassBinding.getInputs()->getChild(i)->impl().getPropertySemantics());
        }
    }

    TEST_F(ARenderPassBinding_WithRamses, TakesInitialValuesFromRamsesRenderPass)
    {
        m_renderPass->setEnabled(false);
        m_renderPass->setRenderOrder(42);
        m_renderPass->setClearColor({0.1f, 0.2f, 0.3f, 0.4f});
        m_renderPass->setRenderOnce(true);

        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        auto inputs = renderPassBinding.getInputs();
        EXPECT_FALSE(*inputs->getChild("enabled")->get<bool>());
        EXPECT_EQ(42, *inputs->getChild("renderOrder")->get<int32_t>());
        EXPECT_EQ(*inputs->getChild("clearColor")->get<vec4f>(), vec4f(0.1f, 0.2f, 0.3f, 0.4f));
        EXPECT_TRUE(inputs->getChild("renderOnce")->set(true));
    }

    TEST_F(ARenderPassBinding_WithRamses, UpdatesRenderPassIfInputValuesWereSet)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        auto inputs = renderPassBinding.getInputs();
        ASSERT_EQ(std::size_t(RenderPassBindingImpl::EPropertyIndex_COUNT), inputs->getChildCount());
        EXPECT_TRUE(inputs->getChild("enabled")->set(false));
        EXPECT_TRUE(inputs->getChild("renderOrder")->set(42));
        EXPECT_TRUE(inputs->getChild("clearColor")->set<vec4f>({ 0.1f, 0.2f, 0.3f, 0.4f }));
        EXPECT_TRUE(inputs->getChild("renderOnce")->set(true));

        EXPECT_TRUE(m_logicEngine->update());

        EXPECT_FALSE(m_renderPass->isEnabled());
        EXPECT_EQ(42, m_renderPass->getRenderOrder());
        vec4f clearColor = m_renderPass->getClearColor();
        EXPECT_EQ(clearColor, vec4f(0.1f, 0.2f, 0.3f, 0.4f));
        EXPECT_TRUE(m_renderPass->isRenderOnce());
    }

    TEST_F(ARenderPassBinding_WithRamses, PropagateItsInputsToRamsesRenderPassOnUpdate_OnlyWhenExplicitlySet)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");

        // Set values directly
        m_renderPass->setEnabled(false);
        m_renderPass->setRenderOrder(3);

        // Set only one of the inputs to the binding object, the other one (enabled) not
        EXPECT_TRUE(renderPassBinding.getInputs()->getChild("renderOrder")->set(99));

        EXPECT_TRUE(m_logicEngine->update());

        // Only propagate the value which was also set in the binding object
        EXPECT_FALSE(m_renderPass->isEnabled());
        EXPECT_EQ(99, m_renderPass->getRenderOrder());
    }

    TEST_F(ARenderPassBinding_WithRamses, PropagatesItsInputsToRamsesRenderPassOnUpdate_WithLinksInsteadOfSetCall)
    {
        auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");

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
        LuaScript* script = m_logicEngine->createLuaScript(scriptSrc);
        ASSERT_TRUE(m_logicEngine->link(*script->getOutputs()->getChild("val"), *renderPassBinding.getInputs()->getChild("renderOrder")));

        EXPECT_TRUE(m_logicEngine->update());

        // Only propagate the value which was also linked over the binding object's input to a script
        EXPECT_EQ(42, m_renderPass->getRenderOrder());
        EXPECT_FALSE(m_renderPass->isEnabled());
    }

    class ARenderPassBinding_WithRamses_AndFiles : public ARenderPassBinding_WithRamses
    {
    public:
        ARenderPassBinding_WithRamses_AndFiles()
        {
            withTempDirectory();
        }
    };

    TEST_F(ARenderPassBinding_WithRamses_AndFiles, KeepsItsPropertiesAfterDeserialization_WhenNoRamsesLinksAndSceneProvided)
    {
        {
            m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
            ASSERT_TRUE(saveToFile("binding.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("binding.bin"));
            ASSERT_TRUE(m_logicEngine != nullptr);
            auto loadedBinding = m_logicEngine->findObject<RenderPassBinding>("renderPass");
            EXPECT_EQ(&loadedBinding->getRamsesRenderPass(), m_renderPass);
            EXPECT_EQ(loadedBinding->getName(), "renderPass");

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(std::size_t(RenderPassBindingImpl::EPropertyIndex_COUNT), loadedBinding->getInputs()->getChildCount());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_Enabled), loadedBinding->getInputs()->getChild("enabled"));
            EXPECT_EQ(EPropertyType::Bool, loadedBinding->getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_Enabled)->getType());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_RenderOrder), loadedBinding->getInputs()->getChild("renderOrder"));
            EXPECT_EQ(EPropertyType::Int32, loadedBinding->getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_RenderOrder)->getType());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_ClearColor), loadedBinding->getInputs()->getChild("clearColor"));
            EXPECT_EQ(EPropertyType::Vec4f, loadedBinding->getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_ClearColor)->getType());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_RenderOnce), loadedBinding->getInputs()->getChild("renderOnce"));
            EXPECT_EQ(EPropertyType::Bool, loadedBinding->getInputs()->getChild(RenderPassBindingImpl::EPropertyIndex_RenderOnce)->getType());
        }
    }

    TEST_F(ARenderPassBinding_WithRamses_AndFiles, KeepsPropertyValueAfterDeserializationWithScene)
    {
        const sceneObjectId_t bindingIdBeforeReload = m_renderPass->getSceneObjectId();
        {
            auto& binding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
            binding.getInputs()->getChild("renderOrder")->set(42);
            m_logicEngine->update();
            EXPECT_EQ(42, m_renderPass->getRenderOrder());
            ASSERT_TRUE(saveToFile("logic.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("logic.bin"));
            ASSERT_TRUE(m_logicEngine != nullptr);
            auto loadedBinding = m_logicEngine->findObject<RenderPassBinding>("renderPass");
            EXPECT_EQ(loadedBinding->getRamsesRenderPass().getSceneObjectId(), bindingIdBeforeReload);

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(std::size_t(RenderPassBindingImpl::EPropertyIndex_COUNT), loadedBinding->getInputs()->getChildCount());
            EXPECT_EQ(42, *loadedBinding->getInputs()->getChild("renderOrder")->get<int32_t>());
            EXPECT_EQ(42, m_renderPass->getRenderOrder());
        }
    }

    // This is sort of a confidence test, testing a combination of:
    // - bindings only propagating their values to ramses if the value was set by an incoming link
    // - saving and loading files
    // - value only re-applied to ramses if changed. Otherwise not.
    // The general expectation is that after loading + update(), the logic scene would overwrite ramses
    // properties wrapped by a LogicBinding if they are linked to a script
    TEST_F(ARenderPassBinding_WithRamses_AndFiles, SetsOnlyRenderPassValuesForWhichTheBindingInputIsLinked_AfterLoadingFromFile_AndCallingUpdate)
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

            LuaScript* script = m_logicEngine->createLuaScript(scriptSrc);
            auto& renderPassBinding = *m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");

            ASSERT_TRUE(m_logicEngine->link(*script->getOutputs()->getChild("val"), *renderPassBinding.getInputs()->getChild("renderOrder")));
            ASSERT_TRUE(m_logicEngine->update());

            // Set renderOrder to a different value than the one set by the link
            m_renderPass->setRenderOrder(13);
            // Set renderOnce to custom value - it should not be overwritten by logic at all, because there is no link
            // or any set() calls to the corresponding RenderPassBinding input
            m_renderPass->setRenderOnce(true);

            ASSERT_TRUE(saveToFile("SomeValuesLinked.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("SomeValuesLinked.bin"));
            ASSERT_TRUE(m_logicEngine != nullptr);

            // nothing happens before update()
            EXPECT_EQ(13, m_renderPass->getRenderOrder());
            EXPECT_TRUE(m_renderPass->isRenderOnce());

            EXPECT_TRUE(m_logicEngine->update());

            // Script is executed -> link is activated -> binding is updated, only for the linked property
            EXPECT_EQ(42, m_renderPass->getRenderOrder());
            EXPECT_TRUE(m_renderPass->isRenderOnce());

            // Reset uniform manually and call update does nothing (must set binding input explicitly to cause overwrite in ramses)
            m_renderPass->setRenderOrder(13);
            EXPECT_TRUE(m_logicEngine->update());
            EXPECT_EQ(13, m_renderPass->getRenderOrder());
            EXPECT_TRUE(m_renderPass->isRenderOnce());
        }
    }
}
