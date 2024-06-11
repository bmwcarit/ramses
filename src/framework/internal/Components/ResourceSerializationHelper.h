//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/EResourceCompressionStatus.h"
#include "internal/SceneGraph/Resource/IResource.h"
#include "ramses/framework/EFeatureLevel.h"
#include <cstdint>

namespace ramses::internal
{
    class IOutputStream;
    class IInputStream;

    // TODO(tobias) must add standalone tests for these functions
    namespace ResourceSerializationHelper
    {
        struct DeserializedResourceHeader
        {
            std::unique_ptr<IResource> resource;
            EResourceCompressionStatus compressionStatus = EResourceCompressionStatus::Uncompressed;
            uint32_t decompressedSize = 0;
            uint32_t compressedSize = 0;
        };

        void SerializeResourceMetadata(IOutputStream& output, const IResource& resource);
        uint32_t ResourceMetadataSize(const IResource& resource);

        DeserializedResourceHeader ResourceFromMetadataStream(IInputStream& input, EFeatureLevel featureLevel);
    }
}
