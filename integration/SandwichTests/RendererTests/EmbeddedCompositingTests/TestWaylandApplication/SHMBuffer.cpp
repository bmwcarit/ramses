//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SHMBuffer.h"
#include "assert.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "Utils/LogMacros.h"

SHMBuffer::SHMBuffer(wl_shm* shm, uint32_t width, uint32_t height, uint32_t id)
    : m_width(width)
    , m_height(height)
    , m_buffer(0)
    , m_pixelData(0)
    , m_id(id)
    , m_isFree(true)
{
    assert(shm);

    uint32_t stride = width * 4;
    m_size = stride * height;

    int fd = CreateAnonymousFile(m_size);
    if (fd < 0)
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "SHMBuffer::SHMBuffer CreateAnonymousFile failed!");
        return;
    }

    m_pixelData = static_cast<uint8_t*>(mmap(0, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (m_pixelData == MAP_FAILED)
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "SHMBuffer::SHMBuffer mmap failed!");
        close(fd);
        return;
    }

    wl_shm_pool* pool = wl_shm_create_pool(shm, fd, m_size);
    m_buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);

    wl_buffer_add_listener(m_buffer, &m_bufferListener, this);
    wl_shm_pool_destroy(pool);
    close(fd);
}

SHMBuffer::~SHMBuffer()
{
    wl_buffer_destroy(m_buffer);
    munmap(m_pixelData, m_size);
}

void SHMBuffer::release()
{
    m_isFree = true;
}

bool SHMBuffer::isFree() const
{
    return m_isFree;
}

uint32_t SHMBuffer::getWidth() const
{
    return m_width;
}

uint32_t SHMBuffer::getHeight() const
{
    return m_height;
}

uint8_t* SHMBuffer::getPixelData()const
{
    return m_pixelData;
}

wl_buffer* SHMBuffer::getWaylandBuffer()const
{
    return m_buffer;
}

uint32_t SHMBuffer::getId()const
{
    return m_id;
}

void SHMBuffer::attachAndCommitToSurface(wl_surface* surface)
{
    m_isFree = false;
    wl_surface_attach(surface, m_buffer, 0, 0);
    wl_surface_damage(surface, 0, 0, m_width, m_height);
    wl_surface_commit(surface);
}

void SHMBuffer::ReleaseCallback(void* data, wl_buffer* buffer)
{
    UNUSED(buffer)
    static_cast<SHMBuffer*>(data)->release();
}

int SHMBuffer::CreateTmpFileCloexec(const ramses_internal::String& tmpname)
{
    int32_t fd = mkostemp(const_cast<char*>(tmpname.c_str()), O_CLOEXEC);
    if (fd >= 0)
    {
        unlink(tmpname.c_str());
    }
    else
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "SHMBuffer::CreateTmpFileCloexec failed!");
    }
    return fd;
}

int SHMBuffer::CreateAnonymousFile(off_t size)
{
    int ret;

    const char* path = getenv("XDG_RUNTIME_DIR");
    if (!path)
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "SHMBuffer::CreateAnonymousFile XDG_RUNTIME_DIR not set!");
        return -1;
    }

    ramses_internal::String name = ramses_internal::String(path) + "/SHMBuffer-XXXXXX";

    int fd = CreateTmpFileCloexec(name.c_str());

    if (fd < 0)
    {
        return -1;
    }

    ret = posix_fallocate(fd, 0, size);
    if (ret != 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}
