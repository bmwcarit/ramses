//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERTESTEVENTHANDLER_H
#define RAMSES_RENDERERTESTEVENTHANDLER_H

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "RamsesRendererImpl.h"
#include "RamsesRendererUtils.h"
#include "PlatformAbstraction/PlatformTime.h"

class RendererTestEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    RendererTestEventHandler(ramses::RamsesRenderer& renderer, const uint64_t timeoutMs = 6000u)
        : m_renderer(renderer)
        , m_timeoutMs(timeoutMs)
    {
    }

    virtual void scenePublished(ramses::sceneId_t sceneId) override
    {
        m_publishedScenes.put(sceneId);
    }

    virtual void sceneUnpublished(ramses::sceneId_t sceneId) override
    {
        m_unpublishedScenes.put(sceneId);
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
            m_unsubscribedScenes.put(sceneId);
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
            m_unmappedScenes.put(sceneId);
        }
    }

    virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_shownScenes.put(sceneId);
        }
    }

    virtual void sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_hiddenScenes.put(sceneId);
        }
    }

    virtual void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion, ramses::ESceneResourceStatus resourceStatus) override
    {
        if (!m_sceneFlushes.contains(sceneId))
        {
            m_sceneFlushes.put(sceneId, SceneFlushSet());
        }

        m_sceneFlushes.get(sceneId)->put(sceneVersion, ramses::ESceneResourceStatus_Ready == resourceStatus);
    }

    virtual void sceneExpired(ramses::sceneId_t sceneId) override
    {
        m_expiredScenes.put(sceneId);
    }

    virtual void sceneRecoveredFromExpiration(ramses::sceneId_t sceneId) override
    {
        m_recoveredScenes.put(sceneId);
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
            m_destroyedDisplays.put(displayId);
        }
    }

    virtual void offscreenBufferCreated(ramses::displayId_t, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_createdOffscreenBuffers.put(offscreenBufferId);
        }
    }

    virtual void offscreenBufferDestroyed(ramses::displayId_t, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_destroyedOffscreenBuffers.put(offscreenBufferId);
        }
    }

    virtual void offscreenBufferLinkedToSceneData(ramses::offscreenBufferId_t providerOffscreenBuffer, ramses::sceneId_t /*consumerScene*/, ramses::dataConsumerId_t /*consumerId*/, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_linkedOffscreenBuffers.put(providerOffscreenBuffer);
        }
    }

    virtual void streamAvailabilityChanged(ramses::streamSource_t streamId, bool available) override
    {
        if(available)
        {
            m_availableStreamSurfaces.put(streamId.getValue());
        }
        else
        {
            m_unavailableStreamSurfaces.put(streamId.getValue());
        }
    }

    bool waitForPublication(ramses::sceneId_t sceneId)
    {
        return waitAndConsumeId(m_publishedScenes, sceneId);
    }

    bool waitForUnpublished(ramses::sceneId_t sceneId)
    {
        return waitAndConsumeId(m_unpublishedScenes, sceneId);
    }

    bool waitForSubscription(ramses::sceneId_t sceneId)
    {
        return waitAndConsumeId(m_subscribedScenes, sceneId);
    }

    bool waitForUnsubscribed(ramses::sceneId_t sceneId)
    {
        return waitAndConsumeId(m_unsubscribedScenes, sceneId);
    }

    bool waitForMapped(ramses::sceneId_t sceneId)
    {
        return waitAndConsumeId(m_mappedScenes, sceneId);
    }

    bool waitForUnmapped(ramses::sceneId_t sceneId)
    {
        return waitAndConsumeId(m_unmappedScenes, sceneId);
    }

    bool waitForShown(ramses::sceneId_t sceneId)
    {
        return waitAndConsumeId(m_shownScenes, sceneId);
    }

    bool waitForHidden(ramses::sceneId_t sceneId)
    {
        return waitAndConsumeId(m_hiddenScenes, sceneId);
    }

    bool waitForDisplayCreation(ramses::displayId_t displayId)
    {
        return waitAndConsumeId(m_createdDisplays, displayId);
    }

    bool waitForDisplayDestruction(ramses::displayId_t displayId)
    {
        return waitAndConsumeId(m_destroyedDisplays, displayId);
    }

    bool waitForOffscreenBufferCreation(ramses::offscreenBufferId_t offscreenBufferId)
    {
        return waitAndConsumeId(m_createdOffscreenBuffers, offscreenBufferId);
    }

    bool waitForOffscreenBufferDestruction(ramses::offscreenBufferId_t offscreenBufferId)
    {
        return waitAndConsumeId(m_destroyedOffscreenBuffers, offscreenBufferId);
    }

    bool waitForOffscreenBufferLink(ramses::offscreenBufferId_t offscreenBufferId)
    {
        return waitAndConsumeId(m_linkedOffscreenBuffers, offscreenBufferId);
    }

    bool waitForSyncFlush(ramses::sceneId_t sceneId)
    {
        const bool success = waitUntilConditionFulfilledOrTimeout([this, sceneId] {return m_sceneFlushes.contains(sceneId); });
        assert(success && "Waiting for event timed out!");
        m_sceneFlushes.remove(sceneId);
        return success;
    }

    bool waitForNamedFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion, bool resourcesMustBeReady)
    {
        auto conditionFunction = [this, sceneId, sceneVersion] {return m_sceneFlushes.contains(sceneId) && m_sceneFlushes.get(sceneId)->contains(sceneVersion); };
        const bool receivedFlush = waitUntilConditionFulfilledOrTimeout(conditionFunction);
        const bool resourceStateAsExpected =
            (resourcesMustBeReady) ?                            // depending on whether resource state is important or not...
            (*m_sceneFlushes.get(sceneId)->get(sceneVersion)) : // check if resources were ready at the time the named flush was applied
            (true);                                             // or don't care otherwise

        assert(receivedFlush && "Waiting for flush timed out!");
        assert(resourceStateAsExpected && "Flush received but resource state not as expected!");

        m_sceneFlushes.remove(sceneId);
        return receivedFlush && resourceStateAsExpected;
    }

    bool checkAndConsumeExpiredScenesEvents(ramses::sceneId_t sceneId)
    {
        m_renderer.dispatchEvents(*this);
        const bool hasEvent = m_expiredScenes.hasElement(sceneId);
        m_expiredScenes.remove(sceneId);
        return hasEvent;
    }

    bool checkAndConsumeRecoveredScenesEvents(ramses::sceneId_t sceneId)
    {
        m_renderer.dispatchEvents(*this);
        const bool hasEvent = m_recoveredScenes.hasElement(sceneId);
        m_recoveredScenes.remove(sceneId);
        return hasEvent;
    }

    void consumePendingEvents()
    {
        m_renderer.dispatchEvents(*this);
    }

    bool waitForStreamSurfaceAvailabilityChange(ramses::streamSource_t streamSource, bool available)
    {
        if(available)
        {
            return waitAndConsumeId(m_availableStreamSurfaces, streamSource.getValue());
        }
        else
        {
            return waitAndConsumeId(m_unavailableStreamSurfaces, streamSource.getValue());
        }
    }

