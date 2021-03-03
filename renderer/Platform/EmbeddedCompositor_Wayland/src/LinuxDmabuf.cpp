//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/LinuxDmabuf.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/ThreadLocalLogForced.h"
#include <cassert>
#include <unistd.h>

namespace ramses_internal
{
    const unsigned int LinuxDmabufBufferData::MAX_DMABUF_PLANES;

    LinuxDmabufBufferData::PlaneData::PlaneData()
        : m_fd(-1)
    {
    }

    LinuxDmabufBufferData::PlaneData::PlaneData(PlaneData const& other)
    {
        if (&other == this)
        {
            return;
        }

        if (other.m_fd >= 0)
        {
            m_fd = ::dup(other.m_fd);
        }
        else
        {
            m_fd = -1;
        }

        m_offset = other.m_offset;
        m_stride = other.m_stride;
        m_modifier = other.m_modifier;
    }

    LinuxDmabufBufferData::PlaneData::~PlaneData()
    {
        if (m_fd >= 0)
        {
            ::close(m_fd);
        }
    }

    LinuxDmabufBufferData::LinuxDmabufBufferData()
        : m_numPlanes(0)
    {
        // Set a valid do-nothing callback
        clearDestroyCallback();
    }

    LinuxDmabufBufferData::~LinuxDmabufBufferData()
    {
        m_destroyCallback(this);
    }

    void LinuxDmabufBufferData::noopDestroyCallback(LinuxDmabufBufferData* dmabuf)
    {
        UNUSED(dmabuf);
    }

    void LinuxDmabufBufferData::clearDestroyCallback()
    {
        setDestroyCallback(std::bind(&LinuxDmabufBufferData::noopDestroyCallback, this, std::placeholders::_1));
    }

    void LinuxDmabufBufferData::setDestroyCallback(const std::function<void(LinuxDmabufBufferData*)>& func)
    {
        m_destroyCallback = func;
    }

    unsigned int LinuxDmabufBufferData::getNumPlanes()
    {
        return m_numPlanes;
    }

    /**
    * Ownership of the file descriptor is not transferred to the
    * caller. If the caller needs their own copy, they should dup()
    * it manually.
    *
    * @return File descriptor of the dmabuf pixel data, or a negative value
    *         if no data has been set for the indicated plane.
    */
    int LinuxDmabufBufferData::getFd(unsigned int planeIndex)
    {
        return m_planeData[planeIndex].m_fd;
    }

    uint32_t LinuxDmabufBufferData::getOffset(unsigned int planeIndex)
    {
        return m_planeData[planeIndex].m_offset;
    }

    uint32_t LinuxDmabufBufferData::getStride(unsigned int planeIndex)
    {
        return m_planeData[planeIndex].m_stride;
    }

    uint64_t LinuxDmabufBufferData::getModifier(unsigned int planeIndex)
    {
        return m_planeData[planeIndex].m_modifier;
    }

    bool LinuxDmabufBufferData::isPlaneDataSet(unsigned int planeIndex)
    {
        return m_planeData[planeIndex].m_fd >= 0;
    }

    /**
    @param fd[in] fd File descriptor to the dmabuf pixel data. Ownership is taken away from the caller.
    */
    void LinuxDmabufBufferData::setPlaneData(unsigned int planeIndex, int fd, uint32_t offset, uint32_t stride, uint64_t modifier)
    {
        // Refuse to overwrite a previous installed set of data
        assert(!isPlaneDataSet(planeIndex));
        if (isPlaneDataSet(planeIndex))
        {
            return;
        }

        m_planeData[planeIndex].m_fd = fd;
        m_planeData[planeIndex].m_offset = offset;
        m_planeData[planeIndex].m_stride = stride;
        m_planeData[planeIndex].m_modifier = modifier;

        m_numPlanes++;

        LOG_INFO(CONTEXT_RENDERER, "LinuxDmabufBufferData::setPlaneData(): plane data is set successfully, no. planes :" << m_numPlanes);
    }

    int32_t LinuxDmabufBufferData::getWidth()
    {
        return m_width;
    }

    void LinuxDmabufBufferData::setWidth(int32_t width)
    {
        m_width = width;
    }

    int32_t LinuxDmabufBufferData::getHeight()
    {
        return m_height;
    }

    void LinuxDmabufBufferData::setHeight(int32_t height)
    {
        m_height = height;
    }

    uint32_t LinuxDmabufBufferData::getFormat()
    {
        return m_format;
    }

    void LinuxDmabufBufferData::setFormat(uint32_t format)
    {
        m_format = format;
    }

    uint32_t LinuxDmabufBufferData::getFlags()
    {
        return m_flags;
    }

    void LinuxDmabufBufferData::setFlags(uint32_t flags)
    {
        m_flags = flags;
    }
}
