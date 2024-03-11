//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneCommandVisitor.h"
#include "SceneCommandBuffer.h"
#include "RamsesObjectTypeUtils.h"
#include "SceneImpl.h"
#include "RamsesClientImpl.h"
#include "LogMemoryUtils.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/SceneObject.h"
#include "Utils/LogMacros.h"
#include "RamsesObjectRegistryIterator.h"
#include "Utils/File.h"
#include "SetProperty.h"
#include <numeric>

namespace ramses_internal
{
    void setProperty(ramses::SceneObject* obj, const String& prop, const String& value)
    {
        const auto id = obj->getSceneObjectId();
        switch (SetProperty::GetPropertyType(obj->getType(), prop.stdRef()))
        {
        case SetProperty::Type::Visible:
        {
            assert(obj->isOfType(ramses::ERamsesObjectType_Node));
            auto nodeObj = static_cast<ramses::Node*>(obj);
            if (value == "0" || value == "off")
            {
                nodeObj->setVisibility(ramses::EVisibilityMode::Off);
            }
            else if (value == "1" || value.find("inv") == 0)
            {
                nodeObj->setVisibility(ramses::EVisibilityMode::Invisible);
            }
            else if (value == "2" || value.find("vis") == 0)
            {
                nodeObj->setVisibility(ramses::EVisibilityMode::Visible);
            }
            else
            {
                LOG_ERROR_P(CONTEXT_CLIENT, "Invalid value for <{}>::{}. Must be (off,inv,vis) or (0,1,2).", id.getValue(), prop);
                return;
            }
            break;
        }
        case SetProperty::Type::DepthFunc: {
            assert(obj->isOfType(ramses::ERamsesObjectType_Appearance));
            auto appObj = static_cast<ramses::Appearance*>(obj);
            if (value == "disabled")
            {
                appObj->setDepthFunction(ramses::EDepthFunc_Disabled);
            }
            else if (value == "always")
            {
                appObj->setDepthFunction(ramses::EDepthFunc_Always);
            }
            else if (value == "less")
            {
                appObj->setDepthFunction(ramses::EDepthFunc_Less);
            }
            else if (value == "lessEqual")
            {
                appObj->setDepthFunction(ramses::EDepthFunc_LessEqual);
            }
            else
            {
                LOG_ERROR_P(CONTEXT_CLIENT, "Invalid value for <{}>::{}. Must be (disabled,always,less,lessEqual).", id.getValue(), prop);
                return;
            }
            break;
        }
        case SetProperty::Type::DepthWrite:
        {
            assert(obj->isOfType(ramses::ERamsesObjectType_Appearance));
            auto appObj = static_cast<ramses::Appearance*>(obj);
            if (value == "off" || value == "0")
            {
                appObj->setDepthWrite(ramses::EDepthWrite_Disabled);
            }
            else if (value == "on" || value == "1")
            {
                appObj->setDepthWrite(ramses::EDepthWrite_Enabled);
            }
            else
            {
                LOG_ERROR_P(CONTEXT_CLIENT, "Invalid value for <{}>::{}. Must be (off,on) or (0,1).", id.getValue(), prop);
                return;
            }
            break;
        }
        case SetProperty::Type::Uniform:
        {
            assert(obj->isOfType(ramses::ERamsesObjectType_Appearance));
            auto appObj = static_cast<ramses::Appearance*>(obj);
            const auto name = prop.stdRef().substr(8);
            auto& effect = appObj->getEffect();
            ramses::UniformInput uniform;
            if (ramses::StatusOK != effect.findUniformInput(name.c_str(), uniform))
            {
                LOG_ERROR_P(CONTEXT_CLIENT, "Unknown uniform '{}' for <{}>", name, id.getValue());
                return;
            }
            if (uniform.getDataType() != ramses::EEffectInputDataType_Float || uniform.getElementCount() != 1)
            {
                LOG_ERROR(CONTEXT_CLIENT, "only float[1] uniforms supported currently");
                return;
            }
            appObj->setInputValueFloat(uniform, static_cast<float>(atof(value.c_str())));
            break;
        }
        case SetProperty::Type::Invalid:
            LOG_ERROR_P(CONTEXT_CLIENT, "Unsupported property:'{}' (obj <{}> is of type {}).",
                prop,
                id.getValue(),
                ramses::RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj->getType()));
            break;
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandSetProperty& cmd)
    {
        LOG_INFO_P(CONTEXT_CLIENT, "SceneCommandVisitor::execute: set property on <{}>: {} -> {}",
                   cmd.id.getValue(),
                   cmd.prop,
                   cmd.value);

        if (cmd.id.isValid())
        {
            auto* obj = m_scene.findObjectById(cmd.id);
            if (!obj)
            {
                LOG_ERROR_P(CONTEXT_CLIENT, "Object not found: {}", cmd.id.getValue());
                return;
            }
            setProperty(obj, cmd.prop, cmd.value);
        }
        else
        {
            ramses::RamsesObjectRegistryIterator iter(m_scene.getObjectRegistry(), cmd.type);
            while (auto* obj = iter.getNextNonConst<ramses::SceneObject>())
            {
                setProperty(obj, cmd.prop, cmd.value);
            }
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandForceFallback& cmd)
    {
        ramses::RamsesObject* object = m_scene.findObjectByName(cmd.streamTextureName.c_str());
        if (object)
        {
            if (object->getType() == ramses::ERamsesObjectType_StreamTexture)
            {
                ramses::StreamTexture& streamtexture = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::StreamTexture>(*object);
                LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::execute: " << (cmd.forceFallback?"Enable":"Disable") << " force fallback image for \"" << cmd.streamTextureName << "\"");
                streamtexture.forceFallbackImage(cmd.forceFallback);
            }
            else
                LOG_ERROR(CONTEXT_CLIENT, "SceneCommandVisitor::execute: Set force fallback setting but the object with name \"" << cmd.streamTextureName << "\" is not a StreamTexture");
        }
        else
            LOG_ERROR(CONTEXT_CLIENT, "SceneCommandVisitor::execute: Set force fallback setting but couldn't find any object with name \"" << cmd.streamTextureName << "\"");
    }

