//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerLog.h"
#include "LogicViewer.h"
#include "LogicViewerSettings.h"
#include "ramses-client-api/Scene.h"
#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LogicNode.h"
#include "ramses-logic/Property.h"
#include "fmt/format.h"

namespace rlogic
{
    LogicViewerLog::LogicViewerLog(rlogic::LogicEngine& engine, const LogicViewerSettings& settings)
        : m_settings(settings)
        , m_logicEngine(engine)
    {
    }

    void LogicViewerLog::logText(const std::string& text)
    {
        m_text.append(text);
    }

    void LogicViewerLog::logInputs(rlogic::LogicNode* obj, const PathVector& path)
    {
        const auto joinedPath = fmt::format("{}", fmt::join(path.begin(), path.end(), "."));
        std::string prefix;
        if ((m_settings.luaPreferObjectIds) || obj->getName().empty())
        {
            prefix = fmt::format("{}[{}]", joinedPath, obj->getId());
        }
        else if (m_settings.luaPreferIdentifiers)
        {
            prefix = fmt::format("{}.{}", joinedPath, obj->getName());
        }
        else
        {
            prefix = fmt::format("{}[\"{}\"]", joinedPath, obj->getName());
        }
        PathVector propertyPath;
        propertyPath.push_back(LogicViewer::ltnIN);
        auto prop = obj->getInputs();
        if (prop)
        {
            for (size_t i = 0U; i < prop->getChildCount(); ++i)
            {
                logProperty(prop->getChild(i), prefix, propertyPath);
            }
        }
    }

    void LogicViewerLog::logProperty(rlogic::Property* prop, const std::string& prefix, PathVector& path)
    {
        if (prop->hasIncomingLink())
            return;

        path.push_back(prop->getName());

        std::string strPath;
        if (m_settings.luaPreferIdentifiers)
        {
            strPath = fmt::format("{}.{}.value", prefix, fmt::join(path.begin(), path.end(), "."));
        }
        else
        {
            strPath = fmt::format("{}[\"{}\"].value", prefix, fmt::join(path.begin(), path.end(), "\"][\""));
        }

        switch (prop->getType())
        {
        case rlogic::EPropertyType::Int32:
            logText(fmt::format("{} = {}\n", strPath, prop->get<int32_t>().value()));
            break;
        case rlogic::EPropertyType::Int64:
            logText(fmt::format("{} = {}\n", strPath, prop->get<int64_t>().value()));
            break;
        case rlogic::EPropertyType::Float:
            logText(fmt::format("{} = {}\n", strPath, prop->get<float>().value()));
            break;
        case rlogic::EPropertyType::Vec2f: {
            auto val = prop->get<rlogic::vec2f>().value();
            logText(fmt::format("{} = {{ {}, {} }}\n", strPath, val[0], val[1]));
            break;
        }
        case rlogic::EPropertyType::Vec3f: {
            auto val = prop->get<rlogic::vec3f>().value();
            logText(fmt::format("{} = {{ {}, {}, {} }}\n", strPath, val[0], val[1], val[2]));
            break;
        }
        case rlogic::EPropertyType::Vec4f: {
            auto val = prop->get<rlogic::vec4f>().value();
            logText(fmt::format("{} = {{ {}, {}, {}, {} }}\n", strPath, val[0], val[1], val[2], val[3]));
            break;
        }
        case rlogic::EPropertyType::Vec2i: {
            auto val = prop->get<rlogic::vec2i>().value();
            logText(fmt::format("{} = {{ {}, {} }}\n", strPath, val[0], val[1]));
            break;
        }
        case rlogic::EPropertyType::Vec3i: {
            auto val = prop->get<rlogic::vec3i>().value();
            logText(fmt::format("{} = {{ {}, {}, {} }}\n", strPath, val[0], val[1], val[2]));
            break;
        }
        case rlogic::EPropertyType::Vec4i: {
            auto val = prop->get<rlogic::vec4i>().value();
            logText(fmt::format("{} = {{ {}, {}, {}, {} }}\n", strPath, val[0], val[1], val[2], val[3]));
            break;
        }
        case rlogic::EPropertyType::Struct:
            for (size_t i = 0U; i < prop->getChildCount(); ++i)
            {
                logProperty(prop->getChild(i), prefix, path);
            }
            break;
        case rlogic::EPropertyType::Bool: {
            logText(fmt::format("{} = {}\n", strPath, prop->get<bool>().value()));
            break;
        }
        case rlogic::EPropertyType::String:
            logText(fmt::format("{} = '{}'\n", strPath, prop->get<std::string>().value()));
            break;
        case rlogic::EPropertyType::Array:
            break;
        }
        path.pop_back();
    }

}

