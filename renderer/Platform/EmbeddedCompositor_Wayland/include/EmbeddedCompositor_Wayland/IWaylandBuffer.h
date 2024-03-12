//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IWAYLANDBUFFER_H
#define RAMSES_IWAYLANDBUFFER_H

namespace ramses_internal
{
    class WaylandBufferResource;
    class RendererLogContext;
    class WaylandEGLExtensionProcs;

    class IWaylandBuffer
    {
    public:
        virtual ~IWaylandBuffer(){}
        virtual WaylandBufferResource& getResource() const = 0;
        virtual void reference() = 0;
        virtual void release() = 0;
        virtual bool isSharedMemoryBuffer() const = 0;
        virtual void logInfos(RendererLogContext& context, const WaylandEGLExtensionProcs& eglExt) const = 0;
    };
}

#endif
