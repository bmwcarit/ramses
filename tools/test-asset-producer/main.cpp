//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/EStandardModule.h"

#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"

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

    ramses::Geometry* geometry = scene.createGeometry(*effect, "triangle geometry");

    const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
    ramses::ArrayResource* vertexPositions = scene.createArrayResource(3u, vertexPositionsData.data());
    geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *vertexPositions);

    ramses::MeshNode* meshNode = scene.createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setIndexCount(3);
    meshNode->setGeometry(*geometry);

    auto* camera = scene.createPerspectiveCamera("triangle camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    ramses::RenderPass* renderPass = scene.createRenderPass("triangle render pass");
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene.createRenderGroup();
    renderGroup->addMeshNode(*meshNode);
    renderPass->addRenderGroup(*renderGroup);
}

void createTriangleLogic(ramses::LogicEngine& logic, ramses::Scene& scene)
{
    auto camera = scene.findObject<ramses::Camera>("triangle camera");
    auto camNodeBinding = logic.createNodeBinding(*camera, ramses::ERotationType::Euler_XYZ, "triangleCamNodeBinding");
    auto camBinding = logic.createCameraBinding(*camera, "triangleCamBinding");

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

    if (args.size() == 2u)
    {
        basePath = args[1];
    }
    else if (args.size() == 3u)
    {
        basePath = args[1];
        ramsesFilename = args[2];
    }

    if (args.size() > 3u)
    {
        std::cerr
            << "Generator of ramses and ramses logic test content.\n\n"
            << "Synopsis:\n"
            << "  test-asset-producer\n"
            << "  test-asset-producer <basePath>\n"
            << "  test-asset-producer <basePath> <ramsesFileName>\n\n";
        return 1;
    }

    ramses::RamsesFrameworkConfig frameworkConfig{ featureLevel };
    ramses::RamsesFramework ramsesFramework{frameworkConfig};
    ramses::RamsesClient* ramsesClient = ramsesFramework.createClient("");

    ramses::Scene* scene = ramsesClient->createScene(ramses::sceneId_t(123u), "");
    scene->flush();
    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine("testAssetLogic") };

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

    ramses::NodeBinding* nodeBinding = logicEngine.createNodeBinding(*node, ramses::ERotationType::Euler_XYZ, "nodebinding");
    ramses::CameraBinding* camBindingOrtho = logicEngine.createCameraBinding(*cameraOrtho, "camerabinding");
    ramses::AppearanceBinding* appBinding = logicEngine.createAppearanceBinding(*appearance, "appearancebinding");
    logicEngine.createCameraBinding(*cameraPersp, "camerabindingPersp");
    logicEngine.createRenderPassBinding(*renderPass, "renderpassbinding");
    logicEngine.createAnchorPoint(*nodeBinding, *camBindingOrtho, "anchorpoint");
    logicEngine.createCameraBindingWithFrustumPlanes(*cameraPersp, "camerabindingPerspWithFrustumPlanes");

    ramses::RenderGroupBindingElements elements;
    elements.addElement(*meshNode, "mesh");
    elements.addElement(*nestedRenderGroup, "nestedRenderGroup");
    logicEngine.createRenderGroupBinding(*renderGroup, elements, "rendergroupbinding");

    logicEngine.createSkinBinding({ nodeBinding }, { ramses::matrix44f{ 0.f } }, * appBinding, *appearance->getEffect().findUniformInput("jointMat"), "skin");
    logicEngine.createDataArray(std::vector<std::vector<float>>{ { 1.f, 2.f, 3.f, 4.f, 5.f }, { 6.f, 7.f, 8.f, 9.f, 10.f } }, "dataarrayOfArrays");

    logicEngine.createMeshNodeBinding(*meshNode, "meshnodebinding");

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
    noValidationConfig.setMetadataString("test-asset-producer");

    if (!scene->saveToFile(basePath +  "/" + ramsesFilename, noValidationConfig))
        return EXIT_FAILURE;

    return 0;
}
