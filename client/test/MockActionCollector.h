//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MOCKACTIONCOLLECTOR_H
#define RAMSES_MOCKACTIONCOLLECTOR_H

#include "gmock/gmock.h"
#include "Components/ISceneGraphConsumerComponent.h"
#include "SceneAPI/SceneId.h"
#include "Scene/SceneActionCollection.h"
#include "SceneRendererHandlerMock.h"
#include "Components/SceneUpdate.h"

namespace ramses
{
    class MockActionCollector : public ramses_internal::SceneRendererHandlerMock
    {
    public:
        MockActionCollector()
            : m_numReceivedActionLists(0)
            , m_sceneGraphConsumer(nullptr)
        {
        }

        void init(ramses_internal::ISceneGraphConsumerComponent& sceneGraphConsumer)
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

        ramses_internal::SceneActionCollection getCopyOfCollectedActions()
        {
            return m_collectedActions.copy();
        }

        uint32_t getNumReceivedActionLists() const
        {
            return m_numReceivedActionLists;
        }

    private:
        ramses_internal::SceneActionCollection  m_collectedActions;
        uint32_t                                m_numReceivedActionLists;

        ramses_internal::ISceneGraphConsumerComponent* m_sceneGraphConsumer;

        void handleInitializeScene(const ramses_internal::SceneInfo& sceneInfo, const ramses_internal::Guid& providerID) override
        {
            ramses_internal::SceneRendererHandlerMock::handleInitializeScene(sceneInfo, providerID);
        }

        void handleSceneUpdate(const ramses_internal::SceneId& sceneId, ramses_internal::SceneUpdate&& sceneUpdate, const ramses_internal::Guid& providerID) override
        {
            m_collectedActions.append(sceneUpdate.actions);
            ramses_internal::SceneRendererHandlerMock::handleSceneUpdate(sceneId, std::move(sceneUpdate), providerID);
            ++m_numReceivedActionLists;
        }

        void handleNewSceneAvailable(const ramses_internal::SceneInfo& newScene, const ramses_internal::Guid& providerID) override
        {
            ramses_internal::SceneRendererHandlerMock::handleNewSceneAvailable(newScene, providerID);
            if (m_sceneGraphConsumer != nullptr)
            {
                m_sceneGraphConsumer->subscribeScene(providerID, newScene.sceneID);
            }
        }

        void handleSceneBecameUnavailable(const ramses_internal::SceneId& unavailableScene, const ramses_internal::Guid& providerID) override
        {
            ramses_internal::SceneRendererHandlerMock::handleSceneBecameUnavailable(unavailableScene, providerID);
        }
    };
}

#endif
