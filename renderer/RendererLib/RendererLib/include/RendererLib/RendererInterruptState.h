//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERINTERRUPTSTATE_H
#define RAMSES_RENDERERINTERRUPTSTATE_H

#include "RendererAPI/Types.h"
#include "RendererAPI/SceneRenderExecutionIterator.h"
#include "SceneAPI/SceneId.h"

namespace ramses_internal
{
    class RendererInterruptState
    {
    public:
        RendererInterruptState(DisplayHandle display = DisplayHandle::Invalid(), DeviceResourceHandle displayBuffer = DeviceResourceHandle::Invalid(), SceneId sceneId = SceneId::DefaultValue(), const SceneRenderExecutionIterator& executorState = {})
            : m_display(display)
            , m_displayBuffer(displayBuffer)
            , m_sceneId(sceneId)
            , m_executorState(executorState)
        {
        }

        Bool isInterrupted() const
        {
            return IsInterrupted(m_executorState);
        }

        Bool isInterrupted(DisplayHandle display) const
        {
            return display == m_display;
        }

        Bool isInterrupted(DisplayHandle display, DeviceResourceHandle displayBuffer) const
        {
            return display == m_display && displayBuffer == m_displayBuffer;
        }

        Bool isInterrupted(DisplayHandle display, DeviceResourceHandle displayBuffer, SceneId sceneId) const
        {
            return display == m_display && displayBuffer == m_displayBuffer && sceneId == m_sceneId;
        }

        const SceneRenderExecutionIterator& getExecutorState() const
        {
            return m_executorState;
        }

        DeviceResourceHandle getInterruptedDisplayBuffer() const
        {
            return m_displayBuffer;
        }

        static Bool IsInterrupted(const SceneRenderExecutionIterator& renderIterator)
        {
            return renderIterator.getFlattenedRenderableIdx() > 0u;
        }

    private:
        DisplayHandle        m_display;
        DeviceResourceHandle m_displayBuffer;
        SceneId              m_sceneId;
        SceneRenderExecutionIterator m_executorState;
    };
}

#endif
