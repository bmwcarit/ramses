//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/ResourceUploadRenderBackend.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"

namespace ramses::internal
{
    ResourceUploadRenderBackend::ResourceUploadRenderBackend(IContext& context, IDevice& device)
        : m_context(context)
        , m_device(device)
    {
    }

    IContext& ResourceUploadRenderBackend::getContext() const
    {
        return m_context;
    }

    IDevice& ResourceUploadRenderBackend::getDevice() const
    {
        return m_device;
    }
}
