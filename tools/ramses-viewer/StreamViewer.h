//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "RendererControl.h"
#include "ImguiClientHelper.h"

#include <unordered_map>
#include <unordered_set>

namespace ramses::internal
{
    class StreamViewer : public RendererSceneControlEventHandlerEmpty
    {
    public:
        explicit StreamViewer(ImguiClientHelper& imguiHelper, RamsesRenderer& renderer, displayId_t displayId);

        void draw(bool defaultOpen);

        void setAutoShow(bool autoShow)
        {
            m_autoShow = autoShow;
        }

        void streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available) override;

        void dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override;

        void dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override;

    private:
        struct StreamEntry
        {
            TextureSampler* sampler = nullptr;
            dataConsumerId_t consumerId;
            streamBufferId_t streamBuffer;
        };
        using StreamEntries = std::unordered_map<waylandIviSurfaceId_t, StreamEntry>;

        StreamEntry& findOrCreateStreamEntry(waylandIviSurfaceId_t surfaceId);

        void setActiveSurface(waylandIviSurfaceId_t surfaceId);

        void createAndLinkStreamBuffer(waylandIviSurfaceId_t surfaceId, StreamEntry& entry);
        void unlinkAndDestroyStreamBuffer(StreamEntry& entry);

        ImguiClientHelper& m_imguiHelper;
        RamsesRenderer& m_renderer;
        displayId_t m_displayId;
        StreamEntries m_streamEntries;
        std::unordered_set<dataConsumerId_t> m_dataConsumers;

        Effect* m_effect = nullptr;
        Appearance* m_appearance = nullptr;
        MeshNode* m_meshNode = nullptr;
        TextureSampler* m_activeStream = nullptr;

        bool m_autoShow = true;
    };
}
