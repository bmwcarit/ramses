//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IMGUICLIENTHELPER_H
#define RAMSES_IMGUICLIENTHELPER_H

#include "imgui.h"
#include "ramses-client.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include <unordered_set>
#include <vector>
#include "absl/types/span.h"

#include "misc/cpp/imgui_stdlib.h"
#include "fmt/format.h"

namespace ramses_internal
{
    struct ImguiComboBoxEntry
    {
        std::string preview;
        uint64_t contentId;
    };

    inline void ImguiEditableComboBox(const char* descriptionText, std::string* inputString, uint64_t& value, std::string& currentComboBoxString, absl::Span < const ImguiComboBoxEntry> comboboxItems)
    {
        bool valid = true;
        try {
            if (std::stoi(*inputString) > 0)
                valid = true;
        }
        catch (const std::out_of_range&)
        {
            valid = false;
        }
        catch (const std::invalid_argument&)
        {
            valid = false;
            value = 0;
        }
        for (size_t n = 0; n < comboboxItems.size(); n++)
        {
            if (value == comboboxItems[n].contentId)
            {
                *inputString = std::to_string(comboboxItems[n].contentId);
                currentComboBoxString = comboboxItems[n].preview;
                break;
            }
        }
        ImGuiStyle& style = ImGui::GetStyle();
        float spacing = style.ItemInnerSpacing.x;
        ImGui::PushItemWidth(100);
        // change color to red if inputbox not convertable to int
        if (!valid)
        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 1.0f, 0.3f, 0.4f, 1.0f });
        }
        if (ImGui::InputText(fmt::format("##inputtext{}",descriptionText).c_str(), inputString, ImGuiInputTextFlags_CharsDecimal))
        {
            try {
                value = std::stoi(*inputString);
            }
            catch (const std::out_of_range&)
            {
                value = 0;
            }
            catch (const std::invalid_argument&)
            {
                value = 0;
            }
            // switch combo to 'custom'
            currentComboBoxString = "Custom";
        }
        if (!valid)
            ImGui::PopStyleColor(1);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f - 100);
        ImGui::SameLine(0, spacing);
        if (ImGui::BeginCombo(fmt::format("##custom combo{}", descriptionText).c_str(), currentComboBoxString.c_str()))
        {
            for (size_t n = 0; n < comboboxItems.size(); n++)
            {
                bool is_selected = (currentComboBoxString == comboboxItems[n].preview);
                if (ImGui::Selectable(comboboxItems[n].preview.c_str(), is_selected))
                {
                    currentComboBoxString = comboboxItems[n].preview;
                    *inputString = std::to_string(comboboxItems[n].contentId);
                    value = std::stoi(*inputString);
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.35f);
        ImGui::SameLine(0, spacing);
        ImGui::Text("%s",descriptionText);
        ImGui::PopItemWidth();
    }

    inline bool ImguiToggleButton(const char* name, bool& value)
    {
        bool valueHasChanged = false;

        const float height = ImGui::GetFrameHeight();
        const float width = height * 2.0f;
        const float radius = height * 0.5f;
        const ImVec2 screenPosition = ImGui::GetCursorScreenPos();

        ImGui::InvisibleButton(name, ImVec2(width, height));
        if (ImGui::IsItemClicked())
        {
            value = !value;
            valueHasChanged = true;
        }

        constexpr auto white { IM_COL32(255, 255, 255, 255) };
        constexpr auto grey  { IM_COL32(200, 200, 200, 255) };
        constexpr auto green { IM_COL32(15, 200, 19, 255) };
        ImU32 backgroundColor;
        float offsetOnPosition = 0;
        if (value)
        {
            backgroundColor = green;
            offsetOnPosition = width - radius * 2.0f;
        }
        else
        {
            backgroundColor = grey;
        }


        const ImVec2 circlePosition = ImVec2(screenPosition.x + radius, screenPosition.y + radius);
        ImGui::GetWindowDrawList()->AddRectFilled(screenPosition, ImVec2(screenPosition.x + width, screenPosition.y + height), backgroundColor, height * 0.5f);
        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(circlePosition.x + offsetOnPosition, circlePosition.y), radius * 0.9f, white);

        return valueHasChanged;
    }

    static const char* ImguiFragShader = R"GLSL(
        #version 100
        uniform sampler2D textureSampler;

        varying lowp vec2 v_texcoord;
        varying lowp vec4 Frag_Color;
        void main(void)
        {
            gl_FragColor = texture2D(textureSampler, v_texcoord.st)*Frag_Color;
        })GLSL";

    static const char* ImguiVertShader = R"GLSL(
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

    class ImguiClientHelper : public ramses::RendererEventHandlerEmpty
    {
    public:
        ImguiClientHelper(ramses::RamsesClient& client, uint32_t width, uint32_t height, ramses::sceneId_t sceneid)
        {
            ramses::SceneConfig local;
            local.setPublicationMode(ramses::EScenePublicationMode_LocalOnly);
            m_imguiscene = client.createScene(sceneid, local, "dcsm example scene");

            imguicamera = m_imguiscene->createOrthographicCamera("my camera");

            imguicamera->setFrustum(0.0f, float(width), -float(height), 0.0f, 0.1f, 1.0f);
            imguicamera->setViewport(0, 0, width, height);
            imguicamera->translate(0.0f, 0.0f, 0.5f);
            imguicamera->scale(1.0, -1.0f, 1.0f);

            ramses::RenderPass* renderPass = m_imguiscene->createRenderPass("my render pass");
            renderPass->setClearFlags(ramses::EClearFlags_None);
            renderPass->setCamera(*imguicamera);
            renderGroup = m_imguiscene->createRenderGroup();
            renderPass->addRenderGroup(*renderGroup);
            ramses::EffectDescription effectDescImgui;
            effectDescImgui.setFragmentShader(ImguiFragShader);
            effectDescImgui.setVertexShader(ImguiVertShader);
            effectDescImgui.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

            effect = m_imguiscene->createEffect(effectDescImgui, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

            effect->findUniformInput("textureSampler", textureInput);

            effect->findAttributeInput("a_position", inputPosition);
            effect->findAttributeInput("a_texcoord", inputUV);
            effect->findAttributeInput("Color", inputColor);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize.x = static_cast<float>(width);
            io.DisplaySize.y = static_cast<float>(height);
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
            for (auto& mouseDown : absl::Span<bool>(io.MouseDown))
                mouseDown = false;
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

        /**
         * @brief Used to filter input events for a certain display only
         * @param[in] displayId The display to receive events for - invalid displayId handles all events
         */
        void setDisplayId(ramses::displayId_t displayId)
        {
            m_displayId = displayId;
        }

        void draw()
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
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
                auto ramsesind = m_imguiscene->createArrayResource(ramses::EDataType::UInt16, cmd_list->IdxBuffer.Size, idx_buffer);
                int idx = 0;
                std::vector <float> positions;
                std::vector <float> uv;
                std::vector <float> color;
                for (auto& v : cmd_list->VtxBuffer)
                {
                    positions.push_back(v.pos.x);
                    positions.push_back(v.pos.y);
                    uv.push_back(v.uv.x);
                    uv.push_back(v.uv.y);
                    uint8_t someByteArray[4];

                    someByteArray[0] = (v.col >> 24) & 0xFF;
                    someByteArray[1] = (v.col >> 16) & 0xFF;
                    someByteArray[2] = (v.col >> 8) & 0xFF;
                    someByteArray[3] = v.col & 0xFF;
                    color.push_back(someByteArray[3] / 255.0f);
                    color.push_back(someByteArray[2] / 255.0f);
                    color.push_back(someByteArray[1] / 255.0f);
                    color.push_back(someByteArray[0] / 255.0f);
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
                    appearance->setScissorTest(ramses::EScissorTest_Enabled, static_cast<int16_t>(pcmd->ClipRect.x - pos.x), static_cast<int16_t>(displaysize.y - pcmd->ClipRect.w - pos.y), static_cast<uint16_t>(pcmd->ClipRect.z - pos.x - pcmd->ClipRect.x - pos.x), static_cast<uint16_t>(pcmd->ClipRect.w - pos.y - pcmd->ClipRect.y - pos.y));
                    mesh->setStartIndex(idx);
                    mesh->setIndexCount(pcmd->ElemCount);
                    renderGroup->addMeshNode(*mesh, meshnr++);
                    idx_buffer += pcmd->ElemCount;
                    idx += pcmd->ElemCount;
                }
            }

            // apply new version of imgui scene
            m_imguiscene->flush();
        }

        virtual void mouseEvent(ramses::displayId_t displayId, ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override
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

        virtual void keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent eventType, uint32_t keyModifiers, ramses::EKeyCode keyCode) override
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

        virtual void windowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height) override
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

        // emulates an English keyboard for the available ramses key events
        // (ramses (on Windows) does not consider keymaps, but sends virtual key events
        static uint32_t GetCharCode(uint32_t keyModifiers, ramses::EKeyCode keyCode)
        {
            if (ramses::EKeyCode_0 <= keyCode && keyCode <= ramses::EKeyCode_9)
            {
                if (keyModifiers & ramses::EKeyModifier_Shift)
                {
                    const char* shift = ")!\"$$%^&*(";
                    return shift[keyCode - ramses::EKeyCode_0];
                }
                else
                    return ('0' + keyCode - ramses::EKeyCode_0);
            }
            else if (ramses::EKeyCode_A <= keyCode && keyCode <= ramses::EKeyCode_Z)
            {
                if (keyModifiers & ramses::EKeyModifier_Shift)
                {
                    return ('A' + keyCode - ramses::EKeyCode_A);
                }
                else
                {
                    return ('a' + keyCode - ramses::EKeyCode_A);
                }
            }
            else if (ramses::EKeyCode_Numpad_0 <= keyCode && keyCode <= ramses::EKeyCode_Numpad_9)
            {
                return ('0' + keyCode - ramses::EKeyCode_Numpad_0);
            }
            else
            {
                switch (keyCode)
                {
                case ramses::EKeyCode_Space:
                    return ' ';
                case ramses::EKeyCode_Numpad_Subtract:
                case ramses::EKeyCode_Minus:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '_' : '-';
                case ramses::EKeyCode_Slash:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '?' : '/';
                case ramses::EKeyCode_Numpad_Decimal:
                    return '.';
                case ramses::EKeyCode_Period:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '>' : '.';
                case ramses::EKeyCode_Comma:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '<' : ',';
                case ramses::EKeyCode_Backslash:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '|' : '\\';
                case ramses::EKeyCode_LeftBracket:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '{' : '[';
                case ramses::EKeyCode_RightBracket:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '}' : ']';
                case ramses::EKeyCode_Equals:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '+' : '=';
                case ramses::EKeyCode_Semicolon:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? ':' : ';';
                case ramses::EKeyCode_Apostrophe:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '@' : '\'';
                case ramses::EKeyCode_Grave:
                    return '`';
                case ramses::EKeyCode_NumberSign:
                    return (keyModifiers & ramses::EKeyModifier_Shift) ? '~' : '#';
                default:
                    break;
                }
            }
            return 0;
        }

        void dispatchClickEvent(std::pair<uint32_t, uint32_t>& clickEventOut)
        {
            clickEventOut = m_clickEvent;
            m_clickEvent = {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
        }

        ramses::Scene* getScene()
        {
            return m_imguiscene;
        }

    private:
        ramses::displayId_t m_displayId;
        ramses::Scene* m_imguiscene = nullptr;
        ramses::OrthographicCamera* imguicamera = nullptr;
        ramses::TextureSampler* sampler = nullptr;
        ramses::Effect* effect = nullptr;
        ramses::RenderGroup* renderGroup = nullptr;
        ramses::UniformInput textureInput;
        ramses::AttributeInput inputPosition;
        ramses::AttributeInput inputUV;
        ramses::AttributeInput inputColor;
        std::vector<ramses::SceneObject*> todeleteMeshes;
        std::vector<ramses::Resource*> todeleteRes;
        std::pair<uint32_t, uint32_t> m_clickEvent;
    };
}

#endif
