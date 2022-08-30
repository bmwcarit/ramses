//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplaySetup.h"
#include "SceneAPI/RenderState.h"

namespace ramses_internal
{
    void DisplaySetup::registerDisplayBuffer(DeviceResourceHandle displayBuffer, const Viewport& viewport, const Vector4& clearColor, Bool isOffscreenBuffer, Bool isInterruptible)
    {
        assert(!isInterruptible || isOffscreenBuffer);
        assert(m_displayBuffers.find(displayBuffer) == m_displayBuffers.cend());

        DisplayBufferInfo bufferInfo{ isOffscreenBuffer, isInterruptible, viewport, EClearFlags_All, clearColor, {}, true };
        m_displayBuffers.emplace(displayBuffer, std::move(bufferInfo));
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

    void DisplaySetup::assignSceneToDisplayBuffer(SceneId sceneId, DeviceResourceHandle displayBuffer, Int32 sceneOrder)
    {
        AssignedSceneInfo sceneInfo{ sceneId, sceneOrder, false };
        const auto displayBufferOld = findDisplayBufferSceneIsAssignedTo(sceneId);
        if (displayBufferOld.isValid())
        {
            sceneInfo.shown = findSceneInfo(sceneId, displayBufferOld).shown;
            unassignScene(sceneId);
        }

        auto& bufferInfo = getDisplayBufferInternal(displayBuffer);
        auto& assignedScenes = bufferInfo.scenes;
        const auto it = std::upper_bound(assignedScenes.begin(), assignedScenes.end(), sceneOrder, [](Int32 order, const AssignedSceneInfo& info) { return order < info.globalSceneOrder; });
        assignedScenes.insert(it, sceneInfo);
        bufferInfo.needsRerender = true;
    }

    void DisplaySetup::unassignScene(SceneId sceneId)
    {
        const auto displayBuffer = findDisplayBufferSceneIsAssignedTo(sceneId);
        auto& bufferInfo = getDisplayBufferInternal(displayBuffer);
        auto& mappedScenes = bufferInfo.scenes;
        const auto it = std::find_if(mappedScenes.begin(), mappedScenes.end(), [sceneId](const AssignedSceneInfo& info) { return info.sceneId == sceneId; });
        assert(it != mappedScenes.end());
        mappedScenes.erase(it);
        bufferInfo.needsRerender = true;
    }

    DeviceResourceHandle DisplaySetup::findDisplayBufferSceneIsAssignedTo(SceneId sceneId) const
    {
        for (const auto& bufferInfoIt : m_displayBuffers)
        {
            const DisplayBufferInfo& info = bufferInfoIt.second;
            const auto it = std::find_if(info.scenes.cbegin(), info.scenes.cend(),
                [sceneId](const auto& mapInfo) { return mapInfo.sceneId == sceneId; });
            if (it != info.scenes.cend())
                return bufferInfoIt.first;
        }

        return DeviceResourceHandle::Invalid();
    }

    AssignedSceneInfo& DisplaySetup::findSceneInfo(SceneId sceneId, DeviceResourceHandle displayBuffer)
    {
        DisplayBufferInfo& info = getDisplayBufferInternal(displayBuffer);
        const auto it = std::find_if(info.scenes.begin(), info.scenes.end(),
            [sceneId](const auto& mapInfo) { return mapInfo.sceneId == sceneId; });
        assert(it != info.scenes.cend());
        return *it;
    }

    void DisplaySetup::setSceneShown(SceneId sceneId, Bool show)
    {
        const auto displayBuffer = findDisplayBufferSceneIsAssignedTo(sceneId);
        findSceneInfo(sceneId, displayBuffer).shown = show;
        getDisplayBufferInternal(displayBuffer).needsRerender = true;
    }

    void DisplaySetup::setClearFlags(DeviceResourceHandle displayBuffer, uint32_t clearFlags)
    {
        getDisplayBufferInternal(displayBuffer).clearFlags = clearFlags;
    }

    void DisplaySetup::setClearColor(DeviceResourceHandle displayBuffer, const Vector4& clearColor)
    {
        auto& bufferInfo = getDisplayBufferInternal(displayBuffer);
        bufferInfo.clearColor = clearColor;
        // for simplicity trigger all buffers on display to re-render
        // otherwise would have to resolve dependencies via OB links
        for (auto& dispBufferInfo : m_displayBuffers)
            dispBufferInfo.second.needsRerender = true;
    }

    void DisplaySetup::setDisplayBufferSize(DeviceResourceHandle displayBuffer, uint32_t width, uint32_t height)
    {
        auto& bufferInfo = getDisplayBufferInternal(displayBuffer);
        bufferInfo.viewport.width = width;
        bufferInfo.viewport.height = height;

        // for simplicity trigger all buffers on display to re-render
        // otherwise would have to resolve dependencies via OB links
        for (auto& dispBufferInfo : m_displayBuffers)
            dispBufferInfo.second.needsRerender = true;
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
