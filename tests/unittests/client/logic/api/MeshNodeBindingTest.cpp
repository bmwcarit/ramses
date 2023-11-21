//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"
#include "RamsesTestUtils.h"
#include "SerializationTestUtils.h"
#include "RamsesObjectResolverMock.h"

#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/Property.h"
#include "impl/logic/MeshNodeBindingImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/TypeData.h"

#include "internal/logic/flatbuffers/generated/MeshNodeBindingGen.h"

namespace ramses::internal
{
    class AMeshNodeBinding : public ALogicEngine
    {
    public:
        AMeshNodeBinding()
        {
            // in order for the tests to be closer to reality, use ramses MeshNode with actual geometry and appearance,
            // the geometry affects some of the values exposed in the binding (namely instanceCount)
            const std::array<ramses::vec3f, 3u> vertexPositionsArray = { ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
            const ramses::ArrayResource* vertexPositions = m_scene->createArrayResource(3u, vertexPositionsArray.data());
            const std::array<uint16_t, 3u> indexArray = { 0, 1, 2 };
            const ramses::ArrayResource* indices = m_scene->createArrayResource(3u, indexArray.data());

            ramses::EffectDescription effectDesc;
            effectDesc.setVertexShader(R"(
                #version 100
                attribute vec3 a_position;
                void main()
                {
                    gl_Position = vec4(a_position, 1.0);
                }
                )");
            effectDesc.setFragmentShader(R"(
                #version 100
                void main(void)
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                }
                )");
            const ramses::Effect* effect = m_scene->createEffect(effectDesc);
            ramses::Appearance* appearance = m_scene->createAppearance(*effect);

            m_geometry = m_scene->createGeometry(*effect);
            const auto positionsInput = effect->findAttributeInput("a_position");
            EXPECT_TRUE(positionsInput.has_value());
            m_geometry->setInputBuffer(*positionsInput, *vertexPositions);
            m_geometry->setIndices(*indices);

            m_meshNodeWithGeometry = m_scene->createMeshNode("meshNode");
            m_meshNodeWithGeometry->setAppearance(*appearance);
            m_meshNodeWithGeometry->setGeometry(*m_geometry);
            EXPECT_EQ(3u, m_meshNodeWithGeometry->getIndexCount());

            m_meshBinding = m_logicEngine->createMeshNodeBinding(*m_meshNodeWithGeometry, "meshBinding");
            EXPECT_EQ(3u, m_meshNodeWithGeometry->getIndexCount());
        }

        bool recreateFromFile(std::string_view filename) override
        {
            const auto geometryId = m_geometry->getSceneObjectId();
            const auto meshNodeId = m_meshNodeWithGeometry->getSceneObjectId();
            const auto bindingId  = m_meshBinding->getSceneObjectId();
            auto res = ALogicEngine::recreateFromFile(filename);
            m_geometry             = m_scene->findObject<Geometry>(geometryId);
            m_meshNodeWithGeometry = m_scene->findObject<MeshNode>(meshNodeId);
            m_meshBinding          = m_logicEngine->findObject(bindingId)->as<MeshNodeBinding>();
            return res && m_geometry != nullptr && m_meshNodeWithGeometry != nullptr && m_meshBinding != nullptr;
        }

    protected:
        ramses::Geometry* m_geometry = nullptr;
        ramses::MeshNode* m_meshNodeWithGeometry = nullptr;
        MeshNodeBinding* m_meshBinding = nullptr;
    };

    TEST_F(AMeshNodeBinding, RefersToGivenRamsesObject)
    {
        EXPECT_EQ(m_meshNodeWithGeometry, &m_meshBinding->getRamsesMeshNode());
        const auto& mbConst = *m_meshBinding;
        EXPECT_EQ(m_meshNodeWithGeometry, &mbConst.getRamsesMeshNode());
        const auto& mbImplConst = m_meshBinding->impl();
        EXPECT_EQ(m_meshNodeWithGeometry, &mbImplConst.getRamsesMeshNode());
    }

