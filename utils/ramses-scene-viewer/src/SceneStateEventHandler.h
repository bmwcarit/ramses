//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_VIEWER_SCENESTATEEVENTHANDLER_H
#define RAMSES_SCENE_VIEWER_SCENESTATEEVENTHANDLER_H

#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "Collections/HashSet.h"

namespace ramses_internal
{

    class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty
    {
    public:
        SceneStateEventHandler(ramses::RamsesRenderer& renderer)
            : m_renderer(renderer)
        {
        }

        virtual void scenePublished(ramses::sceneId_t sceneId) override
        {
            m_publishedScenes.put(sceneId);
        }

        virtual void sceneUnpublished(ramses::sceneId_t sceneId) override
        {
            m_publishedScenes.remove(sceneId);
        }

        virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            if (ramses::ERendererEventResult_OK == result)
            {
                m_subscribedScenes.put(sceneId);
            }
        }

        virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            if (ramses::ERendererEventResult_FAIL != result)
            {
                m_subscribedScenes.remove(sceneId);
            }
        }

        virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            if (ramses::ERendererEventResult_OK == result)
            {
                m_mappedScenes.put(sceneId);
            }
        }

        virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            if (ramses::ERendererEventResult_FAIL != result)
            {
                m_mappedScenes.remove(sceneId);
            }
        }

        virtual void displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
        {
            if (ramses::ERendererEventResult_OK == result)
            {
                m_createdDisplays.put(displayId);
            }
        }

        virtual void displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
        {
            if (ramses::ERendererEventResult_FAIL != result)
            {
                m_createdDisplays.remove(displayId);
            }
        }

        void waitForPublication(const ramses::sceneId_t sceneId)
        {
            waitForSceneInSet(sceneId, m_publishedScenes);
        }

        void waitForSubscription(const ramses::sceneId_t sceneId)
        {
            waitForSceneInSet(sceneId, m_subscribedScenes);
        }

        void waitForMapped(const ramses::sceneId_t sceneId)
        {
            waitForSceneInSet(sceneId, m_mappedScenes);
        }

        void waitForDisplayDestruction(ramses::displayId_t displayId)
        {
            waitForDisplayNotInSet(displayId, m_createdDisplays);
        }

    private:
        typedef HashSet<ramses::sceneId_t> SceneSet;
        typedef HashSet<ramses::sceneId_t> DisplaySet;

        void waitForSceneInSet(const ramses::sceneId_t sceneId, const SceneSet& sceneSet)
        {
            while (!sceneSet.hasElement(sceneId))
            {
                m_renderer.dispatchEvents(*this);
            }
        }

        void waitForDisplayNotInSet(const ramses::sceneId_t sceneId, const DisplaySet& displaySet)
        {
            while (displaySet.hasElement(sceneId))
            {
                m_renderer.dispatchEvents(*this);
            }
        }

        SceneSet m_publishedScenes;
        SceneSet m_subscribedScenes;
        SceneSet m_mappedScenes;
        DisplaySet m_createdDisplays;

        ramses::RamsesRenderer& m_renderer;
    };
}

#endif
