//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-sdk-build-config.h"
#include "ViewerGuiApp.h"
#include "ViewerHeadlessApp.h"
#include "LogicViewer.h"
#include "ViewerSettings.h"
#include "gmock/gmock.h"
#include "RamsesTestUtils.h"
#include "WithTempDirectory.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/RenderBufferBinding.h"
#include "ramses/client/ramses-client.h"
#include "ImguiClientHelper.h"
#include "fmt/format.h"
#include "CLI/CLI.hpp"
#include <fstream>
#include <list>
#include <mutex>

const auto defaultLuaFile = R"(function default()
    --Interfaces
    rlogic.interfaces["myInterface"]["IN"]["paramFloat"].value = 0
    --Scripts
    rlogic.scripts["allTypesScript"]["IN"]["paramBool"].value = false
    rlogic.scripts["allTypesScript"]["IN"]["paramFloat"].value = 0
    rlogic.scripts["allTypesScript"]["IN"]["paramInt32"].value = 0
    rlogic.scripts["allTypesScript"]["IN"]["paramInt64"].value = 0
    rlogic.scripts["allTypesScript"]["IN"]["paramString"].value = ''
    rlogic.scripts["allTypesScript"]["IN"]["paramVec2f"].value = { 0, 0 }
    rlogic.scripts["allTypesScript"]["IN"]["paramVec2i"].value = { 0, 0 }
    rlogic.scripts["allTypesScript"]["IN"]["paramVec3f"].value = { 0, 0, 0 }
    rlogic.scripts["allTypesScript"]["IN"]["paramVec3i"].value = { 0, 0, 0 }
    rlogic.scripts["allTypesScript"]["IN"]["paramVec4f"].value = { 0, 0, 0, 0 }
    rlogic.scripts["allTypesScript"]["IN"]["paramVec4i"].value = { 0, 0, 0, 0 }
    rlogic.scripts["allTypesScript"]["IN"]["struct"]["nested"]["data1"].value = ''
    rlogic.scripts["allTypesScript"]["IN"]["struct"]["nested"]["data2"].value = 0
    --Node bindings
    rlogic.nodeBindings["myNode"]["IN"]["visibility"].value = true
    rlogic.nodeBindings["myNode"]["IN"]["translation"].value = { 0, 0, 0 }
    rlogic.nodeBindings["myNode"]["IN"]["scaling"].value = { 1, 1, 1 }
    rlogic.nodeBindings["myNode"]["IN"]["enabled"].value = true
    --Appearance bindings
    rlogic.appearanceBindings["myAppearance"]["IN"]["green"].value = 0
    rlogic.appearanceBindings["myAppearance"]["IN"]["blue"].value = 0
    --Camera bindings
    rlogic.cameraBindings["myCamera"]["IN"]["viewport"]["offsetX"].value = 0
    rlogic.cameraBindings["myCamera"]["IN"]["viewport"]["offsetY"].value = 0
    rlogic.cameraBindings["myCamera"]["IN"]["viewport"]["width"].value = 800
    rlogic.cameraBindings["myCamera"]["IN"]["viewport"]["height"].value = 800
    rlogic.cameraBindings["myCamera"]["IN"]["frustum"]["nearPlane"].value = 0.1
    rlogic.cameraBindings["myCamera"]["IN"]["frustum"]["farPlane"].value = 100
    rlogic.cameraBindings["myCamera"]["IN"]["frustum"]["fieldOfView"].value = 20
    rlogic.cameraBindings["myCamera"]["IN"]["frustum"]["aspectRatio"].value = 1
    --RenderPass bindings
    rlogic.renderPassBindings["myRenderPass"]["IN"]["enabled"].value = true
    rlogic.renderPassBindings["myRenderPass"]["IN"]["renderOrder"].value = 0
    rlogic.renderPassBindings["myRenderPass"]["IN"]["clearColor"].value = { 0, 0, 0, 1 }
    rlogic.renderPassBindings["myRenderPass"]["IN"]["renderOnce"].value = false
    --RenderGroup bindings
    rlogic.renderGroupBindings["myRenderGroup"]["IN"]["renderOrders"]["myMeshNode"].value = 0
    --MeshNode bindings
    rlogic.meshNodeBindings["myMeshNode"]["IN"]["vertexOffset"].value = 0
    rlogic.meshNodeBindings["myMeshNode"]["IN"]["indexOffset"].value = 0
    rlogic.meshNodeBindings["myMeshNode"]["IN"]["indexCount"].value = 3
    rlogic.meshNodeBindings["myMeshNode"]["IN"]["instanceCount"].value = 1
    --Anchor points
    --Skin bindings
    --RenderBuffer bindings
    rlogic.renderBufferBindings["myRenderBuffer"]["IN"]["width"].value = 1
    rlogic.renderBufferBindings["myRenderBuffer"]["IN"]["height"].value = 2
    rlogic.renderBufferBindings["myRenderBuffer"]["IN"]["sampleCount"].value = 3
end


defaultView = {
    name = "Default",
    description = "",
    update = function(time_ms)
        default()
    end
}

rlogic.views = {defaultView}

-- sample test function for automated image base tests
-- can be executed by command line parameter --exec=test_default
function test_default()
    -- modify properties
    default()
    -- stores a screenshot (relative to the working directory)
    rlogic.screenshot("test_default.png")