    TEST_F(AMeshNodeBinding, HasInputPropertiesAndNoOutputs)
    {
        ASSERT_NE(nullptr, m_meshBinding->getInputs());
        ASSERT_EQ(size_t(MeshNodeBindingImpl::EInputProperty::COUNT), m_meshBinding->getInputs()->getChildCount());
        EXPECT_EQ(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::VertexOffset)), m_meshBinding->getInputs()->getChild("vertexOffset"));
        EXPECT_EQ(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset)), m_meshBinding->getInputs()->getChild("indexOffset"));
        EXPECT_EQ(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount)), m_meshBinding->getInputs()->getChild("indexCount"));
        EXPECT_EQ(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::InstanceCount)), m_meshBinding->getInputs()->getChild("instanceCount"));
        EXPECT_EQ(nullptr, m_meshBinding->getOutputs());
    }

    TEST_F(AMeshNodeBinding, InputPropertiesAreInitializedFromBoundMeshNode)
    {
        m_meshNodeWithGeometry->setStartVertex(42);
        m_meshNodeWithGeometry->setStartIndex(43);
        m_meshNodeWithGeometry->setIndexCount(44);
        m_meshNodeWithGeometry->setInstanceCount(45);
        // create new binding
        const auto binding = m_logicEngine->createMeshNodeBinding(*m_meshNodeWithGeometry);

        EXPECT_EQ(42, *binding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::VertexOffset))->get<int32_t>());
        EXPECT_EQ(43, *binding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset))->get<int32_t>());
        EXPECT_EQ(44, *binding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount))->get<int32_t>());
        EXPECT_EQ(45, *binding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::InstanceCount))->get<int32_t>());
    }

    TEST_F(AMeshNodeBinding, SetsModifiedBoundValuesOnUpdate)
    {
        // initial values
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(0u, m_meshNodeWithGeometry->getStartVertex());
        EXPECT_EQ(0u, m_meshNodeWithGeometry->getStartIndex());
        EXPECT_EQ(3u, m_meshNodeWithGeometry->getIndexCount());
        EXPECT_EQ(1u, m_meshNodeWithGeometry->getInstanceCount());

        EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::VertexOffset))->set(42));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(42u, m_meshNodeWithGeometry->getStartVertex());
        EXPECT_EQ(0u, m_meshNodeWithGeometry->getStartIndex());
        EXPECT_EQ(3u, m_meshNodeWithGeometry->getIndexCount());
        EXPECT_EQ(1u, m_meshNodeWithGeometry->getInstanceCount());

        EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset))->set(43));
        EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount))->set(44));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(42u, m_meshNodeWithGeometry->getStartVertex());
        EXPECT_EQ(43u, m_meshNodeWithGeometry->getStartIndex());
        EXPECT_EQ(44u, m_meshNodeWithGeometry->getIndexCount());
        EXPECT_EQ(1u, m_meshNodeWithGeometry->getInstanceCount());

        EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::InstanceCount))->set(45));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(42u, m_meshNodeWithGeometry->getStartVertex());
        EXPECT_EQ(43u, m_meshNodeWithGeometry->getStartIndex());
        EXPECT_EQ(44u, m_meshNodeWithGeometry->getIndexCount());
        EXPECT_EQ(45u, m_meshNodeWithGeometry->getInstanceCount());
    }

    TEST_F(AMeshNodeBinding, FailsUpdateIfTryingToSetInvalidValue_VertexOffset)
    {
        // mesh vertex offset cannot be negative
        EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::VertexOffset))->set(-1));
        EXPECT_FALSE(m_logicEngine->update());
        expectError("MeshNodeBinding vertex offset cannot be negative", m_meshBinding);
    }

    TEST_F(AMeshNodeBinding, FailsUpdateIfTryingToSetInvalidValue_IndexOffset)
    {
        // mesh index offset cannot be negative
        EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset))->set(-1));
        EXPECT_FALSE(m_logicEngine->update());
        expectError("MeshNodeBinding index offset cannot be negative", m_meshBinding);
    }

    TEST_F(AMeshNodeBinding, FailsUpdateIfTryingToSetInvalidValue_IndexCount)
    {
        // mesh index count cannot be negative
        EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount))->set(-1));
        EXPECT_FALSE(m_logicEngine->update());
        expectError("MeshNodeBinding index count cannot be negative", m_meshBinding);
    }

    TEST_F(AMeshNodeBinding, FailsUpdateIfTryingToSetInvalidValue_InstanceCount)
    {
        // mesh instance count cannot be negative
        EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::InstanceCount))->set(-1));
        EXPECT_FALSE(m_logicEngine->update());
        expectError("MeshNodeBinding instance count cannot be negative", m_meshBinding);
    }

    class AMeshNodeBinding_SerializationLifecycle : public AMeshNodeBinding
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
            UnresolvedMeshNode,
            InvalidBoundObjectType
        };

        std::unique_ptr<MeshNodeBindingImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            {
                auto inputsType = MakeStruct("", {
                    TypeData{"vertexOffset", EPropertyType::Int32},
                    TypeData{"indexOffset", EPropertyType::Int32},
                    (issue == ESerializationIssue::CorruptedInputProperties ? TypeData{"wrong", EPropertyType::Int32} : TypeData{"indexCount", EPropertyType::Int32}),
                    TypeData{"instanceCount", EPropertyType::Int32}
                    });
                auto inputs = std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput);

                SerializationMap serializationMap;
                const auto logicObject = rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    (issue == ESerializationIssue::MissingName ? 0 : m_flatBufferBuilder.CreateString("name")), 1u);
                auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(m_flatBufferBuilder,
                    logicObject,
                    (issue == ESerializationIssue::MissingBoundObject ? 0 : rlogic_serialization::CreateRamsesReference(m_flatBufferBuilder,
                        1u, (issue == ESerializationIssue::InvalidBoundObjectType ? 0 : static_cast<uint32_t>(ramses::ERamsesObjectType::MeshNode)))),
                    (issue == ESerializationIssue::MissingRoot ? 0 : PropertyImpl::Serialize(*inputs, m_flatBufferBuilder, serializationMap)));

                auto fbMeshNodeBinding = rlogic_serialization::CreateMeshNodeBinding(m_flatBufferBuilder, (issue == ESerializationIssue::MissingBase ? 0 : fbRamsesBinding));
                m_flatBufferBuilder.Finish(fbMeshNodeBinding);
            }

            switch (issue)
            {
            case ESerializationIssue::AllValid:
            case ESerializationIssue::InvalidBoundObjectType:
                EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 1u })).WillOnce(::testing::Return(m_meshNodeWithGeometry));
                break;
            case ESerializationIssue::UnresolvedMeshNode:
                EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 1u })).WillOnce(::testing::Return(nullptr));
                break;
            default:
                break;
            }

            DeserializationMap deserializationMap{ m_scene->impl() };
            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::MeshNodeBinding>(m_flatBufferBuilder.GetBufferPointer());
            return MeshNodeBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, deserializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;
        ErrorReporting m_errorReporting;
    };

    TEST_F(AMeshNodeBinding_SerializationLifecycle, CanSerializeWithNoIssue)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(AMeshNodeBinding_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, ReportsSerializationError_MissingBase)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AMeshNodeBinding_SerializationLifecycle::ESerializationIssue::MissingBase));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of MeshNodeBinding from serialized data: missing base class info!");
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, ReportsSerializationError_MissingName)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AMeshNodeBinding_SerializationLifecycle::ESerializationIssue::MissingName));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of MeshNodeBinding from serialized data: missing name and/or ID!");
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, ReportsSerializationError_MissingRoot)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AMeshNodeBinding_SerializationLifecycle::ESerializationIssue::MissingRoot));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of MeshNodeBinding from serialized data: missing root input!");
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, ReportsSerializationError_CorruptedInputProperties)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AMeshNodeBinding_SerializationLifecycle::ESerializationIssue::CorruptedInputProperties));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of MeshNodeBinding from serialized data: corrupted root input!");
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, ReportsSerializationError_MissingBoundObject)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AMeshNodeBinding_SerializationLifecycle::ESerializationIssue::MissingBoundObject));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of MeshNodeBinding from serialized data: missing ramses object reference!");
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, ReportsSerializationError_UnresolvedMeshNode)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AMeshNodeBinding_SerializationLifecycle::ESerializationIssue::UnresolvedMeshNode));
        // error message is generated in resolver which is mocked here
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, ReportsSerializationError_InvalidBoundObjectType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AMeshNodeBinding_SerializationLifecycle::ESerializationIssue::InvalidBoundObjectType));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of MeshNodeBinding from serialized data: loaded object type does not match referenced object type!");
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, KeepsItsPropertiesAfterDeserialization)
    {
        {
            EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset))->set(41));
            EXPECT_TRUE(m_meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::InstanceCount))->set(42));
            EXPECT_TRUE(m_logicEngine->update());
            ASSERT_TRUE(saveToFile("binding.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("binding.bin"));
            const auto loadedBinding = m_logicEngine->findObject<MeshNodeBinding>("meshBinding");
            ASSERT_TRUE(loadedBinding);
            EXPECT_EQ(m_meshNodeWithGeometry, &loadedBinding->getRamsesMeshNode());

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(size_t(MeshNodeBindingImpl::EInputProperty::COUNT), loadedBinding->getInputs()->getChildCount());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::VertexOffset)), loadedBinding->getInputs()->getChild("vertexOffset"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset)), loadedBinding->getInputs()->getChild("indexOffset"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount)), loadedBinding->getInputs()->getChild("indexCount"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::InstanceCount)), loadedBinding->getInputs()->getChild("instanceCount"));
            EXPECT_EQ(nullptr, loadedBinding->getOutputs());

            EXPECT_EQ(41, *loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset))->get<int32_t>());
            EXPECT_EQ(42, *loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::InstanceCount))->get<int32_t>());

            // confidence test - can set new values
            EXPECT_TRUE(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset))->set(43));
            EXPECT_TRUE(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount))->set(44));
            EXPECT_TRUE(m_logicEngine->update());
            EXPECT_EQ(0u, m_meshNodeWithGeometry->getStartVertex()); // did not change
            EXPECT_EQ(43u, m_meshNodeWithGeometry->getStartIndex()); // changed before saving and again after loading
            EXPECT_EQ(44u, m_meshNodeWithGeometry->getIndexCount()); // changed after loading
            EXPECT_EQ(42u, m_meshNodeWithGeometry->getInstanceCount()); // changed before saving
        }
    }

    TEST_F(AMeshNodeBinding_SerializationLifecycle, DoesNotModifyAnyValueIfNotSetDuringSerializationAndDeserialization)
    {
        {
            EXPECT_TRUE(m_logicEngine->update());
            ASSERT_TRUE(saveToFile("binding.bin"));
        }

        {
            ASSERT_TRUE(recreateFromFile("binding.bin"));
            const auto loadedBinding = m_logicEngine->findObject<MeshNodeBinding>("meshBinding");
            ASSERT_TRUE(loadedBinding);
            EXPECT_EQ(m_meshNodeWithGeometry, &loadedBinding->getRamsesMeshNode());

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(size_t(MeshNodeBindingImpl::EInputProperty::COUNT), loadedBinding->getInputs()->getChildCount());
            EXPECT_EQ(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::VertexOffset)), loadedBinding->getInputs()->getChild("vertexOffset"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexOffset)), loadedBinding->getInputs()->getChild("indexOffset"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount)), loadedBinding->getInputs()->getChild("indexCount"));
            EXPECT_EQ(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::InstanceCount)), loadedBinding->getInputs()->getChild("instanceCount"));
            EXPECT_EQ(nullptr, loadedBinding->getOutputs());

            EXPECT_TRUE(m_logicEngine->update());
            EXPECT_EQ(0u, m_meshNodeWithGeometry->getStartVertex());
            EXPECT_EQ(0u, m_meshNodeWithGeometry->getStartIndex());
            EXPECT_EQ(3u, m_meshNodeWithGeometry->getIndexCount());
            EXPECT_EQ(1u, m_meshNodeWithGeometry->getInstanceCount());
        }
    }

    // Note that this is not recommended usage of Ramses binding, modifying the bound object after it is bound
    TEST_F(AMeshNodeBinding, DoesNotModifyIndexCountEvenIfGeometryAssignedAfterBindingIsCreated)
    {
        auto meshNode = m_scene->createMeshNode();
        auto meshBinding = m_logicEngine->createMeshNodeBinding(*meshNode);
        EXPECT_EQ(0u, meshNode->getIndexCount());

        // assign geometry which in ramses automatically updates index count based on its indices
        meshNode->setGeometry(*m_geometry);
        EXPECT_EQ(3u, meshNode->getIndexCount());

        // mesh binding update must not affect that value even though binding's property holds indexCount=0 because it was initialized with it
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(3u, meshNode->getIndexCount());

        // the index count value will be overridden only when explicitly set from binding (or link)
        EXPECT_TRUE(meshBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount))->set(2));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(2u, meshNode->getIndexCount());
    }

    // Note that this is not recommended usage of Ramses binding, modifying the bound object after it is bound
    TEST_F(AMeshNodeBinding, DoesNotModifyIndexCountEvenIfGeometryAssignedAfterBindingIsCreated_SerializedAndDeserialized)
    {
        {
            auto meshNode = m_scene->createMeshNode("mesh01");
            m_logicEngine->createMeshNodeBinding(*meshNode, "bindingWithLateGeometry");
            EXPECT_EQ(0u, meshNode->getIndexCount());

            // assign geometry which in ramses automatically updates index count based on its indices
            meshNode->setGeometry(*m_geometry);
            EXPECT_EQ(3u, meshNode->getIndexCount());

            // mesh binding update must not affect that value even though binding's property holds indexCount=0 because it was initialized with it
            EXPECT_TRUE(m_logicEngine->update());
            EXPECT_EQ(3u, meshNode->getIndexCount());

            ASSERT_TRUE(saveToFile("binding.bin"));
        }

        ASSERT_TRUE(recreateFromFile("binding.bin"));
        auto meshNode = m_scene->findObject<MeshNode>("mesh01");
        ASSERT_TRUE(meshNode != nullptr);
        const auto loadedBinding = m_logicEngine->findObject<MeshNodeBinding>("bindingWithLateGeometry");
        ASSERT_TRUE(loadedBinding);
        EXPECT_EQ(meshNode, &loadedBinding->getRamsesMeshNode());

        // value still unchanged
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(3u, meshNode->getIndexCount());

        // the index count value will be overridden only when explicitly set from binding (or link)
        EXPECT_TRUE(loadedBinding->getInputs()->getChild(size_t(MeshNodeBindingImpl::EInputProperty::IndexCount))->set(2));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(2u, meshNode->getIndexCount());
    }
}
