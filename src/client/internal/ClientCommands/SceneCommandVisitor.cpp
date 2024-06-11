//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneCommandVisitor.h"
#include "SceneCommandBuffer.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/SceneImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/SaveFileConfigImpl.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "LogMemoryUtils.h"
#include "ramses/client/Node.h"
#include "ramses/client/Effect.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/SceneObject.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/File.h"
#include "SetProperty.h"
#include <numeric>

namespace ramses::internal
{
    void setProperty(ramses::SceneObject* obj, const std::string& prop, const std::string& value)
    {
        const auto id = obj->getSceneObjectId();
        switch (SetProperty::GetPropertyType(obj->getType(), prop))
        {
        case SetProperty::Type::Visible:
        {
            assert(obj->isOfType(ramses::ERamsesObjectType::Node));
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
                LOG_ERROR(CONTEXT_CLIENT, "Invalid value for <{}>::{}. Must be (off,inv,vis) or (0,1,2).", id.getValue(), prop);
                return;
            }
            break;
        }
        case SetProperty::Type::DepthFunc: {
            assert(obj->isOfType(ERamsesObjectType::Appearance));
            auto appObj = static_cast<Appearance*>(obj);
            if (value == "disabled")
            {
                appObj->setDepthFunction(EDepthFunc::Disabled);
            }
            else if (value == "always")
            {
                appObj->setDepthFunction(EDepthFunc::Always);
            }
            else if (value == "less")
            {
                appObj->setDepthFunction(EDepthFunc::Less);
            }
            else if (value == "lessEqual")
            {
                appObj->setDepthFunction(EDepthFunc::LessEqual);
            }
            else
            {
                LOG_ERROR(CONTEXT_CLIENT, "Invalid value for <{}>::{}. Must be (disabled,always,less,lessEqual).", id.getValue(), prop);
                return;
            }
            break;
        }
        case SetProperty::Type::DepthWrite:
        {
            assert(obj->isOfType(ERamsesObjectType::Appearance));
            auto appObj = static_cast<Appearance*>(obj);
            if (value == "off" || value == "0")
            {
                appObj->setDepthWrite(EDepthWrite::Disabled);
            }
            else if (value == "on" || value == "1")
            {
                appObj->setDepthWrite(EDepthWrite::Enabled);
            }
            else
            {
                LOG_ERROR(CONTEXT_CLIENT, "Invalid value for <{}>::{}. Must be (off,on) or (0,1).", id.getValue(), prop);
                return;
            }
            break;
        }
        case SetProperty::Type::Uniform:
        {
            assert(obj->isOfType(ERamsesObjectType::Appearance));
            auto appObj = static_cast<ramses::Appearance*>(obj);
            const auto name = prop.substr(8);
            auto& effect = appObj->getEffect();

            auto uniform = effect.findUniformInput(name);
            if (!uniform.has_value())
            {
                LOG_ERROR(CONTEXT_CLIENT, "Unknown uniform '{}' for <{}>", name, id.getValue());
                return;
            }
            if (uniform->getDataType() != ramses::EDataType::Float || uniform->getElementCount() != 1)
            {
                LOG_ERROR(CONTEXT_CLIENT, "only float[1] uniforms supported currently");
                return;
            }
            float uniformValue = 0.f;
            if (!ArgumentConverter<float>::tryConvert(value, uniformValue))
            {
                LOG_ERROR(CONTEXT_CLIENT, "conversion to float failed: {}", value);
                return;
            }
            appObj->setInputValue(*uniform, uniformValue);
            break;
        }
        case SetProperty::Type::Invalid:
            LOG_ERROR(CONTEXT_CLIENT, "Unsupported property:'{}' (obj <{}> is of type {}).",
                prop,
                id.getValue(),
                RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj->getType()));
            break;
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandSetProperty& cmd)
    {
        LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::execute: set property on <{}>: {} -> {}",
                   cmd.id.getValue(),
                   cmd.prop,
                   cmd.value);

        if (cmd.id.isValid())
        {
            auto* obj = m_scene.findObjectById(cmd.id);
            if (!obj)
            {
                LOG_ERROR(CONTEXT_CLIENT, "Object not found: {}", cmd.id.getValue());
                return;
            }
            setProperty(obj, cmd.prop, cmd.value);
        }
        else
        {
            SceneObjectRegistryIterator iter(m_scene.getObjectRegistry(), cmd.type);
            while (auto* obj = iter.getNextNonConst())
            {
                setProperty(obj, cmd.prop, cmd.value);
            }
        }
    }


    void SceneCommandVisitor::operator()(const SceneCommandFlushSceneVersion& cmd)
    {
        LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::execute: set scene version in next flush to \"{}\"", cmd.sceneVersion);
        m_scene.setSceneVersionForNextFlush(cmd.sceneVersion);
    }

    void SceneCommandVisitor::operator()(const SceneCommandValidationRequest& cmd)
    {
        if (cmd.optionalObjectName.empty())
        {
            // no object, validate whole scene
            ValidationReportImpl report;
            m_scene.validate(report);
            LOG_INFO(CONTEXT_CLIENT, "Validation:  {}", report.toString());
        }
        else
        {
            if (const auto* ro = m_scene.findObjectByName<ramses::SceneObject>(cmd.optionalObjectName.c_str()))
            {
                ValidationReportImpl report;
                ro->impl().validate(report);
                LOG_INFO(CONTEXT_CLIENT, "Validation:  {}", report.toString());
            }
            else
                LOG_ERROR(CONTEXT_CLIENT, "Validation could not find requested object with name: {}", cmd.optionalObjectName);
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandDumpSceneToFile& cmd) const
    {
        std::vector<std::byte> sceneData;
        SaveFileConfigImpl config;
        config.setCompressionEnabled(false);
        if (!m_scene.serialize(sceneData, config))
        {
            LOG_ERROR(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed dump scene: {} (file:{} dlt:{})", m_scene.getSceneId(), cmd.fileName, cmd.sendViaDLT);
            return;
        }

        const auto filename = cmd.fileName.empty() ? fmt::format("S_{}.ramses", m_scene.getSceneId()) : cmd.fileName + ".ramses";
        if (!cmd.fileName.empty())
        {
            File file(filename);
            if (file.open(File::Mode::WriteOverWriteOldBinary))
            {
                if (!file.write(sceneData.data(), sceneData.size()))
                {
                    LOG_ERROR(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed to write scene dump: {}", file.getPath());
                }
            }
            else
            {
                LOG_ERROR(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed to open: {}", file.getPath());
            }
        }

        if (cmd.sendViaDLT)
        {
            if (GetRamsesLogger().transmit(std::move(sceneData), filename))
            {
                LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::execute: started dlt file transfer: {}", filename);
            }
            else
            {
                LOG_ERROR(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed to send scene dump file via dlt: {}", filename);
            }
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandLogResourceMemoryUsage& /*cmd*/) const
    {
        MemoryInfoVector memoryInfos = GetMemoryInfoFromScene(m_scene);
        std::sort(memoryInfos.begin(), memoryInfos.end(),[](const MemoryInfo& lhs, const MemoryInfo& rhs){return lhs.memoryUsage > rhs.memoryUsage;});

        LOG_INFO_F(CONTEXT_CLIENT,([&](StringOutputStream& out){
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
