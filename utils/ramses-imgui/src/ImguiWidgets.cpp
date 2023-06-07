//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ImguiWidgets.h"

#include "ramses-client.h"
#include "ramses-utils.h"
#include "Resource/TextureResource.h"
#include "TextureUtils.h"
#include "AppearanceEnumsImpl.h"

#include "fmt/format.h"

namespace ramses_internal
{
    // TODO Find a better solution to this. For now, needed to suppress cast warnings
    WARNINGS_PUSH
    WARNING_DISABLE_LINUX(-Wold-style-cast)
    constexpr auto white{IM_COL32(255u, 255u, 255u, 255u)};
    constexpr auto grey{IM_COL32(200u, 200u, 200u, 255u)};
    constexpr auto green{IM_COL32(15u, 200u, 19u, 255u)};
    WARNINGS_POP

    using ramses::BlendFactorNames;
    using ramses::DepthFuncNames;
    using ramses::BlendOperationNames;
    using ramses::DepthWriteNames;
    using ramses::ScissorTestNames;
    using ramses::StencilFuncNames;
    using ramses::StencilOperationNames;
    using ramses::CullModeNames;
    using ramses::DrawModeNames;

    namespace imgui
    {
        namespace
        {
            uint8_t GetChannelColor(const std::array<uint8_t, 4>& rgba, ETextureChannelColor channelColor)
            {
                switch (channelColor)
                {
                case ETextureChannelColor::Red:
                    return rgba[0];
                case ETextureChannelColor::Green:
                    return rgba[1];
                case ETextureChannelColor::Blue:
                    return rgba[2];
                case ETextureChannelColor::Alpha:
                    return rgba[3];
                case ETextureChannelColor::One:
                    return 255;
                case ETextureChannelColor::Zero:
                case ETextureChannelColor::NUMBER_OF_ELEMENTS:
                    return 0;
                }
                return 0;
            }

            void ApplySwizzle(std::array<uint8_t, 4>& rgba, const ramses_internal::TextureSwizzleArray& swizzle)
            {
                const auto r = GetChannelColor(rgba, swizzle[0]);
                const auto g = GetChannelColor(rgba, swizzle[1]);
                const auto b = GetChannelColor(rgba, swizzle[2]);
                const auto a = GetChannelColor(rgba, swizzle[3]);
                rgba         = {r, g, b, a};
            }

            template <class T> void ConvertToRgba(std::vector<uint8_t>& rgbaData, const Byte* data, const size_t size, const TextureSwizzleArray& swizzle, const T& readPixel)
            {
                const auto* end = data + size;
                const auto* buf = data;
                while (buf < end)
                {
                    std::array<uint8_t, 4> rgba = {0, 0, 0, 255};
                    buf                         = readPixel(buf, rgba);
                    ApplySwizzle(rgba, swizzle);
                    rgbaData.insert(rgbaData.end(), rgba.begin(), rgba.end());
                }
            }
        } // namespace

