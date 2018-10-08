//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererLogContext.h"
#include "PlatformAbstraction/PlatformMath.h"

namespace ramses_internal
{
    const RendererLogContext::NewLineType RendererLogContext::NewLine;

    RendererLogContext::RendererLogContext(ERendererLogLevelFlag logLevel)
        : m_activeLogLevelFlags(GetActiveLogLevelFlags(logLevel))
        , m_nodeHandleFilter()
        , m_lineActive(false)
    {
    }

    void RendererLogContext::indent()
    {
        m_indent.append("  ");
    }

    void RendererLogContext::unindent()
    {
        assert(m_indent.getLength() >= 2u);
        m_indent = m_indent.substr(0u, m_indent.getLength() - 2u);
    }

    Bool RendererLogContext::isLogLevelFlagEnabled(ERendererLogLevelFlag logLevelFlag) const
    {
        return (m_activeLogLevelFlags & logLevelFlag) != 0u;
    }

    void RendererLogContext::setNodeHandleFilter(NodeHandle nodeHandleFilter)
    {
        m_nodeHandleFilter = nodeHandleFilter;
    }

    Bool RendererLogContext::isMatchingNodeHandeFilter(NodeHandle nodeHandleFilter) const
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

    String RendererLogContext::ExtractStringFromFilter(const String& filter)
    {
        String resultString = filter.substr(1u, filter.getLength());

        const UInt lastCharIndex = max<Int>(0, resultString.getLength() - 1);
        if (resultString[lastCharIndex] == '*')
        {
            resultString = resultString.substr(0, lastCharIndex);
        }

        return resultString;
    }

    UInt32 RendererLogContext::GetActiveLogLevelFlags(const ERendererLogLevelFlag logLevel)
    {
        UInt32 activeLogLevels = 0u;
        switch (logLevel)
        {
        case ERendererLogLevelFlag_Details:
            activeLogLevels |= ERendererLogLevelFlag_Details;
        case ERendererLogLevelFlag_Info:
            activeLogLevels |= ERendererLogLevelFlag_Info;
        case ERendererLogLevelFlag_Warn:
            activeLogLevels |= ERendererLogLevelFlag_Warn;
        case ERendererLogLevelFlag_Error:
        default:
            activeLogLevels |= ERendererLogLevelFlag_Error;
        }
        return activeLogLevels;
    }
}
