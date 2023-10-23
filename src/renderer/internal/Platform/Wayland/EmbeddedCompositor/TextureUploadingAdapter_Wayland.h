//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/TextureUploadingAdapter_Base.h"
#include "internal/Platform/Wayland/WaylandEGLExtensionProcs.h"
#include <unordered_map>

namespace ramses::internal
{
    class LinuxDmabufBufferData;

    class TextureUploadingAdapter_Wayland : public TextureUploadingAdapter_Base
    {
    public:
        TextureUploadingAdapter_Wayland(IDevice& device, wl_display* waylandWindowDisplay, wl_display* embeddedCompositingDisplay);
        ~TextureUploadingAdapter_Wayland() override;

        void uploadTextureFromWaylandResource(DeviceResourceHandle textureHandle, wl_resource* bufferResource);
        bool uploadTextureFromLinuxDmabuf(DeviceResourceHandle textureHandle, LinuxDmabufBufferData* dmabuf);

        const WaylandEGLExtensionProcs& getEGLExtension() const;

    private:
        class DmabufEglImage
        {
        public:
            DmabufEglImage(TextureUploadingAdapter_Wayland& parent, EGLImage eglImage, GLenum textureTarget);
            ~DmabufEglImage();

            [[nodiscard]] EGLImage getEglImage() const;
            [[nodiscard]] GLenum getTextureTarget() const;

        private:
            TextureUploadingAdapter_Wayland& m_parent;
            EGLImage m_eglImage = EGL_NO_IMAGE;
            GLenum m_textureTarget;
        };

        void handleDmabufDestroy(LinuxDmabufBufferData* dmabuf);

        DmabufEglImage* importDmabufToEglImage(LinuxDmabufBufferData* dmabuf);

        const WaylandEGLExtensionProcs   m_waylandEglExtensionProcs;
        wl_display* const                m_embeddedCompositingDisplay;
        std::unordered_map<LinuxDmabufBufferData*, DmabufEglImage*> m_dmabufEglImagesMap;

        friend class DmabufEglImage;
    };

    inline const WaylandEGLExtensionProcs& TextureUploadingAdapter_Wayland::getEGLExtension() const
    {
        return m_waylandEglExtensionProcs;
    }
}
