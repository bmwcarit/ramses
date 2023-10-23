//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/Core/Common/BitForgeMacro.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include <cstdint>
#include <string>

namespace ramses::internal
{
    enum ERendererLogLevelFlag : uint32_t
    {
        // Renderer log levels
        ERendererLogLevelFlag_Error                    = BIT(0u),
        ERendererLogLevelFlag_Warn                     = BIT(1u),
        ERendererLogLevelFlag_Info                     = BIT(2u),
        ERendererLogLevelFlag_Details                  = BIT(3u)
    };

    class RendererLogContext
    {
    public:
        explicit RendererLogContext(ERendererLogLevelFlag logLevel);

        void indent();
        void unindent();
        [[nodiscard]] bool isLogLevelFlagEnabled(ERendererLogLevelFlag logLevelFlag) const;

        void setNodeHandleFilter(NodeHandle nodeHandleFilter);
        [[nodiscard]] bool isMatchingNodeHandeFilter(NodeHandle nodeHandleFilter) const;

        template <typename T>
        RendererLogContext& operator<<(const T& value);

        [[nodiscard]] const StringOutputStream& getStream() const;

    private:
        static std::string ExtractStringFromFilter(const std::string& filter);
        static uint32_t GetActiveLogLevelFlags(const ERendererLogLevelFlag logLevel);

        StringOutputStream    m_stream;
        const uint32_t          m_activeLogLevelFlags;
        NodeHandle            m_nodeHandleFilter;
        std::string           m_indent;
        bool                  m_lineActive;

        struct NewLineType
        {
        };

    public:
        static const NewLineType NewLine;
    };

    template <typename T>
    inline RendererLogContext& RendererLogContext::operator << (const T& value)
    {
        if (!m_lineActive)
        {
            m_stream << m_indent;
            m_lineActive = true;
        }
        m_stream << value;
        return *this;
    }

    template <>
    inline RendererLogContext& RendererLogContext::operator << <RendererLogContext::NewLineType> (const RendererLogContext::NewLineType& /*unused*/)
    {
        m_stream << "\n";
        m_lineActive = false;
        return *this;
    }
}
