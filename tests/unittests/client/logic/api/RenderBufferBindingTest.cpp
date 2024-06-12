//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"
#include "RamsesTestUtils.h"
#include "SerializationTestUtils.h"
#include "RamsesObjectResolverMock.h"

#include "ramses/client/logic/RenderBufferBinding.h"
#include "ramses/client/logic/Property.h"
#include "impl/logic/RenderBufferBindingImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/TypeData.h"

#include "internal/logic/flatbuffers/generated/RenderBufferBindingGen.h"

namespace ramses::internal
{
    class ARenderBufferBinding : public ALogicEngine
    {
    protected:
        RenderBufferBinding* m_rbBinding{ m_logicEngine->createRenderBufferBinding(*m_renderBuffer, "binding") };

        static constexpr auto PropCount = static_cast<size_t>(RenderBufferBindingImpl::EInputProperty::COUNT);
        static constexpr auto PropIdxWidth = static_cast<size_t>(RenderBufferBindingImpl::EInputProperty::Width);
        static constexpr auto PropIdxHeight = static_cast<size_t>(RenderBufferBindingImpl::EInputProperty::Height);
        static constexpr auto PropIdxSampleCount = static_cast<size_t>(RenderBufferBindingImpl::EInputProperty::SampleCount);
    };

    TEST_F(ARenderBufferBinding, RefersToGivenRamsesObject)
    {
        EXPECT_EQ(m_renderBuffer, &m_rbBinding->getRenderBuffer());
        const auto& rbConst = *m_rbBinding;
        EXPECT_EQ(m_renderBuffer, &rbConst.getRenderBuffer());
        const auto& rbImplConst = m_rbBinding->impl();
        EXPECT_EQ(m_renderBuffer, &rbImplConst.getRenderBuffer());
        EXPECT_EQ(m_renderBuffer, &rbImplConst.getBoundObject());
    }

    TEST_F(ARenderBufferBinding, HasInputPropertiesAndNoOutputs)
    {
        ASSERT_NE(nullptr, m_rbBinding->getInputs());
        ASSERT_EQ(PropCount, m_rbBinding->getInputs()->getChildCount());
        EXPECT_EQ(m_rbBinding->getInputs()->getChild(PropIdxWidth), m_rbBinding->getInputs()->getChild("width"));
        EXPECT_EQ(m_rbBinding->getInputs()->getChild(PropIdxHeight), m_rbBinding->getInputs()->getChild("height"));
        EXPECT_EQ(m_rbBinding->getInputs()->getChild(PropIdxSampleCount), m_rbBinding->getInputs()->getChild("sampleCount"));
        EXPECT_EQ(nullptr, m_rbBinding->getOutputs());
    }

    TEST_F(ARenderBufferBinding, InputPropertiesAreInitializedFromBoundRenderBuffer)
    {
        EXPECT_EQ(1, *m_rbBinding->getInputs()->getChild(PropIdxWidth)->get<int32_t>());
        EXPECT_EQ(2, *m_rbBinding->getInputs()->getChild(PropIdxHeight)->get<int32_t>());
        EXPECT_EQ(3, *m_rbBinding->getInputs()->getChild(PropIdxSampleCount)->get<int32_t>());
    }