private:
    typedef ramses_internal::HashSet<uint64_t> IdSet;

    bool waitAndConsumeId(IdSet& idSet, const uint64_t id)
    {
        const bool success = waitUntilConditionFulfilledOrTimeout([&idSet, id] {return idSet.hasElement(id); });
        assert(success && "Waiting for event timed out!");
        idSet.remove(id);
        return success;
    }

    bool waitUntilConditionFulfilledOrTimeout(const std::function<bool()> conditionFunction)
    {
        const uint64_t timeoutTime = (m_timeoutMs > 0) ?
            ramses_internal::PlatformTime::GetMillisecondsMonotonic() + m_timeoutMs :
            std::numeric_limits<uint64_t>::max();
        while (ramses_internal::PlatformTime::GetMillisecondsMonotonic() < timeoutTime && !conditionFunction())
        {
            ramses_internal::PlatformThread::Sleep(5u); // will give the renderer time to process changes
            if (!m_renderer.impl.isThreaded())
                ramses::RamsesRendererUtils::DoOneLoop(m_renderer.impl.getRenderer(), ramses_internal::ELoopMode_UpdateOnly, std::chrono::microseconds{ 0u });
            m_renderer.dispatchEvents(*this);
        }

        return conditionFunction();
    }

    IdSet m_publishedScenes;
    IdSet m_unpublishedScenes;
    IdSet m_subscribedScenes;
    IdSet m_unsubscribedScenes;
    IdSet m_mappedScenes;
    IdSet m_unmappedScenes;
    IdSet m_shownScenes;
    IdSet m_hiddenScenes;
    // also store the state of resources
    using SceneFlushSet = ramses_internal::HashMap<ramses::sceneVersionTag_t, bool>;
    using SceneFlushesOfScenes = ramses_internal::HashMap<ramses::sceneId_t, SceneFlushSet>;
    SceneFlushesOfScenes m_sceneFlushes;
    IdSet m_createdDisplays;
    IdSet m_destroyedDisplays;
    IdSet m_createdOffscreenBuffers;
    IdSet m_destroyedOffscreenBuffers;
    IdSet m_linkedOffscreenBuffers;
    IdSet m_availableStreamSurfaces;
    IdSet m_unavailableStreamSurfaces;
    IdSet m_expiredScenes;
    IdSet m_recoveredScenes;

    ramses::RamsesRenderer& m_renderer;
    const uint64_t m_timeoutMs;
};

#endif
