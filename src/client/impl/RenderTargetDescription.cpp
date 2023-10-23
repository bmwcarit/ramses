//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/framework/ValidationReport.h"
#include "impl/APILoggingMacros.h"

// internal
#include "impl/RenderTargetDescriptionImpl.h"

namespace ramses
{
    RenderTargetDescription::RenderTargetDescription()
        : m_impl{ std::make_unique<internal::RenderTargetDescriptionImpl>() }
    {
    }

    RenderTargetDescription::~RenderTargetDescription() = default;

    RenderTargetDescription::RenderTargetDescription(const RenderTargetDescription& other)
        : m_impl{ std::make_unique<internal::RenderTargetDescriptionImpl>(*other.m_impl) }
    {
    }

    RenderTargetDescription::RenderTargetDescription(RenderTargetDescription&& other) noexcept = default;

    RenderTargetDescription& RenderTargetDescription::operator=(const RenderTargetDescription& other)
    {
        m_impl = std::make_unique<internal::RenderTargetDescriptionImpl>(*other.m_impl);
        return *this;
    }

    RenderTargetDescription& RenderTargetDescription::operator=(RenderTargetDescription&& other) noexcept = default;

    void RenderTargetDescription::validate(ValidationReport& report) const
    {
        m_impl->validate(report.impl());
    }

    bool RenderTargetDescription::addRenderBuffer(const RenderBuffer& renderBuffer, std::string* errorMsg)
    {
        const auto status = m_impl->addRenderBuffer(renderBuffer.impl(), errorMsg);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(renderBuffer));
        return status;
    }

    internal::RenderTargetDescriptionImpl& RenderTargetDescription::impl()
    {
        return *m_impl;
    }

    const internal::RenderTargetDescriptionImpl& RenderTargetDescription::impl() const
    {
        return *m_impl;
    }
}
