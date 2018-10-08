//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTUREUPLOADINGADAPTER_WAYLAND_H
#define RAMSES_TEXTUREUPLOADINGADAPTER_WAYLAND_H

#include "Platform_Base/TextureUploadingAdapter_Base.h"
#include "WaylandEGLExtensionProcs.h"

namespace ramses_internal
{
    class TextureUploadingAdapter_Wayland : public TextureUploadingAdapter_Base
    {
    public:
        TextureUploadingAdapter_Wayland(IDevice& device, wl_display* waylandWindowDisplay, wl_display* embeddedCompositingDisplay);
        ~TextureUploadingAdapter_Wayland();

        void uploadTextureFromWaylandResource(DeviceResourceHandle textureHandle, EGLClientBuffer bufferResource);

    private:
        const WaylandEGLExtensionProcs  m_waylandEglExtensionProcs;
        wl_display* const               m_embeddedCompositingDisplay;
    };
}

#endif
