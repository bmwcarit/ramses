//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERLOGCONTEXT_H
#define RAMSES_RENDERERLOGCONTEXT_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/StringOutputStream.h"
#include "Common/BitForgeMacro.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    enum ERendererLogLevelFlag
    {
        // Renderer log levels
        ERendererLogLevelFlag_Error                    = BIT(0),
        ERendererLogLevelFlag_Warn                     = BIT(1),
        ERendererLogLevelFlag_Info                     = BIT(2),
        ERendererLogLevelFlag_Details                  = BIT(3)
    };

    class RendererLogContext
    {
    public:
        RendererLogContext(ERendererLogLevelFlag logLevel);

        void indent();
        void unindent();
        Bool isLogLevelFlagEnabled(ERendererLogLevelFlag level) const;

        void setNodeHandleFilter(NodeHandle nodeHandleFilter);
        Bool isMatchingNodeHandeFilter(NodeHandle nodeHandleFilter) const;

        template <typename T>
        RendererLogContext& operator<<(const T& value);

        const StringOutputStream& getStream() const;

    private:
        static String ExtractStringFromFilter(const String& filter);
        static UInt32 GetActiveLogLevelFlags(const ERendererLogLevelFlag logLevel);

        StringOutputStream    m_stream;
        const UInt32          m_activeLogLevelFlags;
        NodeHandle            m_nodeHandleFilter;
        String                m_indent;
        Bool                  m_lineActive;

        struct NewLineType
        {
            NewLineType() {}
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
    inline RendererLogContext& RendererLogContext::operator << <RendererLogContext::NewLineType> (const RendererLogContext::NewLineType&)
    {
        m_stream << "\n";
        m_lineActive = false;
        return *this;
    }
}

#endif
