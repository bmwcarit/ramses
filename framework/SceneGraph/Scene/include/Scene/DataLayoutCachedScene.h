//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATALAYOUTCACHEDSCENE_H
#define RAMSES_DATALAYOUTCACHEDSCENE_H

#include "Scene/ActionCollectingScene.h"
#include "Collections/Vector.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{
    class DataLayoutCachedScene : public ActionCollectingScene
    {
    public:
        explicit DataLayoutCachedScene(const SceneInfo& sceneInfo = SceneInfo());

        virtual DataLayoutHandle            allocateDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle = DataLayoutHandle::Invalid()) override;
        virtual void                        releaseDataLayout(DataLayoutHandle handle) override;

        UInt32                              getNumDataLayoutReferences(DataLayoutHandle handle) const;

    private:
        struct DataLayoutCacheEntry
        {
            DataFieldInfoVector m_dataFields;
            UInt32              m_usageCount = 0u;
        };

        DataLayoutHandle allocateAndCacheDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle);
        DataLayoutHandle findDataLayoutEntry(const DataFieldInfoVector& dataFields, DataLayoutCacheEntry*& entryOut);

        // Cache entries are stored in groups, each group has data layouts with same number of fields to speed up searching
        typedef HashMap<DataLayoutHandle, DataLayoutCacheEntry> DataLayoutCacheGroup;
        std::vector<DataLayoutCacheGroup> m_dataLayoutCache;
    };
}

#endif
