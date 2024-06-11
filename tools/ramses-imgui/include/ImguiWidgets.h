//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/Warnings.h"

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Wold-style-cast)

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

WARNINGS_POP

#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/Resource/TextureMetaInfo.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include "absl/types/span.h"
#include "fmt/format.h"

#include <string>


namespace ramses
{
    class Scene;
    class TextureSampler;
}

namespace ramses::internal
{
    class TextureResource;
    class IScene;

    namespace imgui
    {
        struct ComboBoxEntry
        {
            std::string preview;
            uint64_t    contentId;
        };

        void
        EditableComboBox(const char* descriptionText, std::string* inputString, uint64_t& value, std::string& currentComboBoxString, absl::Span<const ComboBoxEntry> comboboxItems);

        bool ToggleButton(const char* name, bool& value);

        struct Image
        {
            ramses::TextureSampler* sampler;
            uint32_t                width;
            uint32_t                height;
        };

        void PreviewImage(const Image& img, const ImVec2& size);

        void RenderState(IScene& scene, RenderStateHandle hnd);

        std::string SaveToPng(const uint8_t*             data,
                              size_t                     size,
                              EPixelStorageFormat        fmt,
                              uint32_t                   width,
                              uint32_t                   height,
                              const std::string&         filename,
                              const TextureSwizzleArray& swizzle = DefaultTextureSwizzleArray);

        std::string SaveTextureToPng(const TextureResource* resource, const std::string& filename);

        template <typename... Args>
        void HelpMarker(const char* desc, Args&& ... args)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(fmt::format(desc, args...).c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }
    }
}

