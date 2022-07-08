//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ImguiClientHelper.h"

namespace ramses_internal
{
    namespace
    {
        const char* const ImguiFragShader = R"GLSL(
        #version 100
        uniform sampler2D textureSampler;

        varying lowp vec2 v_texcoord;
        varying lowp vec4 Frag_Color;
        void main(void)
        {
            gl_FragColor = texture2D(textureSampler, v_texcoord.st)*Frag_Color;
        })GLSL";

        const char* const ImguiVertShader = R"GLSL(
        #version 100
        uniform highp mat4 mvpMatrix;
        attribute vec2 a_position;
        attribute vec2 a_texcoord;
        attribute vec4 Color;
        varying vec2 v_texcoord;
        varying vec4 Frag_Color;
        void main(void)
        {
            gl_Position = mvpMatrix*vec4(a_position.xy, 0.0, 1.0);
            v_texcoord = a_texcoord;
            Frag_Color = Color;
        })GLSL";

        uint32_t GetSpecialChar(ramses::EKeyCode keyCode, const bool isShift)
        {
            switch (keyCode)
            {
            case ramses::EKeyCode_Space:
                return ' ';
            case ramses::EKeyCode_Numpad_Subtract:
            case ramses::EKeyCode_Minus:
                return isShift ? '_' : '-';
            case ramses::EKeyCode_Slash:
                return isShift ? '?' : '/';
            case ramses::EKeyCode_Numpad_Decimal:
                return '.';
            case ramses::EKeyCode_Period:
                return isShift ? '>' : '.';
            case ramses::EKeyCode_Comma:
                return isShift ? '<' : ',';
            case ramses::EKeyCode_Backslash:
                return isShift ? '|' : '\\';
            case ramses::EKeyCode_LeftBracket:
                return isShift ? '{' : '[';
            case ramses::EKeyCode_RightBracket:
                return isShift ? '}' : ']';
            case ramses::EKeyCode_Equals:
                return isShift ? '+' : '=';
            case ramses::EKeyCode_Semicolon:
                return isShift ? ':' : ';';
            case ramses::EKeyCode_Apostrophe:
                return isShift ? '@' : '\'';
            case ramses::EKeyCode_Grave:
                return '`';
            case ramses::EKeyCode_NumberSign:
                return isShift ? '~' : '#';
            default:
                break;
            }

            return 0U;
        }

        // emulates an English keyboard for the available ramses key events
        // (ramses (on Windows) does not consider keymaps, but sends virtual key events
        uint32_t GetCharCode(uint32_t keyModifiers, ramses::EKeyCode keyCode)
        {
            const bool isShift = (keyModifiers & ramses::EKeyModifier_Shift) != 0u;
            uint32_t   retval  = 0U;
            if (ramses::EKeyCode_0 <= keyCode && keyCode <= ramses::EKeyCode_9)
            {
                const char* shift = ")!\"$$%^&*(";
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                retval = (isShift) ? shift[keyCode - ramses::EKeyCode_0] : ('0' + keyCode - ramses::EKeyCode_0);
            }
            else if (ramses::EKeyCode_A <= keyCode && keyCode <= ramses::EKeyCode_Z)
            {
                retval = (isShift) ? ('A' + keyCode - ramses::EKeyCode_A) : ('a' + keyCode - ramses::EKeyCode_A);
            }
            else if (ramses::EKeyCode_Numpad_0 <= keyCode && keyCode <= ramses::EKeyCode_Numpad_9)
            {
                retval = ('0' + keyCode - ramses::EKeyCode_Numpad_0);
            }
            else
            {
                retval = GetSpecialChar(keyCode, isShift);
            }
            return retval;
        }
    } // namespace

    ImguiClientHelper::ImguiClientHelper(ramses::RamsesClient& client, uint32_t width, uint32_t height, ramses::sceneId_t sceneid)
    {
        ramses::SceneConfig local;
        m_imguiscene = client.createScene(sceneid, local, "imgui scene");

        imguicamera = m_imguiscene->createOrthographicCamera("imgui camera");

        imguicamera->setFrustum(0.0f, float(width), -float(height), 0.0f, 0.1f, 1.0f);
        imguicamera->setViewport(0, 0, width, height);
        imguicamera->translate(0.0f, 0.0f, 0.5f);
        imguicamera->scale(1.0, -1.0f, 1.0f);

        ramses::RenderPass* renderPass = m_imguiscene->createRenderPass("imgui render pass");
        renderPass->setClearFlags(ramses::EClearFlags_None);
        renderPass->setCamera(*imguicamera);
        renderGroup = m_imguiscene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        ramses::EffectDescription effectDescImgui;
        effectDescImgui.setFragmentShader(ImguiFragShader);
        effectDescImgui.setVertexShader(ImguiVertShader);
        effectDescImgui.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        effect = m_imguiscene->createEffect(effectDescImgui);

        effect->findUniformInput("textureSampler", textureInput);

        effect->findAttributeInput("a_position", inputPosition);
        effect->findAttributeInput("a_texcoord", inputUV);
        effect->findAttributeInput("Color", inputColor);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(width);
        io.DisplaySize.y = static_cast<float>(height);
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        io.MouseDown[0] = false;
        io.MouseDown[1] = false;
        io.MouseDown[2] = false;
        io.MouseDown[3] = false;
        io.MouseDown[4] = false;
        io.KeyMap[ImGuiKey_Tab] = ramses::EKeyCode_Tab;
        io.KeyMap[ImGuiKey_LeftArrow] = ramses::EKeyCode_Left;
        io.KeyMap[ImGuiKey_RightArrow] = ramses::EKeyCode_Right;
        io.KeyMap[ImGuiKey_UpArrow] = ramses::EKeyCode_Up;
        io.KeyMap[ImGuiKey_DownArrow] = ramses::EKeyCode_Down;
        io.KeyMap[ImGuiKey_PageUp] = ramses::EKeyCode_PageUp;
        io.KeyMap[ImGuiKey_PageDown] = ramses::EKeyCode_PageDown;
        io.KeyMap[ImGuiKey_Home] = ramses::EKeyCode_Home;
        io.KeyMap[ImGuiKey_End] = ramses::EKeyCode_End;
        io.KeyMap[ImGuiKey_Insert] = ramses::EKeyCode_Insert;
        io.KeyMap[ImGuiKey_Delete] = ramses::EKeyCode_Delete;
        io.KeyMap[ImGuiKey_Backspace] = ramses::EKeyCode_Backspace;
        io.KeyMap[ImGuiKey_Space] = ramses::EKeyCode_Space;
        io.KeyMap[ImGuiKey_Enter] = ramses::EKeyCode_Return;
        io.KeyMap[ImGuiKey_Escape] = ramses::EKeyCode_Escape;
        io.KeyMap[ImGuiKey_KeyPadEnter] = ramses::EKeyCode_Numpad_Enter;
        io.KeyMap[ImGuiKey_A] = ramses::EKeyCode_A;
        io.KeyMap[ImGuiKey_C] = ramses::EKeyCode_C;
        io.KeyMap[ImGuiKey_V] = ramses::EKeyCode_V;
        io.KeyMap[ImGuiKey_X] = ramses::EKeyCode_X;
        io.KeyMap[ImGuiKey_Y] = ramses::EKeyCode_Y;
        io.KeyMap[ImGuiKey_Z] = ramses::EKeyCode_Z;

        int texturewidth = 0;
        int textureheight = 0;
        unsigned char* pixels = nullptr;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &texturewidth, &textureheight);
        ramses::MipLevelData mipLevelData(static_cast<uint32_t>(texturewidth * textureheight * 4), pixels);
        auto texture = m_imguiscene->createTexture2D(ramses::ETextureFormat::RGBA8, texturewidth, textureheight, 1, &mipLevelData);
        sampler = m_imguiscene->createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
            *texture);

        // At this point you've got the texture data and you need to upload that your your graphic system:
        // After we have created the texture, store its pointer/identifier (_in whichever format your engine uses_) in 'io.Fonts->TexID'.
        // This will be passed back to your via the renderer. Basically ImTextureID == void*. Read FAQ below for details about ImTextureID.
        io.Fonts->TexID = sampler;
        m_imguiscene->publish(ramses::EScenePublicationMode_LocalOnly);
        m_imguiscene->flush();
    }

    ImguiClientHelper::~ImguiClientHelper()
    {
        ImGui::DestroyContext();
    }

    void ImguiClientHelper::setDisplayId(ramses::displayId_t displayId)
    {
        m_displayId = displayId;
    }

    void ImguiClientHelper::draw()
    {
        // cleanup previous frame/objects
        for (auto m : todeleteMeshes)
        {
            m_imguiscene->destroy(*m);
        }
        todeleteMeshes.clear();
        for (auto m : todeleteRes)
        {
            m_imguiscene->destroy(*m);
        }
        todeleteRes.clear();
        renderGroup->removeAllRenderables();

        // let imgui update itself
        ImGui::Render();

        // convert all imgui data to ramses objects
        int meshnr = 1;
        ImDrawData* draw_data = ImGui::GetDrawData();
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) C-style array in 3rd party code. Bounds are checked by loop condition
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
            auto ramsesind = m_imguiscene->createArrayResource(ramses::EDataType::UInt16, cmd_list->IdxBuffer.Size, idx_buffer);
            todeleteRes.push_back(ramsesind);
            unsigned int idx = 0U;
            std::vector <float> positions;
            std::vector <float> uv;
            std::vector <float> color;
            for (auto& v : cmd_list->VtxBuffer)
            {
                positions.push_back(v.pos.x);
                positions.push_back(v.pos.y);
                uv.push_back(v.uv.x);
                uv.push_back(v.uv.y);

                const auto alpha = (v.col >> 24U) & 0xFFU;
                const auto blue  = (v.col >> 16U) & 0xFFU;
                const auto green = (v.col >> 8U) & 0xFFU;
                const auto red   = v.col & 0xFFU;
                color.push_back(static_cast<float>(red) / 255.0f);
                color.push_back(static_cast<float>(green) / 255.0f);
                color.push_back(static_cast<float>(blue) / 255.0f);
                color.push_back(static_cast<float>(alpha) / 255.0f);
            }
            auto ramsespositions = m_imguiscene->createArrayResource(ramses::EDataType::Vector2F, cmd_list->VtxBuffer.Size, positions.data());
            auto ramsesuv = m_imguiscene->createArrayResource(ramses::EDataType::Vector2F, cmd_list->VtxBuffer.Size, uv.data());
            auto ramsescolor = m_imguiscene->createArrayResource(ramses::EDataType::Vector4F, cmd_list->VtxBuffer.Size, color.data());
            todeleteRes.push_back(ramsespositions);
            todeleteRes.push_back(ramsesuv);
            todeleteRes.push_back(ramsescolor);
            auto geobinding = m_imguiscene->createGeometryBinding(*effect);
            todeleteMeshes.push_back(geobinding);

            geobinding->setInputBuffer(inputPosition, *ramsespositions);
            geobinding->setInputBuffer(inputUV, *ramsesuv);
            geobinding->setInputBuffer(inputColor, *ramsescolor);
            geobinding->setIndices(*ramsesind);
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                ramses::Appearance* appearance = m_imguiscene->createAppearance(*effect, "triangle appearance");
                todeleteMeshes.push_back(appearance);
                appearance->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
                appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
                appearance->setDepthFunction(ramses::EDepthFunc_Disabled);
                appearance->setDepthWrite(ramses::EDepthWrite_Disabled);
                appearance->setCullingMode(ramses::ECullMode_Disabled);
                appearance->setInputTexture(textureInput, *static_cast<ramses::TextureSampler*>(pcmd->TextureId));
                auto mesh = m_imguiscene->createMeshNode();
                todeleteMeshes.push_back(mesh);
                mesh->setGeometryBinding(*geobinding);
                mesh->setAppearance(*appearance);
                const ImVec2 pos = draw_data->DisplayPos;
                const ImVec2 displaysize = draw_data->DisplaySize;
                appearance->setScissorTest(ramses::EScissorTest_Enabled,
                                           static_cast<int16_t>(pcmd->ClipRect.x - pos.x),
                                           static_cast<int16_t>(displaysize.y - pcmd->ClipRect.w - pos.y),
                                           static_cast<uint16_t>(pcmd->ClipRect.z - pos.x - pcmd->ClipRect.x - pos.x),
                                           static_cast<uint16_t>(pcmd->ClipRect.w - pos.y - pcmd->ClipRect.y - pos.y));
                mesh->setStartIndex(idx);
                mesh->setIndexCount(pcmd->ElemCount);
                renderGroup->addMeshNode(*mesh, meshnr++);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) C-style array in 3rd party code.
                idx_buffer += pcmd->ElemCount;
                idx += pcmd->ElemCount;
            }
        }

        // apply new version of imgui scene
        m_imguiscene->flush();
    }

    void ImguiClientHelper::sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        m_scenes[sceneId].state = state;
    }

    void ImguiClientHelper::sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion)
    {
        m_scenes[sceneId].version = sceneVersion;
    }

    void ImguiClientHelper::offscreenBufferCreated(ramses::displayId_t /*displayId_t*/, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result)
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_offscreenBuffers.insert(offscreenBufferId);
        }
    }

    void ImguiClientHelper::offscreenBufferLinked(ramses::displayBufferId_t /*offscreenBufferId*/, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t /*consumerId*/, bool success)
    {
        if (success)
        {
            m_scenesConsumingOffscreenBuffer[consumerScene].state = ramses::RendererSceneState::Unavailable;
        }
    }

    void ImguiClientHelper::displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_displays.insert(displayId);
        }
        else
        {
            m_isRunning = false;
        }
    }

    void ImguiClientHelper::displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_displays.erase(displayId);
        }
        else
        {
            m_isRunning = false;
        }
    }

    void ImguiClientHelper::mouseEvent(ramses::displayId_t displayId, ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY)
    {
        if (!m_displayId.isValid() || displayId == m_displayId)
        {
            ImGuiIO& io = ImGui::GetIO();
            io.MousePos = {static_cast<float>(mousePosX), static_cast<float>(mousePosY)};
            switch (eventType)
            {
            case ramses::EMouseEvent_LeftButtonUp:
                io.MouseDown[0] = false;
                m_clickEvent    = {mousePosX, mousePosY};
                break;
            case ramses::EMouseEvent_LeftButtonDown:
                io.MouseDown[0] = true;
                break;
            case ramses::EMouseEvent_WheelUp:
                io.MouseWheel = 1;
                break;
            case ramses::EMouseEvent_WheelDown:
                io.MouseWheel = -1;
                break;
            case ramses::EMouseEvent_RightButtonDown:
                io.MouseDown[1] = true;
                break;
            case ramses::EMouseEvent_RightButtonUp:
                io.MouseDown[1] = false;
                break;

            default:
                break;
            }
        }
    }

    void ImguiClientHelper::keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent eventType, uint32_t keyModifiers, ramses::EKeyCode keyCode)
    {
        if (!m_displayId.isValid() || displayId == m_displayId)
        {
            ImGuiIO&   io        = ImGui::GetIO();
            const bool pressed   = (eventType == ramses::EKeyEvent_Pressed);
            io.KeysDown[keyCode] = pressed;

            switch (keyCode)
            {
            case ramses::EKeyCode_ControlLeft:
            case ramses::EKeyCode_ControlRight:
                io.KeyCtrl = pressed;
                return;
            case ramses::EKeyCode_ShiftLeft:
            case ramses::EKeyCode_ShiftRight:
                io.KeyShift = pressed;
                return;
            case ramses::EKeyCode_AltLeft:
            case ramses::EKeyCode_AltRight:
                io.KeyAlt = pressed;
                return;
            case ramses::EKeyCode_WindowsLeft:
            case ramses::EKeyCode_WindowsRight:
                io.KeySuper = pressed;
                return;
            default:
                break;
            }

            if (pressed)
            {
                const uint32_t charCode = GetCharCode(keyModifiers, keyCode);
                if (charCode != 0U)
                    io.AddInputCharacter(charCode);
            }
        }
    }

    void ImguiClientHelper::windowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height)
    {
        if (!m_displayId.isValid() || displayId == m_displayId)
        {
            imguicamera->setFrustum(0.0f, float(width), -float(height), 0.0f, 0.1f, 1.0f);
            imguicamera->setViewport(0, 0, width, height);
            ImGuiIO& io      = ImGui::GetIO();
            io.DisplaySize.x = static_cast<float>(width);
            io.DisplaySize.y = static_cast<float>(height);
        }
    }

    void ImguiClientHelper::windowClosed(ramses::displayId_t /*displayId*/)
    {
        m_isRunning = false;
    }

    void ImguiClientHelper::framebufferPixelsRead(const uint8_t* pixelData,
                               const uint32_t pixelDataSize,
                               ramses::displayId_t displayId,
                               ramses::displayBufferId_t displayBuffer,
                               ramses::ERendererEventResult result)
    {
        static_cast<void>(displayId);
        static_cast<void>(displayBuffer);
        if (!m_screenshot.empty())
        {
            m_screenshotSaved = false;
            if (result == ramses::ERendererEventResult_OK)
            {
                std::vector<uint8_t> buffer;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                buffer.insert(buffer.end(), &pixelData[0], &pixelData[pixelDataSize]);
                m_screenshotSaved = ramses::RamsesUtils::SaveImageBufferToPng(m_screenshot, buffer, m_screenshotWidth, m_screenshotHeight, true);
            }
            m_screenshot.clear();
        }
    }

    void ImguiClientHelper::dispatchClickEvent(std::pair<uint32_t, uint32_t>& clickEventOut)
    {
        clickEventOut = m_clickEvent;
        m_clickEvent = {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
    }

    void ImguiClientHelper::dispatchEvents()
    {
        m_renderer->dispatchEvents(*this);
        m_renderer->getSceneControlAPI()->dispatchEvents(*this);
    }

    bool ImguiClientHelper::saveScreenshot(const std::string& filename)
    {
        return saveScreenshot(filename, ramses::displayBufferId_t(), 0, 0, imguicamera->getViewportWidth(), imguicamera->getViewportHeight());
    }

    bool ImguiClientHelper::saveScreenshot(const std::string& filename, ramses::displayBufferId_t screenshotBuf, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        if (m_renderer && m_screenshot.empty() && !filename.empty())
        {
            m_screenshotSaved = false;
            m_screenshot = filename;
            m_screenshotWidth = width - x;
            m_screenshotHeight = height - x;
            m_renderer->readPixels(m_displayId, screenshotBuf, x, y, width, height);
            m_renderer->flush();
            return true;
        }
        return false;
    }

    bool ImguiClientHelper::waitForDisplay(ramses::displayId_t displayId)
    {
        return waitUntil([&] { return m_displays.find(displayId) != m_displays.end(); });
    }

    bool ImguiClientHelper:: waitForSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        return waitUntil([&] { return m_scenes[sceneId].state == state; });
    }

    bool ImguiClientHelper::waitForSceneVersion(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t version)
    {
        return waitUntil([&] { return m_scenes[sceneId].version == version; });
    }

    bool ImguiClientHelper::waitForOffscreenBufferCreated(const ramses::displayBufferId_t offscreenBufferId)
    {
        return waitUntil([&] { return m_offscreenBuffers.find(offscreenBufferId) != m_offscreenBuffers.end(); });
    }

    bool ImguiClientHelper::waitForOffscreenBufferLinked(const ramses::sceneId_t sceneId)
    {
        return waitUntil([&] { return m_scenesConsumingOffscreenBuffer.count(sceneId) > 0; });
    }

    bool ImguiClientHelper::waitForScreenshot()
    {
        waitUntil([&] { return m_screenshot.empty(); });
        return m_screenshotSaved;
    }

    bool ImguiClientHelper::waitUntil(const std::function<bool()>& conditionFunction)
    {
        const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{5};
        while (m_isRunning && !conditionFunction() && std::chrono::steady_clock::now() < timeoutTS)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{5}); // will give the renderer time to process changes
            m_renderer->dispatchEvents(*this);
            auto* sceneControl = m_renderer->getSceneControlAPI();
            sceneControl->dispatchEvents(*this);
        }

        return conditionFunction();
    }

}

