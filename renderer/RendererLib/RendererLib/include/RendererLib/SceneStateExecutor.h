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

    class SceneStateExecutor
    {
    public:
        SceneStateExecutor(const Renderer& renderer, ISceneGraphConsumerComponent& sceneGraphConsumerComponent, RendererEventCollector& rendererEventCollector);

        void setPublished                     (SceneId sceneId, const Guid& clientWhereSceneIsAvailable, EScenePublicationMode mode);
        void setUnpublished                   (SceneId sceneId);
        void setSubscriptionRequested         (SceneId sceneId);
        void setSubscriptionPending           (SceneId sceneId);
        void setSubscribed                    (SceneId sceneId);
        void setUnsubscribed                  (SceneId sceneId, bool indirect);
        void setMapRequested                  (SceneId sceneId, DisplayHandle handle);
        void setMappingAndUploading           (SceneId sceneId);
        void setMapped                        (SceneId sceneId);
        void setUnmapped                      (SceneId sceneId);
        void setRenderedRequested             (SceneId sceneId);
        void setRendered                      (SceneId sceneId);
        void setHidden                        (SceneId sceneId);

        Bool checkIfCanBePublished            (SceneId sceneId) const;
        Bool checkIfCanBeUnpublished          (SceneId sceneId) const;
        Bool checkIfCanBeSubscriptionRequested(SceneId sceneId) const;
        Bool checkIfCanBeSubscriptionPending  (SceneId sceneId) const;
        Bool checkIfCanBeSubscribed           (SceneId sceneId) const;
        Bool checkIfCanBeUnsubscribed         (SceneId sceneId) const;
        Bool checkIfCanBeMapRequested         (SceneId sceneId, DisplayHandle handle) const;
        Bool checkIfCanBeMappingAndUploading  (SceneId sceneId) const;
        Bool checkIfCanBeMapped               (SceneId sceneId) const;
        Bool checkIfCanBeUnmapped             (SceneId sceneId) const;
        Bool checkIfCanBeRenderedRequested    (SceneId sceneId) const;
        Bool checkIfCanBeShown                (SceneId sceneId) const;
        Bool checkIfCanBeHidden               (SceneId sceneId) const;

        ESceneState getSceneState(SceneId sceneId) const;
        EScenePublicationMode getScenePublicationMode(SceneId sceneId) const;

    private:
        void rollBackToUnsubscribedAndTriggerIndirectEvents(ESceneState sceneState, SceneId sceneId);

        Bool canBeSubscriptionRequested(SceneId sceneId) const;
        Bool canBeUnsubscribed         (SceneId sceneId) const;
        Bool canBeMapRequested         (SceneId sceneId, DisplayHandle handle) const;
        Bool canBeMappingAndUploading  (SceneId sceneId) const;
        Bool canBeMapped               (SceneId sceneId) const;
        Bool canBeUnmapped             (SceneId sceneId) const;
        Bool canBeRenderedRequested    (SceneId sceneId) const;
        Bool canBeShown                (SceneId sceneId) const;
        Bool canBeHidden               (SceneId sceneId) const;

        const Renderer&               m_renderer;
        ISceneGraphConsumerComponent& m_sceneGraphConsumerComponent;
        RendererEventCollector&       m_rendererEventCollector;

        SceneStateInfo                m_scenesStateInfo;

        friend class RendererLogger;
    };

}

#endif
