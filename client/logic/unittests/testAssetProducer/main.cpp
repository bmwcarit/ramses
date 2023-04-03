//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaInterface.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/RamsesRenderGroupBindingElements.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/EStandardModule.h"

#include "ramses-client.h"
#include "ramses-utils.h"

#include <iostream>

ramses::Appearance* createTestAppearance(ramses::Scene& scene)
{
    const std::string_view vertShader = R"(
                #version 100

                uniform highp float floatUniform;
                uniform highp float animatedFloatUniform;
                uniform highp mat4 jointMat[1];
                attribute vec3 a_position;

                void main()
                {
                    gl_Position = floatUniform * animatedFloatUniform * vec4(a_position, 1.0) * jointMat[0];
                })";

    const std::string_view fragShader = R"(
                #version 100

                void main(void)
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                })";

    ramses::EffectDescription effectDesc;
    effectDesc.setUniformSemantic("u_DisplayBufferResolution", ramses::EEffectUniformSemantic::DisplayBufferResolution);
    effectDesc.setVertexShader(vertShader.data());
    effectDesc.setFragmentShader(fragShader.data());

    return scene.createAppearance(*scene.createEffect(effectDesc), "test appearance");
}

int main(int argc, char* argv[])
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) bounds are checked
    const std::vector<const char*> args(argv, argv + argc);

    std::string basePath {"."};
    std::string ramsesFilename = "testScene.ramses";
    std::string logicFilename = "testLogic.rlogic";

    if (args.size() == 2u)
    {
        basePath = args[1];
    }
    else if (args.size() == 4u)
    {
        basePath = args[1];
        ramsesFilename = args[2];
        logicFilename = args[3];
    }

    if (args.size() == 3 || args.size() > 4u)
    {
        std::cerr
            << "Generator of ramses and ramses logic test content.\n\n"
            << "Synopsis:\n"
            << "  testAssetProducer\n"
            << "  testAssetProducer <basePath>\n"
            << "  testAssetProducer <basePath> <ramsesFileName> <logicFileName>\n\n";
        return 1;
    }

    ramses::RamsesFramework ramsesFramework;
    ramses::RamsesClient* ramsesClient = ramsesFramework.createClient("");

    ramses::Scene* scene = ramsesClient->createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "");
    scene->flush();
    rlogic::LogicEngine logicEngine{ rlogic::EFeatureLevel_05 };

    rlogic::LuaScript* script1 = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.intInput =      Type:Int32()
            IN.int64Input =    Type:Int64()
            IN.vec2iInput =    Type:Vec2i()
            IN.vec3iInput =    Type:Vec3i()
            IN.vec4iInput =    Type:Vec4i()
            IN.floatInput =    Type:Float()
            IN.vec2fInput =    Type:Vec2f()
            IN.vec3fInput =    Type:Vec3f()
            IN.vec4fInput =    Type:Vec4f()
            IN.boolInput  =    Type:Bool()
            IN.stringInput =   Type:String()
            IN.structInput = {
                nested = {
                    data1 = Type:String(),
                    data2 = Type:Int32()
                }
            }
            IN.arrayInput =    Type:Array(9, Type:Float())
            OUT.floatOutput = Type:Float()
            OUT.nodeTranslation = Type:Vec3f()
            OUT.boolOutput = Type:Bool()
        end
        function run(IN,OUT)
            OUT.floatOutput = IN.floatInput
            OUT.nodeTranslation = {IN.floatInput, 2, 3}
            OUT.boolOutput = false
        end
    )", {}, "script1");

    const auto luaNestedModuleMath = logicEngine.createLuaModule(R"(
            local mymath = {}
            function mymath.sub(a,b)
                return a - b
            end
            return mymath
        )", {}, "nestedModuleMath");

    rlogic::LuaConfig config;
    config.addDependency("nestedMath", *luaNestedModuleMath);

    const auto luaModuleMath = logicEngine.createLuaModule(R"(
            modules('nestedMath')
            local mymath = {}
            mymath.sub=nestedMath.sub
            function mymath.add(a,b)
                return a + b
            end
            return mymath
        )", config, "moduleMath");

    const auto luaModuleTypes = logicEngine.createLuaModule(R"(
            local mytypes = {}
            function mytypes.camViewport()
                return {
                    offsetX = Type:Int32(),
                    offsetY = Type:Int32(),
                    width = Type:Int32(),
                    height = Type:Int32()
                }
            end
            return mytypes
        )", {}, "moduleTypes");

    config = {};
    config.addDependency("modulemath", *luaModuleMath);
    config.addDependency("moduletypes", *luaModuleTypes);
    config.addStandardModuleDependency(rlogic::EStandardModule::Math);

    rlogic::LuaScript* script2 = logicEngine.createLuaScript(R"(
        modules("modulemath", "moduletypes")
        function interface(IN,OUT)
            IN.floatInput = Type:Float()
            OUT.cameraViewport = moduletypes.camViewport()
            OUT.floatUniform = Type:Float()
            OUT.nestedModulesResult = Type:Int32()
        end
        function run(IN,OUT)
            OUT.floatUniform = IN.floatInput + 5.0
            local roundedFloat = math.ceil(IN.floatInput)
            OUT.cameraViewport = {
                offsetX = modulemath.add(2, roundedFloat),
                offsetY = modulemath.add(4, roundedFloat),
                width = modulemath.add(100, roundedFloat),
                height = modulemath.add(200, roundedFloat)
            }
            OUT.nestedModulesResult = modulemath.sub(1000, roundedFloat)
        end
    )", config, "script2");

    const auto intf = logicEngine.createLuaInterface(R"(
        function interface(inout)
            inout.struct = { floatInput = Type:Float() }
        end)", "intf");

    ramses::Node* node = { scene->createNode("test node") };
    ramses::OrthographicCamera* cameraOrtho = { scene->createOrthographicCamera("test camera") };
    cameraOrtho->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 10.f);
    ramses::Appearance* appearance = { createTestAppearance(*scene) };
    ramses::RenderPass* renderPass = scene->createRenderPass();
    ramses::PerspectiveCamera* cameraPersp = { scene->createPerspectiveCamera("test persp camera") };
    auto renderGroup = scene->createRenderGroup();
    const auto nestedRenderGroup = scene->createRenderGroup();
    const auto meshNode = scene->createMeshNode();
    renderGroup->addMeshNode(*meshNode);
    renderGroup->addRenderGroup(*nestedRenderGroup);

    rlogic::RamsesNodeBinding* nodeBinding = logicEngine.createRamsesNodeBinding(*node, rlogic::ERotationType::Euler_XYZ, "nodebinding");
    rlogic::RamsesCameraBinding* camBindingOrtho = logicEngine.createRamsesCameraBinding(*cameraOrtho, "camerabinding");
    rlogic::RamsesAppearanceBinding* appBinding = logicEngine.createRamsesAppearanceBinding(*appearance, "appearancebinding");
    logicEngine.createRamsesCameraBinding(*cameraPersp, "camerabindingPersp");
    logicEngine.createRamsesRenderPassBinding(*renderPass, "renderpassbinding");
    logicEngine.createAnchorPoint(*nodeBinding, *camBindingOrtho, "anchorpoint");
    logicEngine.createRamsesCameraBindingWithFrustumPlanes(*cameraPersp, "camerabindingPerspWithFrustumPlanes");

    rlogic::RamsesRenderGroupBindingElements elements;
    elements.addElement(*meshNode, "mesh");
    elements.addElement(*nestedRenderGroup, "nestedRenderGroup");
    logicEngine.createRamsesRenderGroupBinding(*renderGroup, elements, "rendergroupbinding");

    ramses::UniformInput uniform;
    appearance->getEffect().findUniformInput("jointMat", uniform);
    logicEngine.createSkinBinding({ nodeBinding }, { rlogic::matrix44f{ 0.f } }, * appBinding, uniform, "skin");
    logicEngine.createDataArray(std::vector<std::vector<float>>{ { 1.f, 2.f, 3.f, 4.f, 5.f }, { 6.f, 7.f, 8.f, 9.f, 10.f } }, "dataarrayOfArrays");

    logicEngine.createRamsesMeshNodeBinding(*meshNode, "meshnodebinding");

    const auto dataArray = logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f }, "dataarray");
    rlogic::AnimationNodeConfig animConfig;
    animConfig.addChannel({ "channel", dataArray, dataArray, rlogic::EInterpolationType::Linear });
    const auto animNode = logicEngine.createAnimationNode(animConfig, "animNode");
    animConfig.setExposingOfChannelDataAsProperties(true);
    logicEngine.createAnimationNode(animConfig, "animNodeWithDataProperties");
    logicEngine.createTimerNode("timerNode");

    logicEngine.link(*intf->getOutputs()->getChild("struct")->getChild("floatInput"), *script1->getInputs()->getChild("floatInput"));
    logicEngine.link(*script1->getOutputs()->getChild("floatOutput"), *script2->getInputs()->getChild("floatInput"));
    logicEngine.link(*script1->getOutputs()->getChild("nodeTranslation"), *nodeBinding->getInputs()->getChild("translation"));
    logicEngine.link(*script2->getOutputs()->getChild("cameraViewport")->getChild("offsetX"), *camBindingOrtho->getInputs()->getChild("viewport")->getChild("offsetX"));
    logicEngine.link(*script2->getOutputs()->getChild("cameraViewport")->getChild("offsetY"), *camBindingOrtho->getInputs()->getChild("viewport")->getChild("offsetY"));
    logicEngine.link(*script2->getOutputs()->getChild("cameraViewport")->getChild("width"), *camBindingOrtho->getInputs()->getChild("viewport")->getChild("width"));
    logicEngine.link(*script2->getOutputs()->getChild("cameraViewport")->getChild("height"), *camBindingOrtho->getInputs()->getChild("viewport")->getChild("height"));
    logicEngine.link(*script2->getOutputs()->getChild("floatUniform"), *appBinding->getInputs()->getChild("floatUniform"));
    logicEngine.link(*animNode->getOutputs()->getChild("channel"), *appBinding->getInputs()->getChild("animatedFloatUniform"));

    logicEngine.link(*script1->getOutputs()->getChild("boolOutput"), *nodeBinding->getInputs()->getChild("enabled"));

    if (!logicEngine.update())
        return 1;

    rlogic::SaveFileConfig noValidationConfig;
    noValidationConfig.setValidationEnabled(false);
    logicEngine.saveToFile(basePath + "/" + logicFilename, noValidationConfig);

    scene->saveToFile((basePath +  "/" + ramsesFilename).c_str(), false);

    return 0;
}
