//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCONFIGIMPL_H
#define RAMSES_RENDERERCONFIGIMPL_H

#include "RendererLib/RendererConfig.h"
#include "StatusObjectImpl.h"

namespace CLI
{
    class App;
}

namespace ramses
{
    class IBinaryShaderCache;
    class IRendererResourceCache;

    class RendererConfigImpl : public StatusObjectImpl
    {
    public:
        RendererConfigImpl(int32_t argc, char const* const* argv);

        void registerOptions(CLI::App& cli);

        status_t enableSystemCompositorControl();
        status_t setWaylandEmbeddedCompositingSocketGroup(const char* groupname);
        const char* getWaylandSocketEmbeddedGroup() const;

        status_t setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);
        uint32_t getWaylandSocketEmbeddedPermissions() const;

        status_t setWaylandEmbeddedCompositingSocketName(const char* socketname);
        const char* getWaylandEmbeddedCompositingSocketName() const;

        status_t setWaylandEmbeddedCompositingSocketFD(int fd);
        int getWaylandSocketEmbeddedFD() const;

        status_t setSystemCompositorWaylandDisplay(const char* waylandDisplay);
        const char* getSystemCompositorWaylandDisplay() const;

        status_t setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec);

        status_t setBinaryShaderCache(IBinaryShaderCache& cache);
        IBinaryShaderCache* getBinaryShaderCache() const;

        status_t setRendererResourceCache(IRendererResourceCache& cache);
        IRendererResourceCache* getRendererResourceCache() const;

        status_t setOffscreenBufferDoubleBufferingEnabled(bool isDoubleBuffered);
        bool isOffscreenBufferDoubleBufferingEnabled() const;

        status_t setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period);
        std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;

        //impl methods
        const ramses_internal::RendererConfig& getInternalRendererConfig() const;

        virtual status_t validate() const override;

    private:
        ramses_internal::RendererConfig    m_internalConfig;
        IBinaryShaderCache*                m_binaryShaderCache;
        IRendererResourceCache*            m_rendererResourceCache;
    };
}

#endif
