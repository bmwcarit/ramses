//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEUPDATESERIALIZATIONHELPER_H
#define RAMSES_SCENEUPDATESERIALIZATIONHELPER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "absl/types/span.h"
#include <vector>
#include <memory>

namespace ramses_internal
{
    class SceneActionCollection;
    class IResource;
    struct FlushInformation;

    namespace SceneActionSerialization
    {
        absl::Span<const Byte> SerializeDescription(const SceneActionCollection& actions, std::vector<Byte>& workingMemory);
        absl::Span<const Byte> SerializeData(const SceneActionCollection& actions);

        SceneActionCollection Deserialize(absl::Span<const Byte> description, absl::Span<const Byte> data);
    };

    namespace ResourceSerialization
    {
        absl::Span<const Byte> SerializeDescription(const IResource& resource, std::vector<Byte>& workingMemory);
        absl::Span<const Byte> SerializeData(const IResource& resource);

        std::unique_ptr<IResource> Deserialize(absl::Span<const Byte> description, absl::Span<const Byte> data);
    }

    namespace FlushInformationSerialization
    {
        absl::Span<const Byte> SerializeInfos(const FlushInformation& flushInfos, std::vector<Byte>& workingMemory);
        FlushInformation Deserialize(absl::Span<const Byte> flushInfoBlob);
    }
}

#endif
