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
#include "impl/RamsesRenderGroupBindingImpl.h"
#include "impl/PropertyImpl.h"
#include "internals/RamsesHelper.h"
#include "generated/RamsesRenderGroupBindingGen.h"

#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LogicEngine.h"

#include "ramses-client-api/RenderGroup.h"

#include "ramses-utils.h"

namespace rlogic::internal
{
    class ARamsesRenderGroupBinding : public ALogicEngine
    {
    public:
        ARamsesRenderGroupBinding() : ALogicEngine{ EFeatureLevel_03 }
        {
        }
    };

    TEST_F(ARamsesRenderGroupBinding, HasANameAfterCreation)
    {
        auto& renderGroupBinding = *createRenderGroupBinding();
        EXPECT_EQ("renderGroupBinding", renderGroupBinding.getName());
    }

    TEST_F(ARamsesRenderGroupBinding, RefersToGivenRenderGroup)
    {
        auto& renderGroupBinding = *createRenderGroupBinding();
        EXPECT_EQ(m_renderGroup, &renderGroupBinding.getRamsesRenderGroup());
        const auto& rpConst = renderGroupBinding;
        EXPECT_EQ(m_renderGroup, &rpConst.getRamsesRenderGroup());
        const auto& rpImplConst = renderGroupBinding.m_renderGroupBinding;
        EXPECT_EQ(m_renderGroup, &rpImplConst.getRamsesRenderGroup());
    }

