//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderBuffer.h"

// internal
#include "RenderTargetDescriptionImpl.h"

namespace ramses
{
    RenderTargetDescription::RenderTargetDescription()
        : StatusObject{ std::make_unique<RenderTargetDescriptionImpl>() }
        , m_impl{ static_cast<RenderTargetDescriptionImpl&>(*StatusObject::m_impl) }
    {
    }

    RenderTargetDescription::~RenderTargetDescription() = default;

    RenderTargetDescription::RenderTargetDescription(const RenderTargetDescription& other)
        : StatusObject{ std::make_unique<RenderTargetDescriptionImpl>(other.m_impl) }
        , m_impl{ static_cast<RenderTargetDescriptionImpl&>(*StatusObject::m_impl) }
    {
    }

    RenderTargetDescription::RenderTargetDescription(RenderTargetDescription&& other) noexcept
        : StatusObject{ std::move(other.StatusObject::m_impl) }
        , m_impl{ static_cast<RenderTargetDescriptionImpl&>(*StatusObject::m_impl) }
    {
    }

    RenderTargetDescription& RenderTargetDescription::operator=(const RenderTargetDescription& other)
    {
        StatusObject::m_impl = std::make_unique<RenderTargetDescriptionImpl>(other.m_impl);
        m_impl = static_cast<RenderTargetDescriptionImpl&>(*StatusObject::m_impl);
        return *this;
    }

    RenderTargetDescription& RenderTargetDescription::operator=(RenderTargetDescription&& other) noexcept
    {
        StatusObject::m_impl = std::move(other.StatusObject::m_impl);
        m_impl = static_cast<RenderTargetDescriptionImpl&>(*StatusObject::m_impl);
        return *this;
    }

    status_t RenderTargetDescription::addRenderBuffer(const RenderBuffer& renderBuffer)
    {
        const status_t status = m_impl.get().addRenderBuffer(renderBuffer.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(renderBuffer));
        return status;
    }
}
