//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Components/ISceneGraphConsumerComponent.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "SceneRendererHandlerMock.h"
#include "internal/Components/SceneUpdate.h"
#include "TestEqualHelper.h"

namespace ramses::internal
{
    class MockActionCollector : public ramses::internal::SceneRendererHandlerMock
    {
    public:
        void init(ramses::internal::ISceneGraphConsumerComponent& sceneGraphConsumer)
        {
            m_sceneGraphConsumer = &sceneGraphConsumer;
        }

        void resetCollecting()
        {
            m_collectedActions.clear();
            m_numReceivedActionLists = 0u;
        }

        uint32_t getNumberOfActions() const
        {
            return m_collectedActions.numberOfActions();
        }

        ramses::internal::SceneActionCollection getCopyOfCollectedActions()
        {
            return m_collectedActions.copy();
        }

        uint32_t getNumReceivedActionLists() const
        {
            return m_numReceivedActionLists;
        }

    private:
        ramses::internal::SceneActionCollection m_collectedActions{};
        uint32_t                               m_numReceivedActionLists{0};

        ramses::internal::ISceneGraphConsumerComponent* m_sceneGraphConsumer{nullptr};

        void handleInitializeScene(const ramses::internal::SceneInfo& sceneInfo, const ramses::internal::Guid& providerID) override
        {
            ramses::internal::SceneRendererHandlerMock::handleInitializeScene(sceneInfo, providerID);
        }

        void handleSceneUpdate(const ramses::internal::SceneId& sceneId, ramses::internal::SceneUpdate&& sceneUpdate, const ramses::internal::Guid& providerID) override
        {
            m_collectedActions.append(sceneUpdate.actions);
            ramses::internal::SceneRendererHandlerMock::handleSceneUpdate(sceneId, std::move(sceneUpdate), providerID);
            ++m_numReceivedActionLists;
        }

        void handleNewSceneAvailable(const ramses::internal::SceneInfo& newScene, const ramses::internal::Guid& providerID) override
        {
            ramses::internal::SceneRendererHandlerMock::handleNewSceneAvailable(newScene, providerID);
            if (m_sceneGraphConsumer != nullptr)
            {
                m_sceneGraphConsumer->subscribeScene(providerID, newScene.sceneID);
            }
        }

        void handleSceneBecameUnavailable(const ramses::internal::SceneId& unavailableScene, const ramses::internal::Guid& providerID) override
        {
            ramses::internal::SceneRendererHandlerMock::handleSceneBecameUnavailable(unavailableScene, providerID);
        }
    };
}
