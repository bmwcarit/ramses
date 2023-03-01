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
#include "Utils/LogMacros.h"
#include <numeric>

namespace ramses_internal
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
            m_scene.validate();
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Validation:  " << m_scene.getValidationReport(cmd.severity));
        }
        else
        {
            if (const ramses::RamsesObject* ro = m_scene.findObjectByName(cmd.optionalObjectName.c_str()))
            {
                std::ignore = ro->validate();
                LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Validation:  " << ro->getValidationReport(cmd.severity));
            }
            else
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Validation could not find requested object with name: " << cmd.optionalObjectName);
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandDumpSceneToFile& cmd) const
    {
        const String sceneDumpFileWithExtension = cmd.fileName + ".ramses";
        const ramses::status_t status = m_scene.saveToFile(sceneDumpFileWithExtension.c_str(), false);
        if (status == ramses::StatusOK)
        {
            if (cmd.sendViaDLT)
                SceneCommandVisitor::SendSceneViaDLT(sceneDumpFileWithExtension);
        }
        else
            LOG_WARN(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed to dump scene to file: " << m_scene.getStatusMessage(status));
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

    void SceneCommandVisitor::SendSceneViaDLT(const String& sceneDumpFileName)
    {
        if (GetRamsesLogger().transmitFile(sceneDumpFileName, false))
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::sendSceneAndResourceFilesViaDLT: started dlt file transfer: " << sceneDumpFileName);
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "SceneCommandVisitor::sendSceneAndResourceFilesViaDLT: failed to send scene dump file via dlt: " << sceneDumpFileName);
        }
    }
}