    void SceneCommandVisitor::operator()(const SceneCommandFlushSceneVersion& cmd)
    {
        LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::execute: set scene version in next flush to \"" << cmd.sceneVersion << "\"");
        m_scene.setSceneVersionForNextFlush(cmd.sceneVersion);
    }

    void SceneCommandVisitor::operator()(const SceneCommandValidationRequest& cmd)
    {
        if (cmd.optionalObjectName.empty())
        {
            // no object, validate whole scene
            m_scene.validate();
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Validation:  " << m_scene.getValidationReport(cmd.severity));
        }
        else
        {
            if (const ramses::RamsesObject* ro = m_scene.findObjectByName(cmd.optionalObjectName.c_str()))
            {
                ro->validate();
                LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Validation:  " << ro->getValidationReport(cmd.severity));
            }
            else
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Validation could not find requested object with name: " << cmd.optionalObjectName);
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandDumpSceneToFile& cmd) const
    {
        std::vector<Byte> sceneData;
        const auto status = m_scene.serialize(sceneData, false);
        if (status != ramses::StatusOK)
        {
            LOG_ERROR_P(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed dump scene: {} (file:{} dlt:{})", m_scene.getSceneId(), cmd.fileName, cmd.sendViaDLT);
            return;
        }

        const auto filename = String(cmd.fileName.empty() ? fmt::format("S_{}.ramses", m_scene.getSceneId()) : cmd.fileName.stdRef() + ".ramses");
        if (!cmd.fileName.empty())
        {
            File file(filename);
            if (file.open(File::Mode::WriteOverWriteOldBinary))
            {
                if (!file.write(sceneData.data(), sceneData.size()))
                {
                    LOG_ERROR_P(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed to write scene dump: {}", file.getPath());
                }
            }
            else
            {
                LOG_ERROR_P(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed to open: {}", file.getPath());
            }
        }

        if (cmd.sendViaDLT)
        {
            if (GetRamsesLogger().transmit(std::move(sceneData), filename))
            {
                LOG_INFO_P(CONTEXT_CLIENT, "SceneCommandVisitor::execute: started dlt file transfer: {}", filename);
            }
            else
            {
                LOG_ERROR_P(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed to send scene dump file via dlt: {}", filename);
            }
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandLogResourceMemoryUsage& /*cmd*/) const
    {
        MemoryInfoVector memoryInfos = GetMemoryInfoFromScene(m_scene);
        std::sort(memoryInfos.begin(), memoryInfos.end(),[](const MemoryInfo& lhs, const MemoryInfo& rhs){return lhs.memoryUsage > rhs.memoryUsage;});

        LOG_INFO_F(ramses_internal::CONTEXT_CLIENT,([&](ramses_internal::StringOutputStream& out){
            const uint32_t memTotal = std::accumulate(memoryInfos.begin(), memoryInfos.end(), 0u,
                                                      [](uint32_t v, const MemoryInfo& info){return v + info.memoryUsage;});
            out << "\n\rTotal    memory usage: " << memTotal;
            out << "\n\rDetailed memory usage:";
            for (const auto info : memoryInfos)
            {
                out << "\n\r";
                out << info.memoryUsage << "\t";
                out << info.logInfoMesage;
            }
        }));
    }
}
