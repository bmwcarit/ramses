//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/RendererConfigData.h"

namespace ramses
{
    class IBinaryShaderCache;
}

namespace ramses::internal
{
    class RendererConfigImpl
    {
    public:
        RendererConfigImpl();

        [[nodiscard]] bool enableSystemCompositorControl();

        [[nodiscard]] bool setSystemCompositorWaylandDisplay(std::string_view waylandDisplay);
        [[nodiscard]] std::string_view getSystemCompositorWaylandDisplay() const;

        [[nodiscard]] bool setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec);

        [[nodiscard]] bool setBinaryShaderCache(IBinaryShaderCache& cache);
        [[nodiscard]] ramses::IBinaryShaderCache* getBinaryShaderCache() const;

        [[nodiscard]] bool setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period);
        [[nodiscard]] std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;

        //impl methods
        [[nodiscard]] const ramses::internal::RendererConfigData& getInternalRendererConfig() const;

    private:
        ramses::internal::RendererConfigData  m_internalConfig;
        IBinaryShaderCache*                m_binaryShaderCache{nullptr};
    };
}
