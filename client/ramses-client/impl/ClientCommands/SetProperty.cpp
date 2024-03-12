//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SetProperty.h"
#include "ramses-client-api/Scene.h"
#include "SceneImpl.h"
#include "RamsesClientImpl.h"
#include "Utils/LogMacros.h"
#include "ramses-framework-api/EValidationSeverity.h"
#include "RamsesObjectTypeUtils.h"

namespace ramses_internal
{
    namespace
    {
        const char* SetPropertyTypeNames[] = {"visible", "uniform.*", "depth.write", "depth.func"};

        struct SupportedObjectType
        {
            std::string typeName;
            ramses::ERamsesObjectType type;
            std::vector<SetProperty::Type> properties;
        };

        const std::vector<SupportedObjectType>& GetSupportedProperties()
        {
            static std::vector<SupportedObjectType> properties = {
                {"node", ramses::ERamsesObjectType_Node, {SetProperty::Type::Visible}},
                {"meshnode", ramses::ERamsesObjectType_MeshNode, {SetProperty::Type::Visible}},
                {"appearance", ramses::ERamsesObjectType_Appearance, {SetProperty::Type::Uniform, SetProperty::Type::DepthWrite, SetProperty::Type::DepthFunc}},
            };
            return properties;
        }

        std::vector<std::string> GetPropertyNames()
        {
            std::vector<std::string> result;
            auto len = sizeof(SetPropertyTypeNames) / sizeof(SetPropertyTypeNames[0]);
            result.reserve(len);
            for (size_t i = 0; i < len; ++i)
                result.push_back(SetPropertyTypeNames[i]);
            return result;
        }

        std::vector<std::string> GetObjectTypeNames()
        {
            auto& supportedProperties = GetSupportedProperties();
            std::vector<std::string> result;
            result.reserve(supportedProperties.size());
            for (const auto& t : supportedProperties)
                result.push_back(t.typeName);
            return result;
        }

        ramses::ERamsesObjectType FindObjectType(const char* name)
        {
            ramses::ERamsesObjectType found = ramses::ERamsesObjectType_Invalid;
            auto& supportedProperties = GetSupportedProperties();
            for (auto& t : supportedProperties)
            {
                if (t.typeName.find(name) == 0)
                {
                    if (found != ramses::ERamsesObjectType_Invalid)
                        return ramses::ERamsesObjectType_Invalid; // ambiguous result
                    found = t.type;
                }
            }
            return found;
        }

        SetProperty::Type FindProperty(const SupportedObjectType& t, const std::string& name)
        {
            SetProperty::Type found = SetProperty::Type::Invalid;
            for (auto& p : t.properties)
            {
                std::string fullName = SetPropertyTypeNames[static_cast<size_t>(p)];
                bool wildcardMatch = (fullName.back() == '*' && name.find(fullName.substr(0, fullName.size() - 1)) == 0);
                if (fullName.find(name) == 0 || wildcardMatch)
                {
                    if (found != SetProperty::Type::Invalid)
                        return SetProperty::Type::Invalid; // ambigous result
                    found = p;
                }
            }
            return found;
        }
    }
}

namespace ramses_internal
{
    SetProperty::SetProperty(ramses::RamsesClientImpl& client)
        : m_client(client)
    {
        description = fmt::format("Usage: sceneId sceneObjId {} value - modifies a scene object", fmt::join(GetPropertyNames(), "|"));
        registerKeyword("setprop");
    }

    bool SetProperty::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 5)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "{}", description);
            return false;
        }
        ramses::sceneId_t sceneId;
        ArgumentConverter<uint64_t>::tryConvert(input[1], sceneId.getReference());
        if (!sceneId.isValid())
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid SceneId: {}", input[1]);
            return false;
        }
        ramses::sceneObjectId_t objectId;
        ArgumentConverter<uint64_t>::tryConvert(input[2], objectId.getReference());
        if (!objectId.isValid())
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid SceneObjectId: {}", input[2]);
            return false;
        }

        SceneCommandSetProperty command;
        command.prop = input[3];
        command.value = input[4];
        command.id = ramses::sceneObjectId_t(objectId);
        m_client.enqueueSceneCommand(ramses::sceneId_t(sceneId), std::move(command));
        return true;
    }

    SetProperty::Type SetProperty::GetPropertyType(ramses::ERamsesObjectType objType, const std::string& name)
    {
        SetProperty::Type found = SetProperty::Type::Invalid;
        auto& supportedProperties = GetSupportedProperties();
        for (auto& t : supportedProperties)
        {
            if (ramses::RamsesObjectTypeUtils::IsTypeMatchingBaseType(objType, t.type))
            {
                found = FindProperty(t, name);
                if (found != SetProperty::Type::Invalid)
                    return found; // continue if not found: there may be a more specialized type
            }
        }
        return found;
    }
}

namespace ramses_internal
{
    SetPropertyAll::SetPropertyAll(ramses::RamsesClientImpl& client)
        : m_client(client)
    {
        registerKeyword("setall");
        description = fmt::format("Usage: sceneId objType {} value - modifies all scene objects of given type", fmt::join(GetPropertyNames(), "|"));
    }

    bool SetPropertyAll::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 5)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "{}", description);
            return false;
        }
        ramses::sceneId_t sceneId;
        ArgumentConverter<uint64_t>::tryConvert(input[1], sceneId.getReference());
        if (!sceneId.isValid())
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid SceneId: {}", input[1]);
            return false;
        }
        SceneCommandSetProperty command;
        auto type = input[2];
        command.prop = input[3];
        command.value = input[4];
        command.type = FindObjectType(type.c_str());
        if (command.type == ramses::ERamsesObjectType_Invalid)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid typename:{}. Must be one of ({})", type, fmt::join(GetObjectTypeNames(), "|"));
            return false;
        }
        if (SetProperty::GetPropertyType(command.type, command.prop.stdRef()) == SetProperty::Type::Invalid)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Unsupported property '{}' for type {}", command.prop, type);
            return false;
        }
        m_client.enqueueSceneCommand(sceneId, std::move(command));
        return true;
    }
}
