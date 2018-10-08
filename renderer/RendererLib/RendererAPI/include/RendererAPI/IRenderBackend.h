//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDERBACKEND_H
#define RAMSES_IRENDERBACKEND_H

namespace ramses_internal
{
    class ISurface;
    class IDevice;
    class IEmbeddedCompositor;
    class ITextureUploadingAdapter;

    class IRenderBackend
    {
    public:
        virtual ~IRenderBackend() {};

        // Get components
        virtual ISurface&                       getSurface() const = 0;
        virtual IDevice&                        getDevice() const = 0;
        virtual IEmbeddedCompositor&            getEmbeddedCompositor() const = 0;
        virtual ITextureUploadingAdapter&       getTextureUploadingAdapter()  const = 0;
    };
}

#endif
