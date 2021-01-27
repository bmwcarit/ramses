//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/ResourceUploadRenderBackend.h"
#include "RendererAPI/IContext.h"
#include "RendererAPI/IDevice.h"

namespace ramses_internal
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
