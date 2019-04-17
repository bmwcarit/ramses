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
#include "Utils/CommandLineParser.h"
#include "StatusObjectImpl.h"

namespace ramses
{
    class IBinaryShaderCache;
    class IRendererResourceCache;

    class RendererConfigImpl : public StatusObjectImpl
    {
    public:
        RendererConfigImpl(int32_t argc, char const* const* argv);

        status_t enableSystemCompositorControl();
        status_t setWaylandSocketEmbeddedGroup(const char* groupname);
        const char* getWaylandSocketEmbeddedGroup() const;

        status_t setWaylandSocketEmbedded(const char* socketname);
        const char* getWaylandSocketEmbedded() const;

        status_t setWaylandSocketEmbeddedFD(int fd);
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


        //impl methods
        const ramses_internal::RendererConfig& getInternalRendererConfig() const;

        virtual status_t validate(uint32_t indent) const override;

    private:
        ramses_internal::RendererConfig    m_internalConfig;
        IBinaryShaderCache*                m_binaryShaderCache;
        IRendererResourceCache*            m_rendererResourceCache;
    };
}

#endif
