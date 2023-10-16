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
#include "LogMemoryUtils.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/File.h"
#include <numeric>

namespace ramses::internal
{

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
            ValidationReportImpl report;
            m_scene.validate(report);
            LOG_INFO(CONTEXT_CLIENT, "Validation:  " << report.toString());
        }
        else
        {
            if (const auto* ro = m_scene.findObjectByName<ramses::SceneObject>(cmd.optionalObjectName.c_str()))
            {
                ValidationReportImpl report;
                ro->impl().validate(report);
                LOG_INFO(CONTEXT_CLIENT, "Validation:  " << report.toString());
            }
            else
                LOG_ERROR(CONTEXT_CLIENT, "Validation could not find requested object with name: " << cmd.optionalObjectName);
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandDumpSceneToFile& cmd) const
    {
        std::vector<std::byte> sceneData;
        SaveFileConfigImpl config;
        config.setValidationEnabled(false);
        config.setCompressionEnabled(false);
        if (!m_scene.serialize(sceneData, config))
        {
            LOG_ERROR_P(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed dump scene: {} (file:{} dlt:{})", m_scene.getSceneId(), cmd.fileName, cmd.sendViaDLT);
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
