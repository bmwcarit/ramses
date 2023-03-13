//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENESTATEEXECUTOR_H
#define RAMSES_SCENESTATEEXECUTOR_H

#include "RendererLib/SceneStateInfo.h"
#include "Scene/EScenePublicationMode.h"

namespace ramses_internal
{
    class Renderer;
    class ISceneGraphConsumerComponent;
    class RendererEventCollector;
    class RendererScenes;
    class IRendererSceneEventSender;

    class SceneStateExecutor
    {
    public:
        SceneStateExecutor(const Renderer& renderer, IRendererSceneEventSender& rendererSceneSender, RendererEventCollector& rendererEventCollector);

        void setPublished                     (SceneId sceneId, EScenePublicationMode mode);
        void setUnpublished                   (SceneId sceneId);
        void setSubscriptionRequested         (SceneId sceneId);
        void setSubscriptionPending           (SceneId sceneId);
        void setSubscribed                    (SceneId sceneId);
        void setUnsubscribed                  (SceneId sceneId, bool indirect);
        void setMapRequested                  (SceneId sceneId);
        void setMappingAndUploading           (SceneId sceneId);
        void setMapped                        (SceneId sceneId);
        void setUnmapped                      (SceneId sceneId);
        void setRenderedRequested             (SceneId sceneId);
        void setRendered                      (SceneId sceneId);
        void setHidden                        (SceneId sceneId);

        [[nodiscard]] Bool checkIfCanBePublished            (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeUnpublished          (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeSubscriptionRequested(SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeSubscriptionPending  (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeSubscribed           (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeUnsubscribed         (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeMapRequested         (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeMapped               (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeUnmapped             (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeRenderedRequested    (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeShown                (SceneId sceneId) const;
        [[nodiscard]] Bool checkIfCanBeHidden               (SceneId sceneId) const;

        [[nodiscard]] ESceneState getSceneState(SceneId sceneId) const;
        [[nodiscard]] EScenePublicationMode getScenePublicationMode(SceneId sceneId) const;

    private:
        void rollBackToUnsubscribedAndTriggerIndirectEvents(ESceneState sceneState, SceneId sceneId);

        [[nodiscard]] Bool canBeSubscriptionRequested(SceneId sceneId) const;
        [[nodiscard]] Bool canBeUnsubscribed         (SceneId sceneId) const;
        [[nodiscard]] Bool canBeMapRequested         (SceneId sceneId) const;
        [[nodiscard]] Bool canBeMappingAndUploading  (SceneId sceneId) const;
        [[nodiscard]] Bool canBeMapped               (SceneId sceneId) const;
        [[nodiscard]] Bool canBeUnmapped             (SceneId sceneId) const;
        [[nodiscard]] Bool canBeRenderedRequested    (SceneId sceneId) const;
        [[nodiscard]] Bool canBeShown                (SceneId sceneId) const;
        [[nodiscard]] Bool canBeHidden               (SceneId sceneId) const;

        const Renderer&               m_renderer;
        RendererEventCollector&       m_rendererEventCollector;
        IRendererSceneEventSender&    m_rendererSceneEventSender;
        SceneStateInfo                m_scenesStateInfo;

        friend class RendererLogger;
    };

}

#endif
