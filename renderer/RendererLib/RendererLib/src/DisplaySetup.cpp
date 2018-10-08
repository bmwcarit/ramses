//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplaySetup.h"

namespace ramses_internal
{
    void DisplaySetup::registerDisplayBuffer(DeviceResourceHandle displayBuffer, const Viewport& viewport, const Vector4& clearColor, Bool isOffscreenBuffer, Bool isInterruptible)
    {
        assert(!isInterruptible || isOffscreenBuffer);
        assert(m_displayBuffers.find(displayBuffer) == m_displayBuffers.cend());

        const DisplayBufferInfo bufferInfo{ isOffscreenBuffer, isInterruptible, viewport, clearColor, {}, true };
        m_displayBuffers.insert({ displayBuffer, std::move(bufferInfo) });
    }

    void DisplaySetup::unregisterDisplayBuffer(DeviceResourceHandle displayBuffer)
    {
        assert(m_displayBuffers.find(displayBuffer) != m_displayBuffers.cend());
        m_displayBuffers.erase(displayBuffer);
    }

    const DisplayBufferInfo& DisplaySetup::getDisplayBuffer(DeviceResourceHandle displayBuffer) const
    {
        const auto it = m_displayBuffers.find(displayBuffer);
        assert(it != m_displayBuffers.cend());
        return it->second;
    }

    void DisplaySetup::setDisplayBufferToBeRerendered(DeviceResourceHandle displayBuffer, Bool rerender)
    {
        getDisplayBufferInternal(displayBuffer).needsRerender = rerender;
    }

    void DisplaySetup::mapSceneToDisplayBuffer(SceneId sceneId, DeviceResourceHandle displayBuffer, Int32 sceneOrder)
    {
        assert(!findDisplayBufferSceneIsMappedTo(sceneId).isValid());
        auto& bufferInfo = getDisplayBufferInternal(displayBuffer);
        auto& mappedScenes = bufferInfo.mappedScenes;
        const auto it = std::upper_bound(mappedScenes.begin(), mappedScenes.end(), sceneOrder, [](Int32 order, const MappedSceneInfo& info) { return order < info.globalSceneOrder; });
        mappedScenes.insert(it, { sceneId, sceneOrder, false });
        bufferInfo.needsRerender = true;
    }

    void DisplaySetup::unmapScene(SceneId sceneId)
    {
        const auto displayBuffer = findDisplayBufferSceneIsMappedTo(sceneId);
        assert(displayBuffer.isValid());
        auto& bufferInfo = getDisplayBufferInternal(displayBuffer);
        auto& mappedScenes = bufferInfo.mappedScenes;
        const auto it = std::find_if(mappedScenes.begin(), mappedScenes.end(), [sceneId](const MappedSceneInfo& info) { return info.sceneId == sceneId; });
        assert(it != mappedScenes.end());
        mappedScenes.erase(it);
        bufferInfo.needsRerender = true;
    }

    DeviceResourceHandle DisplaySetup::findDisplayBufferSceneIsMappedTo(SceneId sceneId) const
    {
        auto isMappedScene = [sceneId](const MappedSceneInfo& mapInfo) { return mapInfo.sceneId == sceneId; };
        auto findByMappedScene = [isMappedScene](const DisplayBufferInfo& info)
        {
            const auto it = std::find_if(info.mappedScenes.cbegin(), info.mappedScenes.cend(), isMappedScene);
            return it != info.mappedScenes.cend();
        };

        for (const auto& bufferInfoIt : m_displayBuffers)
        {
            if (findByMappedScene(bufferInfoIt.second))
                return bufferInfoIt.first;
        }

        return DeviceResourceHandle::Invalid();
    }

    void DisplaySetup::setSceneShown(SceneId sceneId, Bool show)
    {
        const auto displayBuffer = findDisplayBufferSceneIsMappedTo(sceneId);
        assert(displayBuffer.isValid());
        auto& bufferInfo = getDisplayBufferInternal(displayBuffer);
        auto& mappedScenes = bufferInfo.mappedScenes;
        auto it = std::find_if(mappedScenes.begin(), mappedScenes.end(), [sceneId](const MappedSceneInfo& info) { return info.sceneId == sceneId; });
        assert(it != mappedScenes.end());
        assert(it->shown != show);
        it->shown = show;
        bufferInfo.needsRerender = true;
    }

    void DisplaySetup::setClearColor(DeviceResourceHandle displayBuffer, const Vector4& clearColor)
    {
        auto& bufferInfo = getDisplayBufferInternal(displayBuffer);
        bufferInfo.clearColor = clearColor;
        bufferInfo.needsRerender = true;
    }

    const DeviceHandleVector& DisplaySetup::getNonInterruptibleOffscreenBuffersToRender() const
    {
        m_buffersToRender.clear();
        for (const auto& buffer : m_displayBuffers)
        {
            if (buffer.second.isOffscreenBuffer && !buffer.second.isInterruptible && buffer.second.needsRerender)
                m_buffersToRender.push_back(buffer.first);
        }

        return m_buffersToRender;
    }

    const DeviceHandleVector& DisplaySetup::getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle interruptedDisplayBuffer) const
    {
        m_buffersToRender.clear();

        // if there is interruption, put the interrupted buffer to re-render and start from there, skipping previous buffers
        auto bufferToRenderBegin = m_displayBuffers.begin();
        if (interruptedDisplayBuffer.isValid())
        {
            bufferToRenderBegin = m_displayBuffers.find(interruptedDisplayBuffer);
            assert(bufferToRenderBegin != m_displayBuffers.end());
            m_buffersToRender.push_back(bufferToRenderBegin->first);
            ++bufferToRenderBegin;
        }

        for (auto it = bufferToRenderBegin; it != m_displayBuffers.end(); ++it)
        {
            if (it->second.isInterruptible && it->second.needsRerender)
                m_buffersToRender.push_back(it->first);
        }

        return m_buffersToRender;
    }

    const DisplayBuffersMap& DisplaySetup::getDisplayBuffers() const
    {
        return m_displayBuffers;
    }

    DisplayBufferInfo& DisplaySetup::getDisplayBufferInternal(DeviceResourceHandle displayBuffer)
    {
        const auto it = m_displayBuffers.find(displayBuffer);
        assert(it != m_displayBuffers.cend());
        return it->second;
    }
}
