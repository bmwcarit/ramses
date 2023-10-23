//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneReferencing/SceneReferenceAction.h"
#include "internal/RendererLib/RendererEvent.h"
#include <unordered_map>
#include <unordered_set>

namespace ramses::internal
{
    class RendererScenes;
    class IRendererSceneControlLogic;
    class IRendererSceneUpdater;
    class IRendererSceneEventSender;
    class SceneReferenceOwnership;

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
        SceneReferenceLogic(const RendererScenes& scenes, IRendererSceneControlLogic& sceneLogicIRendererSceneControl, IRendererSceneUpdater& sceneUpdater, IRendererSceneEventSender& sender, SceneReferenceOwnership& sharedOwnership);

        void addActions(SceneId masterScene, const SceneReferenceActionVector& actions) override;
        void update() override;

        void extractAndSendSceneReferenceEvents(RendererEventVector& events);
        [[nodiscard]] bool hasAnyReferencedScenes() const;

    private:
        void updateReferencedScenes();
        void cleanupDestroyedMasterScenes();
        void cleanupReleasedReferences();
        void executePendingActions();
        void consolidateExpirationState(SceneId masterSceneId, RendererEventVector& events);
        [[nodiscard]] SceneId findMasterSceneForReferencedScene(SceneId sceneId) const;

        enum class ExpirationState
        {
            MonitoringDisabled,
            MonitoringEnabled,
            Expired
        };

        struct MasterSceneInfo
        {
            std::unordered_map<SceneId, SceneReferenceHandle> sceneReferences;
            SceneReferenceActionVector pendingActions;
            std::unordered_set<SceneId> sceneReferencesWithFlushNotification;

            // this set contains either references or master itself
            std::unordered_map<SceneId, ExpirationState> expirationStates;
            // last reported consolidated state of master
            ExpirationState consolidatedExpirationState = ExpirationState::MonitoringDisabled;

            bool destroyed = false;
        };
        std::unordered_map<SceneId, MasterSceneInfo> m_masterScenes;

        const RendererScenes& m_rendererScenes;
        // for state change requests
        IRendererSceneControlLogic& m_sceneLogic;
        // for direct control (scene reference actions)
        IRendererSceneUpdater& m_sceneUpdater;

        IRendererSceneEventSender& m_eventSender;
        // Additional thread-safe storage of ref-master relation can be queried outside of display thread.
        // It is used to write only on change so need for locking is minimal.
        SceneReferenceOwnership& m_sharedOwnership;

        std::vector<SceneId> m_masterScenesWithChangedExpirationState;

        friend class RendererLogger;
    };
}
