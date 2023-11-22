//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "RamsesTestUtils.h"
#include "FeatureLevelTestValues.h"
#include "PropertyLinkTestUtils.h"

#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/RenderBufferBinding.h"

#include "ramses/client/EffectDescription.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Scene.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/UniformInput.h"
#include "ramses/framework/RamsesVersion.h"

#include "impl/logic/DataArrayImpl.h"
#include "impl/logic/LogicNodeImpl.h"
#include "impl/logic/LogicEngineImpl.h"
#include "impl/logic/LuaScriptImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/logic/AppearanceBindingImpl.h"
#include "impl/logic/CameraBindingImpl.h"
#include "impl/logic/MeshNodeBindingImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/RenderGroupBindingImpl.h"
#include "impl/logic/RenderPassBindingImpl.h"
#include "impl/logic/RenderBufferBindingImpl.h"
#include "internal/logic/ApiObjects.h"
#include "internal/logic/FileUtils.h"
#include "LogTestUtils.h"
#include "FileDescriptorHelper.h"

#include "internal/logic/flatbuffers/generated/LogicEngineGen.h"
#include "ramses-sdk-build-config.h"
#include "fmt/format.h"

#include <fstream>
#include <deque>

namespace ramses::internal
{
    class ALogicEngine_Serialization : public ALogicEngineBase, public ::testing::TestWithParam<ramses::EFeatureLevel>
    {
    public:
        ALogicEngine_Serialization() : ALogicEngineBase{ GetParam() }
        {
            withTempDirectory();
        }

    protected:
        static std::vector<char> CreateTestBuffer(const SaveFileConfig& config = {})
        {
            RamsesTestSetup ramsesSetup{ GetParam() };
            LogicEngine& logicEngineForSaving{ *ramsesSetup.createScene()->createLogicEngine() };

            const std::string src = R"(
                function interface(IN,OUT)
                    IN.param = Type:Int32()
                    OUT.param2 = Type:Int32()
                end
                function run(IN,OUT)
                end
            )";

            // Create simple (and compact) valid setup, where two (identical) scripts are created
            // and their inputs and outputs are cross linked, so that none of the scripts
            // generate a warning for having unlinked inputs or outputs

            auto* script1 = logicEngineForSaving.createLuaScript(src, {}, "luascript");
            auto* script2 = logicEngineForSaving.createLuaScript(src, {}, "luascript2");

            //link output of 1st script to input of 2nd script
            logicEngineForSaving.link(*script1->getOutputs()->getChild("param2"), *script2->getInputs()->getChild("param"));
            //link output of 2nd script to input of 1st script, use weak link to avoid circular dependancy
            logicEngineForSaving.linkWeak(*script2->getOutputs()->getChild("param2"), *script1->getInputs()->getChild("param"));

            EXPECT_TRUE(logicEngineForSaving.getScene().saveToFile("tempfile.bin", config));

            return *FileUtils::LoadBinary("tempfile.bin");
        }

        static void SaveBufferToFile(const std::vector<char>& bufferData, const std::string& file)
        {
            FileUtils::SaveBinary(file, static_cast<const void*>(bufferData.data()), bufferData.size());
        }

        std::vector<LogicObject*> saveAndLoadAllTypesOfObjects()
        {
            LogicEngine& logicEngine = *m_logicEngine;

            logicEngine.createLuaModule(m_moduleSourceCode, {}, "module");
            logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
            auto* nodeBinding = logicEngine.createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "nodeBinding");
            auto* appearanceBinding = logicEngine.createAppearanceBinding(*m_appearance, "appearanceBinding");
            auto* cameraBinding = logicEngine.createCameraBinding(*m_camera, "cameraBinding");
            const auto* dataArray = logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataArray");
            AnimationNodeConfig config;
            config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
            logicEngine.createAnimationNode(config, "animNode");
            logicEngine.createTimerNode("timerNode");
            logicEngine.createLuaInterface(R"(
                function interface(IN, OUT)
                end
            )", "intf");

            logicEngine.createRenderPassBinding(*m_renderPass, "rpBinding");
            logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
            createRenderGroupBinding(logicEngine);
            createSkinBinding(*nodeBinding, *appearanceBinding, logicEngine);
            logicEngine.createMeshNodeBinding(*m_meshNode, "mb");
            logicEngine.createRenderBufferBinding(*m_renderBuffer, "rb");

