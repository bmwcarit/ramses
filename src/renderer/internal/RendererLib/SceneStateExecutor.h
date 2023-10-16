//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/SceneStateInfo.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"

namespace ramses::internal
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

        [[nodiscard]] bool checkIfCanBePublished            (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeUnpublished          (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeSubscriptionRequested(SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeSubscriptionPending  (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeSubscribed           (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeUnsubscribed         (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeMapRequested         (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeMapped               (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeUnmapped             (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeRenderedRequested    (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeShown                (SceneId sceneId) const;
        [[nodiscard]] bool checkIfCanBeHidden               (SceneId sceneId) const;

        [[nodiscard]] ESceneState getSceneState(SceneId sceneId) const;
        [[nodiscard]] std::optional<EScenePublicationMode> getScenePublicationMode(SceneId sceneId) const;

    private:
        void rollBackToUnsubscribedAndTriggerIndirectEvents(ESceneState sceneState, SceneId sceneId);

        [[nodiscard]] bool canBeSubscriptionRequested(SceneId sceneId) const;
        [[nodiscard]] bool canBeUnsubscribed         (SceneId sceneId) const;
        [[nodiscard]] bool canBeMapRequested         (SceneId sceneId) const;
        [[nodiscard]] bool canBeMappingAndUploading  (SceneId sceneId) const;
        [[nodiscard]] bool canBeMapped               (SceneId sceneId) const;
        [[nodiscard]] bool canBeUnmapped             (SceneId sceneId) const;
        [[nodiscard]] bool canBeRenderedRequested    (SceneId sceneId) const;
        [[nodiscard]] bool canBeShown                (SceneId sceneId) const;
        [[nodiscard]] bool canBeHidden               (SceneId sceneId) const;

        const Renderer&               m_renderer;
        RendererEventCollector&       m_rendererEventCollector;
        IRendererSceneEventSender&    m_rendererSceneEventSender;
        SceneStateInfo                m_scenesStateInfo;

        friend class RendererLogger;
    };

}
