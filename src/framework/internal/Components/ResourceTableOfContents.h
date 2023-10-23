//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "ResourcePersistation.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "internal/SceneGraph/Resource/ResourceInfo.h"

namespace ramses::internal
{
    struct ResourceFileEntry
    {
        uint32_t offsetInBytes = 0;
        uint32_t sizeInBytes = 0;
        ResourceInfo resourceInfo;
    };

    using TableOfContentsMap = HashMap<ResourceContentHash, ResourceFileEntry>;

    class ResourceTableOfContents
    {
    public:
        [[nodiscard]] bool containsResource(const ResourceContentHash& hash) const;
        void registerContents(const ResourceInfo& info, uint32_t offsetInBytes, uint32_t sizeInBytes);
        [[nodiscard]] const ResourceFileEntry& getEntryForHash(const ResourceContentHash& hash) const;
        [[nodiscard]] const TableOfContentsMap& getFileContents() const;
        bool readTOCPosAndTOCFromStream(IInputStream& instream);
        void writeTOCToStream(IOutputStream& outstream);

    private:
        TableOfContentsMap m_fileContents;
    };
}
