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
#include "WithTempDirectory.h"

#include "impl/LogicEngineImpl.h"
#include "impl/AnchorPointImpl.h"
#include "impl/PropertyImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "impl/RamsesCameraBindingImpl.h"
#include "generated/AnchorPointGen.h"

#include "ramses-logic/AnchorPoint.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LogicEngine.h"

#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"

namespace rlogic::internal
{
    class AnAnchorPoint : public ALogicEngine
    {
    public:
        AnAnchorPoint() : ALogicEngine{ EFeatureLevel_02 }
        {
        }

    protected:
        RamsesNodeBinding& m_nodeBinding{ *m_logicEngine.createRamsesNodeBinding(*m_node) };
        RamsesCameraBinding& m_cameraBinding{ *m_logicEngine.createRamsesCameraBinding(*m_camera) };
    };

    TEST_F(AnAnchorPoint, HasANameAndIdAfterCreation)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_cameraBinding, "anchor");
        EXPECT_EQ("anchor", anchorPoint.getName());
        EXPECT_EQ(3u, anchorPoint.getId());
    }

    TEST_F(AnAnchorPoint, ReferencesBindings)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_cameraBinding, "anchor");
        EXPECT_EQ(&m_nodeBinding.getRamsesNode(), &anchorPoint.getRamsesNode());
        EXPECT_EQ(&m_cameraBinding.getRamsesCamera(), &anchorPoint.getRamsesCamera());
        EXPECT_EQ(&m_nodeBinding.m_nodeBinding, &anchorPoint.m_anchorPointImpl.getRamsesNodeBinding());
        EXPECT_EQ(&m_cameraBinding.m_cameraBinding, &anchorPoint.m_anchorPointImpl.getRamsesCameraBinding());
    }

    TEST_F(AnAnchorPoint, HasNoInputsAfterCreation)
    {
        auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_cameraBinding, "anchor");
        EXPECT_EQ(nullptr, anchorPoint.getInputs());
    }

    TEST_F(AnAnchorPoint, HasOutputsAfterCreation)
    {
        auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_cameraBinding, "anchor");
        ASSERT_NE(nullptr, anchorPoint.getOutputs());
        ASSERT_EQ(2u, anchorPoint.getOutputs()->getChildCount());
        EXPECT_EQ(anchorPoint.getOutputs()->getChild(0u), anchorPoint.getOutputs()->getChild("viewportCoords"));
        EXPECT_EQ(EPropertyType::Vec2f, anchorPoint.getOutputs()->getChild(0u)->getType());
        EXPECT_EQ(anchorPoint.getOutputs()->getChild(1u), anchorPoint.getOutputs()->getChild("depth"));
        EXPECT_EQ(EPropertyType::Float, anchorPoint.getOutputs()->getChild(1u)->getType());
    }

    TEST_F(AnAnchorPoint, ProducesErrorOnUpdateIfCameraNotInitialized)
    {
        const auto uninitializedCamera = m_scene->createOrthographicCamera();
        m_logicEngine.createAnchorPoint(m_nodeBinding, *m_logicEngine.createRamsesCameraBinding(*uninitializedCamera), "anchor");
        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Failed to retrieve projection matrix from Ramses camera!");
    }

    class AnAnchorPoint_Serialization : public AnAnchorPoint
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            MissingName,
            MissingRootOutput,
            RootOutputNotStruct,
            OutputPropertyInvalid,
            CannotResolveNodeBinding,
            CannotResolveCameraBinding
        };

        std::unique_ptr<AnchorPointImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            {
                const EPropertyType propType = (issue == ESerializationIssue::OutputPropertyInvalid ? EPropertyType::Float : EPropertyType::Vec2f);
                auto outputsType = MakeStruct("", {
                        TypeData{"viewportCoords", propType},
                        TypeData{"depth", EPropertyType::Float},
                    });

                HierarchicalTypeData outputs = (issue == ESerializationIssue::RootOutputNotStruct ? MakeType("", EPropertyType::Bool) : outputsType);
                auto outputsImpl = std::make_unique<PropertyImpl>(std::move(outputs), EPropertySemantics::ScriptOutput);

                auto fbAnchorPoint = rlogic_serialization::CreateAnchorPoint(
                    m_flatBufferBuilder,
                    rlogic_serialization::CreateLogicObject(m_flatBufferBuilder, (issue == ESerializationIssue::MissingName ? 0 : m_flatBufferBuilder.CreateString("name")), 1u, 0u, 0u),
                    m_nodeBinding.getId(),
                    m_cameraBinding.getId(),
                    0, // no inputs
                    (issue == ESerializationIssue::MissingRootOutput ? 0 : PropertyImpl::Serialize(*outputsImpl, m_flatBufferBuilder, m_serializationMap)));
                m_flatBufferBuilder.Finish(fbAnchorPoint);
            }

            if (issue != ESerializationIssue::CannotResolveNodeBinding)
                m_deserializationMap.storeLogicObject(m_nodeBinding.getId(), m_nodeBinding.m_nodeBinding);
            if (issue != ESerializationIssue::CannotResolveCameraBinding)
                m_deserializationMap.storeLogicObject(m_cameraBinding.getId(), m_cameraBinding.m_cameraBinding);

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::AnchorPoint>(m_flatBufferBuilder.GetBufferPointer());
            return AnchorPointImpl::Deserialize(serialized, m_errorReporting, m_deserializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        ErrorReporting m_errorReporting;
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap;
    };

    TEST_F(AnAnchorPoint_Serialization, DeserializesAllData)
    {
        {
            AnchorPointImpl anchor(m_nodeBinding.m_nodeBinding, m_cameraBinding.m_cameraBinding, "name", 1u);
            anchor.createRootProperties();
            (void)AnchorPointImpl::Serialize(anchor, m_flatBufferBuilder, m_serializationMap, m_logicEngine.getFeatureLevel());
        }
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::AnchorPoint>(m_flatBufferBuilder.GetBufferPointer());

        m_deserializationMap.storeLogicObject(m_nodeBinding.getId(), m_nodeBinding.m_nodeBinding);
        m_deserializationMap.storeLogicObject(m_cameraBinding.getId(), m_cameraBinding.m_cameraBinding);

        std::unique_ptr<AnchorPointImpl> deserialized = AnchorPointImpl::Deserialize(serialized, m_errorReporting, m_deserializationMap);
        ASSERT_TRUE(deserialized);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());

        EXPECT_EQ(deserialized->getName(), "name");
        EXPECT_EQ(deserialized->getId(), 1u);
        EXPECT_FALSE(deserialized->getInputs());
        ASSERT_TRUE(deserialized->getOutputs());
        ASSERT_EQ(2u, deserialized->getOutputs()->getChildCount());
        EXPECT_EQ("viewportCoords", deserialized->getOutputs()->getChild(0u)->getName());
        EXPECT_EQ(EPropertyType::Vec2f, deserialized->getOutputs()->getChild(0u)->getType());
        EXPECT_EQ("depth", deserialized->getOutputs()->getChild(1u)->getName());
        EXPECT_EQ(EPropertyType::Float, deserialized->getOutputs()->getChild(1u)->getType());
        EXPECT_EQ(&m_nodeBinding.m_nodeBinding, &deserialized->getRamsesNodeBinding());
        EXPECT_EQ(&m_cameraBinding.m_cameraBinding, &deserialized->getRamsesCameraBinding());
    }

    TEST_F(AnAnchorPoint_Serialization, CanSerializeWithNoIssue)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(AnAnchorPoint_Serialization::ESerializationIssue::AllValid));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_F(AnAnchorPoint_Serialization, ReportsSerializationError_MissingName)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AnAnchorPoint_Serialization::ESerializationIssue::MissingName));
        ASSERT_EQ(2u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LogicObject base from serialized data: missing name!");
        EXPECT_EQ(m_errorReporting.getErrors()[1].message, "Fatal error during loading of AnchorPoint from serialized data: missing name and/or ID!");
    }

    TEST_F(AnAnchorPoint_Serialization, ReportsSerializationError_MissingRootOutput)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AnAnchorPoint_Serialization::ESerializationIssue::MissingRootOutput));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of AnchorPoint from serialized data: missing root output!");
    }

    TEST_F(AnAnchorPoint_Serialization, ReportsSerializationError_RootOutputNotStruct)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AnAnchorPoint_Serialization::ESerializationIssue::RootOutputNotStruct));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of AnchorPoint from serialized data: root output has unexpected type!");
    }

    TEST_F(AnAnchorPoint_Serialization, ReportsSerializationError_PropertyInvalid)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AnAnchorPoint_Serialization::ESerializationIssue::OutputPropertyInvalid));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of AnchorPoint: missing or invalid properties!");
    }

    TEST_F(AnAnchorPoint_Serialization, ReportsSerializationError_UnresolvedNodeBinding)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AnAnchorPoint_Serialization::ESerializationIssue::CannotResolveNodeBinding));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of AnchorPoint: could not resolve NodeBinding and/or CameraBinding!");
    }

    TEST_F(AnAnchorPoint_Serialization, ReportsSerializationError_UnresolvedCameraBinding)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(AnAnchorPoint_Serialization::ESerializationIssue::CannotResolveCameraBinding));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of AnchorPoint: could not resolve NodeBinding and/or CameraBinding!");
    }

    class AnAnchorPoint_Math : public AnAnchorPoint
    {
    protected:
        void SetUp() override
        {
            m_node->setTranslation(1.f, 2.f, 3.f);
            m_node->setRotation(-1.f, -2.f, -3.f);
            m_node->setScaling(1.f, 2.f, 3.f);

            auto nodeTransNode = m_scene->createNode();
            nodeTransNode->setTranslation(1.f, 2.f, -3.f);
            nodeTransNode->addChild(*m_node);

            m_perspCamera.setTranslation(1.f, 2.f, -3.f);
            m_perspCamera.setRotation(-1.f, -2.f, 3.f);
            m_perspCamera.setScaling(1.f, 2.f, -3.f);
            m_perspCamera.setFrustum(75.f, 0.8f, 0.01f, 100.f);
            m_perspCamera.setViewport(10, 20, 30u, 40u);
            ASSERT_EQ(ramses::StatusOK, m_perspCamera.validate());

            m_orthoCamera.setTranslation(1.f, 2.f, -3.f);
            m_orthoCamera.setRotation(-1.f, -2.f, 3.f);
            m_orthoCamera.setScaling(1.f, 2.f, -3.f);
            m_orthoCamera.setFrustum(-0.5f, 0.5f, -1.f, 2.f, 0.01f, 100.f);
            m_orthoCamera.setViewport(10, 20, 30u, 40u);
            ASSERT_EQ(ramses::StatusOK, m_orthoCamera.validate());

            auto camTransNode = m_scene->createNode();
            camTransNode->setTranslation(1.f, 2.f, -3.f);
            camTransNode->addChild(m_perspCamera);
            camTransNode->addChild(m_orthoCamera);
        }

        ramses::PerspectiveCamera& m_perspCamera{ *m_scene->createPerspectiveCamera() };
        ramses::OrthographicCamera& m_orthoCamera{ *m_scene->createOrthographicCamera() };
        RamsesCameraBinding& m_perspCameraBinding{ *m_logicEngine.createRamsesCameraBinding(m_perspCamera) };
        RamsesCameraBinding& m_orthoCameraBinding{ *m_logicEngine.createRamsesCameraBinding(m_orthoCamera) };
    };

    TEST_F(AnAnchorPoint_Math, CalculatesCoords_PerspCamera)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_perspCameraBinding, "anchor");
        EXPECT_TRUE(m_logicEngine.update());
        const auto coords = *anchorPoint.getOutputs()->getChild(0u)->get<vec2f>();
        EXPECT_FLOAT_EQ(17.560308f, coords[0]);
        EXPECT_FLOAT_EQ(19.317562f, coords[1]);
        EXPECT_FLOAT_EQ(0.99509573f, *anchorPoint.getOutputs()->getChild(1u)->get<float>());
    }

    TEST_F(AnAnchorPoint_Math, CalculatesCoords_OrthoCamera)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_orthoCameraBinding, "anchor");
        EXPECT_TRUE(m_logicEngine.update());
        const auto coords = *anchorPoint.getOutputs()->getChild(0u)->get<vec2f>();
        EXPECT_FLOAT_EQ(21.281908f, coords[0]);
        EXPECT_FLOAT_EQ(12.63566f, coords[1]);
        EXPECT_FLOAT_EQ(0.019886762f, *anchorPoint.getOutputs()->getChild(1u)->get<float>());
    }

    class AnAnchorPoint_Dirtiness : public AnAnchorPoint_Math
    {
    protected:
        void SetUp() override
        {
            AnAnchorPoint_Math::SetUp();
            m_logicEngine.enableUpdateReport(true);
        }

        void expectNodeExecuted(const LogicNode& node) const
        {
            const auto report = m_logicEngine.getLastUpdateReport();
            const auto& executedNodes = report.getNodesExecuted();
            const auto it = std::find_if(executedNodes.cbegin(), executedNodes.cend(), [&node](const auto& timedNode) { return timedNode.first == &node; });
            EXPECT_NE(it, executedNodes.cend());
        }

        void expectNodeSkipped(const LogicNode& node) const
        {
            const auto report = m_logicEngine.getLastUpdateReport();
            const auto& skippedNodes = report.getNodesSkippedExecution();
            const auto it = std::find(skippedNodes.cbegin(), skippedNodes.cend(), &node);
            EXPECT_NE(it, skippedNodes.cend());
        }
    };

    TEST_F(AnAnchorPoint_Dirtiness, RecalculatesWhenModelMatrixChanges)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_perspCameraBinding, "anchor");
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);

        m_node->setTranslation(123.f, 231.f, 321.f);
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);
    }

    TEST_F(AnAnchorPoint_Dirtiness, RecalculatesWhenViewMatrixChanges)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_perspCameraBinding, "anchor");
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);

        m_perspCamera.setTranslation(123.f, 231.f, 321.f);
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);
    }

    TEST_F(AnAnchorPoint_Dirtiness, RecalculatesWhenProjectionMatrixChanges)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_perspCameraBinding, "anchor");
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);

        m_perspCamera.setFrustum(10.f, 1.f, 1000.f, 2000.f);
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);
    }

    TEST_F(AnAnchorPoint_Dirtiness, RecalculatesWhenNodeParentTransChanges)
    {
        auto parentNode = m_scene->createNode();
        parentNode->addChild(*m_node);

        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_perspCameraBinding, "anchor");
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);

        parentNode->setTranslation(123.f, 231.f, 321.f);
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);
    }

    TEST_F(AnAnchorPoint_Dirtiness, RecalculatesWhenCameraParentTransChanges)
    {
        auto parentNode = m_scene->createNode();
        parentNode->addChild(m_perspCamera);

        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_perspCameraBinding, "anchor");
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);

        parentNode->setTranslation(123.f, 231.f, 321.f);
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);
    }

    TEST_F(AnAnchorPoint_Dirtiness, RecalculatesWhenViewportChanges)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_perspCameraBinding, "anchor");
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);

        m_perspCamera.setViewport(666, -100, 333u, 444u);
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(anchorPoint);
    }

    TEST_F(AnAnchorPoint_Dirtiness, OutputIsNotDirtyIfNothingChanged)
    {
        const auto& anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBinding, m_perspCameraBinding, "anchor");
        const auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.coords = Type:Vec2f()
                OUT.y = Type:Float()
            end

            function run(IN,OUT)
                OUT.y = IN.coords[2];
            end
        )");

        ASSERT_TRUE(m_logicEngine.link(*anchorPoint.getOutputs()->getChild("viewportCoords"), *script->getInputs()->getChild("coords")));
        EXPECT_TRUE(m_logicEngine.update());
        expectNodeExecuted(*script);

        EXPECT_TRUE(m_logicEngine.update());
        expectNodeSkipped(*script);

        EXPECT_TRUE(m_logicEngine.update());
        expectNodeSkipped(*script);
    }

    // This test is to cover update order of anchor point with unknown dependencies.
    // Anchor point is special in sense it depends on ramses node (via binding), however those depend on other ramses nodes
    // (transformation topology), e.g. node ancestor, which can be affected by another node binding via script for example.
    // Workaround for case of unknown dependency is dual update, which is tested here.
    //
    // Test setup consists of script bound to nodeA and anchor point attached to nodeB. nodeA and nodeB are interdependent
    // inside ramses, unknown to rlogic.
    // There is one test for each order of creation which affects also order of execution, result is expected same after dual update.
    class AnAnchorPoint_UpdateOrder : public AnAnchorPoint_Math
    {
    protected:
        void SetUp() override
        {
            AnAnchorPoint_Math::SetUp();

            m_nodeAnchor.setParent(m_nodeScript);
        }

        ramses::Node& m_nodeAnchor = *m_scene->createNode();
        RamsesNodeBinding& m_nodeBindingAnchor = *m_logicEngine.createRamsesNodeBinding(m_nodeAnchor);

        const std::string_view m_scriptSrc = R"(
            function interface(IN,OUT)
                IN.trans = Type:Vec3f()
                OUT.trans = Type:Vec3f()
            end

            function run(IN,OUT)
                OUT.trans = IN.trans;
            end
        )";
        ramses::Node& m_nodeScript = *m_scene->createNode();
        RamsesNodeBinding& m_nodeBindingScript = *m_logicEngine.createRamsesNodeBinding(m_nodeScript);
    };

    TEST_F(AnAnchorPoint_UpdateOrder, OutputIsCalculatedWithDoubleUpdateIfDependencyUnknown_createAnchorFirst)
    {
        // create anchor then script
        AnchorPoint& m_anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBindingAnchor, m_perspCameraBinding);
        LuaScript& m_passThruScript = *m_logicEngine.createLuaScript(m_scriptSrc);
        m_logicEngine.link(*m_passThruScript.getOutputs()->getChild("trans"), *m_nodeBindingScript.getInputs()->getChild("translation"));

        const vec2f initialCoordsExpected{ -9.3664684f, -6.0138535f };
        const float initialDepthExpected = 0.99510324f;
        EXPECT_TRUE(m_logicEngine.update());
        const auto coordsInitial = *m_anchorPoint.getOutputs()->getChild(0u)->get<vec2f>();
        EXPECT_EQ(initialCoordsExpected, coordsInitial);
        EXPECT_EQ(initialDepthExpected, *m_anchorPoint.getOutputs()->getChild(1u)->get<float>());

        m_passThruScript.getInputs()->getChild("trans")->set(vec3f{ 666.f, 333.f, 111.f });
        const vec2f coordsAfterChangeExpected{ 525.07275f, 136.18787f };
        const float depthAfterChangeExpected = 0.99979484f;

        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.update());

        const auto coordsAfterChange = *m_anchorPoint.getOutputs()->getChild(0u)->get<vec2f>();
        EXPECT_EQ(coordsAfterChangeExpected, coordsAfterChange);
        EXPECT_EQ(depthAfterChangeExpected, *m_anchorPoint.getOutputs()->getChild(1u)->get<float>());
    }

    TEST_F(AnAnchorPoint_UpdateOrder, OutputIsCalculatedWithDoubleUpdateIfDependencyUnknown_createAnchorLast)
    {
        // create script then anchor
        LuaScript& m_passThruScript = *m_logicEngine.createLuaScript(m_scriptSrc);
        m_logicEngine.link(*m_passThruScript.getOutputs()->getChild("trans"), *m_nodeBindingScript.getInputs()->getChild("translation"));
        AnchorPoint& m_anchorPoint = *m_logicEngine.createAnchorPoint(m_nodeBindingAnchor, m_perspCameraBinding);

        const vec2f initialCoordsExpected{ -9.3664684f, -6.0138535f };
        const float initialDepthExpected = 0.99510324f;
        EXPECT_TRUE(m_logicEngine.update());
        const auto coordsInitial = *m_anchorPoint.getOutputs()->getChild(0u)->get<vec2f>();
        EXPECT_EQ(initialCoordsExpected, coordsInitial);
        EXPECT_EQ(initialDepthExpected, *m_anchorPoint.getOutputs()->getChild(1u)->get<float>());

        m_passThruScript.getInputs()->getChild("trans")->set(vec3f{ 666.f, 333.f, 111.f });
        const vec2f coordsAfterChangeExpected{ 525.07275f, 136.18787f };
        const float depthAfterChangeExpected = 0.99979484f;

        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.update());

        const auto coordsAfterChange = *m_anchorPoint.getOutputs()->getChild(0u)->get<vec2f>();
        EXPECT_EQ(coordsAfterChangeExpected, coordsAfterChange);
        EXPECT_EQ(depthAfterChangeExpected, *m_anchorPoint.getOutputs()->getChild(1u)->get<float>());
    }
}
