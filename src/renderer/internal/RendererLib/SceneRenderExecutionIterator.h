//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace ramses::internal
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

        [[nodiscard]] uint32_t getRenderPassIdx() const
        {
            return m_renderPassIdx;
        }

        [[nodiscard]] uint32_t getRenderableIdx() const
        {
            return m_renderableIdx;
        }

        [[nodiscard]] uint32_t getFlattenedRenderableIdx() const
        {
            return m_flattenedRenderableIdx;
        }

        bool operator==(const SceneRenderExecutionIterator& other) const
        {
            return m_renderPassIdx == other.m_renderPassIdx
                && m_renderableIdx == other.m_renderableIdx
                && m_flattenedRenderableIdx == other.m_flattenedRenderableIdx;
        }

        bool operator!=(const SceneRenderExecutionIterator& other) const
        {
            return !operator==(other);
        }

    private:
        uint32_t m_renderPassIdx = 0u;
        uint32_t m_renderableIdx = 0u;
        uint32_t m_flattenedRenderableIdx = 0u;
    };
}