            EXPECT_TRUE(logicEngine.update());
            EXPECT_TRUE(saveToFile("LogicEngine.bin"));

            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
            expectNoError();

            const std::vector<std::string> names{ "module", "script", "nodeBinding", "appearanceBinding", "cameraBinding", "dataArray", "animNode",
                "timerNode", "intf", "rpBinding", "anchor", "renderGroupBinding", "skin", "mb", "rb" };

            std::vector<LogicObject*> objects;
            for (const auto& name : names)
            {
                auto obj = m_logicEngine->findObject<LogicObject>(name);
                EXPECT_NE(nullptr, obj);
                objects.push_back(obj);
            }

            return objects;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_SerializationTests,
        ALogicEngine_Serialization,
        GetFeatureLevelTestValues());

    TEST_P(ALogicEngine_Serialization, ProducesErrorWhenProvidingAFolderAsTargetForSaving)
    {
        fs::create_directories("folder");
        EXPECT_FALSE(saveToFile("folder"));
        EXPECT_EQ("Scene::saveToFile failed, could not open file for writing: 'folder'", getLastErrorMessage());
    }

    TEST_P(ALogicEngine_Serialization, DISABLED_PrintsMetadataInfoOnLoad)
    {
        SaveFileConfig config;
        config.setMetadataString("This is a scene exported for tests");
        config.setExporterVersion(3, 1, 2, 42);

        // Test different constructor variations
        SaveFileConfig config2(config);
        config2 = config;
        config2 = std::move(config);

        SaveBufferToFile(CreateTestBuffer(config2), "LogicEngine.bin");

        TestLogCollector logCollector(CONTEXT_CLIENT, ELogLevel::Info);

        // Test with file API
        {
            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));

            ASSERT_EQ(logCollector.logs.size(), 3u);
            EXPECT_THAT(logCollector.logs[0].message, ::testing::HasSubstr("Loading logic engine content from 'file 'LogicEngine.bin'"));
            EXPECT_THAT(logCollector.logs[1].message, ::testing::HasSubstr("Logic Engine content metadata: 'This is a scene exported for tests'"));
            EXPECT_THAT(logCollector.logs[2].message, ::testing::HasSubstr("Exporter version: 3.1.2 (file format version 42)"));
        }
    }

    TEST_P(ALogicEngine_Serialization, DISABLED_PrintsMetadataInfoOnLoad_NoVersionInfoProvided)
    {
        SaveFileConfig config;
        SaveBufferToFile(CreateTestBuffer(config), "LogicEngine.bin");

        TestLogCollector logCollector(CONTEXT_CLIENT, ELogLevel::Info);

        // Test with file API
        {
            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));

            ASSERT_EQ(logCollector.logs.size(), 3u);
            EXPECT_THAT(logCollector.logs[0].message, ::testing::HasSubstr("Loading logic engine content from 'file 'LogicEngine.bin'"));
            EXPECT_THAT(logCollector.logs[1].message, ::testing::HasSubstr("Logic Engine content metadata: ''"));
            EXPECT_THAT(logCollector.logs[2].message, ::testing::HasSubstr("Exporter version: 0.0.0 (file format version 0)"));
        }
    }

// The Windows API doesn't allow non-admin access to symlinks, this breaks on dev machines
#ifndef _WIN32
    TEST_P(ALogicEngine_Serialization, CanBeDeserializedFromHardLink)
    {
        ASSERT_TRUE(saveToFile("testfile.bin"));
        fs::create_hard_link("testfile.bin", "hardlink");
        EXPECT_TRUE(recreateFromFile("hardlink"));
    }

    TEST_P(ALogicEngine_Serialization, CanBeDeserializedFromSymLink)
    {
        ASSERT_TRUE(saveToFile("testfile.bin"));
        fs::create_symlink("testfile.bin", "symlink");
        EXPECT_TRUE(recreateFromFile("symlink"));
    }