    TEST_F(ARenderBufferBinding, SetsModifiedBoundValuesOnUpdate)
    {
        // initial values
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(1u, m_renderBuffer->getWidth());
        EXPECT_EQ(2u, m_renderBuffer->getHeight());
        EXPECT_EQ(3u, m_renderBuffer->getSampleCount());

        EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxWidth)->set(42));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(42u, m_renderBuffer->getWidth());
        EXPECT_EQ(2u, m_renderBuffer->getHeight());
        EXPECT_EQ(3u, m_renderBuffer->getSampleCount());

        EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxHeight)->set(43));
        EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxSampleCount)->set(44));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(42u, m_renderBuffer->getWidth());
        EXPECT_EQ(43u, m_renderBuffer->getHeight());
        EXPECT_EQ(44u, m_renderBuffer->getSampleCount());
    }

    TEST_F(ARenderBufferBinding, FailsUpdateIfTryingToSetInvalidValue_Width)
    {
        EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxWidth)->set(-1));
        EXPECT_FALSE(m_logicEngine->update());
        expectError("RenderBufferBinding input cannot be negative", m_rbBinding);
    }

    TEST_F(ARenderBufferBinding, FailsUpdateIfTryingToSetInvalidValue_Height)
    {
        EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxHeight)->set(-1));
        EXPECT_FALSE(m_logicEngine->update());
        expectError("RenderBufferBinding input cannot be negative", m_rbBinding);
    }

    TEST_F(ARenderBufferBinding, FailsUpdateIfTryingToSetInvalidValue_SampleCount)
    {
        EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxSampleCount)->set(-1));
        EXPECT_FALSE(m_logicEngine->update());
        expectError("RenderBufferBinding input cannot be negative", m_rbBinding);
    }

    TEST_F(ARenderBufferBinding, FailsUpdateIfReferencedRenderBufferSetFails)
    {
        // binding only checks for negative values, render buffer setter for other criteria, e.g. width/height cant be zero
        EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxHeight)->set(0));
        EXPECT_FALSE(m_logicEngine->update());
        expectError("RenderBuffer::setProperties: width and height cannot be zero", m_rbBinding);
    }

    class ARenderBufferBinding_SerializationLifecycle : public ARenderBufferBinding
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            MissingBase,
            MissingName,
            MissingRoot,
            CorruptedInputProperties,
            MissingBoundObject,
            UnresolvedRenderBuffer,
            InvalidBoundObjectType
        };

        std::unique_ptr<RenderBufferBindingImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            {
                auto inputsType = MakeStruct("", {
                    TypeData{"width", EPropertyType::Int32},
                    TypeData{"height", EPropertyType::Int32},
                    (issue == ESerializationIssue::CorruptedInputProperties ? TypeData{"wrong", EPropertyType::Int32} : TypeData{"sampleCount", EPropertyType::Int32})
                    });
                auto inputs = std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput);

                SerializationMap serializationMap;
                const auto logicObject = rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    (issue == ESerializationIssue::MissingName ? 0 : m_flatBufferBuilder.CreateString("name")), 1u);
                auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(m_flatBufferBuilder,
                    logicObject,
                    (issue == ESerializationIssue::MissingBoundObject ? 0 : rlogic_serialization::CreateRamsesReference(m_flatBufferBuilder,
                        1u, (issue == ESerializationIssue::InvalidBoundObjectType ? 0 : static_cast<uint32_t>(ramses::ERamsesObjectType::RenderBuffer)))),
                    (issue == ESerializationIssue::MissingRoot ? 0 : PropertyImpl::Serialize(*inputs, m_flatBufferBuilder, serializationMap)));

                auto fbRenderBufferBinding = rlogic_serialization::CreateRenderBufferBinding(m_flatBufferBuilder, (issue == ESerializationIssue::MissingBase ? 0 : fbRamsesBinding));
                m_flatBufferBuilder.Finish(fbRenderBufferBinding);
            }

            switch (issue)
            {
            case ESerializationIssue::AllValid:
            case ESerializationIssue::InvalidBoundObjectType:
                EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 1u })).WillOnce(::testing::Return(m_renderBuffer));
                break;
            case ESerializationIssue::UnresolvedRenderBuffer:
                EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 1u })).WillOnce(::testing::Return(nullptr));
                break;
            default:
                break;
            }

            DeserializationMap deserializationMap{ m_scene->impl() };
            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RenderBufferBinding>(m_flatBufferBuilder.GetBufferPointer());
            return RenderBufferBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, deserializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;
        ErrorReporting m_errorReporting;
    };

    TEST_F(ARenderBufferBinding_SerializationLifecycle, CanSerializeWithNoIssue)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ARenderBufferBinding_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, ReportsSerializationError_MissingBase)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderBufferBinding_SerializationLifecycle::ESerializationIssue::MissingBase));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderBufferBinding from serialized data: missing base class info!");
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, ReportsSerializationError_MissingName)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderBufferBinding_SerializationLifecycle::ESerializationIssue::MissingName));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderBufferBinding from serialized data: missing name and/or ID!");
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, ReportsSerializationError_MissingRoot)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderBufferBinding_SerializationLifecycle::ESerializationIssue::MissingRoot));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderBufferBinding from serialized data: missing root input!");
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, ReportsSerializationError_CorruptedInputProperties)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderBufferBinding_SerializationLifecycle::ESerializationIssue::CorruptedInputProperties));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderBufferBinding from serialized data: corrupted root input!");
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, ReportsSerializationError_MissingBoundObject)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderBufferBinding_SerializationLifecycle::ESerializationIssue::MissingBoundObject));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderBufferBinding from serialized data: missing ramses object reference!");
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, ReportsSerializationError_UnresolvedRenderBuffer)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderBufferBinding_SerializationLifecycle::ESerializationIssue::UnresolvedRenderBuffer));
        // error message is generated in resolver which is mocked here
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, ReportsSerializationError_InvalidBoundObjectType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARenderBufferBinding_SerializationLifecycle::ESerializationIssue::InvalidBoundObjectType));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderBufferBinding from serialized data: loaded object type does not match referenced object type!");
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, KeepsItsPropertiesAfterDeserialization)
    {
        {
            EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxHeight)->set(41));
            EXPECT_TRUE(m_rbBinding->getInputs()->getChild(PropIdxSampleCount)->set(42));
            EXPECT_TRUE(m_logicEngine->update());
            ASSERT_TRUE(saveToFile("binding.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("binding.bin"));
            const auto loadedBinding = m_logicEngine->findObject<RenderBufferBinding>("binding");
            ASSERT_TRUE(loadedBinding);
            EXPECT_EQ(m_renderBuffer, &loadedBinding->getRenderBuffer());

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(PropCount, loadedBinding->getInputs()->getChildCount());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(PropIdxWidth), loadedBinding->getInputs()->getChild("width"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(PropIdxHeight), loadedBinding->getInputs()->getChild("height"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(PropIdxSampleCount), loadedBinding->getInputs()->getChild("sampleCount"));
            EXPECT_EQ(nullptr, loadedBinding->getOutputs());

            EXPECT_EQ(41, *loadedBinding->getInputs()->getChild(PropIdxHeight)->get<int32_t>());
            EXPECT_EQ(42, *loadedBinding->getInputs()->getChild(PropIdxSampleCount)->get<int32_t>());

            // confidence test - can set new values
            EXPECT_TRUE(loadedBinding->getInputs()->getChild(PropIdxHeight)->set(43));
            EXPECT_TRUE(loadedBinding->getInputs()->getChild(PropIdxSampleCount)->set(44));
            EXPECT_TRUE(m_logicEngine->update());
            EXPECT_EQ(1u, m_renderBuffer->getWidth()); // did not change
            EXPECT_EQ(43u, m_renderBuffer->getHeight()); // changed before saving and again after loading
            EXPECT_EQ(44u, m_renderBuffer->getSampleCount()); // changed after loading
        }
    }

    TEST_F(ARenderBufferBinding_SerializationLifecycle, DoesNotModifyAnyValueIfNotSetDuringSerializationAndDeserialization)
    {
        {
            EXPECT_TRUE(m_logicEngine->update());
            ASSERT_TRUE(saveToFile("binding.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("binding.bin"));
            const auto loadedBinding = m_logicEngine->findObject<RenderBufferBinding>("binding");
            ASSERT_TRUE(loadedBinding);
            EXPECT_EQ(m_renderBuffer, &loadedBinding->getRenderBuffer());

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(PropCount, loadedBinding->getInputs()->getChildCount());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(PropIdxWidth), loadedBinding->getInputs()->getChild("width"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(PropIdxHeight), loadedBinding->getInputs()->getChild("height"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(PropIdxSampleCount), loadedBinding->getInputs()->getChild("sampleCount"));
            EXPECT_EQ(nullptr, loadedBinding->getOutputs());

            EXPECT_TRUE(m_logicEngine->update());
            EXPECT_EQ(1u, m_renderBuffer->getWidth());
            EXPECT_EQ(2u, m_renderBuffer->getHeight());
            EXPECT_EQ(3u, m_renderBuffer->getSampleCount());
        }
    }
}
