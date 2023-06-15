//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LINUXDMABUF_H
#define RAMSES_LINUXDMABUF_H

#include <cstdint>
#include <functional>

namespace ramses_internal
{
    class LinuxDmabufParams;

    class LinuxDmabufBufferData
    {
    public:
        static unsigned int const MAX_DMABUF_PLANES = 4;

        LinuxDmabufBufferData();
        ~LinuxDmabufBufferData();

        void setDestroyCallback(const std::function<void(LinuxDmabufBufferData*)>& func);
        void clearDestroyCallback();

        unsigned int getNumPlanes();

        int getFd(unsigned int planeIndex);
        uint32_t getOffset(unsigned int planeIndex);
        uint32_t getStride(unsigned int planeIndex);
        uint64_t getModifier(unsigned int planeIndex);

        bool isPlaneDataSet(unsigned int planeIndex);
        void setPlaneData(unsigned int planeIndex, int fd, uint32_t offset, uint32_t stride, uint64_t modifier);

        int32_t getWidth();
        void setWidth(int32_t width);

        int32_t getHeight();
        void setHeight(int32_t height);

        uint32_t getFormat();
        void setFormat(uint32_t format);

        uint32_t getFlags();
        void setFlags(uint32_t flags);

    private:
        struct PlaneData
        {
            PlaneData();
            PlaneData(PlaneData const& other);
            ~PlaneData();

            int m_fd;
            uint32_t m_offset;
            uint32_t m_stride;
            uint64_t m_modifier;
        };

        void noopDestroyCallback(LinuxDmabufBufferData* dmabuf);

        std::function<void(LinuxDmabufBufferData*)> m_destroyCallback;

        std::array<PlaneData, MAX_DMABUF_PLANES> m_planeData;

        int32_t m_width;
        int32_t m_height;
        uint32_t m_format;
        uint32_t m_flags;
        unsigned int m_numPlanes;
    };
}

#endif