#endif

    TEST_P(ALogicEngine_Serialization, ProducesNoErrorIfDeserializedWithNoScriptsAndNoNodeBindings)
    {
        {
            ASSERT_TRUE(saveToFile("LogicEngine.bin"));
        }
        {
            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
            expectNoError();
        }
    }

    TEST_P(ALogicEngine_Serialization, ProducesNoErrorIfDeserializedWithNoScripts)
    {
        {
            LogicEngine& logicEngine = *m_logicEngine;
            logicEngine.createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "binding");

            ASSERT_TRUE(saveToFile("LogicEngine.bin"));
        }
        {
            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
            expectNoError();

            {
                auto rNodeBinding = m_logicEngine->findObject<NodeBinding>("binding");
                ASSERT_NE(nullptr, rNodeBinding);
            }
        }
    }

    TEST_P(ALogicEngine_Serialization, ProducesNoErrorIfDeserializedWithoutNodeBindings)
    {
        {
            LogicEngine& logicEngine = *m_logicEngine;
            logicEngine.createLuaScript(R"(
                function interface(IN,OUT)
                    IN.param = Type:Int32()
                end
                function run(IN,OUT)
                end
            )", {}, "luascript");

            ASSERT_TRUE(saveToFile("LogicEngine.bin"));
        }
        {
            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
            expectNoError();

            {
                auto script = m_logicEngine->findObject<LuaScript>("luascript");
                ASSERT_NE(nullptr, script);
                const auto inputs = script->getInputs();
                ASSERT_NE(nullptr, inputs);
                EXPECT_EQ(1u, inputs->getChildCount());
            }
        }
    }

    TEST_P(ALogicEngine_Serialization, ProducesNoErrorIfDeserilizedSuccessfully)
    {
        saveAndLoadAllTypesOfObjects();
        {
            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
            expectNoError();

            {
                auto moduleByName = m_logicEngine->findObject<LuaModule>("module");
                auto moduleById = m_logicEngine->findObject(sceneObjectId_t{ 10u });
                ASSERT_NE(nullptr, moduleByName);
                ASSERT_EQ(moduleById, moduleByName);
            }
            {
                auto scriptByName = m_logicEngine->findObject<LuaScript>("script");
                auto scriptById = m_logicEngine->findObject(sceneObjectId_t{ 11u });
                ASSERT_NE(nullptr, scriptByName);
                ASSERT_EQ(scriptById, scriptByName);
                const auto inputs = scriptByName->getInputs();
                ASSERT_NE(nullptr, inputs);
                EXPECT_EQ(0u, inputs->getChildCount());
                EXPECT_TRUE(scriptByName->impl().isDirty());
            }
            {
                auto rNodeBindingByName = m_logicEngine->findObject<NodeBinding>("nodeBinding");
                auto rNodeBindingById = m_logicEngine->findObject(sceneObjectId_t{ 12u });
                ASSERT_NE(nullptr, rNodeBindingByName);
                ASSERT_EQ(rNodeBindingById, rNodeBindingByName);
                const auto inputs = rNodeBindingByName->getInputs();
                ASSERT_NE(nullptr, inputs);
                EXPECT_EQ(5u, inputs->getChildCount());
                EXPECT_FALSE(rNodeBindingByName->impl().isDirty());
            }
            {
                auto rCameraBindingByName = m_logicEngine->findObject<CameraBinding>("cameraBinding");
                auto rCameraBindingById = m_logicEngine->findObject(sceneObjectId_t{ 14u });
                ASSERT_NE(nullptr, rCameraBindingByName);
                ASSERT_EQ(rCameraBindingById, rCameraBindingByName);
                const auto inputs = rCameraBindingByName->getInputs();
                ASSERT_NE(nullptr, inputs);
                EXPECT_EQ(2u, inputs->getChildCount());
                EXPECT_FALSE(rCameraBindingByName->impl().isDirty());
            }
            {
                auto rAppearanceBindingByName = m_logicEngine->findObject<AppearanceBinding>("appearanceBinding");
                auto rAppearanceBindingById = m_logicEngine->findObject(sceneObjectId_t{ 13u });
                ASSERT_NE(nullptr, rAppearanceBindingByName);
                ASSERT_EQ(rAppearanceBindingById, rAppearanceBindingByName);
                const auto inputs = rAppearanceBindingByName->getInputs();
                ASSERT_NE(nullptr, inputs);

                ASSERT_EQ(1u, inputs->getChildCount());
                auto floatUniform = inputs->getChild(0);
                ASSERT_NE(nullptr, floatUniform);
                EXPECT_EQ("floatUniform", floatUniform->getName());
                EXPECT_EQ(EPropertyType::Float, floatUniform->getType());
                EXPECT_FALSE(rAppearanceBindingByName->impl().isDirty());
            }
            {
                const auto dataArrayByName = m_logicEngine->findObject<DataArray>("dataArray");
                const auto dataArrayById = m_logicEngine->findObject(sceneObjectId_t{ 15u });
                ASSERT_NE(nullptr, dataArrayByName);
                ASSERT_EQ(dataArrayById, dataArrayByName);
                EXPECT_EQ(EPropertyType::Float, dataArrayByName->getDataType());
                ASSERT_NE(nullptr, dataArrayByName->getData<float>());
                const std::vector<float> expectedData{ 1.f, 2.f, 3.f };
                EXPECT_EQ(expectedData, *m_logicEngine->findObject<DataArray>("dataArray")->getData<float>());

                const auto animNodeByName = m_logicEngine->findObject<AnimationNode>("animNode");
                const auto animNodeById = m_logicEngine->findObject(sceneObjectId_t{ 16u });
                ASSERT_NE(nullptr, animNodeByName);
                ASSERT_EQ(animNodeById, animNodeByName);
                ASSERT_EQ(1u, animNodeByName->getChannels().size());
                EXPECT_EQ(dataArrayByName, animNodeByName->getChannels().front().timeStamps);
                EXPECT_EQ(dataArrayByName, animNodeByName->getChannels().front().keyframes);
            }
            {
                auto rpBindingByName = m_logicEngine->findObject<RenderPassBinding>("rpBinding");
                auto rpBindingById = m_logicEngine->findObject(sceneObjectId_t{ 19u });
                ASSERT_NE(nullptr, rpBindingByName);
                ASSERT_EQ(rpBindingById, rpBindingByName);
                const auto inputs = rpBindingByName->getInputs();
                ASSERT_NE(nullptr, inputs);
                EXPECT_EQ(4u, inputs->getChildCount());
                EXPECT_FALSE(rpBindingByName->impl().isDirty());

                auto anchorByName = m_logicEngine->findObject<AnchorPoint>("anchor");
                auto anchorById = m_logicEngine->findObject(sceneObjectId_t{ 20u });
                ASSERT_NE(nullptr, anchorByName);
                ASSERT_EQ(anchorById, anchorByName);
                const auto outputs = anchorByName->getOutputs();
                ASSERT_NE(nullptr, outputs);
                EXPECT_EQ(2u, outputs->getChildCount());
            }
            {
                auto rgBindingByName = m_logicEngine->findObject<RenderGroupBinding>("renderGroupBinding");
                auto rgBindingById = m_logicEngine->findObject(sceneObjectId_t{ 21u });
                ASSERT_NE(nullptr, rgBindingByName);
                ASSERT_EQ(rgBindingById, rgBindingByName);
                const auto inputs = rgBindingByName->getInputs();
                ASSERT_NE(nullptr, inputs);
                EXPECT_EQ(1u, inputs->getChildCount());
                EXPECT_EQ(1u, inputs->getChild("renderOrders")->getChildCount());
                EXPECT_FALSE(rgBindingByName->impl().isDirty());
            }
            {
                auto skinByName = m_logicEngine->findObject<SkinBinding>("skin");
                auto skinById = m_logicEngine->findObject(sceneObjectId_t{ 22u });
                ASSERT_NE(nullptr, skinByName);
                ASSERT_EQ(skinById, skinByName);
            }
            {
                auto meshBindingByName = m_logicEngine->findObject<MeshNodeBinding>("mb");
                auto meshBindingById = m_logicEngine->findObject(sceneObjectId_t{ 23u });
                ASSERT_NE(nullptr, meshBindingByName);
                ASSERT_EQ(meshBindingById, meshBindingByName);
                const auto inputs = meshBindingByName->getInputs();
                ASSERT_NE(nullptr, inputs);
                EXPECT_EQ(4u, inputs->getChildCount());
                EXPECT_FALSE(meshBindingByName->impl().isDirty());
            }
            {
                auto rbBindingByName = m_logicEngine->findObject<RenderBufferBinding>("rb");
                auto rbBindingById = m_logicEngine->findObject(sceneObjectId_t{ 24u });
                ASSERT_NE(nullptr, rbBindingByName);
                ASSERT_EQ(rbBindingById, rbBindingByName);
                const auto inputs = rbBindingByName->getInputs();
                ASSERT_NE(nullptr, inputs);
                EXPECT_EQ(3u, inputs->getChildCount());
                EXPECT_FALSE(rbBindingByName->impl().isDirty());
            }
        }
    }

    TEST_P(ALogicEngine_Serialization, ReplacesCurrentStateWithStateFromFile)
    {
        {
            LogicEngine& logicEngine = *m_logicEngine;
            logicEngine.createLuaScript(R"(
                function interface(IN,OUT)
                    IN.param = Type:Int32()
                end
                function run(IN,OUT)
                end
            )", {}, "luascript");

            logicEngine.createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "binding");
            ASSERT_TRUE(saveToFile("LogicEngine.bin"));
        }
        {
            m_logicEngine->createLuaScript(R"(
                function interface(IN,OUT)
                    IN.param2 = Type:Float()
                end
                function run(IN,OUT)
                end
            )", {}, "luascript2");

            m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "binding2");
            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
            expectNoError();

            {
                ASSERT_EQ(nullptr, m_logicEngine->findObject<LuaScript>("luascript2"));
                ASSERT_EQ(nullptr, m_logicEngine->findObject<NodeBinding>("binding2"));

                auto script = m_logicEngine->findObject<LuaScript>("luascript");
                ASSERT_NE(nullptr, script);
                auto rNodeBinding = m_logicEngine->findObject<NodeBinding>("binding");
                ASSERT_NE(nullptr, rNodeBinding);
                EXPECT_EQ(m_node, &rNodeBinding->getRamsesNode());
            }
        }
    }

    TEST_P(ALogicEngine_Serialization, DeserializesLinks)
    {
        {
            std::string_view scriptSource = R"(
                function interface(IN,OUT)
                    IN.input = Type:Int32()
                    OUT.output = Type:Int32()
                end
                function run(IN,OUT)
                end
            )";

            LogicEngine& logicEngine = *m_logicEngine;
            auto sourceScript1 = logicEngine.createLuaScript(scriptSource, {}, "SourceScript1");
            auto targetScript1 = logicEngine.createLuaScript(scriptSource, {}, "TargetScript1");
            auto sourceScript2 = logicEngine.createLuaScript(scriptSource, {}, "SourceScript2");
            auto targetScript2 = logicEngine.createLuaScript(scriptSource, {}, "TargetScript2");
            logicEngine.createLuaScript(scriptSource, {}, "NotLinkedScript");

            auto srcOutput1 = sourceScript1->getOutputs()->getChild("output");
            auto tgtInput1 = targetScript1->getInputs()->getChild("input");
            auto srcOutput2 = sourceScript2->getOutputs()->getChild("output");
            auto tgtInput2 = targetScript2->getInputs()->getChild("input");

            EXPECT_TRUE(logicEngine.link(*srcOutput1, *tgtInput1));
            EXPECT_TRUE(logicEngine.linkWeak(*srcOutput2, *tgtInput2));

            ASSERT_TRUE(saveToFile("LogicEngine.bin"));
        }
        {
            EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
            expectNoError();

            auto sourceScript1 = m_logicEngine->findObject<LuaScript>("SourceScript1");
            auto targetScript1 = m_logicEngine->findObject<LuaScript>("TargetScript1");
            auto sourceScript2 = m_logicEngine->findObject<LuaScript>("SourceScript2");
            auto targetScript2 = m_logicEngine->findObject<LuaScript>("TargetScript2");
            auto notLinkedScript = m_logicEngine->findObject<LuaScript>("NotLinkedScript");

            EXPECT_TRUE(m_logicEngine->isLinked(*sourceScript1));
            EXPECT_TRUE(m_logicEngine->isLinked(*targetScript1));
            EXPECT_TRUE(m_logicEngine->isLinked(*sourceScript2));
            EXPECT_TRUE(m_logicEngine->isLinked(*targetScript2));
            EXPECT_FALSE(m_logicEngine->isLinked(*notLinkedScript));

            const LogicNodeDependencies& internalNodeDependencies = m_logicEngine->impl().getApiObjects().getLogicNodeDependencies();
            EXPECT_TRUE(internalNodeDependencies.isLinked(sourceScript1->impl()));
            EXPECT_TRUE(internalNodeDependencies.isLinked(targetScript1->impl()));
            EXPECT_TRUE(internalNodeDependencies.isLinked(sourceScript2->impl()));
            EXPECT_TRUE(internalNodeDependencies.isLinked(targetScript2->impl()));

            const auto srcOutput1 = sourceScript1->getOutputs()->getChild("output");
            const auto tgtInput1 = targetScript1->getInputs()->getChild("input");
            const auto srcOutput2 = sourceScript2->getOutputs()->getChild("output");
            const auto tgtInput2 = targetScript2->getInputs()->getChild("input");
            PropertyLinkTestUtils::ExpectLinks(*m_logicEngine, {
                { srcOutput1, tgtInput1, false },
                { srcOutput2, tgtInput2, true }
                });

            const auto& srcOutLinks1 = srcOutput1->impl().getOutgoingLinks();
            const auto& srcOutLinks2 = srcOutput2->impl().getOutgoingLinks();
            ASSERT_EQ(1u, srcOutLinks1.size());
            ASSERT_EQ(1u, srcOutLinks2.size());
            EXPECT_EQ(&tgtInput1->impl(), srcOutLinks1[0].property);
            EXPECT_EQ(&tgtInput2->impl(), srcOutLinks2[0].property);
            EXPECT_FALSE(srcOutLinks1[0].isWeakLink);
            EXPECT_TRUE(srcOutLinks2[0].isWeakLink);
            EXPECT_EQ(&srcOutput1->impl(), tgtInput1->impl().getIncomingLink().property);
            EXPECT_EQ(&srcOutput2->impl(), tgtInput2->impl().getIncomingLink().property);
            EXPECT_FALSE(tgtInput1->impl().getIncomingLink().isWeakLink);
            EXPECT_TRUE(tgtInput2->impl().getIncomingLink().isWeakLink);
        }
    }

    TEST_P(ALogicEngine_Serialization, InternalLinkDataIsDeletedAfterDeserialization)
    {
        std::string_view scriptSource = R"(
            function interface(IN,OUT)
                IN.input = Type:Int32()
                OUT.output = Type:Int32()
            end
            function run(IN,OUT)
            end
        )";

        auto sourceScript = m_logicEngine->createLuaScript(scriptSource, {}, "SourceScript");
        auto targetScript = m_logicEngine->createLuaScript(scriptSource, {}, "TargetScript");

        // Save logic engine state without links to file
        ASSERT_TRUE(saveToFile("LogicEngine.bin"));

        // Create link (should be wiped after loading from file)
        auto output = sourceScript->getOutputs()->getChild("output");
        auto input = targetScript->getInputs()->getChild("input");
        m_logicEngine->link(*output, *input);

        EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));

        auto sourceScriptAfterLoading = m_logicEngine->findObject<LuaScript>("SourceScript");
        auto targetScriptAfterLoading = m_logicEngine->findObject<LuaScript>("TargetScript");

        // Make a copy of the object so that we can call non-const methods on it too (getTopologicallySortedNodes())
        // This can't happen in user code, we only do this to test internal data
        LogicNodeDependencies internalNodeDependencies = m_logicEngine->impl().getApiObjects().getLogicNodeDependencies();
        ASSERT_TRUE(internalNodeDependencies.getTopologicallySortedNodes().has_value());

        // New objects are not linked (because they weren't before saving)
        EXPECT_FALSE(m_logicEngine->isLinked(*sourceScriptAfterLoading));
        EXPECT_FALSE(m_logicEngine->isLinked(*targetScriptAfterLoading));
        EXPECT_FALSE(internalNodeDependencies.isLinked(sourceScriptAfterLoading->impl()));
        EXPECT_FALSE(internalNodeDependencies.isLinked(sourceScriptAfterLoading->impl()));

        // Internal topological graph has two unsorted nodes, before and after update()
        EXPECT_EQ(2u, (*internalNodeDependencies.getTopologicallySortedNodes()).size());
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(2u, (*internalNodeDependencies.getTopologicallySortedNodes()).size());
    }

    TEST_P(ALogicEngine_Serialization, PreviouslyCreatedModulesAreDeletedInSolStateAfterDeserialization)
    {
        {
            LogicEngine& logicEngineForSaving = *m_logicEngine;
            const std::string_view moduleSrc = R"(
                local mymath = {}
                mymath.PI=3.1415
                return mymath
            )";

            std::string_view script = R"(
                modules("mymath")
                function interface(IN,OUT)
                    OUT.pi = Type:Float()
                end
                function run(IN,OUT)
                    OUT.pi = mymath.PI
                end
            )";

            LuaModule* mymath = logicEngineForSaving.createLuaModule(moduleSrc, {}, "mymath");
            LuaConfig config;
            config.addDependency("mymath", *mymath);
            logicEngineForSaving.createLuaScript(script, config, "script");

            ASSERT_TRUE(saveToFile("LogicEngine.bin"));
        }

        // Create a module with name colliding with the one from file - it should be deleted
        const std::string_view moduleToBeWipedSrc = R"(
                local mymath = {}
                mymath.PI=4
                return mymath
            )";

        // This module will be overwritten when loading the file below. The logic engine should not
        // keep any leftovers from modules or scripts when loading from file - all content should be
        // taken from the file!
        LuaModule* moduleToBeWiped = m_logicEngine->createLuaModule(moduleToBeWipedSrc, {}, "mymath");
        EXPECT_NE(nullptr, moduleToBeWiped);

        ASSERT_TRUE(recreateFromFile("LogicEngine.bin"));

        m_logicEngine->update();

        auto script = m_logicEngine->findObject<LuaScript>("script");

        // This is the PI from the loaded module, not from 'moduleToBeWiped'
        EXPECT_FLOAT_EQ(3.1415f, *script->getOutputs()->getChild("pi")->get<float>());
    }

    TEST_P(ALogicEngine_Serialization, IDsAfterDeserializationAreUnique)
    {
        sceneObjectId_t serializedId{};
        {
            LogicEngine& logicEngine = *m_logicEngine;
            const auto script = logicEngine.createLuaScript(R"(
                function interface(IN,OUT)
                    IN.param = Type:Int32()
                end
                function run(IN,OUT)
                end
            )", {}, "luascript");
            serializedId = script->getSceneObjectId();

            ASSERT_TRUE(saveToFile("LogicEngine.bin"));
        }

        EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
        expectNoError();

        auto script = m_logicEngine->findObject(serializedId);
        ASSERT_NE(nullptr, script);
        EXPECT_EQ("luascript", script->getName());

        const auto anotherScript = m_logicEngine->createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
        )");
        EXPECT_GT(anotherScript->getSceneObjectId().getValue(), script->getSceneObjectId().getValue());
    }

    TEST_P(ALogicEngine_Serialization, persistsUserIds)
    {
        {
            LogicEngine& logicEngine = *m_logicEngine;
            auto* luaModule = logicEngine.createLuaModule(m_moduleSourceCode, {}, "module");
            auto* luaScript = logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
            auto* nodeBinding = logicEngine.createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "nodeBinding");
            auto* appearanceBinding = logicEngine.createAppearanceBinding(*m_appearance, "appearanceBinding");
            auto* cameraBinding = logicEngine.createCameraBinding(*m_camera, "cameraBinding");
            auto* dataArray = logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataArray");
            AnimationNodeConfig config;
            config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
            auto* animationNode = logicEngine.createAnimationNode(config, "animNode");
            auto* timerNode = logicEngine.createTimerNode("timerNode");
            auto* luaInterface = logicEngine.createLuaInterface(R"(
                function interface(IN, OUT)
                end
            )", "intf");
            auto* rpBinding = logicEngine.createRenderPassBinding(*m_renderPass, "rpBinding");
            auto* anchor = logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
            auto* rgBinding = createRenderGroupBinding(logicEngine);
            auto* skin = createSkinBinding(logicEngine);
            auto* meshBinding = logicEngine.createMeshNodeBinding(*m_meshNode, "mb");

            EXPECT_TRUE(luaModule->setUserId(1u, 2u));
            EXPECT_TRUE(luaScript->setUserId(3u, 4u));
            EXPECT_TRUE(nodeBinding->setUserId(5u, 6u));
            EXPECT_TRUE(appearanceBinding->setUserId(7u, 8u));
            EXPECT_TRUE(cameraBinding->setUserId(9u, 10u));
            EXPECT_TRUE(dataArray->setUserId(11u, 12u));
            EXPECT_TRUE(animationNode->setUserId(13u, 14u));
            EXPECT_TRUE(timerNode->setUserId(15u, 16u));
            EXPECT_TRUE(luaInterface->setUserId(17u, 18u));
            EXPECT_TRUE(rpBinding->setUserId(19u, 20u));
            EXPECT_TRUE(anchor->setUserId(21u, 22u));
            EXPECT_TRUE(rgBinding->setUserId(23u, 24u));
            EXPECT_TRUE(skin->setUserId(25u, 26u));
            EXPECT_TRUE(meshBinding->setUserId(27u, 28u));

            ASSERT_TRUE(saveToFile("LogicEngine.bin"));
        }

        EXPECT_TRUE(recreateFromFile("LogicEngine.bin"));
        expectNoError();

        auto obj = m_logicEngine->findObject<LogicObject>("module");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(1u, obj->getUserId().first);
        EXPECT_EQ(2u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("script");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(3u, obj->getUserId().first);
        EXPECT_EQ(4u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("nodeBinding");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(5u, obj->getUserId().first);
        EXPECT_EQ(6u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("appearanceBinding");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(7u, obj->getUserId().first);
        EXPECT_EQ(8u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("cameraBinding");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(9u, obj->getUserId().first);
        EXPECT_EQ(10u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("dataArray");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(11u, obj->getUserId().first);
        EXPECT_EQ(12u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("animNode");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(13u, obj->getUserId().first);
        EXPECT_EQ(14u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("timerNode");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(15u, obj->getUserId().first);
        EXPECT_EQ(16u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("intf");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(17u, obj->getUserId().first);
        EXPECT_EQ(18u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("rpBinding");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(19u, obj->getUserId().first);
        EXPECT_EQ(20u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("anchor");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(21u, obj->getUserId().first);
        EXPECT_EQ(22u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("renderGroupBinding");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(23u, obj->getUserId().first);
        EXPECT_EQ(24u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("skin");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(25u, obj->getUserId().first);
        EXPECT_EQ(26u, obj->getUserId().second);

        obj = m_logicEngine->findObject<LogicObject>("mb");
        ASSERT_NE(nullptr, obj);
        EXPECT_EQ(27u, obj->getUserId().first);
        EXPECT_EQ(28u, obj->getUserId().second);
    }

    TEST_P(ALogicEngine_Serialization, persistsLogicObjectImplToHLObjectMapping)
    {
        const auto objects = saveAndLoadAllTypesOfObjects();
        for (const auto& obj : objects)
        {
            EXPECT_EQ(obj, &obj->impl().getLogicObject());
        }
    }

    TEST_P(ALogicEngine_Serialization, persistsOwnershipOfAllPropertiesByTheirLogicNode)
    {
        const auto objects = saveAndLoadAllTypesOfObjects();

        int propsCount = 0;
        for (const auto& obj : objects)
        {
            const auto logicNode = obj->as<LogicNode>();
            if (!logicNode)
                continue;

            std::deque<const Property*> props{ logicNode->getInputs(), logicNode->getOutputs() };
            while (!props.empty())
            {
                const auto prop = props.back();
                props.pop_back();
                if (prop == nullptr)
                    continue;

                propsCount++;
                EXPECT_EQ(obj, &prop->getOwningLogicNode());

                for (size_t i = 0u; i < prop->getChildCount(); ++i)
                    props.push_back(prop->getChild(i));
            }
        }

        // just check that the iterating over all props works
        EXPECT_EQ(54, propsCount);
    }

    TEST_P(ALogicEngine_Serialization, persistsPropertyImplToHLObjectMapping)
    {
        const auto objects = saveAndLoadAllTypesOfObjects();

        int propsCount = 0;
        for (const auto& obj : objects)
        {
            const auto logicNode = obj->as<LogicNode>();
            if (!logicNode)
                continue;

            std::deque<const Property*> props{ logicNode->getInputs(), logicNode->getOutputs() };
            while (!props.empty())
            {
                const auto prop = props.back();
                props.pop_back();
                if (prop == nullptr)
                    continue;

                propsCount++;
                EXPECT_EQ(prop, &prop->impl().getPropertyInstance());

                for (size_t i = 0u; i < prop->getChildCount(); ++i)
                    props.push_back(prop->getChild(i));
            }
        }

        // just check that the iterating over all props works
        EXPECT_EQ(54, propsCount);
    }
}
