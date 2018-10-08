//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENELINKSMANAGER_H
#define RAMSES_SCENELINKSMANAGER_H

#include "RendererLib/TransformationLinkManager.h"
#include "RendererLib/DataReferenceLinkManager.h"
#include "RendererLib/TextureLinkManager.h"
#include "SceneAPI/DataSlot.h"

namespace ramses_internal
{
    class RendererScenes;
    class RendererEventCollector;

    class SceneLinksManager
    {
    public:
        explicit SceneLinksManager(RendererScenes& rendererScenes, RendererEventCollector& rendererEventCollector);

        void createDataLink(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId);
        void createBufferLink(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerId);
        void removeDataLink(SceneId consumerSceneId, DataSlotId consumerId);

        void handleSceneRemoved(SceneId sceneId);
        void handleSceneUnmapped(SceneId sceneId);
        void handleDataSlotCreated(SceneId sceneId, DataSlotHandle dataSlotHandle);
        void handleDataSlotDestroyed(SceneId sceneId, DataSlotHandle dataSlotHandle);
        void handleBufferDestroyed(OffscreenBufferHandle providerBuffer);

        const TransformationLinkManager& getTransformationLinkManager() const;
        const DataReferenceLinkManager&  getDataReferenceLinkManager() const;
        const TextureLinkManager&        getTextureLinkManager() const;

    private:
        template <typename LINKMANAGER>
        void removeLinksToProvider(SceneId sceneId, DataSlotHandle providerSlotHandle, LINKMANAGER& manager) const;

        const RendererScenes&     m_rendererScenes;
        RendererEventCollector&   m_rendererEventCollector;

        TransformationLinkManager m_transformationLinkManager;
        DataReferenceLinkManager  m_dataReferenceLinkManager;
        TextureLinkManager        m_textureLinkManager;
    };
}

#endif
