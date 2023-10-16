//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererLogContext.h"
#include "internal/PlatformAbstraction/PlatformMath.h"
#include "internal/PlatformAbstraction/Macros.h"
#include <cassert>

namespace ramses::internal
{
    const RendererLogContext::NewLineType RendererLogContext::NewLine;

    RendererLogContext::RendererLogContext(ERendererLogLevelFlag logLevel)
        : m_activeLogLevelFlags(GetActiveLogLevelFlags(logLevel))
        , m_lineActive(false)
    {
    }

    void RendererLogContext::indent()
    {
        m_indent.append("  ");
    }

    void RendererLogContext::unindent()
    {
        assert(m_indent.size() >= 2u);
        m_indent = m_indent.substr(0u, m_indent.size() - 2u);
    }

    bool RendererLogContext::isLogLevelFlagEnabled(ERendererLogLevelFlag logLevelFlag) const
    {
        return (m_activeLogLevelFlags & logLevelFlag) != 0u;
    }

    void RendererLogContext::setNodeHandleFilter(NodeHandle nodeHandleFilter)
    {
        m_nodeHandleFilter = nodeHandleFilter;
    }

    bool RendererLogContext::isMatchingNodeHandeFilter(NodeHandle nodeHandleFilter) const
    {
        if (!m_nodeHandleFilter.isValid())
        {
            return true;
        }

        return m_nodeHandleFilter == nodeHandleFilter;
    }

    const StringOutputStream& RendererLogContext::getStream() const
    {
        return m_stream;
    }

    std::string RendererLogContext::ExtractStringFromFilter(const std::string& filter)
    {
        auto resultString = filter.substr(1u, filter.size());

        const size_t lastCharIndex{resultString.size() > 1 ? resultString.size() - 1 : 0};
        if (resultString[lastCharIndex] == '*')
        {
            resultString = resultString.substr(0, lastCharIndex);
        }

        return resultString;
    }

    uint32_t RendererLogContext::GetActiveLogLevelFlags(const ERendererLogLevelFlag logLevel)
    {
        uint32_t activeLogLevels = 0u;
        switch (logLevel)
        {
        case ERendererLogLevelFlag_Details:
            activeLogLevels |= ERendererLogLevelFlag_Details;
            RFALLTHROUGH;
        case ERendererLogLevelFlag_Info:
            activeLogLevels |= ERendererLogLevelFlag_Info;
            RFALLTHROUGH;
        case ERendererLogLevelFlag_Warn:
            activeLogLevels |= ERendererLogLevelFlag_Warn;
            RFALLTHROUGH;
        case ERendererLogLevelFlag_Error:
        default:
            activeLogLevels |= ERendererLogLevelFlag_Error;
        }
        return activeLogLevels;
    }
}
