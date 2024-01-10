//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/logic/LogicEngine.h"
#include <string>

namespace ramses::internal
{
    struct ViewerSettings;

    class LogicViewerLog
    {
    public:
        explicit LogicViewerLog(const ViewerSettings& settings);

        void logText(const std::string& text);

        template<class T> void logAllInputs(ramses::LogicEngine& logicEngine, std::string_view headline, std::string_view ltn);

        void logInputs(ramses::LogicEngine& engine, ramses::LogicNode* obj, std::string_view ltn);

        [[nodiscard]] const std::string& getText() const
        {
            return m_text;
        }

        void setIndent(std::string indent)
        {
            m_indent = std::move(indent);
        }

    private:
        using PathVector = std::vector<std::string_view>;
        void logProperty(ramses::Property* prop, const std::string& prefix, PathVector& path);

        const ViewerSettings& m_settings;
        std::string m_text;
        std::string m_indent;
    };

    template<class T>
    inline void LogicViewerLog::logAllInputs(ramses::LogicEngine& logicEngine, std::string_view headline, std::string_view ltn)
    {
        logText(m_indent + headline.data());
        for (auto* node : logicEngine.getCollection<T>())
        {
            logInputs(logicEngine, node, ltn);
        }
    }
}

