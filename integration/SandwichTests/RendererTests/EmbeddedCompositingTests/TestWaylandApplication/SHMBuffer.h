//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHMBUFFER_H
#define RAMSES_SHMBUFFER_H

#include "Collections/String.h"
#include "wayland-client.h"

namespace ramses_internal
{
    class SHMBuffer
    {
    public:
        SHMBuffer(wl_shm* shm, uint32_t width, uint32_t height, uint32_t id);
        ~SHMBuffer();
        bool isFree()const;
        uint32_t getWidth()const;
        uint32_t getHeight()const;
        uint8_t* getPixelData()const;
        wl_buffer* getWaylandBuffer()const;
        uint32_t getId()const;
        void attachAndCommitToSurface(wl_surface* surface);

    private:

        void release();
        static void ReleaseCallback(void* data, wl_buffer* buffer);

        static int CreateAnonymousFile(off_t size);
        static int CreateTmpFileCloexec(const String& tmpname);

        const struct Buffer_Listener : public wl_buffer_listener
        {
            Buffer_Listener()
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
#endif
