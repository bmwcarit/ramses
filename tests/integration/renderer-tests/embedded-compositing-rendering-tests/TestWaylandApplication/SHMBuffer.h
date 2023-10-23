//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "wayland-client.h"

#include <string>

namespace ramses::internal
{
    class SHMBuffer
    {
    public:
        SHMBuffer(wl_shm* shm, uint32_t width, uint32_t height, uint32_t id);
        ~SHMBuffer();
        [[nodiscard]] bool isFree()const;
        [[nodiscard]] uint32_t getWidth()const;
        [[nodiscard]] uint32_t getHeight()const;
        [[nodiscard]] uint8_t* getPixelData()const;
        [[nodiscard]] wl_buffer* getWaylandBuffer()const;
        [[nodiscard]] uint32_t getId()const;
        void attachAndCommitToSurface(wl_surface* surface);

    private:

        void release();
        static void ReleaseCallback(void* data, wl_buffer* buffer);

        static int CreateAnonymousFile(off_t size);
        static int CreateTmpFileCloexec(const std::string& tmpname);

        const struct Buffer_Listener : public wl_buffer_listener
        {
            Buffer_Listener() : wl_buffer_listener()
            {
                release = ReleaseCallback;
            }
        } m_bufferListener;

        uint32_t m_width;
        uint32_t m_height;
        wl_buffer* m_buffer;
        uint8_t* m_pixelData;
        uint32_t m_id;
        uint32_t m_size;
        bool m_isFree;
    };
}

