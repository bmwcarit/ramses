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
        explicit RendererInterruptState(DeviceResourceHandle displayBuffer = DeviceResourceHandle::Invalid(), SceneId sceneId = SceneId::Invalid(), const SceneRenderExecutionIterator& executorState = {})
            : m_displayBuffer(displayBuffer)
            , m_sceneId(sceneId)
            , m_executorState(executorState)
        {
        }

        [[nodiscard]] bool isInterrupted() const
        {
            return IsInterrupted(m_executorState);
        }

        [[nodiscard]] bool isInterrupted(DeviceResourceHandle displayBuffer) const
        {
            return displayBuffer == m_displayBuffer;
        }

        [[nodiscard]] bool isInterrupted(DeviceResourceHandle displayBuffer, SceneId sceneId) const
        {
            return displayBuffer == m_displayBuffer && sceneId == m_sceneId;
        }

        [[nodiscard]] const SceneRenderExecutionIterator& getExecutorState() const
        {
            return m_executorState;
        }

        [[nodiscard]] DeviceResourceHandle getInterruptedDisplayBuffer() const
        {
            return m_displayBuffer;
        }

        static bool IsInterrupted(const SceneRenderExecutionIterator& renderIterator)
        {
            return renderIterator.getFlattenedRenderableIdx() > 0u;
        }

    private:
        DeviceResourceHandle m_displayBuffer;
        SceneId              m_sceneId;
        SceneRenderExecutionIterator m_executorState;
    };
}

#endif
