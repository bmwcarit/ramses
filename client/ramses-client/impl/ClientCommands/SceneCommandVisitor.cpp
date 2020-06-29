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
#include "ResourceFileDescriptionSetImpl.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "Utils/LogMacros.h"
#include <numeric>

namespace ramses_internal
{
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
        if (0 == cmd.optionalObjectName.size())
        {
            // no object, validate whole scene
            ramses::StatusObjectImpl::StatusObjectSet visitedObjects;
            m_scene.validate(0u, visitedObjects);
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
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Validation could not find request object with name: " << cmd.optionalObjectName);
        }
    }

    void SceneCommandVisitor::operator()(const SceneCommandDumpSceneToFile& cmd) const
    {
        ramses::ResourceFileDescriptionSet resourceFileDescriptionSet;
        const String resourceDumpFileWithExtension = cmd.fileName + ".ramres";
        ramses::ResourceFileDescription resourceFileDescription(resourceDumpFileWithExtension.c_str());
        ramses::RamsesObjectVector resources = m_scene.getClientImpl().getListOfResourceObjects(ramses::ERamsesObjectType_Resource);
        for (const auto it : resources)
        {
            const ramses::Resource& resource = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Resource>(*it);
            resourceFileDescription.add(&resource);
        }
        resourceFileDescriptionSet.add(resourceFileDescription);

        const String sceneDumpFileWithExtension = cmd.fileName + ".ramses";
        const ramses::status_t status = m_scene.getClientImpl().saveSceneToFile(m_scene, sceneDumpFileWithExtension.c_str(), resourceFileDescriptionSet, false);
        if (status == ramses::StatusOK)
        {
            if (cmd.sendViaDLT)
                SceneCommandVisitor::SendSceneAndResourceFilesViaDLT(sceneDumpFileWithExtension, resourceFileDescriptionSet);
        }
        else
            LOG_WARN(CONTEXT_CLIENT, "SceneCommandVisitor::execute: failed to dump scene to file: " << m_scene.getClientImpl().getStatusMessage(status));
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

    void SceneCommandVisitor::SendSceneAndResourceFilesViaDLT(const String& sceneDumpFileName, const ramses::ResourceFileDescriptionSet& resourceFileDescriptionSet)
    {
        if (GetRamsesLogger().transmitFile(sceneDumpFileName, false))
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::sendSceneAndResourceFilesViaDLT: scene file successfully send via dlt: " << sceneDumpFileName);
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "SceneCommandVisitor::sendSceneAndResourceFilesViaDLT: failed to send scene dump file via dlt: " << sceneDumpFileName);
        }

        for (const auto& resourceFileDescription : resourceFileDescriptionSet.impl->descriptions)
        {
            if (GetRamsesLogger().transmitFile(resourceFileDescription.getFilename(), false))
            {
                LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::sendSceneAndResourceFilesViaDLT: resource file successfully send via dlt: " << resourceFileDescription.getFilename());
            }
            else
            {
                LOG_INFO(CONTEXT_CLIENT, "SceneCommandVisitor::sendSceneAndResourceFilesViaDLT: failed to send resource file via dlt: " << resourceFileDescription.getFilename());
            }
        }
    }
}
