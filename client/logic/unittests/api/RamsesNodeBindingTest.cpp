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
#include "WithTempDirectory.h"
#include "LogTestUtils.h"

#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LogicEngine.h"

#include "impl/RamsesNodeBindingImpl.h"
#include "impl/PropertyImpl.h"
#include "impl/LogicEngineImpl.h"

#include "ramses-client-api/Node.h"
#include "ramses-client-api/MeshNode.h"

#include "generated/RamsesNodeBindingGen.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses::internal
{
    class ARamsesNodeBinding : public ALogicEngine
    {
    protected:
        static void ExpectDefaultValues(ramses::Node& node, ENodePropertyStaticIndex prop)
        {
            switch (prop)
            {
            case ENodePropertyStaticIndex::Rotation:
                ExpectValues(node, ENodePropertyStaticIndex::Rotation, vec3f{ 0.0f, 0.0f, 0.0f });
                break;
            case ENodePropertyStaticIndex::Translation:
                ExpectValues(node, ENodePropertyStaticIndex::Translation, vec3f{ 0.0f, 0.0f, 0.0f });
                break;
            case ENodePropertyStaticIndex::Scaling:
                ExpectValues(node, ENodePropertyStaticIndex::Scaling, vec3f{ 1.0f, 1.0f, 1.0f });
                break;
            // Enabled and Visibility bind to a single 3-state node flag
            case ENodePropertyStaticIndex::Visibility:
            case ENodePropertyStaticIndex::Enabled:
                EXPECT_EQ(node.getVisibility(), ramses::EVisibilityMode::Visible);
                break;
            }
        }

        static void ExpectDefaultValues(ramses::Node& node)
        {
            ExpectDefaultValues(node, ENodePropertyStaticIndex::Translation);
            ExpectDefaultValues(node, ENodePropertyStaticIndex::Rotation);
            ExpectDefaultValues(node, ENodePropertyStaticIndex::Scaling);
            ExpectDefaultValues(node, ENodePropertyStaticIndex::Visibility);
            ExpectDefaultValues(node, ENodePropertyStaticIndex::Enabled);
        }

        static void ExpectValues(ramses::Node& node, ENodePropertyStaticIndex prop, vec3f expectedValues)
        {
            vec3f values = {0.0f, 0.0f, 0.0f};
            if (prop == ENodePropertyStaticIndex::Rotation)
            {
                node.getRotation(values);
            }
            if (prop == ENodePropertyStaticIndex::Translation)
            {
                node.getTranslation(values);
            }
            if (prop == ENodePropertyStaticIndex::Scaling)
            {
                node.getScaling(values);
            }
            EXPECT_FLOAT_EQ(values[0], expectedValues[0]);
            EXPECT_FLOAT_EQ(values[1], expectedValues[1]);
            EXPECT_FLOAT_EQ(values[2], expectedValues[2]);
        }

        static void ExpectQuat(ramses::Node& node, const vec4f& expectedValues)
        {
            quat quat;
            node.getRotation(quat);
            EXPECT_FLOAT_EQ(quat.x, expectedValues[0]);
            EXPECT_FLOAT_EQ(quat.y, expectedValues[1]);
            EXPECT_FLOAT_EQ(quat.z, expectedValues[2]);
            EXPECT_FLOAT_EQ(quat.w, expectedValues[3]);
        }
    };

    TEST_F(ARamsesNodeBinding, KeepsNameProvidedDuringConstruction)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        EXPECT_EQ("NodeBinding", nodeBinding.getName());
    }

    TEST_F(ARamsesNodeBinding, KeepsIdProvidedDuringConstruction)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        EXPECT_EQ(nodeBinding.getId(), 1u);
    }

    TEST_F(ARamsesNodeBinding, ReturnsNullptrForOutputs)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "");
        EXPECT_EQ(nullptr, nodeBinding.getOutputs());
    }

    TEST_F(ARamsesNodeBinding, ProvidesAccessToAllNodePropertiesInItsInputs)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "");

        auto inputs = nodeBinding.getInputs();
        ASSERT_NE(nullptr, inputs);
        EXPECT_EQ(5u, inputs->getChildCount());

        auto rotation = inputs->getChild("rotation");
        auto scaling = inputs->getChild("scaling");
        auto translation = inputs->getChild("translation");
        auto visibility = inputs->getChild("visibility");

        // Test that internal indices match properties resolved by name
        EXPECT_EQ(rotation, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Rotation)));
        EXPECT_EQ(scaling, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Scaling)));
        EXPECT_EQ(translation, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Translation)));
        EXPECT_EQ(visibility, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Visibility)));

        ASSERT_NE(nullptr, rotation);
        EXPECT_EQ(EPropertyType::Vec3f, rotation->getType());
        EXPECT_EQ(0u, rotation->getChildCount());

        ASSERT_NE(nullptr, scaling);
        EXPECT_EQ(EPropertyType::Vec3f, scaling->getType());
        EXPECT_EQ(0u, scaling->getChildCount());

        ASSERT_NE(nullptr, translation);
        EXPECT_EQ(EPropertyType::Vec3f, translation->getType());
        EXPECT_EQ(0u, translation->getChildCount());

        ASSERT_NE(nullptr, visibility);
        EXPECT_EQ(EPropertyType::Bool, visibility->getType());
        EXPECT_EQ(0u, visibility->getChildCount());

        auto enabled = inputs->getChild("enabled");
        ASSERT_NE(nullptr, enabled);
        EXPECT_EQ(enabled, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Enabled)));
        EXPECT_EQ(EPropertyType::Bool, enabled->getType());
        EXPECT_EQ(0u, enabled->getChildCount());
    }

    TEST_F(ARamsesNodeBinding, InitializesInputPropertiesToMatchRamsesDefaultValues)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "");

        auto inputs = nodeBinding.getInputs();
        ASSERT_NE(nullptr, inputs);
        EXPECT_EQ(5u, inputs->getChildCount());

        vec3f zeroes;
        vec3f ones;

        //Check that the default values we assume are indeed the ones in ramses
        m_node->getRotation(zeroes);
        EXPECT_EQ(zeroes, vec3f(0.f, 0.f, 0.f));

        EXPECT_EQ(ramses::ERotationType::Euler_XYZ, m_node->getRotationType());
        m_node->getTranslation(zeroes);
        EXPECT_EQ(zeroes, vec3f(0.f, 0.f, 0.f));
        m_node->getScaling(ones);
        EXPECT_EQ(ones, vec3f(1.f, 1.f, 1.f));
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Visible);

        EXPECT_EQ(zeroes, inputs->getChild("rotation")->get<vec3f>());
        EXPECT_EQ(zeroes, inputs->getChild("translation")->get<vec3f>());
        EXPECT_EQ(ones, inputs->getChild("scaling")->get<vec3f>());
        EXPECT_EQ(true, *inputs->getChild("visibility")->get<bool>());
        EXPECT_EQ(true, *inputs->getChild("enabled")->get<bool>());
    }

    TEST_F(ARamsesNodeBinding, MarksInputsAsBindingInputs)
    {
        auto*      nodeBinding     = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        auto       inputs     = nodeBinding->getInputs();
        const auto inputCount = inputs->getChildCount();
        for (size_t i = 0; i < inputCount; ++i)
        {
            EXPECT_EQ(EPropertySemantics::BindingInput, inputs->getChild(i)->m_impl->getPropertySemantics());
        }
    }

    TEST_F(ARamsesNodeBinding, ReturnsNodePropertiesForInputsConst)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "");
        const auto inputs = nodeBinding.getInputs();
        ASSERT_NE(nullptr, inputs);
        EXPECT_EQ(5u, inputs->getChildCount());

        auto rotation = inputs->getChild("rotation");
        auto scaling = inputs->getChild("scaling");
        auto translation = inputs->getChild("translation");
        auto visibility = inputs->getChild("visibility");

        ASSERT_NE(nullptr, rotation);
        EXPECT_EQ(EPropertyType::Vec3f, rotation->getType());
        EXPECT_EQ(0u, rotation->getChildCount());

        ASSERT_NE(nullptr, scaling);
        EXPECT_EQ(EPropertyType::Vec3f, scaling->getType());
        EXPECT_EQ(0u, scaling->getChildCount());

        ASSERT_NE(nullptr, translation);
        EXPECT_EQ(EPropertyType::Vec3f, translation->getType());
        EXPECT_EQ(0u, translation->getChildCount());

        ASSERT_NE(nullptr, visibility);
        EXPECT_EQ(EPropertyType::Bool, visibility->getType());
        EXPECT_EQ(0u, visibility->getChildCount());

        auto enabled = inputs->getChild("enabled");
        ASSERT_NE(nullptr, enabled);
        EXPECT_EQ(EPropertyType::Bool, enabled->getType());
        EXPECT_EQ(0u, enabled->getChildCount());
    }

    TEST_F(ARamsesNodeBinding, ReturnsBoundRamsesNode)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "");
        EXPECT_EQ(m_node, &nodeBinding.getRamsesNode());
    }

    TEST_F(ARamsesNodeBinding, DoesNotModifyRamsesWithoutUpdateBeingCalled)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "");

        auto inputs = nodeBinding.getInputs();
        inputs->getChild("rotation")->set<vec3f>(vec3f{ 0.1f, 0.2f, 0.3f });
        inputs->getChild("scaling")->set<vec3f>(vec3f{ 1.1f, 1.2f, 1.3f });
        inputs->getChild("translation")->set<vec3f>(vec3f{ 2.1f, 2.2f, 2.3f });
        inputs->getChild("visibility")->set<bool>(false);
        inputs->getChild("enabled")->set<bool>(false);

        ExpectDefaultValues(*m_node);
    }

    // This test is a bit too big, but splitting it creates a lot of test code duplication... Better keep it like this, it documents behavior quite well
    TEST_F(ARamsesNodeBinding, ModifiesRamsesOnUpdate_OnlyAfterExplicitlyAssignedToInputs)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "");

        nodeBinding.m_nodeBinding.update();

        ExpectDefaultValues(*m_node);

        auto inputs = nodeBinding.getInputs();
        inputs->getChild("rotation")->set<vec3f>(vec3f{ 0.1f, 0.2f, 0.3f });

        // Updte not called yet -> still default values
        ExpectDefaultValues(*m_node);

        nodeBinding.m_nodeBinding.update();
        // Only propagated rotation, the others still have default values
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 0.1f, 0.2f, 0.3f });
        ExpectDefaultValues(*m_node, ENodePropertyStaticIndex::Translation);
        ExpectDefaultValues(*m_node, ENodePropertyStaticIndex::Scaling);
        ExpectDefaultValues(*m_node, ENodePropertyStaticIndex::Visibility);
        ExpectDefaultValues(*m_node, ENodePropertyStaticIndex::Enabled);

        // Set and test all properties
        inputs->getChild("rotation")->set<vec3f>(vec3f{ 42.1f, 42.2f, 42.3f });
        inputs->getChild("scaling")->set<vec3f>(vec3f{ 1.1f, 1.2f, 1.3f });
        inputs->getChild("translation")->set<vec3f>(vec3f{ 2.1f, 2.2f, 2.3f });
        inputs->getChild("visibility")->set<bool>(true);
        inputs->getChild("enabled")->set<bool>(false);
        nodeBinding.m_nodeBinding.update();

        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 42.1f, 42.2f, 42.3f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, vec3f{ 1.1f, 1.2f, 1.3f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, vec3f{ 2.1f, 2.2f, 2.3f });
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Off);
    }

    TEST_F(ARamsesNodeBinding, PropagatesItsInputsToRamsesNodeOnUpdate)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");

        auto inputs = nodeBinding.getInputs();
        inputs->getChild("rotation")->set<vec3f>(vec3f{0.1f, 0.2f, 0.3f});
        inputs->getChild("scaling")->set<vec3f>(vec3f{1.1f, 1.2f, 1.3f});
        inputs->getChild("translation")->set<vec3f>(vec3f{2.1f, 2.2f, 2.3f});
        inputs->getChild("visibility")->set<bool>(false);
        inputs->getChild("enabled")->set<bool>(true);

        nodeBinding.m_nodeBinding.update();

        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 0.1f, 0.2f, 0.3f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, vec3f{ 1.1f, 1.2f, 1.3f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, vec3f{ 2.1f, 2.2f, 2.3f });
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Invisible);
    }

    TEST_F(ARamsesNodeBinding, PropagatesItsInputsToRamsesNodeOnUpdate_WithLinksInsteadOfSetCall)
    {
        const std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                OUT.rotation = Type:Vec3f()
                OUT.visibility = Type:Bool()
            end
            function run(IN,OUT)
                OUT.rotation = {1, 2, 3}
                OUT.visibility = false
            end
        )";

        LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);

        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");

        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("rotation"), *nodeBinding.getInputs()->getChild("rotation")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("visibility"), *nodeBinding.getInputs()->getChild("visibility")));

        // Links have no effect before update() explicitly called
        ExpectDefaultValues(*m_node);

        m_logicEngine.update();

        // Linked values got updates, not-linked values were not modified
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 1.f, 2.f, 3.f });
        ExpectDefaultValues(*m_node, ENodePropertyStaticIndex::Scaling);
        ExpectDefaultValues(*m_node, ENodePropertyStaticIndex::Translation);
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Invisible);
    }

    TEST_F(ARamsesNodeBinding, DoesNotOverrideExistingValuesAfterRamsesNodeIsAssignedToBinding)
    {
        m_node->setVisibility(ramses::EVisibilityMode::Off);
        m_node->setRotation({0.1f, 0.2f, 0.3f}, ramses::ERotationType::Euler_ZYX);
        m_node->setScaling({1.1f, 1.2f, 1.3f});
        m_node->setTranslation({2.1f, 2.2f, 2.3f});

        m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");

        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 0.1f, 0.2f, 0.3f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, vec3f{ 1.1f, 1.2f, 1.3f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, vec3f{ 2.1f, 2.2f, 2.3f });
    }

    TEST_F(ARamsesNodeBinding, DoesNotOverrideExistingQuaternionAfterRamsesNodeIsAssignedToBinding)
    {
        quat quaternion{0.5f, 0.5f, -0.5f, 0.5f};
        m_node->setRotation(quaternion);

        m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Quaternion, "NodeBinding");

        quat qOut;
        m_node->getRotation(qOut);
        EXPECT_EQ(quaternion, qOut);
    }

    TEST_F(ARamsesNodeBinding, InitializesVisibilityPropertiesFromRamsesNode)
    {
        // visible
        m_node->setVisibility(ramses::EVisibilityMode::Visible);
        RamsesNodeBinding* nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        EXPECT_TRUE(*nodeBinding->getInputs()->getChild("visibility")->get<bool>());
        EXPECT_TRUE(*nodeBinding->getInputs()->getChild("enabled")->get<bool>());
        m_logicEngine.destroy(*nodeBinding);

        // invisible
        m_node->setVisibility(ramses::EVisibilityMode::Invisible);
        nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        EXPECT_FALSE(*nodeBinding->getInputs()->getChild("visibility")->get<bool>());
        EXPECT_TRUE(*nodeBinding->getInputs()->getChild("enabled")->get<bool>());
        m_logicEngine.destroy(*nodeBinding);

        // off
        m_node->setVisibility(ramses::EVisibilityMode::Off);
        nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        EXPECT_FALSE(*nodeBinding->getInputs()->getChild("visibility")->get<bool>());
        EXPECT_FALSE(*nodeBinding->getInputs()->getChild("enabled")->get<bool>());
        m_logicEngine.destroy(*nodeBinding);
    }

    TEST_F(ARamsesNodeBinding, ResolvesVisibilityModeFromProperties)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        auto inputs = nodeBinding.getInputs();

        // visible
        inputs->getChild("visibility")->set<bool>(true);
        inputs->getChild("enabled")->set<bool>(true);
        nodeBinding.m_nodeBinding.update();
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Visible);

        // invisible
        inputs->getChild("visibility")->set<bool>(false);
        inputs->getChild("enabled")->set<bool>(true);
        nodeBinding.m_nodeBinding.update();
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Invisible);

        // off with visible (off overrides visibility)
        inputs->getChild("visibility")->set<bool>(true);
        inputs->getChild("enabled")->set<bool>(false);
        nodeBinding.m_nodeBinding.update();
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Off);

        // off with invisible (off overrides visibility)
        inputs->getChild("visibility")->set<bool>(false);
        inputs->getChild("enabled")->set<bool>(false);
        nodeBinding.m_nodeBinding.update();
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Off);
    }

    class ARamsesNodeBinding_RotationTypes : public ARamsesNodeBinding
    {
    };

    TEST_F(ARamsesNodeBinding_RotationTypes, HasRightHandedEulerXYZRotationConventionByDefault)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node);

        EXPECT_EQ(ramses::ERotationType::Euler_XYZ, nodeBinding.getRotationType());
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, AppliesAllEulerConventions)
    {
        std::vector<ramses::ERotationType> enumValues =
        {
            ramses::ERotationType::Euler_XYZ,
            ramses::ERotationType::Euler_XZY,
            ramses::ERotationType::Euler_YXZ,
            ramses::ERotationType::Euler_YZX,
            ramses::ERotationType::Euler_ZXY,
            ramses::ERotationType::Euler_ZYX,
            ramses::ERotationType::Euler_XYX,
            ramses::ERotationType::Euler_XZX,
            ramses::ERotationType::Euler_YXY,
            ramses::ERotationType::Euler_YZY,
            ramses::ERotationType::Euler_ZXZ,
            ramses::ERotationType::Euler_ZYZ,
        };

        for (const auto& rotationType : enumValues)
        {
            m_node->setRotation({0.f, 0.f, 0.f}, rotationType);

            RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, rotationType);
            nodeBinding.getInputs()->getChild("rotation")->set<vec3f>({ 30, 45, 60 });

            m_logicEngine.update();

            vec3f rotValues;
            m_node->getRotation(rotValues);

            EXPECT_EQ(rotationType, m_node->getRotationType());
            EXPECT_EQ(rotValues, vec3f(30.f, 45.f, 60.f));
        }
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, TakesOverRotationConventionFromRamsesNode_WhenEnumMatches)
    {
        const std::vector<ramses::ERotationType> enumValues =
        {
            ramses::ERotationType::Euler_XYZ,
            ramses::ERotationType::Euler_XZY,
            ramses::ERotationType::Euler_YXZ,
            ramses::ERotationType::Euler_YZX,
            ramses::ERotationType::Euler_ZXY,
            ramses::ERotationType::Euler_ZYX,
            ramses::ERotationType::Euler_XYX,
            ramses::ERotationType::Euler_XZX,
            ramses::ERotationType::Euler_YXY,
            ramses::ERotationType::Euler_YZY,
            ramses::ERotationType::Euler_ZXZ,
            ramses::ERotationType::Euler_ZYZ,
        };

        for (const auto& rotationType : enumValues)
        {
            m_node->setRotation({1, 2, 3}, rotationType);
            ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, rotationType, "NodeBinding");

            EXPECT_EQ(*binding->getInputs()->getChild("rotation")->get<vec3f>(), vec3f(1.f, 2.f, 3.f));
            EXPECT_EQ(binding->getRotationType(), rotationType);

            ASSERT_TRUE(m_logicEngine.destroy(*binding));
        }
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, TakesOverQuaternionFromRamsesNode_WhenEnumMatches)
    {
        m_node->setRotation(quat(1.f, 2.f, 3.f, 4.f));
        ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Quaternion, "NodeBinding");

        EXPECT_EQ(*binding->getInputs()->getChild("rotation")->get<vec4f>(), vec4f(2.f, 3.f, 4.f, 1.f));
        EXPECT_EQ(binding->getRotationType(), ramses::ERotationType::Quaternion);

        ASSERT_TRUE(m_logicEngine.destroy(*binding));
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, InitializesRotationWithZeroAndPrintsWarning_WhenConventionEnumsDontMatch)
    {
        const std::vector<std::pair<ramses::ERotationType, ramses::ERotationType>> mismatchedEnumPairs =
        {
            {ramses::ERotationType::Euler_ZYX, ramses::ERotationType::Euler_XYZ},
            {ramses::ERotationType::Euler_YZX, ramses::ERotationType::Euler_ZYX},
            {ramses::ERotationType::Euler_ZXY, ramses::ERotationType::Euler_YZX},
            {ramses::ERotationType::Euler_XZY, ramses::ERotationType::Euler_ZXY},
            {ramses::ERotationType::Euler_YXZ, ramses::ERotationType::Euler_XZY},
            {ramses::ERotationType::Euler_XYZ, ramses::ERotationType::Euler_YXZ},
        };

        std::string warningMessage;
        ELogLevel messageType;
        ScopedLogContextLevel scopedLogs(ELogLevel::Warn, [&warningMessage, &messageType](ELogLevel msgType, std::string_view message) {
            warningMessage = message;
            messageType = msgType;
            });

        for (const auto& [logicEnum, ramsesEnum] : mismatchedEnumPairs)
        {
            m_node->setRotation({1, 2, 3}, ramsesEnum);
            ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, logicEnum, "NodeBinding");

            EXPECT_EQ(messageType, ELogLevel::Warn);
            EXPECT_EQ(warningMessage, fmt::format("Initial rotation values for RamsesNodeBinding '{}' will not be imported from bound Ramses node due to mismatching rotation type.", binding->m_impl.getIdentificationString()));

            EXPECT_EQ(*binding->getInputs()->getChild("rotation")->get<vec3f>(), vec3f(0.f, 0.f, 0.f));
            EXPECT_EQ(binding->getRotationType(), logicEnum);

            ASSERT_TRUE(m_logicEngine.destroy(*binding));

            messageType = ELogLevel::Off;
            warningMessage = "";
        }
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, InitializesQuaternionWithIdentityAndPrintsWarning_WhenConventionEnumsDontMatch)
    {
        std::string warningMessage;
        ELogLevel messageType;
        ScopedLogContextLevel scopedLogs(ELogLevel::Warn, [&warningMessage, &messageType](ELogLevel msgType, std::string_view message) {
            warningMessage = message;
            messageType = msgType;
            });

        m_node->setRotation({1, 2, 3});
        ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Quaternion, "NodeBinding");

        EXPECT_EQ(messageType, ELogLevel::Warn);
        EXPECT_EQ(warningMessage, fmt::format("Initial rotation values for RamsesNodeBinding '{}' will not be imported from bound Ramses node due to mismatching rotation type. Expected Quaternion, got Euler.", binding->m_impl.getIdentificationString()));

        EXPECT_EQ(*binding->getInputs()->getChild("rotation")->get<vec4f>(), vec4f(0.f, 0.f, 0.f, 1.f));
        EXPECT_EQ(binding->getRotationType(), ramses::ERotationType::Quaternion);

        ASSERT_TRUE(m_logicEngine.destroy(*binding));

        messageType = ELogLevel::Off;
        warningMessage = "";
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, PrintsNoWarning_WhenConventionEnumsDontMatch_InSpecialCaseWhereRamsesRotationValuesAreZero)
    {
        ScopedLogContextLevel scopedLogs(ELogLevel::Warn, [](ELogLevel /*unused*/, std::string_view /*unused*/) {
            FAIL() << "Should not cause any warnings!";
            });

        m_node->setRotation({0.f, 0.f, 0.f}, ramses::ERotationType::Euler_XYX);
        ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");

        EXPECT_EQ(*binding->getInputs()->getChild("rotation")->get<vec3f>(), vec3f(0.f, 0.f, 0.f));
        EXPECT_EQ(binding->getRotationType(), ramses::ERotationType::Euler_XYZ);

        ASSERT_TRUE(m_logicEngine.destroy(*binding));
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, PrintsNoWarning_WhenNoQuaternion_InSpecialCaseWhereRamsesRotationValuesAreZero)
    {
        ScopedLogContextLevel scopedLogs(ELogLevel::Warn, [](ELogLevel /*unused*/, std::string_view /*unused*/) {
            FAIL() << "Should not cause any warnings!";
            });

        m_node->setRotation({0.f, 0.f, 0.f}, ramses::ERotationType::Euler_XYX);
        ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Quaternion, "NodeBinding");

        EXPECT_EQ(*binding->getInputs()->getChild("rotation")->get<vec4f>(), vec4f(0.f, 0.f, 0.f, 1.f));
        EXPECT_EQ(binding->getRotationType(), ramses::ERotationType::Quaternion);

        ASSERT_TRUE(m_logicEngine.destroy(*binding));
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, InitializesQuaternionAsVec4f)
    {
        ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Quaternion);

        EXPECT_EQ(5u, binding->getInputs()->getChildCount());
        EXPECT_EQ(EPropertyType::Vec4f, binding->getInputs()->getChild("rotation")->getType());

        // Test other inputs to check they are not affected
        EXPECT_EQ(EPropertyType::Vec3f, binding->getInputs()->getChild("translation")->getType());
        EXPECT_EQ(EPropertyType::Vec3f, binding->getInputs()->getChild("scaling")->getType());
        EXPECT_EQ(EPropertyType::Bool, binding->getInputs()->getChild("visibility")->getType());
        EXPECT_EQ(EPropertyType::Bool, binding->getInputs()->getChild("enabled")->getType());
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, InitializesQuaternionValues_WhichResultsToNoRotationInEuler)
    {
        ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Quaternion);
        ramses::Property* quatInput = binding->getInputs()->getChild("rotation");

        ASSERT_EQ(quatInput->getType(), EPropertyType::Vec4f);
        EXPECT_EQ(*quatInput->get<vec4f>(), vec4f(0.f, 0.f, 0.f, 1.f));

        EXPECT_TRUE(m_logicEngine.update());

        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 0.0f, 0.0f, 0.0f });
    }

    TEST_F(ARamsesNodeBinding_RotationTypes, QuaternionValuesAreReturned)
    {
        ramses::RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Quaternion);
        ramses::Property* quatInput = binding->getInputs()->getChild("rotation");

        quatInput->set<vec4f>({0.5f, 0, 0, 0.8660254f});
        EXPECT_TRUE(m_logicEngine.update());
        ExpectQuat(*m_node, vec4f{0.5f, 0.0f, 0.0f, 0.8660254f});

        quatInput->set<vec4f>({ 0, 0.258819f, 0, 0.9659258f });
        EXPECT_TRUE(m_logicEngine.update());
        ExpectQuat(*m_node, vec4f{0, 0.258819f, 0, 0.9659258f});

        quatInput->set<vec4f>({ 0, 0, 0.7071068f, 0.7071068f });
        EXPECT_TRUE(m_logicEngine.update());
        ExpectQuat(*m_node, vec4f{0, 0, 0.7071068f, 0.7071068f});
    }

    // This is a confidence test that checks that no matter which rotation type is used, the final result (matrix)
    // is always the same as if each rotation/axis was put in its own node with the order given by the node hierarchy
    TEST_F(ARamsesNodeBinding_RotationTypes, Confidence_ComplexEulerRotations_ProduceTheSameRotationResult_AsWithSeparateRamsesNodesWithSingleAxisRotations)
    {
        std::array<ramses::Node*, 3> threeNodes = {
            m_scene->createNode(),
            m_scene->createNode(),
            m_scene->createNode(),
        };

        threeNodes[2]->setParent(*threeNodes[1]);
        threeNodes[1]->setParent(*threeNodes[0]);

        const std::vector<std::tuple<ramses::ERotationType, vec3i>> combinationsToTest =
        {
            {ramses::ERotationType::Euler_ZYX, {2, 1, 0}},
            {ramses::ERotationType::Euler_YZX, {1, 2, 0}},
            {ramses::ERotationType::Euler_ZXY, {2, 0, 1}},
            {ramses::ERotationType::Euler_XZY, {0, 2, 1}},
            {ramses::ERotationType::Euler_YXZ, {1, 0, 2}},
            {ramses::ERotationType::Euler_XYZ, {0, 1, 2}},
        };

        for (const auto& [rotationType, axesOrdering] : combinationsToTest)
        {
            // Set the rotations for all 3 nodes according to the rotationType, use the same rotation values always
            for (glm::length_t i = 0; i < 3; ++i)
            {
                const auto axis = axesOrdering[i];
                const auto nodeIndex = 2-i;
                if (axis == 0)
                {
                    threeNodes[nodeIndex]->setRotation({10, 0, 0}, ramses::ERotationType::Euler_XYZ);
                }
                else if (axis == 1)
                {
                    threeNodes[nodeIndex]->setRotation({0, 20, 0}, ramses::ERotationType::Euler_XYZ);
                }
                else
                {
                    threeNodes[nodeIndex]->setRotation({0, 0, 30}, ramses::ERotationType::Euler_XYZ);
                }
            }

            m_node->setRotation({0, 0, 0}, rotationType);
            RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, rotationType);
            binding->getInputs()->getChild("rotation")->set<vec3f>({10, 20, 30});
            EXPECT_TRUE(m_logicEngine.update());

            matrix44f matFromLogic;
            matrix44f matFromRamses;

            m_node->getModelMatrix(matFromLogic);
            threeNodes[2]->getModelMatrix(matFromRamses);

            for (glm::length_t i = 0u; i < 4; i++)
            {
                for (glm::length_t k = 0u; k < 4; k++)
                {
                    EXPECT_NEAR(matFromLogic[i][k], matFromRamses[i][k], 1.0e-6f);
                }
            }

            m_logicEngine.destroy(*binding);
        }
    }

    // This fixture only contains serialization unit tests, for higher order tests see `ARamsesNodeBinding_SerializationWithFile`
    class ARamsesNodeBinding_SerializationLifecycle : public ARamsesNodeBinding
    {
    protected:
        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils{ m_flatBufferBuilder };
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;
        ErrorReporting m_errorReporting;
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap;
    };

    // More unit tests with inputs/outputs declared in LogicNode (base class) serialization tests
    TEST_F(ARamsesNodeBinding_SerializationLifecycle, RemembersBaseClassData)
    {
        // Serialize
        {
            RamsesNodeBindingImpl binding(*m_node, ramses::ERotationType::Euler_XYZ, "name", 1u);
            binding.createRootProperties();
            (void)RamsesNodeBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedBinding.base());
        ASSERT_TRUE(serializedBinding.base()->base());
        ASSERT_TRUE(serializedBinding.base()->base()->name());
        EXPECT_EQ(serializedBinding.base()->base()->name()->string_view(), "name");
        EXPECT_EQ(serializedBinding.base()->base()->id(), 1u);

        ASSERT_TRUE(serializedBinding.base()->rootInput());
        EXPECT_EQ(serializedBinding.base()->rootInput()->rootType(), rlogic_serialization::EPropertyRootType::Struct);
        ASSERT_TRUE(serializedBinding.base()->rootInput()->children());
        EXPECT_EQ(serializedBinding.base()->rootInput()->children()->size(), 5u);

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("name"), m_node->getSceneObjectId())).WillOnce(::testing::Return(m_node));
            std::unique_ptr<RamsesNodeBindingImpl> deserializedBinding = RamsesNodeBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(deserializedBinding->getName(), "name");
            EXPECT_EQ(deserializedBinding->getId(), 1u);
            EXPECT_EQ(deserializedBinding->getInputs()->getType(), EPropertyType::Struct);
            EXPECT_EQ(deserializedBinding->getInputs()->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            EXPECT_EQ(deserializedBinding->getInputs()->getName(), "");
            EXPECT_EQ(deserializedBinding->getInputs()->getChildCount(), 5u);
        }
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, RemembersRamsesNodeId)
    {
        // Serialize
        {
            RamsesNodeBindingImpl binding(*m_node, ramses::ERotationType::Euler_XYZ, "node", 1u);
            binding.createRootProperties();
            (void)RamsesNodeBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectId(), m_node->getSceneObjectId().getValue());
        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectType(), static_cast<uint32_t>(ramses::ERamsesObjectType::Node));

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("node"), m_node->getSceneObjectId())).WillOnce(::testing::Return(m_node));
            std::unique_ptr<RamsesNodeBindingImpl> deserializedBinding = RamsesNodeBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(&deserializedBinding->getRamsesNode(), m_node);
        }
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, DoesNotOverwriteRamsesValuesAfterLoad)
    {
        // Serialize
        {
            RamsesNodeBindingImpl binding(*m_node, ramses::ERotationType::Euler_XYZ, "node", 1u);
            binding.createRootProperties();
            // Set non-standard values. These will not be used after deserialization, instead the binding
            // will re-load the values from ramses
            binding.getInputs()->getChild("rotation")->set<vec3f>({100, 200, 300});
            binding.update();
            (void)RamsesNodeBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap);
        }

        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());

        // Deserialize
        {
            // Set values different than the ones during serialization so that we can check after
            // deserialization they were not touched
            m_node->setRotation({11, 12, 13});

            EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("node"), m_node->getSceneObjectId())).WillOnce(::testing::Return(m_node));
            std::unique_ptr<RamsesNodeBindingImpl> deserializedBinding = RamsesNodeBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            EXPECT_EQ(&deserializedBinding->getRamsesNode(), m_node);

            deserializedBinding->update();
            vec3f rotation;
            m_node->getRotation(rotation);
            EXPECT_FLOAT_EQ(rotation[0], 11);
            EXPECT_FLOAT_EQ(rotation[1], 12);
            EXPECT_FLOAT_EQ(rotation[2], 13);
        }
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, ErrorWhenNoBindingBaseData)
    {
        {
            auto binding = rlogic_serialization::CreateRamsesNodeBinding(
                m_flatBufferBuilder,
                0 // no base binding info
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesNodeBindingImpl> deserialized = RamsesNodeBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesNodeBinding from serialized data: missing base class info!");
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, ErrorWhenNoBindingName)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    0, // no name!
                    1u)
            );
            auto binding = rlogic_serialization::CreateRamsesNodeBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesNodeBindingImpl> deserialized = RamsesNodeBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing name!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of RamsesNodeBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, ErrorWhenNoBindingId)
    {
        {
            auto base    = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    0));
            auto binding = rlogic_serialization::CreateRamsesNodeBinding(m_flatBufferBuilder, base);
            m_flatBufferBuilder.Finish(binding);
        }

        const auto&                            serialized   = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesNodeBindingImpl> deserialized = RamsesNodeBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing or invalid ID!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of RamsesNodeBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, ErrorWhenNoRootInput)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0 // no root input
            );
            auto binding = rlogic_serialization::CreateRamsesNodeBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesNodeBindingImpl> deserialized = RamsesNodeBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesNodeBinding from serialized data: missing root input!");
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, ErrorWhenRootInputHasErrors)
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
            auto binding = rlogic_serialization::CreateRamsesNodeBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesNodeBindingImpl> deserialized = RamsesNodeBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, ErrorWhenBoundNodeCannotBeResolved)
    {
        const ramses::sceneObjectId_t mockObjectId{ 12 };
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
            auto binding = rlogic_serialization::CreateRamsesNodeBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("name"), mockObjectId)).WillOnce(::testing::Return(nullptr));

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesNodeBindingImpl> deserialized = RamsesNodeBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
    }

    TEST_F(ARamsesNodeBinding_SerializationLifecycle, ErrorWhenSavedNodeTypeDoesNotMatchResolvedNodeType)
    {
        RamsesTestSetup ramses;
        ramses::Scene* scene = ramses.createScene();
        auto* meshNode = scene->createMeshNode();

        const ramses::sceneObjectId_t mockObjectId{ 12 };
        {
            auto ramsesRef = rlogic_serialization::CreateRamsesReference(
                m_flatBufferBuilder,
                mockObjectId.getValue(),
                uint32_t(ramses::ERamsesObjectType::Node) // save normal node
            );
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                ramsesRef,
                m_testUtils.serializeTestProperty("")
            );
            auto binding = rlogic_serialization::CreateRamsesNodeBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        // resolver returns mesh node, but normal node is expected -> error
        EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("name"), mockObjectId)).WillOnce(::testing::Return(meshNode));

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesNodeBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesNodeBindingImpl> deserialized = RamsesNodeBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesNodeBinding from serialized data: loaded node type does not match referenced node type!");
    }

    // TODO Violin needs more tests here:
    // - deserialized with wrong object type which is not compatible to node
    // - deserialized with properties but without node, or the other way around
    // - rotation type different than the one in ramses node

    class ARamsesNodeBinding_SerializationWithFile : public ARamsesNodeBinding
    {
    protected:
        WithTempDirectory tempFolder;
    };

    TEST_F(ARamsesNodeBinding_SerializationWithFile, ContainsItsDataAfterDeserialization)
    {
        {
            LogicEngine tempEngineForSaving{ m_logicEngine.getFeatureLevel() };
            RamsesNodeBinding& nodeBinding = *tempEngineForSaving.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_YXZ, "NodeBinding");
            nodeBinding.getInputs()->getChild("rotation")->set<vec3f>(vec3f{ 0.1f, 0.2f, 0.3f });
            nodeBinding.getInputs()->getChild("translation")->set<vec3f>(vec3f{ 1.1f, 1.2f, 1.3f });
            nodeBinding.getInputs()->getChild("scaling")->set<vec3f>(vec3f{ 2.1f, 2.2f, 2.3f });
            nodeBinding.getInputs()->getChild("visibility")->set(true);
            nodeBinding.getInputs()->getChild("enabled")->set(true);
            tempEngineForSaving.update();
            EXPECT_TRUE(SaveToFileWithoutValidation(tempEngineForSaving, "OneBinding.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("OneBinding.bin", m_scene));
            const auto& nodeBinding = *m_logicEngine.findByName<RamsesNodeBinding>("NodeBinding");
            EXPECT_EQ("NodeBinding", nodeBinding.getName());
            EXPECT_EQ(nodeBinding.getId(), 1u);

            const auto& inputs = nodeBinding.getInputs();
            ASSERT_EQ(inputs->getChildCount(), 5u);

            auto rotation       = inputs->getChild("rotation");
            auto translation    = inputs->getChild("translation");
            auto scaling        = inputs->getChild("scaling");
            auto visibility     = inputs->getChild("visibility");
            EXPECT_EQ(ramses::ERotationType::Euler_YXZ, nodeBinding.getRotationType());
            ASSERT_NE(nullptr, rotation);
            EXPECT_EQ("rotation", rotation->getName());
            EXPECT_EQ(EPropertyType::Vec3f, rotation->getType());
            EXPECT_EQ(EPropertySemantics::BindingInput, rotation->m_impl->getPropertySemantics());
            EXPECT_EQ(*rotation->get<vec3f>(), vec3f(0.1f, 0.2f, 0.3f));
            ASSERT_NE(nullptr, translation);
            EXPECT_EQ("translation", translation->getName());
            EXPECT_EQ(EPropertyType::Vec3f, translation->getType());
            EXPECT_EQ(EPropertySemantics::BindingInput, translation->m_impl->getPropertySemantics());
            EXPECT_EQ(*translation->get<vec3f>(), vec3f(1.1f, 1.2f, 1.3f));
            ASSERT_NE(nullptr, scaling);
            EXPECT_EQ("scaling", scaling->getName());
            EXPECT_EQ(EPropertyType::Vec3f, scaling->getType());
            EXPECT_EQ(EPropertySemantics::BindingInput, scaling->m_impl->getPropertySemantics());
            EXPECT_EQ(*scaling->get<vec3f>(), vec3f(2.1f, 2.2f, 2.3f));
            ASSERT_NE(nullptr, visibility);
            EXPECT_EQ("visibility", visibility->getName());
            EXPECT_EQ(EPropertyType::Bool, visibility->getType());
            EXPECT_EQ(EPropertySemantics::BindingInput, visibility->m_impl->getPropertySemantics());
            EXPECT_TRUE(*visibility->get<bool>());

            // Test that internal indices match properties resolved by name
            EXPECT_EQ(rotation, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Rotation)));
            EXPECT_EQ(scaling, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Scaling)));
            EXPECT_EQ(translation, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Translation)));
            EXPECT_EQ(visibility, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Visibility)));

            auto enabled = inputs->getChild("enabled");
            EXPECT_EQ("enabled", enabled->getName());
            EXPECT_EQ(EPropertyType::Bool, enabled->getType());
            EXPECT_EQ(EPropertySemantics::BindingInput, enabled->m_impl->getPropertySemantics());
            EXPECT_TRUE(*enabled->get<bool>());
            EXPECT_EQ(enabled, inputs->m_impl->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Enabled)));
        }
    }

    TEST_F(ARamsesNodeBinding_SerializationWithFile, RestoresLinkToRamsesNodeAfterLoadingFromFile)
    {
        {
            LogicEngine tempEngineForSaving{ m_logicEngine.getFeatureLevel() };
            tempEngineForSaving.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(tempEngineForSaving, "OneBinding.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("OneBinding.bin", m_scene));
            const auto& nodeBinding = *m_logicEngine.findByName<RamsesNodeBinding>("NodeBinding");
            EXPECT_EQ(&nodeBinding.getRamsesNode(), m_node);
        }
    }

    TEST_F(ARamsesNodeBinding_SerializationWithFile, ProducesErrorWhenDeserializingFromFile_WhenHavingLinkToRamsesNode_ButNoSceneWasProvided)
    {
        {
            LogicEngine tempEngineForSaving{ m_logicEngine.getFeatureLevel() };
            tempEngineForSaving.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(tempEngineForSaving, "WithRamsesNode.bin"));
        }
        {
            EXPECT_FALSE(m_logicEngine.loadFromFile("WithRamsesNode.bin"));
            auto errors = m_logicEngine.getErrors();
            ASSERT_EQ(errors.size(), 1u);
            EXPECT_EQ(errors[0].message, "Fatal error during loading from file! File contains references to Ramses objects but no Ramses scene was provided!");
        }
    }

    TEST_F(ARamsesNodeBinding_SerializationWithFile, ProducesErrorWhenDeserializingFromFile_WhenHavingLinkToRamsesNode_WhichWasDeleted)
    {
        {
            LogicEngine tempEngineForSaving{ m_logicEngine.getFeatureLevel() };
            tempEngineForSaving.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(tempEngineForSaving, "RamsesNodeDeleted.bin"));
        }

        m_scene->destroy(*m_node);

        {
            EXPECT_FALSE(m_logicEngine.loadFromFile("RamsesNodeDeleted.bin", m_scene));
            auto errors = m_logicEngine.getErrors();
            ASSERT_EQ(errors.size(), 1u);
            EXPECT_EQ(errors[0].message, "Fatal error during loading from file! Serialized Ramses Logic object 'NodeBinding' points to a Ramses object (id: 1) which couldn't be found in the provided scene!");
        }
    }

    TEST_F(ARamsesNodeBinding_SerializationWithFile, DoesNotModifyRamsesNodePropertiesAfterLoadingFromFile_WhenNoValuesWereExplicitlySetBeforeSaving)
    {
        {
            LogicEngine tempEngineForSaving{ m_logicEngine.getFeatureLevel() };
            tempEngineForSaving.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(tempEngineForSaving, "NoValuesSet.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("NoValuesSet.bin", m_scene));
            EXPECT_TRUE(m_logicEngine.update());

            ExpectDefaultValues(*m_node);
        }
    }

    // Tests that the node properties don't overwrite ramses' values after loading from file, until
    // set() is called again explicitly after loadFromFile()
    TEST_F(ARamsesNodeBinding_SerializationWithFile, DoesNotReapplyPropertiesToRamsesAfterLoading_UntilExplicitlySetAgain)
    {
        {
            LogicEngine tempEngineForSaving{ m_logicEngine.getFeatureLevel() };
            RamsesNodeBinding& nodeBinding = *tempEngineForSaving.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
            // Set some values to the binding's inputs
            nodeBinding.getInputs()->getChild("translation")->set<vec3f>(vec3f{ 1.1f, 1.2f, 1.3f });
            nodeBinding.getInputs()->getChild("rotation")->set<vec3f>(vec3f{ 2.1f, 2.2f, 2.3f });
            nodeBinding.getInputs()->getChild("scaling")->set<vec3f>(vec3f{ 3.1f, 3.2f, 3.3f });
            nodeBinding.getInputs()->getChild("visibility")->set<bool>(true);
            nodeBinding.getInputs()->getChild("enabled")->set<bool>(false);
            tempEngineForSaving.update();
            EXPECT_TRUE(SaveToFileWithoutValidation(tempEngineForSaving, "AllValuesSet.bin"));
        }

        // Set properties to other values to check if they are overwritten after load
        m_node->setTranslation({100.f, 100.f, 100.f});
        m_node->setRotation({100.f, 100.f, 100.f}, ramses::ERotationType::Euler_ZYX);
        m_node->setScaling({100.f, 100.f, 100.f});
        m_node->setVisibility(ramses::EVisibilityMode::Invisible);

        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("AllValuesSet.bin", m_scene));

            EXPECT_TRUE(m_logicEngine.update());

            // Node binding does not re-apply its values to ramses node
            ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, vec3f{ 100.f, 100.f, 100.f });
            ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 100.f, 100.f, 100.f });
            ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, vec3f{ 100.f, 100.f, 100.f });
            EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Invisible);

            // Set only scaling. Use the same value as before save on purpose! Calling set forces set on ramses
            m_logicEngine.findByName<RamsesNodeBinding>("NodeBinding")->getInputs()->getChild("scaling")->set<vec3f>(vec3f{ 3.1f, 3.2f, 3.3f });
            EXPECT_TRUE(m_logicEngine.update());

            // Only scaling changed, the rest is unchanged
            ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, vec3f{ 100.f, 100.f, 100.f });
            ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 100.f, 100.f, 100.f });
            ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, vec3f{ 3.1f, 3.2f, 3.3f });
            EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Invisible);
        }
    }

    // This is sort of a confidence test, testing a combination of:
    // - bindings only propagating their values to ramses node if the value was set by an incoming link
    // - saving and loading files
    // The general expectation is that after loading + update(), the logic scene would overwrite only ramses
    // properties wrapped by a LogicBinding which is linked to a script
    TEST_F(ARamsesNodeBinding_SerializationWithFile, SetsOnlyRamsesNodePropertiesForWhichTheBindingInputIsLinked_AfterLoadingFromFile_AndCallingUpdate)
    {
        // These values should not be overwritten by logic on update()
        m_node->setScaling({22, 33, 44});
        m_node->setTranslation({100, 200, 300});

        {
            LogicEngine tempEngineForSaving{ m_logicEngine.getFeatureLevel() };

            const std::string_view scriptSrc = R"(
                function interface(IN,OUT)
                    OUT.rotation = Type:Vec3f()
                    OUT.visibility = Type:Bool()
                end
                function run(IN,OUT)
                    OUT.rotation = {1, 2, 3}
                    OUT.visibility = false
                end
            )";

            LuaScript* script = tempEngineForSaving.createLuaScript(scriptSrc);

            RamsesNodeBinding& nodeBinding = *tempEngineForSaving.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");

            ASSERT_TRUE(tempEngineForSaving.link(*script->getOutputs()->getChild("rotation"), *nodeBinding.getInputs()->getChild("rotation")));
            ASSERT_TRUE(tempEngineForSaving.link(*script->getOutputs()->getChild("visibility"), *nodeBinding.getInputs()->getChild("visibility")));

            tempEngineForSaving.update();
            EXPECT_TRUE(SaveToFileWithoutValidation(tempEngineForSaving, "SomeInputsLinked.bin"));
        }

        // Modify 'linked' properties before loading to check if logic will overwrite them after load + update
        m_node->setRotation({100, 100, 100}, ramses::ERotationType::Euler_ZYX);
        m_node->setVisibility(ramses::EVisibilityMode::Visible);

        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("SomeInputsLinked.bin", m_scene));

            EXPECT_TRUE(m_logicEngine.update());

            // Translation and Scaling were not linked -> their values are not modified
            ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, { 100.f, 200.f, 300.f });
            ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, { 22.f, 33.f, 44.f });
            // Rotation and visibility are linked -> values were updated
            ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 1.f, 2.f, 3.f });
            EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Invisible);

            // Manually setting values on ramses followed by a logic update has no effect
            // Logic is not "dirty" and it doesn't know it needs to update ramses
            m_node->setRotation({1, 2, 3}, ramses::ERotationType::Euler_ZYX);
            EXPECT_TRUE(m_logicEngine.update());
            ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 1.f, 2.f, 3.f });
        }
    }

    // Larger confidence tests which verify and document the entire data flow cycle of bindings
    // There are smaller tests which test only properties and their data propagation rules (see property unit tests)
    // There are also "dirtiness" tests which test when a node is being re-updated (see logic engine dirtiness tests)
    // These tests test everything in combination
    class ARamsesNodeBinding_DataFlow : public ARamsesNodeBinding
    {
    };

    TEST_F(ARamsesNodeBinding_DataFlow, WithExplicitSet)
    {
        // Create node and preset values
        m_node->setRotation({1.f, 1.f, 1.f}, ramses::ERotationType::Euler_ZYX);
        m_node->setScaling({2.f, 2.f, 2.f});
        m_node->setTranslation({3.f, 3.f, 3.f});
        m_node->setVisibility(ramses::EVisibilityMode::Invisible);

        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "");

        m_logicEngine.update();

        // Nothing happened - binding did not overwrite preset values because no user value set()
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 1.f, 1.f, 1.f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, vec3f{ 2.f, 2.f, 2.f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, vec3f{ 3.f, 3.f, 3.f });
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Invisible);

        // Set rotation only
        Property* inputs = nodeBinding.getInputs();
        inputs->getChild("rotation")->set<vec3f>(vec3f{ 42.f, 42.f, 42.f });

        // Update not called yet -> still has preset values for rotation in ramses node
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 1.f, 1.f, 1.f });

        // Update() only propagates rotation and does not touch other data
        m_logicEngine.update();
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 42.f, 42.f, 42.f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, vec3f{ 2.f, 2.f, 2.f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, vec3f{ 3.f, 3.f, 3.f });
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Invisible);

        // Calling update again does not "rewrite" the data to ramses. Check this by setting a value manually and call update() again
        m_node->setRotation({1.f, 1.f, 1.f}, ramses::ERotationType::Euler_ZYX);
        m_logicEngine.update();
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 1.f, 1.f, 1.f });

        // Set all properties manually this time
        inputs->getChild("rotation")->set<vec3f>(vec3f{ 100.f, 100.f, 100.f });
        inputs->getChild("scaling")->set<vec3f>(vec3f{ 200.f, 200.f, 200.f });
        inputs->getChild("translation")->set<vec3f>(vec3f{ 300.f, 300.f, 300.f });
        inputs->getChild("visibility")->set<bool>(true);
        inputs->getChild("enabled")->set<bool>(true);
        m_logicEngine.update();

        // All of the property values were passed to ramses
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 100.f, 100.f, 100.f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Scaling, vec3f{ 200.f, 200.f, 200.f });
        ExpectValues(*m_node, ENodePropertyStaticIndex::Translation, vec3f{ 300.f, 300.f, 300.f });
        EXPECT_EQ(m_node->getVisibility(), ramses::EVisibilityMode::Visible);
    }

    TEST_F(ARamsesNodeBinding_DataFlow, WithLinks)
    {
        // Create node and preset values
        m_node->setRotation({1.f, 1.f, 1.f}, ramses::ERotationType::Euler_ZYX);
        m_node->setScaling({2.f, 2.f, 2.f});
        m_node->setTranslation({3.f, 3.f, 3.f});
        m_node->setVisibility(ramses::EVisibilityMode::Off);

        const std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                IN.rotation = Type:Vec3f()
                OUT.rotation = Type:Vec3f()
            end
            function run(IN,OUT)
                OUT.rotation = IN.rotation
            end
        )";

        LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);
        script->getInputs()->getChild("rotation")->set<vec3f>({ 1.f, 2.f, 3.f });
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");

        // Adding and removing link does not set anything in ramses
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("rotation"), *nodeBinding.getInputs()->getChild("rotation")));
        ASSERT_TRUE(m_logicEngine.unlink(*script->getOutputs()->getChild("rotation"), *nodeBinding.getInputs()->getChild("rotation")));
        m_logicEngine.update();
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 1.f, 1.f, 1.f });

        // Create link and calling update -> sets values to ramses
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("rotation"), *nodeBinding.getInputs()->getChild("rotation")));
        m_logicEngine.update();
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 1.f, 2.f, 3.f });

        // Link does not overwrite manually set value ...
        m_node->setRotation({100.f, 100.f, 100.f}, ramses::ERotationType::Euler_ZYX);
        m_logicEngine.update();
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 100.f, 100.f, 100.f });

        // ... until the linked script is re-executed and provides a new value
        script->getInputs()->getChild("rotation")->set<vec3f>({11.f, 12.f, 13.f});
        m_logicEngine.update();
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 11.f, 12.f, 13.f });

        // Remove link -> value is not overwritten any more
        ASSERT_TRUE(m_logicEngine.unlink(*script->getOutputs()->getChild("rotation"), *nodeBinding.getInputs()->getChild("rotation")));
        m_node->setRotation({100.f, 100.f, 100.f}, ramses::ERotationType::Euler_ZYX);
        m_logicEngine.update();
        ExpectValues(*m_node, ENodePropertyStaticIndex::Rotation, vec3f{ 100.f, 100.f, 100.f });
    }
}