end
)";

const auto iniFile = R"(
[Window][Logic]
Pos=0,0
Size=540,720
Collapsed=0

[Window][Scene[1]: simple triangle scene (FeatureLevel 01)]
Pos=0,0
Size=540,720
Collapsed=0

[ramses-viewer][Settings]
ShowLogicWindow=1
ShowUpdateReport=0
LuaPreferObjectIds=0
LuaPreferIdentifiers=0
)";

class UI
{
public:
    // ui positions / sizes (in pixels)
    const int32_t fontHeight        = 13;
    const int32_t padding           = 3;
    const int32_t spacing           = 4;
    const int32_t titleBar          = fontHeight + 2 * padding;
    const int32_t buttonHeight      = fontHeight + 2 * padding + spacing;
    const int32_t smallButtonHeight = fontHeight + spacing;
    const int32_t hline             = 4;
    const int32_t yMiddle           = padding + fontHeight / 2; // half height of a button

    void setup(const ramses::internal::ViewerSettings* settings)
    {
        m_settings = settings;
    }

    [[nodiscard]] int32_t updateReport() const
    {
        return 66 + yMiddle ;
    }

private:
    const ramses::internal::ViewerSettings* m_settings = nullptr;
};

std::string rtrim(const std::string& str)
{
    const auto it = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch) == 0; });
    return std::string(str.begin(), it.base());
}


namespace ramses::internal
{
    const char* const emptySceneFile = "ALogicViewerAppTestEmptyScene.ramses";
    const char* const ramsesFile = "ALogicViewerAppTest.ramses";
    const char* const luaFile    = "ALogicViewerAppTest.lua";

    class LogHandler
    {
    public:
        void add(ELogLevel level, std::string_view context, std::string_view msg)
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_log.push_back({level, std::string(context), std::string(msg)});
        }

