//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneCommandExecutor.h"

#include "SceneCommandBuffer.h"
#include "ResourceFileDescriptionSetImpl.h"
#include "Utils/LogMacros.h"

#include "ramses-client-api/StreamTexture.h"
#include "RamsesObjectTypeUtils.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "RamsesClientImpl.h"
#include "Utils/RamsesLogger.h"
#include "LogMemoryUtils.h"

#include <numeric>

namespace ramses_internal
{
    void SceneCommandExecutor::SendSceneAndResourceFilesViaDLT(const String& sceneDumpFileName, const ramses::ResourceFileDescriptionSet& resourceFileDescriptionSet)
    {
        if (GetRamsesLogger().transmitFile(sceneDumpFileName, false))
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneCommandExecutor::sendSceneAndResourceFilesViaDLT: scene file successfully send via dlt: " << sceneDumpFileName);
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "SceneCommandExecutor::sendSceneAndResourceFilesViaDLT: failed to send scene dump file via dlt: " << sceneDumpFileName);
        }

        for (const auto& resourceFileDescription : resourceFileDescriptionSet.impl->descriptions)
        {
            if (GetRamsesLogger().transmitFile(resourceFileDescription.getFilename(), false))
            {
                LOG_INFO(CONTEXT_CLIENT, "SceneCommandExecutor::sendSceneAndResourceFilesViaDLT: resource file successfully send via dlt: " << resourceFileDescription.getFilename());
            }
            else
            {
                LOG_INFO(CONTEXT_CLIENT, "SceneCommandExecutor::sendSceneAndResourceFilesViaDLT: failed to send resource file via dlt: " << resourceFileDescription.getFilename());
            }
        }
    }

    void SceneCommandExecutor::Execute(ramses::SceneImpl& scene, SceneCommandBuffer& commandBuffer)
    {
        SceneCommandContainer commands;
        commandBuffer.exchangeContainerData(commands);

        const uint32_t numberOfCommands = commands.getTotalCommandCount();
        for(uint32_t i=0; i < numberOfCommands; ++i)
        {
            const ESceneCommand type = commands.getCommandType(i);
            switch(type)
            {
            case ESceneCommand_ForceFallbackImage:
            {
                const ForceFallbackCommand& ffc = commands.getCommandData<ForceFallbackCommand>(i);
                ramses::RamsesObject* object = scene.findObjectByName(ffc.streamTextureName.c_str());
                if (object)
                {
                    if (object->getType() == ramses::ERamsesObjectType_StreamTexture)
                    {
                        ramses::StreamTexture& streamtexture = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::StreamTexture>(*object);
                        LOG_INFO(CONTEXT_CLIENT, "SceneCommandExecutor::execute: " << (ffc.forceFallback?"Enable":"Disable") << " force fallback image for \"" << ffc.streamTextureName << "\"");
                        streamtexture.forceFallbackImage(ffc.forceFallback);
                    }
                    else
                    {
                        LOG_ERROR(CONTEXT_CLIENT, "SceneCommandExecutor::execute: Set force fallback setting but the object with name \"" << ffc.streamTextureName << "\" is not a StreamTexture");
                    }
                }
                else
                {
                    LOG_ERROR(CONTEXT_CLIENT, "SceneCommandExecutor::execute: Set force fallback setting but couldn't find any object with name \"" << ffc.streamTextureName << "\"");
                }
                break;
            }
            case ESceneCommand_FlushSceneVersion:
            {
                const FlushSceneVersionCommand& sceneVersionCommand = commands.getCommandData<FlushSceneVersionCommand>(i);
                LOG_INFO(CONTEXT_CLIENT, "SceneCommandExecutor::execute: set scene version in next flush to \"" << sceneVersionCommand.sceneVersion << "\"");
                scene.setSceneVersionForNextFlush(sceneVersionCommand.sceneVersion);
                break;
            }
            case ESceneCommand_ValidationRequest:
            {
                const ValidationRequestCommand& validationRequestCommand = commands.getCommandData<ValidationRequestCommand>(i);
                if (0 == validationRequestCommand.optionalObjectName.getLength())
                {
                    // no object, validate whole scene
                    scene.validate(0u);
                    LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Validation:  " << scene.getValidationReport(validationRequestCommand.severity));
                }
                else
                {
                    const ramses::RamsesObject* ro = scene.findObjectByName(validationRequestCommand.optionalObjectName.c_str());
                    if (0 != ro)
                    {
                        ro->validate();
                        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Validation:  " << ro->getValidationReport(validationRequestCommand.severity));
                    }
                    else
                    {
                        LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Validation could not find request object with name: " << validationRequestCommand.optionalObjectName);
                    }
                }
                break;
            }
            case ESceneCommand_LogResourceMemoryUsage:
            {
                MemoryInfoVector memoryInfos = GetMemoryInfoFromScene(scene);
                std::sort(memoryInfos.begin(), memoryInfos.end(),[](const MemoryInfo& lhs, const MemoryInfo& rhs){return lhs.memoryUsage > rhs.memoryUsage;});

                LOG_INFO_F(ramses_internal::CONTEXT_CLIENT,([&](ramses_internal::StringOutputStream& out){
                    const uint32_t memTotal = std::accumulate(memoryInfos.begin(), memoryInfos.end(), 0u, [](uint32_t v, const MemoryInfo& info){return v + info.memoryUsage;});
                    out << "\n\rTotal    memory usage: " << memTotal;
                    out << "\n\rDetailed memory usage:";
                    for (const auto info : memoryInfos)
                    {
                        out << "\n\r";
                        out << info.memoryUsage << "\t";
                        out << info.logInfoMesage;
                    }
                }));
                break;
            }
            case  ESceneCommand_DumpSceneToFile:
            {
                const DumpSceneToFileCommand& dumpSceneToFileCommand = commands.getCommandData<DumpSceneToFileCommand>(i);

                ramses::ResourceFileDescriptionSet resourceFileDescriptionSet;
                const String resourceDumpFileWithExtension = dumpSceneToFileCommand.fileName + ".ramres";
                ramses::ResourceFileDescription resourceFileDescription(resourceDumpFileWithExtension.c_str());
                ramses::RamsesObjectVector resources = scene.getClientImpl().getListOfResourceObjects(ramses::ERamsesObjectType_Resource);
                for (const auto it : resources)
                {
                    const ramses::Resource& resource = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Resource>(*it);
                    resourceFileDescription.add(&resource);
                }
                resourceFileDescriptionSet.add(resourceFileDescription);

                const String sceneDumpFileWithExtension = dumpSceneToFileCommand.fileName + ".ramses";
                const ramses::status_t status = scene.getClientImpl().saveSceneToFile(scene, sceneDumpFileWithExtension.c_str(), resourceFileDescriptionSet, false);
                if (status == ramses::StatusOK)
                {
                    if (dumpSceneToFileCommand.sendViaDLT)
                    {
                        SceneCommandExecutor::SendSceneAndResourceFilesViaDLT(sceneDumpFileWithExtension, resourceFileDescriptionSet);
                    }
                }
                else
                {
                    LOG_WARN(CONTEXT_CLIENT, "SceneCommandExecutor::execute: failed to dump scene to file: " << scene.getClientImpl().getStatusMessage(status));
                }
                break;
            }
            default:
                LOG_ERROR(CONTEXT_CLIENT, "SceneCommandExecutor::execute: encountered unknown command type: " << getSceneCommandName(type));
                break;
            }
        }
    }
}
