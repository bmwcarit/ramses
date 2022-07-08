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

#include "fmt/format.h"


namespace ramses_internal
{
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

            template <class T> void ConvertToRgba(std::vector<uint8_t>& rgbaData, const ResourceBlob& blob, const TextureSwizzleArray& swizzle, const T& readPixel)
            {
                const auto* end = blob.data() + blob.size();
                const auto* buf = blob.data();
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

            constexpr auto white{IM_COL32(255, 255, 255, 255)};
            constexpr auto grey{IM_COL32(200, 200, 200, 255)};
            constexpr auto green{IM_COL32(15, 200, 19, 255)};
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
                const auto&                             blob            = textureResource->getResourceData();
                std::vector<uint8_t>                    imageData;
                const auto byteSize = static_cast<size_t>(resource->getWidth()) * resource->getHeight() * 4u;
                imageData.reserve(byteSize);
                if (resource->getTextureFormat() == ETextureFormat::RGBA8)
                {
                    // just copy the blob
                    imageData.insert(imageData.end(), blob.data(), blob.data() + blob.size());
                }
                else
                {
                    // convert to rgba
                    const auto swizzle = resource->getTextureSwizzle();
                    switch (resource->getTextureFormat())
                    {
                    case ETextureFormat::R8:
                        ConvertToRgba(imageData, blob, swizzle, [](const Byte* ptr, std::array<uint8_t, 4>& rgba) {
                            rgba[0] = *ptr++;
                            return ptr;
                        });
                        break;
                    case ETextureFormat::RG8:
                        ConvertToRgba(imageData, blob, swizzle, [](const Byte* ptr, std::array<uint8_t, 4>& rgba) {
                            rgba[0] = *ptr++;
                            rgba[1] = *ptr++;
                            return ptr;
                        });
                        break;
                    case ETextureFormat::RGB8:
                        ConvertToRgba(imageData, blob, swizzle, [](const Byte* ptr, std::array<uint8_t, 4>& rgba) {
                            rgba[0] = *ptr++;
                            rgba[1] = *ptr++;
                            rgba[2] = *ptr++;
                            return ptr;
                        });
                        break;
                    default:
                        errorMsg = fmt::format("Cannot save: {} (Image format is not supported: {}).", filename, EnumToString(resource->getTextureFormat()));
                        break;
                    }
                }

                if (imageData.size() > byteSize)
                {
                    // only save the first miplevel
                    imageData.resize(byteSize);
                }

                if (errorMsg.empty() && !ramses::RamsesUtils::SaveImageBufferToPng(filename, imageData, resource->getWidth(), resource->getHeight()))
                {
                    errorMsg = fmt::format("Cannot save: {}", filename);
                }
            }
            return errorMsg;
        }
    } // namespace imgui
} // namespace ramses_internal