    TEST_F(ARamsesRenderGroupBinding, HasInputsAfterCreationWithCorrectNamesAndValues)
    {
        const auto mn1 = m_scene->createMeshNode("meshNodeName1");
        const auto mn2 = m_scene->createMeshNode("meshNodeName2");
        const auto rg1 = m_scene->createRenderGroup("renderGroupName1");
        const auto rg2 = m_scene->createRenderGroup("renderGroupName2");
        // in order to control render order of these objects they have to be contained in the rendergroup to bind
        m_renderGroup->addMeshNode(*mn1, -100);
        m_renderGroup->addMeshNode(*mn2, 33);
        m_renderGroup->addRenderGroup(*rg1, -60);
        m_renderGroup->addRenderGroup(*rg2, 42);

        // add objects as elements to control through rendergroup binding
        // (intentionally in specific order and using both object or element name)
        RamsesRenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*mn2, "elementMesh2Name"));
        EXPECT_TRUE(elements.addElement(*rg2, "elementRG2Name"));
        EXPECT_TRUE(elements.addElement(*mn1));
        EXPECT_TRUE(elements.addElement(*rg1));

        const auto& renderGroupBinding = *m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements);
        ASSERT_NE(nullptr, renderGroupBinding.getInputs());
        ASSERT_EQ(1u, renderGroupBinding.getInputs()->getChildCount());
        const auto renderOrdersProp = renderGroupBinding.getInputs()->getChild("renderOrders");
        ASSERT_NE(nullptr, renderOrdersProp);
        ASSERT_EQ(4u, renderOrdersProp->getChildCount());

        for (size_t i = 0u; i < renderOrdersProp->getChildCount(); ++i)
        {
            ASSERT_EQ(EPropertyType::Int32, renderOrdersProp->getChild(i)->getType());
            EXPECT_EQ(EPropertySemantics::BindingInput, renderOrdersProp->getChild(i)->m_impl->getPropertySemantics());
        }

        const auto prop1 = renderOrdersProp->getChild(0u);
        EXPECT_EQ("elementMesh2Name", prop1->getName());
        EXPECT_EQ(33, *prop1->get<int32_t>());

        const auto prop2 = renderOrdersProp->getChild(1u);
        EXPECT_EQ("elementRG2Name", prop2->getName());
        EXPECT_EQ(42, *prop2->get<int32_t>());

        const auto prop3 = renderOrdersProp->getChild(2u);
        EXPECT_EQ("meshNodeName1", prop3->getName());
        EXPECT_EQ(-100, *prop3->get<int32_t>());

        const auto prop4 = renderOrdersProp->getChild(3u);
        EXPECT_EQ("renderGroupName1", prop4->getName());
        EXPECT_EQ(-60, *prop4->get<int32_t>());
    }

    TEST_F(ARamsesRenderGroupBinding, HasNoOutputsAfterCreation)
    {
        auto& renderGroupBinding = *createRenderGroupBinding();
        EXPECT_EQ(nullptr, renderGroupBinding.getOutputs());
    }

    TEST_F(ARamsesRenderGroupBinding, FailsToBeCreatedIfNoProvidedElement)
    {
        RamsesRenderGroupBindingElements emptyElements;
        EXPECT_EQ(nullptr, m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, emptyElements));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create RamsesRenderGroupBinding, there were no elements provided.");
    }

    TEST_F(ARamsesRenderGroupBinding, FailsToBeCreatedIfProvidedElementNotContainedInRenderGroupInRamses)
    {
        const auto mn1 = m_scene->createMeshNode("meshNodeName1");
        RamsesRenderGroupBindingElements elements1;
        EXPECT_TRUE(elements1.addElement(*m_meshNode)); // contained
        EXPECT_TRUE(elements1.addElement(*mn1)); // not contained
        EXPECT_EQ(nullptr, m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements1));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create RamsesRenderGroupBinding, one or more of the provided elements is not contained in the RenderGroup to bind.");

        const auto rg1 = m_scene->createRenderGroup("renderGroupName1");
        RamsesRenderGroupBindingElements elements2;
        EXPECT_TRUE(elements2.addElement(*m_meshNode)); // contained
        EXPECT_TRUE(elements2.addElement(*rg1)); // not contained
        EXPECT_EQ(nullptr, m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements2));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create RamsesRenderGroupBinding, one or more of the provided elements is not contained in the RenderGroup to bind.");
    }

    TEST_F(ARamsesRenderGroupBinding, RemovesLinkWhenDestroyed)
    {
        auto& renderGroupBinding = *createRenderGroupBinding();

        const std::string_view scriptSrc = R"(
                function interface(IN,OUT)
                    OUT.val = Type:Int32()
                end
                function run(IN,OUT)
                    OUT.val = 42
                end
            )";
        const LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("val"), *renderGroupBinding.getInputs()->getChild("renderOrders")->getChild(0u)));
        EXPECT_TRUE(script->getOutputs()->getChild("val")->isLinked());
        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_TRUE(m_logicEngine.destroy(renderGroupBinding));
        EXPECT_FALSE(script->getOutputs()->getChild("val")->isLinked());
    }

    TEST_F(ARamsesRenderGroupBinding, TakesInitialValuesFromRamsesRenderGroup)
    {
        const auto nestedRG = m_scene->createRenderGroup();
        m_renderGroup->addRenderGroup(*nestedRG, 41);
        m_renderGroup->addMeshNode(*m_meshNode, 42);

        RamsesRenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
        EXPECT_TRUE(elements.addElement(*nestedRG, "rg"));
        const auto& renderGroupBinding = m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements);

        EXPECT_EQ(41, *renderGroupBinding->getInputs()->getChild("renderOrders")->getChild("rg")->get<int32_t>());
        EXPECT_EQ(42, *renderGroupBinding->getInputs()->getChild("renderOrders")->getChild("mesh")->get<int32_t>());
    }

    TEST_F(ARamsesRenderGroupBinding, AppliesChangesToBoundObject)
    {
        auto& renderGroupBinding = *createRenderGroupBinding();
        EXPECT_TRUE(renderGroupBinding.getInputs()->getChild(0u)->getChild(0u)->set(42));
        m_logicEngine.update();
        int32_t renderOrder = -1;
        m_renderGroup->getMeshNodeOrder(*m_meshNode, renderOrder);
        EXPECT_EQ(42, renderOrder);

        EXPECT_TRUE(renderGroupBinding.getInputs()->getChild(0u)->getChild(0u)->set(-100));
        m_logicEngine.update();
        m_renderGroup->getMeshNodeOrder(*m_meshNode, renderOrder);
        EXPECT_EQ(-100, renderOrder);
    }

    TEST_F(ARamsesRenderGroupBinding, PropagateItsInputsToRamsesRenderGroupOnUpdate_OnlyWhenExplicitlySet)
    {
        const auto nestedRG = m_scene->createRenderGroup();
        m_renderGroup->addRenderGroup(*nestedRG, 41);
        m_renderGroup->addMeshNode(*m_meshNode, 42);

        RamsesRenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
        EXPECT_TRUE(elements.addElement(*nestedRG, "rg"));
        auto& renderGroupBinding = *m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements);

        m_logicEngine.update();
        int32_t renderOrder = -1;
        m_renderGroup->getRenderGroupOrder(*nestedRG, renderOrder);
        EXPECT_EQ(41, renderOrder);
        m_renderGroup->getMeshNodeOrder(*m_meshNode, renderOrder);
        EXPECT_EQ(42, renderOrder);

        // update only mesh
        EXPECT_TRUE(renderGroupBinding.getInputs()->getChild("renderOrders")->getChild("mesh")->set(-100));
        m_logicEngine.update();
        m_renderGroup->getRenderGroupOrder(*nestedRG, renderOrder);
        EXPECT_EQ(41, renderOrder); // stays unchanged
        m_renderGroup->getMeshNodeOrder(*m_meshNode, renderOrder);
        EXPECT_EQ(-100, renderOrder);
    }

    TEST_F(ARamsesRenderGroupBinding, PropagatesItsInputsToRamsesRenderGroupOnUpdate_WithLinksInsteadOfSetCall)
    {
        auto& renderGroupBinding = *createRenderGroupBinding();

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
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("val"), *renderGroupBinding.getInputs()->getChild("renderOrders")->getChild(0u)));
        EXPECT_TRUE(m_logicEngine.update());

        int32_t renderOrder = -1;
        m_renderGroup->getMeshNodeOrder(*m_meshNode, renderOrder);
        EXPECT_EQ(42, renderOrder);
    }

    TEST_F(ARamsesRenderGroupBinding, GracefullyFailsUpdateIfChangingRenderOrderOfMeshNodeWhichWasRemovedFromBoundRenderGroup)
    {
        auto& renderGroupBinding = *createRenderGroupBinding();
        EXPECT_TRUE(renderGroupBinding.getInputs()->getChild(0u)->getChild(0u)->set(42));
        EXPECT_TRUE(m_logicEngine.update());

        // remove mesh from rendergroup using ramses API
        // this is illegal but can happen
        m_renderGroup->removeMeshNode(*m_meshNode);

        EXPECT_TRUE(renderGroupBinding.getInputs()->getChild(0u)->getChild(0u)->set(-42));
        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot set render order of MeshNode which is not contained in bound RenderGroup.");
    }

    TEST_F(ARamsesRenderGroupBinding, GracefullyFailsUpdateIfChangingRenderOrderOfRenderGroupWhichWasRemovedFromBoundRenderGroup)
    {
        const auto nestedRG = m_scene->createRenderGroup();
        m_renderGroup->addRenderGroup(*nestedRG);

        RamsesRenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
        EXPECT_TRUE(elements.addElement(*nestedRG, "rg"));
        auto& renderGroupBinding = *m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements);

        EXPECT_TRUE(renderGroupBinding.getInputs()->getChild(0u)->getChild("rg")->set(42));
        EXPECT_TRUE(m_logicEngine.update());

        // remove nested rendergroup from bound rendergroup using ramses API
        // this is illegal but can happen
        m_renderGroup->removeRenderGroup(*nestedRG);

        EXPECT_TRUE(renderGroupBinding.getInputs()->getChild(0u)->getChild("rg")->set(-42));
        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot set render order of RenderGroup which is not contained in bound RenderGroup.");
    }

    class ARamsesRenderGroupBinding_SerializationLifecycle : public ARamsesRenderGroupBinding
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            MissingName,
            MissingRootInput,
            RootInputNotStruct,
            MissingRamsesRefObject,
            RamsesRefObjectNotInScene,
            MismatchedRamsesRefObjectType,
            PropertiesDoNotMatchElements,
            MissingElementRamsesRefObject,
            ElementRamsesRefObjectNotInScene,
            MismatchedElementRamsesRefObjectType
        };

        std::unique_ptr<RamsesRenderGroupBindingImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            {
                auto inputsType = MakeStruct("", {
                    TypeData{"renderOrders", EPropertyType::Struct},
                });

                if (issue != ESerializationIssue::PropertiesDoNotMatchElements)
                    inputsType.children.front().children.push_back(MakeType("elementName", EPropertyType::Int32));

                HierarchicalTypeData inputs = (issue == ESerializationIssue::RootInputNotStruct ? MakeType("", EPropertyType::Bool) : inputsType);
                auto inputsImpl = std::make_unique<PropertyImpl>(std::move(inputs), EPropertySemantics::BindingInput);

                auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(m_flatBufferBuilder,
                    rlogic_serialization::CreateLogicObject(m_flatBufferBuilder, (issue == ESerializationIssue::MissingName ? 0 : m_flatBufferBuilder.CreateString("name")), 1u, 0u, 0u),
                    (issue == ESerializationIssue::MissingRamsesRefObject ? 0 : rlogic_serialization::CreateRamsesReference(m_flatBufferBuilder,
                        1u, (issue == ESerializationIssue::MismatchedRamsesRefObjectType ? 0 : static_cast<uint32_t>(ramses::ERamsesObjectType_RenderGroup)))),
                    (issue == ESerializationIssue::MissingRootInput ? 0 : PropertyImpl::Serialize(*inputsImpl, m_flatBufferBuilder, m_serializationMap)));

                std::vector<flatbuffers::Offset<rlogic_serialization::Element>> elementsFB;
                elementsFB.push_back(rlogic_serialization::CreateElement(
                    m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("elementName"),
                    (issue == ESerializationIssue::MissingElementRamsesRefObject ? 0 : rlogic_serialization::CreateRamsesReference(m_flatBufferBuilder,
                        2u, (issue == ESerializationIssue::MismatchedElementRamsesRefObjectType ? 0 : static_cast<uint32_t>(m_meshNode->getType()))))));

                auto fbRenderGroupBinding = rlogic_serialization::CreateRamsesRenderGroupBinding(m_flatBufferBuilder, fbRamsesBinding, m_flatBufferBuilder.CreateVector(elementsFB));
                m_flatBufferBuilder.Finish(fbRenderGroupBinding);
            }

            switch (issue)
            {
            case ESerializationIssue::AllValid:
            case ESerializationIssue::MismatchedRamsesRefObjectType:
            case ESerializationIssue::PropertiesDoNotMatchElements:
            case ESerializationIssue::MissingElementRamsesRefObject:
            case ESerializationIssue::ElementRamsesRefObjectNotInScene:
            case ESerializationIssue::MismatchedElementRamsesRefObjectType:
                EXPECT_CALL(m_resolverMock, findRamsesRenderGroupInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 1u })).WillOnce(::testing::Return(m_renderGroup));
                break;
            case ESerializationIssue::RamsesRefObjectNotInScene:
                EXPECT_CALL(m_resolverMock, findRamsesRenderGroupInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 1u })).WillOnce(::testing::Return(nullptr));
                break;
            default:
                break;
            }

            switch (issue)
            {
            case ESerializationIssue::AllValid:
            case ESerializationIssue::MismatchedElementRamsesRefObjectType:
                EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 2u })).WillOnce(::testing::Return(m_meshNode));
                break;
            case ESerializationIssue::ElementRamsesRefObjectNotInScene:
                EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("name"), ramses::sceneObjectId_t{ 2u })).WillOnce(::testing::Return(nullptr));
                break;
            default:
                break;
            }

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesRenderGroupBinding>(m_flatBufferBuilder.GetBufferPointer());
            return RamsesRenderGroupBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils {m_flatBufferBuilder};
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;
        ErrorReporting m_errorReporting;
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap;
    };

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, CanSerializeWithNoIssue)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_MissingName)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::MissingName));
        ASSERT_EQ(2u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LogicObject base from serialized data: missing name!");
        EXPECT_EQ(m_errorReporting.getErrors()[1].message, "Fatal error during loading of RamsesRenderGroupBinding from serialized data: missing name and/or ID!");
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_MissingRootInput)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::MissingRootInput));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderGroupBinding from serialized data: missing root input!");
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_RootInputNotStruct)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::RootInputNotStruct));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderGroupBinding from serialized data: root input has unexpected type!");
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_MissingRamsesRefObject)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::MissingRamsesRefObject));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderGroupBinding from serialized data: missing ramses object reference!");
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_RamsesRefObjectNotInScene)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::RamsesRefObjectNotInScene));
        // error log is in ramses object resolver which is mocked
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_MismatchedRamsesRefObjectType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::MismatchedRamsesRefObjectType));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderGroupBinding from serialized data: loaded object type does not match referenced object type!");
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_PropertiesDoNotMatchElements)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::PropertiesDoNotMatchElements));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderGroupBinding from serialized data: input properties do not match RenderGroup's elements!");
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_MissingElementRamsesRefObject)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::MissingElementRamsesRefObject));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderGroupBinding 'name' elements data: missing name or Ramses reference!");
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_ElementRamsesRefObjectNotInScene)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::ElementRamsesRefObjectNotInScene));
        // error log is in ramses object resolver which is mocked
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, ReportsSerializationError_MismatchedElementRamsesRefObjectType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ARamsesRenderGroupBinding_SerializationLifecycle::ESerializationIssue::MismatchedElementRamsesRefObjectType));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderGroupBinding 'name' elements data: loaded element object type does not match referenced object type!");
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, KeepsItsPropertiesAfterDeserialization)
    {
        {
            const auto nestedRG = m_scene->createRenderGroup();
            m_renderGroup->addRenderGroup(*nestedRG);
            RamsesRenderGroupBindingElements elements;
            EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
            EXPECT_TRUE(elements.addElement(*nestedRG, "rg"));
            auto& renderGroupBinding = *m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements, "renderGroupBinding");

            EXPECT_TRUE(renderGroupBinding.getInputs()->getChild(0u)->getChild("mesh")->set(41));
            EXPECT_TRUE(renderGroupBinding.getInputs()->getChild(0u)->getChild("rg")->set(42));
            EXPECT_TRUE(m_logicEngine.update());

            // binding has no inputs linked, validatation would fail
            ASSERT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "binding.bin"));
        }

        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("binding.bin", m_scene));
            const auto loadedBinding = m_logicEngine.findByName<RamsesRenderGroupBinding>("renderGroupBinding");
            EXPECT_EQ(&loadedBinding->getRamsesRenderGroup(), m_renderGroup);

            ASSERT_NE(nullptr, loadedBinding->getInputs());
            ASSERT_EQ(1u, loadedBinding->getInputs()->getChildCount());
            const auto renderOrdersProp = loadedBinding->getInputs()->getChild("renderOrders");
            ASSERT_NE(nullptr, renderOrdersProp);
            ASSERT_EQ(2u, renderOrdersProp->getChildCount());

            const auto prop1 = renderOrdersProp->getChild(0u);
            EXPECT_EQ("mesh", prop1->getName());
            EXPECT_EQ(EPropertyType::Int32, prop1->getType());
            EXPECT_EQ(EPropertySemantics::BindingInput, prop1->m_impl->getPropertySemantics());
            EXPECT_EQ(41, *prop1->get<int32_t>());

            const auto prop2 = renderOrdersProp->getChild(1u);
            EXPECT_EQ("rg", prop2->getName());
            EXPECT_EQ(EPropertyType::Int32, prop2->getType());
            EXPECT_EQ(EPropertySemantics::BindingInput, prop2->m_impl->getPropertySemantics());
            EXPECT_EQ(42, *prop2->get<int32_t>());
        }
    }

    TEST_F(ARamsesRenderGroupBinding_SerializationLifecycle, FailsToLoadWhenNoSceneProvided)
    {
        {
            createRenderGroupBinding();
            ASSERT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "binding.bin"));
        }

        {
            EXPECT_FALSE(m_logicEngine.loadFromFile("binding.bin"));
            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Fatal error during loading from file! File contains references to Ramses objects but no Ramses scene was provided!");
        }
    }
}
