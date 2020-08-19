//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEREFERENCELOGIC_H
#define RAMSES_SCENEREFERENCELOGIC_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include "SceneReferencing/SceneReferenceAction.h"
#include "RendererLib/RendererEvent.h"
#include <unordered_map>
#include <unordered_set>

namespace ramses_internal
{
    class RendererScenes;
    class IRendererSceneControlLogic;
    class IRendererSceneControl;
    class IRendererSceneEventSender;

    class ISceneReferenceLogic
    {
    public:
        virtual void addActions(SceneId masterScene, const SceneReferenceActionVector& actions) = 0;
        virtual void update() = 0;

        virtual ~ISceneReferenceLogic() = default;
    };

    class SceneReferenceLogic : public ISceneReferenceLogic
    {
    public:
        SceneReferenceLogic(const RendererScenes& scenes, IRendererSceneControlLogic& sceneLogicIRendererSceneControl, IRendererSceneControl& sceneControl, IRendererSceneEventSender& sender);

        virtual void addActions(SceneId masterScene, const SceneReferenceActionVector& actions) override;
        virtual void update() override;

        void extractAndSendSceneReferenceEvents(RendererEventVector& events);
        bool hasAnyReferencedScenes() const;

    private:
        void updateReferencedScenes();
        void cleanupDestroyedMasterScenes();
        void cleanupReleasedReferences();
        void executePendingActions();

        SceneId findMasterSceneForReferencedScene(SceneId sceneId) const;

        struct MasterSceneInfo
        {
            std::unordered_map<SceneId, SceneReferenceHandle> sceneReferences;
            SceneReferenceActionVector pendingActions;
            std::unordered_set<SceneId> sceneReferencesWithFlushNotification;
            std::unordered_set<SceneId> expiredSceneReferences;
            bool reportedAsExpired = false;
            bool destroyed = false;
        };
        std::unordered_map<SceneId, MasterSceneInfo> m_masterScenes;

        const RendererScenes& m_rendererScenes;
        // for state change requests
        IRendererSceneControlLogic& m_sceneLogic;
        // for direct control (scene reference actions)
        IRendererSceneControl& m_sceneControl;

        IRendererSceneEventSender& m_eventSender;

        std::vector<SceneId> m_masterScenesWithChangedExpirationState;
    };
}

#endif
