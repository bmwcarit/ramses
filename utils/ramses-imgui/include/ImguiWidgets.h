//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#include "absl/types/span.h"
#include "SceneAPI/TextureEnums.h"
#include "Resource/TextureMetaInfo.h"
#include <string>
#include "SceneAPI/Handles.h"


namespace ramses
{
    class Scene;
    class TextureSampler;
}

namespace ramses_internal
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

        std::string SaveToPng(const Byte*                data,
                              UInt                       size,
                              ETextureFormat             fmt,
                              uint32_t                   width,
                              uint32_t                   height,
                              const std::string&         filename,
                              const TextureSwizzleArray& swizzle = DefaultTextureSwizzleArray);

        std::string SaveTextureToPng(const TextureResource* resource, const std::string& filename);
    }
}