        void EditableComboBox(
            const char* descriptionText, std::string* inputString, uint64_t& value, std::string& currentComboBoxString, absl::Span<const ComboBoxEntry> comboboxItems)
        {
            bool valid = true;
            try
            {
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
                    *inputString          = std::to_string(comboboxItems[n].contentId);
                    currentComboBoxString = comboboxItems[n].preview;
                    break;
                }
            }
            ImGuiStyle& style   = ImGui::GetStyle();
            float       spacing = style.ItemInnerSpacing.x;
            ImGui::PushItemWidth(100);
            // change color to red if inputbox not convertable to int
            if (!valid)
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{1.0f, 0.3f, 0.4f, 1.0f});
            }
            if (ImGui::InputText(fmt::format("##inputtext{}", descriptionText).c_str(), inputString, ImGuiInputTextFlags_CharsDecimal))
            {
                try
                {
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
                        *inputString          = std::to_string(comboboxItems[n].contentId);
                        value                 = std::stoi(*inputString);
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
            ImGui::Text("%s", descriptionText);
            ImGui::PopItemWidth();
        }

        bool ToggleButton(const char* name, bool& value)
        {
            bool valueHasChanged = false;

            const float  height         = ImGui::GetFrameHeight();
            const float  width          = height * 2.0f;
            const float  radius         = height * 0.5f;
            const ImVec2 screenPosition = ImGui::GetCursorScreenPos();

            ImGui::InvisibleButton(name, ImVec2(width, height));
            if (ImGui::IsItemClicked())
            {
                value           = !value;
                valueHasChanged = true;
            }

            ImU32          backgroundColor;
            float          offsetOnPosition = 0;
            if (value)
            {
                backgroundColor  = green;
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

        void PreviewImage(const Image& img, const ImVec2& size)
        {
            if (img.sampler != nullptr)
            {
                if (ImGui::ImageButton(img.sampler, size))
                    ImGui::OpenPopup("image_full");
                if (ImGui::BeginPopup("image_full", ImGuiWindowFlags_HorizontalScrollbar))
                {
                    ImGui::Image(img.sampler, ImVec2(static_cast<float>(img.width), static_cast<float>(img.height)));
                    ImGui::EndPopup();
                }
            }
        }

        void RenderState(IScene& scene, RenderStateHandle hnd)
        {
            assert(scene.isRenderStateAllocated(hnd));

            auto& obj = scene.getRenderState(hnd);
            if (ImGui::TreeNode("Blending"))
            {
                std::array rgba = {obj.blendColor.r, obj.blendColor.g, obj.blendColor.b, obj.blendColor.a};
                if (ImGui::ColorEdit4("BlendingColor", rgba.data()))
                    scene.setRenderStateBlendColor(hnd, {rgba[0], rgba[1], rgba[2], rgba[3]});

                int srcColor  = static_cast<int>(obj.blendFactorSrcColor);
                int destColor = static_cast<int>(obj.blendFactorDstColor);
                int srcAlpha  = static_cast<int>(obj.blendFactorSrcAlpha);
                int destAlpha = static_cast<int>(obj.blendFactorDstAlpha);
                auto setBlendFactors = [&]() {
                    scene.setRenderStateBlendFactors(hnd,
                                                      static_cast<EBlendFactor>(srcColor),
                                                      static_cast<EBlendFactor>(destColor),
                                                      static_cast<EBlendFactor>(srcAlpha),
                                                      static_cast<EBlendFactor>(destAlpha));
                };
                if (ImGui::Combo("srcColor", &srcColor, BlendFactorNames.data(), static_cast<int>(BlendFactorNames.size())))
                    setBlendFactors();
                if (ImGui::Combo("destColor", &destColor, BlendFactorNames.data(), static_cast<int>(BlendFactorNames.size())))
                    setBlendFactors();
                if (ImGui::Combo("srcAlpha", &srcAlpha, BlendFactorNames.data(), static_cast<int>(BlendFactorNames.size())))
                    setBlendFactors();
                if (ImGui::Combo("destAlpha", &destAlpha, BlendFactorNames.data(), static_cast<int>(BlendFactorNames.size())))
                    setBlendFactors();

                int blendingOperationColor = static_cast<int>(obj.blendOperationColor);
                int blendingOperationAlpha = static_cast<int>(obj.blendOperationAlpha);
                auto setBlendOperations = [&]() {
                    scene.setRenderStateBlendOperations(
                        hnd, static_cast<EBlendOperation>(blendingOperationColor), static_cast<EBlendOperation>(blendingOperationAlpha));
                };
                if (ImGui::Combo("colorOperation", &blendingOperationColor, BlendOperationNames.data(), static_cast<int>(BlendOperationNames.size())))
                    setBlendOperations();
                if (ImGui::Combo("alphaOperation", &blendingOperationAlpha, BlendOperationNames.data(), static_cast<int>(BlendOperationNames.size())))
                    setBlendOperations();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Depth"))
            {
                int depthFunc = static_cast<int>(obj.depthFunc);
                if (ImGui::Combo("depthFunc", &depthFunc, DepthFuncNames.data(), static_cast<int>(DepthFuncNames.size())))
                    scene.setRenderStateDepthFunc(hnd, static_cast<EDepthFunc>(depthFunc));
                int depthWrite = static_cast<int>(obj.depthWrite);
                if (ImGui::Combo("depthWrite", &depthWrite, DepthWriteNames.data(), static_cast<int>(DepthFuncNames.size())))
                    scene.setRenderStateDepthWrite(hnd, static_cast<EDepthWrite>(depthWrite));
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Scissor"))
            {
                int mode = static_cast<int>(obj.scissorTest);
                std::array<int, 4> xywh = {
                    obj.scissorRegion.x,
                    obj.scissorRegion.y,
                    obj.scissorRegion.width,
                    obj.scissorRegion.height
                };
                auto setScissorTest = [&]() {
                    scene.setRenderStateScissorTest(hnd, static_cast<EScissorTest>(mode),
                        {static_cast<int16_t>(xywh[0]), static_cast<int16_t>(xywh[1]), static_cast<uint16_t>(xywh[2]), static_cast<uint16_t>(xywh[3])});
                };
                if (ImGui::Combo("scissorTest", &mode, ScissorTestNames.data(), static_cast<int>(ScissorTestNames.size())))
                    setScissorTest();
                if (ImGui::DragInt4("Region", xywh.data()))
                    setScissorTest();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Stencil"))
            {
                int func = static_cast<int>(obj.stencilFunc);
                std::array<int, 2> refMask = {
                    obj.stencilRefValue,
                    obj.stencilMask
                };
                auto setStencilFunc = [&]() {
                    scene.setRenderStateStencilFunc(hnd, static_cast<EStencilFunc>(func), static_cast<uint8_t>(refMask[0]), static_cast<uint8_t>(refMask[1]));
                };
                if (ImGui::Combo("stencilFunc", &func, StencilFuncNames.data(), static_cast<int>(StencilFuncNames.size())))
                    setStencilFunc();
                if (ImGui::DragInt2("RefValue, Mask", refMask.data()))
                    setStencilFunc();

                int sfail = static_cast<int>(obj.stencilOpFail);
                int dpfail = static_cast<int>(obj.stencilOpDepthFail);
                int dppass = static_cast<int>(obj.stencilOpDepthPass);
                auto setStencilOps = [&]() {
                    scene.setRenderStateStencilOps(hnd, static_cast<EStencilOp>(sfail), static_cast<EStencilOp>(dpfail), static_cast<EStencilOp>(dppass));
                };
                if (ImGui::Combo("fail operation", &sfail, StencilOperationNames.data(), static_cast<int>(StencilOperationNames.size())))
                    setStencilOps();
                if (ImGui::Combo("depth fail operation", &dpfail, StencilOperationNames.data(), static_cast<int>(StencilOperationNames.size())))
                    setStencilOps();
                if (ImGui::Combo("depth pass operation", &dppass, StencilOperationNames.data(), static_cast<int>(StencilOperationNames.size())))
                    setStencilOps();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("DrawMode"))
            {
                int culling = static_cast<int>(obj.cullMode);
                if (ImGui::Combo("Culling", &culling, CullModeNames.data(), static_cast<int>(CullModeNames.size())))
                    scene.setRenderStateCullMode(hnd, static_cast<ECullMode>(culling));
                int drawMode = static_cast<int>(obj.drawMode);
                if (ImGui::Combo("DrawMode", &drawMode, DrawModeNames.data(), static_cast<int>(DrawModeNames.size())))
                    scene.setRenderStateDrawMode(hnd, static_cast<EDrawMode>(drawMode));
                uint32_t colorFlags = obj.colorWriteMask;
                ImGui::Text("ColorWriteMask");
                auto setColorWriteMask = [&]() { scene.setRenderStateColorWriteMask(hnd, static_cast<uint8_t>(colorFlags)); };
                if (ImGui::CheckboxFlags("Red", &colorFlags, EColorWriteFlag_Red))
                    setColorWriteMask();
                if (ImGui::CheckboxFlags("Green", &colorFlags, EColorWriteFlag_Green))
                    setColorWriteMask();
                if (ImGui::CheckboxFlags("Blue", &colorFlags, EColorWriteFlag_Blue))
                    setColorWriteMask();
                if (ImGui::CheckboxFlags("Alpha", &colorFlags, EColorWriteFlag_Alpha))
                    setColorWriteMask();
                ImGui::TreePop();
            }
        }

        std::string SaveToPng(const Byte* data, size_t size, ETextureFormat fmt, uint32_t width, uint32_t height, const std::string& filename, const TextureSwizzleArray& swizzle)
        {
            std::string errorMsg;
            std::vector<uint8_t> imageData;
            const auto byteSize = width * height * 4u;
            imageData.reserve(byteSize);
            if (fmt == ETextureFormat::RGBA8)
            {
                // just copy the blob
                imageData.insert(imageData.end(), data, data + size);
            }
            else
            {
                // convert to rgba
                switch (fmt)
                {
                case ETextureFormat::R8:
                    ConvertToRgba(imageData, data, size, swizzle, [](const Byte* ptr, std::array<uint8_t, 4>& rgba) {
                        rgba[0] = *ptr++;
                        return ptr;
                    });
                    break;
                case ETextureFormat::RG8:
                    ConvertToRgba(imageData, data, size, swizzle, [](const Byte* ptr, std::array<uint8_t, 4>& rgba) {
                        rgba[0] = *ptr++;
                        rgba[1] = *ptr++;
                        return ptr;
                    });
                    break;
                case ETextureFormat::RGB8:
                    ConvertToRgba(imageData, data, size, swizzle, [](const Byte* ptr, std::array<uint8_t, 4>& rgba) {
                        rgba[0] = *ptr++;
                        rgba[1] = *ptr++;
                        rgba[2] = *ptr++;
                        return ptr;
                    });
                    break;
                default:
                    errorMsg = fmt::format("Cannot save: {} (Image format is not supported: {}).", filename, EnumToString(fmt));
                    break;
                }
            }

            if (imageData.size() > byteSize)
            {
                // only save the first miplevel
                imageData.resize(byteSize);
            }

            if (errorMsg.empty() && !ramses::RamsesUtils::SaveImageBufferToPng(filename, imageData, width, height))
            {
                errorMsg = fmt::format("Cannot save: {}", filename);
            }

            return errorMsg;
        }

        std::string SaveTextureToPng(const TextureResource* resource, const std::string& filename)
        {
            std::string errorMsg;
            if (resource)
            {
                if (resource->isCompressedAvailable() && !resource->isDeCompressedAvailable())
                {
                    resource->decompress();
                }
                const ramses_internal::TextureResource* textureResource = resource->convertTo<ramses_internal::TextureResource>();
                const auto& blob = textureResource->getResourceData();
                errorMsg = SaveToPng(blob.data(), blob.size(), resource->getTextureFormat(), resource->getWidth(), resource->getHeight(), filename, resource->getTextureSwizzle());
            }
            return errorMsg;
        }
    } // namespace imgui
} // namespace ramses_internal
