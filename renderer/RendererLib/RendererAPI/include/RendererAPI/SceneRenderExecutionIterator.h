//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENERENDEREXECUTIONITERATOR_H
#define RAMSES_SCENERENDEREXECUTIONITERATOR_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    class SceneRenderExecutionIterator
    {
    public:
        void incrementRenderPassIdx()
        {
            m_renderPassIdx++;
            m_renderableIdx = 0u;
        }

        void incrementRenderableIdx()
        {
            m_renderableIdx++;
            m_flattenedRenderableIdx++;
        }

        UInt32 getRenderPassIdx() const
        {
            return m_renderPassIdx;
        }

        UInt32 getRenderableIdx() const
        {
            return m_renderableIdx;
        }

        UInt32 getFlattenedRenderableIdx() const
        {
            return m_flattenedRenderableIdx;
        }

        Bool operator==(const SceneRenderExecutionIterator& other) const
        {
            return m_renderPassIdx == other.m_renderPassIdx
                && m_renderableIdx == other.m_renderableIdx
                && m_flattenedRenderableIdx == other.m_flattenedRenderableIdx;
        }

        Bool operator!=(const SceneRenderExecutionIterator& other) const
        {
            return !operator==(other);
        }

    private:
        UInt32 m_renderPassIdx = 0u;
        UInt32 m_renderableIdx = 0u;
        UInt32 m_flattenedRenderableIdx = 0u;
    };
}

#endif
