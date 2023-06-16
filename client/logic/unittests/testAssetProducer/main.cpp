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

void createTriangle(ramses::Scene& scene)
{
    const std::string_view vertShader = R"(
                #version 100

                uniform highp mat4 mvpMatrix;
                attribute vec3 a_position;

                void main()
                {
                    gl_Position = mvpMatrix * vec4(a_position, 1.0);
                })";

    const std::string_view fragShader = R"(
                #version 100

                void main(void)
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                })";

    ramses::EffectDescription effectDesc;
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
    effectDesc.setVertexShader(vertShader.data());
    effectDesc.setFragmentShader(fragShader.data());
    auto effect = scene.createEffect(effectDesc);
    auto appearance = scene.createAppearance(*effect, "triangle appearance");

    ramses::GeometryBinding* geometry = scene.createGeometryBinding(*effect, "triangle geometry");

    const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
    ramses::ArrayResource* vertexPositions = scene.createArrayResource(3u, vertexPositionsData.data());
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::MeshNode* meshNode = scene.createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setIndexCount(3);
    meshNode->setGeometryBinding(*geometry);

    auto* camera = scene.createPerspectiveCamera("triangle camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    ramses::RenderPass* renderPass = scene.createRenderPass("triangle render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene.createRenderGroup();
    renderGroup->addMeshNode(*meshNode);
    renderPass->addRenderGroup(*renderGroup);
}

void createTriangleLogic(ramses::LogicEngine& logic, ramses::Scene& scene)
{
    auto camera = ramses::RamsesUtils::TryConvert<ramses::Camera>(*scene.findObjectByName("triangle camera"));
    auto camNodeBinding = logic.createRamsesNodeBinding(*camera, ramses::ERotationType::Euler_XYZ, "triangleCamNodeBinding");
    auto camBinding = logic.createRamsesCameraBinding(*camera, "triangleCamBinding");

    auto intf = logic.createLuaInterface(R"(
        function interface(inout)
            inout.CraneGimbal = { Yaw = Type:Float(), Pitch = Type:Float() }
            inout.Viewport = { Width = Type:Int32(), Height = Type:Int32() }
        end
        )", "Interface_CameraCrane");

    auto script = logic.createLuaScript(R"(
        function interface(IN,OUT)
            IN.yaw = Type:Float()
            IN.pitch = Type:Float()
            IN.width = Type:Int32()
            IN.height = Type:Int32()
            OUT.translation = Type:Vec3f()
        end
        function run(IN,OUT)
            OUT.translation = { IN.yaw/20, IN.pitch/50, 5 }
        end
    )", {}, "CameraCrane");

    logic.link(*intf->getOutputs()->getChild("CraneGimbal")->getChild("Yaw"), *script->getInputs()->getChild("yaw"));
    logic.link(*intf->getOutputs()->getChild("CraneGimbal")->getChild("Pitch"), *script->getInputs()->getChild("pitch"));
    logic.link(*intf->getOutputs()->getChild("Viewport")->getChild("Width"), *camBinding->getInputs()->getChild("viewport")->getChild("width"));
    logic.link(*intf->getOutputs()->getChild("Viewport")->getChild("Height"), *camBinding->getInputs()->getChild("viewport")->getChild("height"));

    logic.link(*script->getOutputs()->getChild("translation"), *camNodeBinding->getInputs()->getChild("translation"));

    intf->getInputs()->getChild("CraneGimbal")->getChild("Yaw")->set(0.f);
    intf->getInputs()->getChild("CraneGimbal")->getChild("Pitch")->set(0.f);
    intf->getInputs()->getChild("Viewport")->getChild("Width")->set(1280);
    intf->getInputs()->getChild("Viewport")->getChild("Height")->set(480);
}

int main(int argc, char* argv[])
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) bounds are checked
    const std::vector<const char*> args(argv, argv + argc);

    constexpr ramses::EFeatureLevel featureLevel = ramses::EFeatureLevel_Latest;

    std::string basePath {"."};
    std::string ramsesFilename = std::string("testScene_0") + std::to_string(featureLevel) + ".ramses";
    std::string logicFilename = std::string("testLogic_0") + std::to_string(featureLevel) + ".rlogic";

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

    ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
    ramses::RamsesFramework ramsesFramework{frameworkConfig};
    ramses::RamsesClient* ramsesClient = ramsesFramework.createClient("");

    ramses::Scene* scene = ramsesClient->createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "");
    scene->flush();
    ramses::LogicEngine logicEngine{ featureLevel };

    ramses::LuaScript* script1 = logicEngine.createLuaScript(R"(
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

    ramses::LuaConfig config;
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
    config.addStandardModuleDependency(ramses::EStandardModule::Math);

    ramses::LuaScript* script2 = logicEngine.createLuaScript(R"(
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

    // create triangle that can actually be visible when rendered
    createTriangle(*scene);
    createTriangleLogic(logicEngine, *scene);

    ramses::RamsesNodeBinding* nodeBinding = logicEngine.createRamsesNodeBinding(*node, ramses::ERotationType::Euler_XYZ, "nodebinding");
    ramses::RamsesCameraBinding* camBindingOrtho = logicEngine.createRamsesCameraBinding(*cameraOrtho, "camerabinding");
    ramses::RamsesAppearanceBinding* appBinding = logicEngine.createRamsesAppearanceBinding(*appearance, "appearancebinding");
    logicEngine.createRamsesCameraBinding(*cameraPersp, "camerabindingPersp");
    logicEngine.createRamsesRenderPassBinding(*renderPass, "renderpassbinding");
    logicEngine.createAnchorPoint(*nodeBinding, *camBindingOrtho, "anchorpoint");
    logicEngine.createRamsesCameraBindingWithFrustumPlanes(*cameraPersp, "camerabindingPerspWithFrustumPlanes");

    ramses::RamsesRenderGroupBindingElements elements;
    elements.addElement(*meshNode, "mesh");
    elements.addElement(*nestedRenderGroup, "nestedRenderGroup");
    logicEngine.createRamsesRenderGroupBinding(*renderGroup, elements, "rendergroupbinding");

    ramses::UniformInput uniform;
    appearance->getEffect().findUniformInput("jointMat", uniform);
    logicEngine.createSkinBinding({ nodeBinding }, { ramses::matrix44f{ 0.f } }, * appBinding, uniform, "skin");
    logicEngine.createDataArray(std::vector<std::vector<float>>{ { 1.f, 2.f, 3.f, 4.f, 5.f }, { 6.f, 7.f, 8.f, 9.f, 10.f } }, "dataarrayOfArrays");

    logicEngine.createRamsesMeshNodeBinding(*meshNode, "meshnodebinding");

    const auto dataArray = logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f }, "dataarray");
    ramses::AnimationNodeConfig animConfig;
    animConfig.addChannel({ "channel", dataArray, dataArray, ramses::EInterpolationType::Linear });
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

    ramses::SaveFileConfig noValidationConfig;
    noValidationConfig.setValidationEnabled(false);
    logicEngine.saveToFile(basePath + "/" + logicFilename, noValidationConfig);

    [[maybe_unused]] auto status = scene->saveToFile(basePath +  "/" + ramsesFilename, false);

    return 0;
}