        void clear()
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_log.clear();
        }

        [[nodiscard]] size_t find(const std::string& token)
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            size_t                      count = 0u;
            for (const auto& entry : m_log)
            {
                if (entry.msg.find(token) != std::string::npos)
                {
                    ++count;
                }
            }
            return count;
        }

    private:
        struct LogEntry
        {
            ELogLevel level;
            std::string       context;
            std::string       msg;
        };
        std::mutex          m_mutex;
        std::list<LogEntry> m_log;
    };


    template <class T>
    class ALogicViewerAppBase
    {
    public:
        ALogicViewerAppBase()
        {
            setupLogic();
            [[maybe_unused]] auto status = m_scene.scene->saveToFile(ramsesFile, {});
            [[maybe_unused]] auto status2 = m_emptyScene->saveToFile(emptySceneFile, {});
            auto handler = [&](ELogLevel level, std::string_view context, std::string_view msg) { m_log.add(level, context, msg); };
            ramses::RamsesFramework::SetLogHandler(handler);
        }

        virtual ~ALogicViewerAppBase()
        {
            ramses::RamsesFramework::SetLogHandler(ramses::LogHandlerFunc());
        }

        void setupLogic()
        {
            LogicEngine& engine{ *m_scene.scene->createLogicEngine() };

            auto* interface = engine.createLuaInterface(R"(
                function interface(IN,OUT)
                    IN.paramFloat = Type:Float()
                end
            )", "myInterface");

            auto* script = engine.createLuaScript(R"(
                function interface(IN,OUT)
                    IN.paramFloat = Type:Float()
                    OUT.paramVec3f = Type:Vec3f()
                end

                function run(IN,OUT)
                    OUT.paramVec3f = {0, 0, IN.paramFloat}
                end
            )", {}, "myScript");

            engine.createLuaScript(R"(
                function interface(IN,OUT)
                    IN.paramBool = Type:Bool()
                    IN.paramInt32 = Type:Int32()
                    IN.paramInt64 = Type:Int64()
                    IN.paramFloat = Type:Float()
                    IN.paramString = Type:String()
                    IN.paramVec2f = Type:Vec2f()
                    IN.paramVec3f = Type:Vec3f()
                    IN.paramVec4f = Type:Vec4f()
                    IN.paramVec2i = Type:Vec2i()
                    IN.paramVec3i = Type:Vec3i()
                    IN.paramVec4i = Type:Vec4i()
                    IN.array = Type:Array(5, Type:Float())
                    IN.struct = {
                        nested = {
                            data1 = Type:String(),
                            data2 = Type:Int32()
                        }
                    }
                    OUT.paramBool = Type:Bool()
                    OUT.paramInt32 = Type:Int32()
                    OUT.paramInt64 = Type:Int64()
                    OUT.paramFloat = Type:Float()
                    OUT.paramString = Type:String()
                    OUT.paramVec2f = Type:Vec2f()
                    OUT.paramVec3f = Type:Vec3f()
                    OUT.paramVec4f = Type:Vec4f()
                    OUT.paramVec2i = Type:Vec2i()
                    OUT.paramVec3i = Type:Vec3i()
                    OUT.paramVec4i = Type:Vec4i()
                    OUT.array = Type:Array(5, Type:Float())
                    OUT.struct = {
                        nested = {
                            data1 = Type:String(),
                            data2 = Type:Int32()
                        }
                    }
                end
                function run(IN,OUT)
                    OUT.paramBool = IN.paramBool
                    OUT.paramInt32 = IN.paramInt32
                    OUT.paramInt64 = IN.paramInt64
                    OUT.paramFloat = IN.paramFloat
                    OUT.paramString = IN.paramString
                    OUT.paramVec2f = IN.paramVec2f
                    OUT.paramVec3f = IN.paramVec3f
                    OUT.paramVec4f = IN.paramVec4f
                    OUT.paramVec2i = IN.paramVec2i
                    OUT.paramVec3i = IN.paramVec3i
                    OUT.paramVec4i = IN.paramVec4i
                    OUT.array = IN.array
                    OUT.struct = IN.struct
                end
            )", {}, "allTypesScript");

            auto* nodeBinding = engine.createNodeBinding(*m_scene.meshNode, ramses::ERotationType::Euler_XYZ, "myNode");

            engine.createAppearanceBinding(*m_scene.appearance, "myAppearance");
            engine.createCameraBinding(*m_scene.camera, "myCamera");
            engine.createRenderPassBinding(*m_scene.renderPass, "myRenderPass");
            ramses::RenderGroupBindingElements rgElements;
            rgElements.addElement(*m_scene.meshNode, "myMeshNode");
            engine.createRenderGroupBinding(*m_scene.renderGroup, rgElements, "myRenderGroup");
            engine.createMeshNodeBinding(*m_scene.meshNode, "myMeshNode");
            engine.createTimerNode("myTimer");
            engine.createRenderBufferBinding(*m_scene.renderBuffer, "myRenderBuffer");

            ramses::DataArray* animTimestamps = engine.createDataArray(std::vector<float>{ 0.f, 0.5f, 1.f, 1.5f }); // will be interpreted as seconds
            ramses::DataArray* animKeyframes = engine.createDataArray(std::vector<ramses::vec3f>{ {0.f, 0.f, 0.f}, {0.f, 0.f, 180.f}, {0.f, 0.f, 100.f}, {0.f, 0.f, 360.f} });
            const ramses::AnimationChannel stepAnimChannel { "rotationZstep", animTimestamps, animKeyframes, ramses::EInterpolationType::Step };
            ramses::AnimationNodeConfig config;
            config.addChannel(stepAnimChannel);
            engine.createAnimationNode(config, "myAnimation");

            engine.link(*interface->getOutputs()->getChild("paramFloat"), *script->getInputs()->getChild("paramFloat"));
            engine.link(*script->getOutputs()->getChild("paramVec3f"), *nodeBinding->getInputs()->getChild("rotation"));

            engine.update();
        }

        [[nodiscard]] int createApp(const std::vector<std::string>& argsList = {})
        {
            std::vector<const char*> args;
            args.resize(argsList.size());
            std::transform(argsList.begin(), argsList.end(), args.begin(), [](const auto& str) { return str.c_str(); });
            args.insert(args.begin(), "viewer"); // 1st is executable name to comply with standard application args
            m_cli = std::make_unique<CLI::App>();
            m_app = std::make_unique<T>();
            m_app->registerOptions(*m_cli);
            CLI11_PARSE(*m_cli, static_cast<int>(args.size()), args.data());
            return 0;
        }

        [[nodiscard]] bool runUntil(const std::string& message)
        {
            const size_t maxCycles = 200u; // timeout:3.2s (200 * 16ms)
            bool         running   = true;
            for (size_t i = 0u; running && i < maxCycles; ++i)
            {
                running = m_app->doOneLoop();
                const auto count = m_log.find(message);
                m_log.clear();
                if (count > 0)
                {
                    return true;
                }
            }
            return false;
        }

        static void SaveFile(std::string_view text, std::string_view filename = luaFile)
        {
            std::ofstream fileStream(filename.data(), std::ofstream::binary);
            if (!fileStream.is_open())
            {
                throw std::runtime_error("filestream not open");
            }
            fileStream << text;
            if (fileStream.bad())
            {
                throw std::runtime_error("bad filestream");
            }
        }

        [[nodiscard]] static testing::AssertionResult CompareImage(std::string_view actual, std::string_view expected, const float tolerance = 0.1f)
        {
            const std::string diffDir        = "../res/diff/";
            const std::string resourceDir    = "../res/";
            const std::string expectedPath   = resourceDir + expected.data();
            const std::string filenameOut    = diffDir + "OUT_" + expected.data();
            const std::string filenameDiff   = diffDir + "DIFF_" + expected.data();
            const std::string filenameStderr = diffDir + "STDERR_" + expected.data();
            const std::string tempDiff       = "diff.png";
            const std::string tempStderr     = "diff.txt";

            if (!fs::exists(actual))
            {
                return testing::AssertionFailure() << "file does not exist: " << actual;
            }

            if (!fs::exists(expectedPath))
            {
                return testing::AssertionFailure() << "file does not exist: " << expectedPath;
            }

            if (!fs::exists(MAGICK_COMPARE))
            {
                return testing::AssertionFailure() << "image magick compare not found: " << MAGICK_COMPARE;
            }

            const auto cmd = fmt::format(R"("{}" -metric AE -fuzz {}% {} {} {} 2> {})", MAGICK_COMPARE, tolerance, actual, expectedPath, tempDiff, tempStderr);
            // NOLINTNEXTLINE(cert-env33-c) inputs are known at compile time
            const auto retval = std::system(cmd.c_str());
            if (retval != 0)
            {
                auto result = testing::AssertionFailure() << "image compare failed - cmd:" << cmd;
                fs::create_directory(diffDir);
                fs::copy(actual, filenameOut, fs::copy_options::overwrite_existing);
                if (fs::exists(tempDiff))
                {
                    fs::copy(tempDiff, filenameDiff, fs::copy_options::overwrite_existing);
                }
                if (fs::exists(tempStderr))
                {
                    std::ifstream ifs(tempStderr);
                    result << std::endl << "stderr:" << ifs.rdbuf();
                    fs::copy(tempStderr, filenameStderr, fs::copy_options::overwrite_existing);
                }
                return result;
            }

            return testing::AssertionSuccess();
        }

    protected:
        WithTempDirectory m_withTempDirectory;
        RamsesTestSetup   m_ramses;
        TriangleTestScene m_scene = m_ramses.createTriangleTestScene(ramses::sceneId_t(1));
        Scene*            m_emptyScene = m_ramses.createScene(ramses::sceneId_t(2));
        LogHandler m_log;
        std::unique_ptr<CLI::App> m_cli;
        std::unique_ptr<T> m_app;
    };

    class ALogicViewerGuiApp : public ALogicViewerAppBase<ViewerGuiApp>, public ::testing::Test
    {
    };

    class ALogicViewerHeadlessApp : public ALogicViewerAppBase<ViewerHeadlessApp>, public ::testing::Test
    {
    };

    template <typename T>
    class ALogicViewerApp_T : public ALogicViewerAppBase<T>, public ::testing::Test
    {
    };

    using LogicViewerAppTypes = ::testing::Types< ViewerGuiApp, ViewerHeadlessApp>;

    TYPED_TEST_SUITE(ALogicViewerApp_T, LogicViewerAppTypes);

    class ALogicViewerAppUIBase : public ALogicViewerAppBase<ViewerGuiApp>
    {
    public:
        void setup(std::string_view ini, std::vector<std::string> args = {})
        {
            SaveFile(ini, "imgui.ini");
            args.emplace_back(ramsesFile);
            EXPECT_EQ(0, this->createApp(args));
            EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->setup());

            ui.setup(m_app->getSettings());
            ImGui::GetIO().GetClipboardTextFn = GetClipboardText;
            ImGui::GetIO().SetClipboardTextFn = SetClipboardText;
            EXPECT_TRUE(runUntil("is in state RENDERED caused by command SHOW"));
            EXPECT_TRUE(m_app->doOneLoop());
        }

        [[nodiscard]] bool click(int32_t x, int32_t y)
        {
            auto imgui = m_app->getImguiClientHelper();
            imgui->mouseEvent(ramses::displayId_t(0), ramses::EMouseEvent::LeftButtonDown, x, y);
            EXPECT_TRUE(m_app->doOneLoop());
            EXPECT_TRUE(m_app->doOneLoop());
            imgui->mouseEvent(ramses::displayId_t(0), ramses::EMouseEvent::LeftButtonUp, x, y);
            EXPECT_TRUE(m_app->doOneLoop());
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give some time to render
            return m_app->doOneLoop();
        }

        [[nodiscard]] bool rightClick(int32_t x, int32_t y)
        {
            auto imgui = m_app->getImguiClientHelper();
            imgui->mouseEvent(ramses::displayId_t(0), ramses::EMouseEvent::RightButtonDown, x, y);
            EXPECT_TRUE(m_app->doOneLoop());
            EXPECT_TRUE(m_app->doOneLoop());
            imgui->mouseEvent(ramses::displayId_t(0), ramses::EMouseEvent::RightButtonUp, x, y);
            EXPECT_TRUE(m_app->doOneLoop());
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give some time to render
            return m_app->doOneLoop();
        }

        [[nodiscard]] bool keyPress(ramses::EKeyCode keyCode)
        {
            auto imgui = m_app->getImguiClientHelper();
            imgui->keyEvent(ramses::displayId_t(0), ramses::EKeyEvent::Pressed, {}, keyCode);
            EXPECT_TRUE(m_app->doOneLoop());
            imgui->keyEvent(ramses::displayId_t(0), ramses::EKeyEvent::Released, {}, keyCode);
            return m_app->doOneLoop();
        }

        [[nodiscard]] bool contextMenuSelect(int32_t x, int32_t y, int32_t item = 0)
        {
            EXPECT_TRUE(rightClick(x, y));
            return click(x + 9, y + (item * ui.smallButtonHeight) + 9);
        }

        [[nodiscard]] bool dragX(int32_t x1, int32_t x2, int32_t y)
        {
            auto imgui = m_app->getImguiClientHelper();
            imgui->mouseEvent(ramses::displayId_t(0), ramses::EMouseEvent::LeftButtonDown, x1, y);
            EXPECT_TRUE(m_app->doOneLoop());
            EXPECT_TRUE(m_app->doOneLoop());
            imgui->mouseEvent(ramses::displayId_t(0), ramses::EMouseEvent::LeftButtonDown, x2, y);
            EXPECT_TRUE(m_app->doOneLoop());
            EXPECT_TRUE(m_app->doOneLoop());
            imgui->mouseEvent(ramses::displayId_t(0), ramses::EMouseEvent::LeftButtonUp, x2, y);
            EXPECT_TRUE(m_app->doOneLoop());
            return m_app->doOneLoop();
        }

        static void SetClipboardText(void* /*unused*/, const char* str)
        {
            s_clipboard = str;
        }

        [[nodiscard]] static const char* GetClipboardText(void* /*unused*/)
        {
            return s_clipboard.c_str();
        }

        // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) must be static and non-const
        static std::string s_clipboard;

        UI ui;
    };

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) must be static and non-const
    std::string ALogicViewerAppUIBase::s_clipboard;

    class ALogicViewerAppUI : public ALogicViewerAppUIBase, public ::testing::Test
    {
    public:
        ALogicViewerAppUI()
        {
            // use --offscreen to get screenshots without ui
            setup(iniFile, {"--gui=on"});
        }
    };

    TEST_F(ALogicViewerGuiApp, ImageCompareSelfTest)
    {
        EXPECT_TRUE(CompareImage("../res/ALogicViewerApp_red.png", "ALogicViewerApp_red.png"));
        EXPECT_TRUE(CompareImage("../res/ALogicViewerApp_red.png", "ALogicViewerApp_white.png", 100));
        EXPECT_FALSE(CompareImage("../res/ALogicViewerApp_red.png", "ALogicViewerApp_white.png"));
        const std::string diffDir = "../res/diff/";
        EXPECT_TRUE(fs::exists(diffDir + "OUT_ALogicViewerApp_white.png"));
        EXPECT_TRUE(fs::exists(diffDir + "DIFF_ALogicViewerApp_white.png"));
        EXPECT_TRUE(fs::exists(diffDir + "STDERR_ALogicViewerApp_white.png"));
        std::ifstream stream(diffDir + "STDERR_ALogicViewerApp_white.png");
        std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        EXPECT_EQ("143262", str); // image differences
    }

    TYPED_TEST(ALogicViewerApp_T, emptyParam)
    {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::RequiredError), this->createApp());
    }

    TYPED_TEST(ALogicViewerApp_T, version)
    {
        testing::internal::CaptureStdout();
        EXPECT_EQ(0, this->createApp({ "--version" }));
        EXPECT_THAT(testing::internal::GetCapturedStdout(), testing::StartsWith(ramses_sdk::RAMSES_SDK_RAMSES_VERSION));
    }

    TYPED_TEST(ALogicViewerApp_T, ramsesFileDoesNotExist)
    {
        testing::internal::CaptureStderr();
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::ValidationError), this->createApp({"notExisting.ramses"}));
        EXPECT_THAT(testing::internal::GetCapturedStderr(), testing::HasSubstr("File does not exist: notExisting.ramses"));
    }

    TYPED_TEST(ALogicViewerApp_T, luaFileDoesNotExist)
    {
        testing::internal::CaptureStderr();
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::ValidationError), this->createApp({ ramsesFile, "notExisting.lua" }));
        EXPECT_THAT(testing::internal::GetCapturedStderr(), testing::HasSubstr("File does not exist: notExisting.lua"));
    }

    TYPED_TEST(ALogicViewerApp_T, writeDefaultLuaConfiguration)
    {
        EXPECT_EQ(0, this->createApp({ "--write-config", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, this->m_app->run());
        EXPECT_TRUE(fs::exists(luaFile));
        std::ifstream str(luaFile);
        std::string   genfile((std::istreambuf_iterator<char>(str)), std::istreambuf_iterator<char>());
        EXPECT_EQ(defaultLuaFile, genfile);
    }

    TEST_F(ALogicViewerGuiApp, writeDefaultLuaConfigurationHeadless)
    {
        EXPECT_EQ(0, this->createApp({ "--write-config", "--headless", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, this->m_app->run());
        EXPECT_TRUE(fs::exists(luaFile));
        std::ifstream str(luaFile);
        std::string   genfile((std::istreambuf_iterator<char>(str)), std::istreambuf_iterator<char>());
        EXPECT_EQ(defaultLuaFile, genfile);
    }

    TYPED_TEST(ALogicViewerApp_T, writeDefaultLuaConfigurationToOtherFile)
    {
        EXPECT_EQ(0, this->createApp({ "--write-config=foobar.lua", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, this->m_app->run());
        EXPECT_TRUE(fs::exists("foobar.lua"));
        std::ifstream str("foobar.lua");
        std::string   genfile((std::istreambuf_iterator<char>(str)), std::istreambuf_iterator<char>());
        EXPECT_EQ(defaultLuaFile, genfile);
    }

    TEST_F(ALogicViewerGuiApp, runInteractive)
    {
        EXPECT_EQ(0, createApp({ ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->setup());
        EXPECT_TRUE(runUntil("is in state RENDERED caused by command SHOW"));
        EXPECT_TRUE(m_app->doOneLoop());
        EXPECT_TRUE(m_app->doOneLoop());
        EXPECT_TRUE(m_app->doOneLoop());
        auto imgui = m_app->getImguiClientHelper();
        imgui->windowClosed(ramses::displayId_t());
        EXPECT_FALSE(m_app->doOneLoop());
    }

    TEST_F(ALogicViewerGuiApp, runInteractiveWithEmptyScene)
    {
        EXPECT_EQ(0, createApp({ emptySceneFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->setup());
        EXPECT_TRUE(runUntil("is in state RENDERED caused by command SHOW"));
        EXPECT_TRUE(m_app->doOneLoop());
        EXPECT_TRUE(m_app->doOneLoop());
        EXPECT_TRUE(m_app->doOneLoop());
        auto imgui = m_app->getImguiClientHelper();
        imgui->windowClosed(ramses::displayId_t());
        EXPECT_FALSE(m_app->doOneLoop());
    }

    TEST_F(ALogicViewerHeadlessApp, runInteractive)
    {
        EXPECT_EQ(0, createApp({ ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->run());
    }

    TYPED_TEST(ALogicViewerApp_T, exec_luaFileMissing)
    {
        // implicit filename
        testing::internal::CaptureStderr();
        EXPECT_EQ(0, this->createApp({ "--exec=test_default", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::ErrorLoadLua, this->m_app->run());
        EXPECT_THAT(testing::internal::GetCapturedStderr(), testing::HasSubstr("cannot open ALogicViewerAppTest.lua: No such file or directory"));
        // explicit filename
        testing::internal::CaptureStderr();
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::ValidationError), this->createApp({ "--exec=test_default", "--lua=NotExistingLuaFile.lua", ramsesFile }));
        EXPECT_THAT(testing::internal::GetCapturedStderr(), testing::HasSubstr("File does not exist: NotExistingLuaFile.lua"));
    }

    TEST_F(ALogicViewerGuiApp, exec_screenshot)
    {
        SaveFile(R"(
            function test_default()
                -- stores a screenshot (relative to the working directory)
                rlogic.screenshot("test_red.png")
                rlogic.appearanceBindings.myAppearance.IN.green.value = 1
                rlogic.screenshot("test_yellow.png")
            end
        )");
        EXPECT_EQ(0, createApp({ "--exec=test_default", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->run());
        EXPECT_TRUE(CompareImage("test_red.png", "ALogicViewerApp_red.png"));
        EXPECT_TRUE(CompareImage("test_yellow.png", "ALogicViewerApp_yellow.png"));
    }

    TYPED_TEST(ALogicViewerApp_T, exec_luaError)
    {
        this->SaveFile(R"(
            function test_default()
                -- stores a screenshot (relative to the working directory)
                rlogic.screenshot("test_red.png")
                rlogic.appearanceBindings.myAppearance.IN.green.value = 1
                rlogic.screenshot("test_yellow.png")
        )");
        EXPECT_EQ(0, this->createApp({ "--exec=test_default", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::ErrorLoadLua, this->m_app->run());
    }

    TEST_F(ALogicViewerGuiApp, exec_lua_function)
    {
        SaveFile(R"(
            function test_default(filename, greenVal)
                rlogic.appearanceBindings.myAppearance.IN.green.value = greenVal
                rlogic.screenshot(filename)
            end
        )");
        EXPECT_EQ(0, createApp({ R"(--exec-lua=test_default('almost_yellow.png', 0.9))", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->run());
        auto appearance = m_app->getLogicViewer()->getLogic().findObject<ramses::AppearanceBinding>("myAppearance");
        ASSERT_TRUE(appearance != nullptr);
        auto prop = appearance->getInputs()->getChild("green");
        ASSERT_TRUE(prop != nullptr);
        EXPECT_FLOAT_EQ(0.9f, prop->get<float>().value());
        EXPECT_TRUE(CompareImage("almost_yellow.png", "ALogicViewerApp_yellow.png", 15.f));
    }

    TYPED_TEST(ALogicViewerApp_T, exec_lua_code)
    {
        EXPECT_EQ(0, this->createApp({ R"(--exec-lua=rlogic.appearanceBindings.myAppearance.IN.green.value = 0.44)", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, this->m_app->run());
        auto appearance = this->m_app->getLogicViewer()->getLogic().template findObject<typename ramses::AppearanceBinding>("myAppearance");
        ASSERT_TRUE(appearance != nullptr);
        auto prop = appearance->getInputs()->getChild("green");
        ASSERT_TRUE(prop != nullptr);
        EXPECT_FLOAT_EQ(0.44f, prop->template get<float>().value());
    }

    TYPED_TEST(ALogicViewerApp_T, exec_lua_error)
    {
        testing::internal::CaptureStderr();
        EXPECT_EQ(0, this->createApp({ R"(--exec-lua=rlogic.appearanceBindings.myAppearance.IN.green = 0.44)", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::ErrorLoadLua, this->m_app->run());
        EXPECT_THAT(testing::internal::GetCapturedStderr(), testing::HasSubstr("sol: cannot set (new_index) into this object"));
    }

    TEST_F(ALogicViewerGuiApp, exec_lua_headless)
    {
        EXPECT_EQ(0, createApp({ R"(--exec-lua=rlogic.appearanceBindings.myAppearance.IN.green.value = 0.24)", "--headless", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->run());
        auto appearance = m_app->getLogicViewer()->getLogic().findObject<ramses::AppearanceBinding>("myAppearance");
        ASSERT_TRUE(appearance != nullptr);
        auto prop = appearance->getInputs()->getChild("green");
        ASSERT_TRUE(prop != nullptr);
        EXPECT_FLOAT_EQ(0.24f, prop->get<float>().value());
    }

    TEST_F(ALogicViewerGuiApp, exec_lua_screenshot_headless)
    {
        testing::internal::CaptureStderr();
        EXPECT_EQ(0, this->createApp({ R"(--exec-lua=rlogic.screenshot("screenshot.png"))", "--headless", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::ErrorLoadLua, this->m_app->run());
        EXPECT_THAT(testing::internal::GetCapturedStderr(), testing::HasSubstr("No screenshots available in current configuration"));
    }

    TEST_F(ALogicViewerGuiApp, interactive_luaError)
    {
        SaveFile(R"(
            function test_default()
                -- stores a screenshot (relative to the working directory)
                rlogic.screenshot("test_red.png")
                rlogic.appearanceBindings.myAppearance.IN.green.value = 1
                rlogic.screenshot("test_yellow.png")
        )");
        EXPECT_EQ(0, createApp({ ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->setup());
        EXPECT_TRUE(runUntil("is in state RENDERED caused by command SHOW"));
        EXPECT_THAT(m_app->getLogicViewer()->getLastResult().getMessage(), testing::HasSubstr("ALogicViewerAppTest.lua:7: 'end' expected"));
        EXPECT_TRUE(m_app->doOneLoop());
        EXPECT_TRUE(m_app->doOneLoop());
        // does not terminate
    }

    TEST_F(ALogicViewerGuiApp, no_offscreen)
    {
        SaveFile(R"(
            function test_default()
                -- stores a screenshot (relative to the working directory)
                rlogic.screenshot("test_red.png")
            end
        )");
        EXPECT_EQ(0, createApp({ "--exec=test_default", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->run());
        EXPECT_TRUE(CompareImage("test_red.png", "ALogicViewerApp_red.png"));
    }

    TEST_F(ALogicViewerGuiApp, windowSize)
    {
        SaveFile(R"(
            function test_default()
                -- stores a screenshot (relative to the working directory)
                rlogic.screenshot("test_red.png")
            end
        )");
        EXPECT_EQ(0, createApp({ "--exec=test_default", "--width", "500", "--height", "700", ramsesFile }));
        EXPECT_EQ(ViewerApp::ExitCode::Ok, m_app->run());
        EXPECT_TRUE(CompareImage("test_red.png", "ALogicViewerApp_red_500x700.png"));
    }

    TEST_F(ALogicViewerAppUI, reloadConfiguration)
    {
        m_app->getSettings()->showLogicWindow = false;
        const int32_t xFile = 25;
        auto* script = m_app->getLogicViewer()->getLogic().findObject<ramses::LuaScript>("allTypesScript");
        ASSERT_TRUE(script != nullptr);
        auto* prop = script->getOutputs()->getChild("paramString");
        ASSERT_TRUE(prop != nullptr);

        SaveFile(R"(rlogic.scripts.allTypesScript.IN.paramString.value = "Hello")");
        EXPECT_TRUE(m_app->doOneLoop());
        EXPECT_EQ("", prop->get<std::string>().value()); // not reloaded automatically

        EXPECT_TRUE(keyPress(ramses::EKeyCode_F5)); // reload configuration
        EXPECT_EQ("Hello", prop->get<std::string>().value());

        SaveFile(R"(rlogic.scripts.allTypesScript.IN.paramString.value = "World")");
        EXPECT_TRUE(click(xFile, ui.titleBar + ui.yMiddle));
        EXPECT_TRUE(click(xFile, ui.titleBar + ui.buttonHeight + 4*ui.smallButtonHeight + ui.yMiddle));
        EXPECT_EQ("World", prop->get<std::string>().value());

        SaveFile(R"(rlogic.scripts.allTypesScript.IN.paramString.value = "Foo")");
        EXPECT_TRUE(contextMenuSelect(700, 350, 8));
        EXPECT_EQ("Foo", prop->get<std::string>().value());
    }

    TEST_F(ALogicViewerAppUI, reloadConfigurationWithHiddenUI)
    {
        auto* settings = m_app->getSettings();
        settings->showWindow = false;
        auto* script = m_app->getLogicViewer()->getLogic().findObject<ramses::LuaScript>("allTypesScript");
        ASSERT_TRUE(script != nullptr);
        auto* prop = script->getOutputs()->getChild("paramString");
        ASSERT_TRUE(prop != nullptr);

        SaveFile(R"(rlogic.scripts.allTypesScript.IN.paramString.value = "Hello")");
        EXPECT_TRUE(m_app->doOneLoop());
        EXPECT_EQ("", prop->get<std::string>().value()); // not reloaded automatically

        EXPECT_TRUE(keyPress(ramses::EKeyCode_F5)); // reload configuration
        EXPECT_EQ("Hello", prop->get<std::string>().value());
    }

    TEST_F(ALogicViewerAppUI, changeView)
    {
        SaveFile(R"(
            red = {
                name = "Red",
                description = "Shows a red triangle",
                update = function(time_ms)
                    rlogic.appearanceBindings["myAppearance"]["IN"]["green"].value = 0
                    rlogic.appearanceBindings.myAppearance.IN.blue.value = 0
                end
            }

            yellow = {
                name = "Yellow",
                description = "Shows a yellow triangle",
                update = function(time_ms)
                    rlogic.appearanceBindings["myAppearance"]["IN"]["green"].value = 1
                end,
                inputs = {rlogic.appearanceBindings.myAppearance.IN.blue}
            }

            rlogic.views = {red, yellow}

            function screenshot()
                rlogic.screenshot("screenshot.png")
            end
        )");

        EXPECT_TRUE(keyPress(ramses::EKeyCode_F5)); // reload configuration
        EXPECT_EQ(2u, m_app->getLogicViewer()->getViewCount());
        EXPECT_EQ(1u, m_app->getLogicViewer()->getCurrentView());
        EXPECT_EQ(Result(), m_app->getLogicViewer()->call("screenshot"));
        EXPECT_TRUE(CompareImage("screenshot.png", "ALogicViewerApp_red.png"));

        EXPECT_TRUE(keyPress(ramses::EKeyCode_Right)); // next view
        EXPECT_EQ(2u, m_app->getLogicViewer()->getCurrentView());
        EXPECT_EQ(Result(), m_app->getLogicViewer()->call("screenshot"));
        EXPECT_TRUE(CompareImage("screenshot.png", "ALogicViewerApp_yellow.png"));

        // Modify the "blue" input 0 -> 1
        EXPECT_TRUE(dragX(100, 100 + 10, ui.titleBar + 1 * ui.buttonHeight + 2 * ui.smallButtonHeight + ui.yMiddle));
        EXPECT_EQ(Result(), m_app->getLogicViewer()->call("screenshot"));
        EXPECT_TRUE(CompareImage("screenshot.png", "ALogicViewerApp_white.png"));

        EXPECT_TRUE(keyPress(ramses::EKeyCode_Left)); // previous view
        EXPECT_EQ(1u, m_app->getLogicViewer()->getCurrentView());
        EXPECT_EQ(Result(), m_app->getLogicViewer()->call("screenshot"));
        EXPECT_TRUE(CompareImage("screenshot.png", "ALogicViewerApp_red.png"));
    }

    TEST_F(ALogicViewerAppUI, settings)
    {
        auto* settings = m_app->getSettings();
        EXPECT_TRUE(settings->showLogicWindow);
        EXPECT_TRUE(contextMenuSelect(700, 350, 1));
        EXPECT_FALSE(settings->showLogicWindow);
        EXPECT_TRUE(contextMenuSelect(700, 350, 1));
        EXPECT_TRUE(settings->showLogicWindow);
    }

    class ALogicViewerAppUIClearColor : public ALogicViewerAppUIBase, public ::testing::TestWithParam<bool>
    {
    public:
        ALogicViewerAppUIClearColor()
        {
            if (GetParam())
            {
                setup(iniFile, {"--gui", "on", "--clear", "0,0,0.5,1"});
            }
            else
            {
                setup(iniFile, { "--clear", "0,0,0.5,1" });
            }
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        ALogicViewerAppUIClearColor_TestInstances,
        ALogicViewerAppUIClearColor,
        ::testing::Values(
            false, // no offscreen
            true)  // offscreen
        );

    TEST_P(ALogicViewerAppUIClearColor, usesClearColorFromCommandLine)
    {
        SaveFile(R"(
            function screenshot()
                rlogic.screenshot("screenshot.png")
            end
        )");
        EXPECT_TRUE(keyPress(ramses::EKeyCode_F5)); // reload configuration
        EXPECT_TRUE(keyPress(ramses::EKeyCode_F11)); // hide UI

        EXPECT_EQ(Result(), m_app->getLogicViewer()->call("screenshot"));
        EXPECT_TRUE(CompareImage("screenshot.png", "ALogicViewerApp_clearColorCmdLine.png", 0.5f)); // increased tolerance due to some platforms being 1/255 off covering large area (as background)
    }

    class ALogicViewerAppUIUpdateReport : public ALogicViewerAppUIBase, public ::testing::Test
    {
    };

    TEST_F(ALogicViewerAppUIUpdateReport, updateReport)
    {
        setup(R"([Window][Logic]
Pos=0,0
Size=540,720
Collapsed=0

[ramses-viewer][Settings]
ShowLogicWindow=1
LuaPreferObjectIds=0
LuaPreferIdentifiers=0
)");
        m_app->getSettings()->showUpdateReportDetails = true;
        const int32_t mouseX = 100;

        const auto& report = m_app->getLogicViewer()->getUpdateReport();

        // set update interval to 1 to avoid random test failures
        // (only the longest update is reported for an interval)
        EXPECT_EQ(60u, report.getInterval());
        EXPECT_TRUE(dragX(mouseX, mouseX - 120, ui.updateReport() + 2 * ui.buttonHeight));
        EXPECT_EQ(1u, report.getInterval());
        EXPECT_EQ(12, report.getNodesExecuted().size() + report.getNodesSkippedExecution().size());

        auto* interface = m_app->getLogicViewer()->getLogic().findObject<ramses::LuaInterface>("myInterface");
        ASSERT_TRUE(interface != nullptr);
        auto* prop = interface->getInputs()->getChild("paramFloat");
        ASSERT_TRUE(prop != nullptr);
        prop->set(2.1f);
        EXPECT_TRUE(m_app->doOneLoop());
        EXPECT_EQ(4, report.getNodesExecuted().size());
        EXPECT_EQ(8, report.getNodesSkippedExecution().size());
    }
}
