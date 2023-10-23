//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "absl/types/span.h"

#include <cstdint>
#include <vector>
#include <memory>

namespace ramses::internal
{
    class SceneActionCollection;
    class IResource;
    struct FlushInformation;

    namespace SceneActionSerialization
    {
        absl::Span<const std::byte> SerializeDescription(const SceneActionCollection& actions, std::vector<std::byte>& workingMemory);
        absl::Span<const std::byte> SerializeData(const SceneActionCollection& actions);

        SceneActionCollection Deserialize(absl::Span<const std::byte> description, absl::Span<const std::byte> data);
    };

    namespace ResourceSerialization
    {
        absl::Span<const std::byte> SerializeDescription(const IResource& resource, std::vector<std::byte>& workingMemory);
        absl::Span<const std::byte> SerializeData(const IResource& resource);

        std::unique_ptr<IResource> Deserialize(absl::Span<const std::byte> description, absl::Span<const std::byte> data);
    }

    namespace FlushInformationSerialization
    {
        absl::Span<const std::byte> SerializeInfos(const FlushInformation& flushInfos, std::vector<std::byte>& workingMemory);
        FlushInformation Deserialize(absl::Span<const std::byte> flushInfoBlob);
    }
}
