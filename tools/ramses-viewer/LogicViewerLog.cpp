//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerLog.h"
#include "LogicViewer.h"
#include "ViewerSettings.h"
#include "ramses/client/Scene.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LogicNode.h"
#include "ramses/client/logic/Property.h"
#include "fmt/format.h"

namespace ramses::internal
{
    LogicViewerLog::LogicViewerLog(ramses::LogicEngine& engine, const ViewerSettings& settings)
        : m_settings(settings)
        , m_logicEngine(engine)
    {
    }

    void LogicViewerLog::logText(const std::string& text)
    {
        m_text.append(text);
    }

    void LogicViewerLog::logInputs(ramses::LogicNode* obj, const PathVector& path)
    {
        const auto joinedPath = fmt::format("{}", fmt::join(path.begin(), path.end(), "."));
        std::string prefix;
        if ((m_settings.luaPreferObjectIds) || obj->getName().empty())
        {
            prefix = fmt::format("{}[{}]", joinedPath, obj->getSceneObjectId().getValue());
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

    void LogicViewerLog::logProperty(ramses::Property* prop, const std::string& prefix, PathVector& path)
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
        case ramses::EPropertyType::Int32:
            logText(fmt::format("{} = {}\n", strPath, prop->get<int32_t>().value()));
            break;
        case ramses::EPropertyType::Int64:
            logText(fmt::format("{} = {}\n", strPath, prop->get<int64_t>().value()));
            break;
        case ramses::EPropertyType::Float:
            logText(fmt::format("{} = {}\n", strPath, prop->get<float>().value()));
            break;
        case ramses::EPropertyType::Vec2f: {
            auto val = prop->get<ramses::vec2f>().value();
            logText(fmt::format("{} = {{ {}, {} }}\n", strPath, val[0], val[1]));
            break;
        }
        case ramses::EPropertyType::Vec3f: {
            auto val = prop->get<ramses::vec3f>().value();
            logText(fmt::format("{} = {{ {}, {}, {} }}\n", strPath, val[0], val[1], val[2]));
            break;
        }
        case ramses::EPropertyType::Vec4f: {
            auto val = prop->get<ramses::vec4f>().value();
            logText(fmt::format("{} = {{ {}, {}, {}, {} }}\n", strPath, val[0], val[1], val[2], val[3]));
            break;
        }
        case ramses::EPropertyType::Vec2i: {
            auto val = prop->get<ramses::vec2i>().value();
            logText(fmt::format("{} = {{ {}, {} }}\n", strPath, val[0], val[1]));
            break;
        }
        case ramses::EPropertyType::Vec3i: {
            auto val = prop->get<ramses::vec3i>().value();
            logText(fmt::format("{} = {{ {}, {}, {} }}\n", strPath, val[0], val[1], val[2]));
            break;
        }
        case ramses::EPropertyType::Vec4i: {
            auto val = prop->get<ramses::vec4i>().value();
            logText(fmt::format("{} = {{ {}, {}, {}, {} }}\n", strPath, val[0], val[1], val[2], val[3]));
            break;
        }
        case ramses::EPropertyType::Struct:
            for (size_t i = 0U; i < prop->getChildCount(); ++i)
            {
                logProperty(prop->getChild(i), prefix, path);
            }
            break;
        case ramses::EPropertyType::Bool: {
            logText(fmt::format("{} = {}\n", strPath, prop->get<bool>().value()));
            break;
        }
        case ramses::EPropertyType::String:
            logText(fmt::format("{} = '{}'\n", strPath, prop->get<std::string>().value()));
            break;
        case ramses::EPropertyType::Array:
            break;
        }
        path.pop_back();
    }

}

