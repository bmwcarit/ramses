//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SetProperty.h"
#include "ramses/client/Scene.h"
#include "impl/SceneImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "internal/Core/Utils/LogMacros.h"
#include <array>

namespace ramses::internal
{
    namespace
    {
        std::array SetPropertyTypeNames = {"visible", "uniform.*", "depth.write", "depth.func"};

        struct SupportedObjectType
        {
            std::string typeName;
            ERamsesObjectType type;
            std::vector<SetProperty::Type> properties;
        };

        const std::vector<SupportedObjectType>& GetSupportedProperties()
        {
            static std::vector<SupportedObjectType> properties = {
                {"node", ERamsesObjectType::Node, {SetProperty::Type::Visible}},
                {"meshnode", ERamsesObjectType::MeshNode, {SetProperty::Type::Visible}},
                {"appearance", ERamsesObjectType::Appearance, {SetProperty::Type::Uniform, SetProperty::Type::DepthWrite, SetProperty::Type::DepthFunc}},
            };
            return properties;
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

        ERamsesObjectType FindObjectType(const char* name)
        {
            ERamsesObjectType found = ERamsesObjectType::Invalid;
            auto& supportedProperties = GetSupportedProperties();
            for (auto& t : supportedProperties)
            {
                if (t.typeName.find(name) == 0)
                {
                    if (found != ERamsesObjectType::Invalid)
                        return ERamsesObjectType::Invalid; // ambiguous result
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

namespace ramses::internal
{
    SetProperty::SetProperty(RamsesClientImpl& client)
        : m_client(client)
    {
        description = fmt::format("Usage: sceneId sceneObjId {} value - modifies a scene object", fmt::join(SetPropertyTypeNames, "|"));
        registerKeyword("setprop");
    }

    bool SetProperty::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 5)
        {
            LOG_ERROR(CONTEXT_RAMSH, "{}", description);
            return false;
        }
        ramses::sceneId_t sceneId;
        ArgumentConverter<uint64_t>::tryConvert(input[1], sceneId.getReference());
        if (!sceneId.isValid())
        {
            LOG_ERROR(CONTEXT_RAMSH, "Invalid SceneId: {}", input[1]);
            return false;
        }
        ramses::sceneObjectId_t objectId;
        ArgumentConverter<uint64_t>::tryConvert(input[2], objectId.getReference());
        if (!objectId.isValid())
        {
            LOG_ERROR(CONTEXT_RAMSH, "Invalid SceneObjectId: {}", input[2]);
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
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(objType, t.type))
            {
                found = FindProperty(t, name);
                if (found != SetProperty::Type::Invalid)
                    return found; // continue if not found: there may be a more specialized type
            }
        }
        return found;
    }
}

namespace ramses::internal
{
    SetPropertyAll::SetPropertyAll(RamsesClientImpl& client)
        : m_client(client)
    {
        registerKeyword("setall");
        description = fmt::format("Usage: sceneId objType {} value - modifies all scene objects of given type", fmt::join(SetPropertyTypeNames, "|"));
    }

    bool SetPropertyAll::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 5)
        {
            LOG_ERROR(CONTEXT_RAMSH, "{}", description);
            return false;
        }
        ramses::sceneId_t sceneId;
        ArgumentConverter<uint64_t>::tryConvert(input[1], sceneId.getReference());
        if (!sceneId.isValid())
        {
            LOG_ERROR(CONTEXT_RAMSH, "Invalid SceneId: {}", input[1]);
            return false;
        }
        SceneCommandSetProperty command;
        auto type = input[2];
        command.prop = input[3];
        command.value = input[4];
        command.type = FindObjectType(type.c_str());
        if (command.type == ERamsesObjectType::Invalid)
        {
            LOG_ERROR(CONTEXT_RAMSH, "Invalid typename:{}. Must be one of ({})", type, fmt::join(GetObjectTypeNames(), "|"));
            return false;
        }
        if (SetProperty::GetPropertyType(command.type, command.prop) == SetProperty::Type::Invalid)
        {
            LOG_ERROR(CONTEXT_RAMSH, "Unsupported property '{}' for type {}", command.prop, type);
            return false;
        }
        m_client.enqueueSceneCommand(sceneId, std::move(command));
        return true;
    }
}
