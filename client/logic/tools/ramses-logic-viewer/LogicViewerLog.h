//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/Collection.h"
#include "LogicViewer.h"
#include <string>

namespace rlogic
{
    class LogicEngine;
    class Property;
    class LogicNode;
    struct LogicViewerSettings;

    class LogicViewerLog
    {
    public:
        explicit LogicViewerLog(rlogic::LogicEngine& engine, const LogicViewerSettings& settings);

        void logText(const std::string& text);

        template<class T> void logAllInputs(std::string_view headline, std::string_view ltn);

        using PathVector = std::vector<std::string_view>;
        void logInputs(rlogic::LogicNode* obj, const PathVector& path);
        void logProperty(rlogic::Property* prop, const std::string& prefix, PathVector& path);

        [[nodiscard]] const std::string& getText() const
        {
            return m_text;
        }

    private:
        const LogicViewerSettings& m_settings;
        rlogic::LogicEngine& m_logicEngine;
        std::string m_text;
    };

    template<class T>
    inline void LogicViewerLog::logAllInputs(std::string_view headline, std::string_view ltn)
    {
        PathVector path;
        const std::string indent = "    ";
        std::string name = indent + LogicViewer::ltnModule + "." + ltn.data();
        path.push_back(name);
        logText(indent + headline.data());
        for (auto* node : m_logicEngine.getCollection<T>())
        {
            logInputs(node, path);
        }
    }
}

