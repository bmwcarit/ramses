//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAREFERENCELINKMANAGER_H
#define RAMSES_DATAREFERENCELINKMANAGER_H

#include "RendererLib/LinkManagerBase.h"

namespace ramses_internal
{
    class RendererScenes;
    class DataReferenceLinkCachedScene;

    class DataReferenceLinkManager : private LinkManagerBase
    {
    public:
        explicit DataReferenceLinkManager(RendererScenes& rendererScenes);

        void removeSceneLinks(SceneId sceneId);

        bool createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        bool removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle, SceneId* providerSceneIdOut = nullptr);

        void resolveLinksForConsumerScene(DataReferenceLinkCachedScene& consumerScene) const;
        void updateFallbackValue(SceneId consumerSceneId, DataInstanceHandle dataInstance) const;

        using LinkManagerBase::getDependencyChecker;
        using LinkManagerBase::getSceneLinks;
    };
}

#endif
