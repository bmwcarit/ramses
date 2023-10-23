//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/ActionCollectingScene.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"

namespace ramses::internal
{
    class DataLayoutCachedScene : public ActionCollectingScene
    {
    public:
        explicit DataLayoutCachedScene(const SceneInfo& sceneInfo = SceneInfo());

        DataLayoutHandle            allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle) override;
        void                        releaseDataLayout(DataLayoutHandle handle) override;

        [[nodiscard]] uint32_t      getNumDataLayoutReferences(DataLayoutHandle handle) const;

    private:
        struct DataLayoutCacheEntry
        {
            ResourceContentHash m_effectHash;
            DataFieldInfoVector m_dataFields;
            uint32_t              m_usageCount = 0u;
        };

        DataLayoutHandle allocateAndCacheDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle);
        DataLayoutHandle findDataLayoutEntry(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutCacheEntry*& entryOut);

        // Cache entries are stored in groups, each group has data layouts with same number of fields to speed up searching
        using DataLayoutCacheGroup = HashMap<DataLayoutHandle, DataLayoutCacheEntry>;
        std::vector<DataLayoutCacheGroup> m_dataLayoutCache;
    };
}
