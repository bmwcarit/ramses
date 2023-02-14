//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IWindow.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererAPI/IContext.h"
#include "SceneAPI/StreamTexture.h"
#include "RendererLib/RendererLogger.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/Renderer.h"
#include "RendererLib/DisplayController.h"
#include "RendererLib/SceneStateInfo.h"
#include "RendererLib/EResourceStatus.h"
#include "RendererLib/ResourceDescriptor.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/LoggingDevice.h"
#include "RendererLib/TransformationLinkManager.h"
#include "RendererLib/DataReferenceLinkManager.h"
#include "RendererLib/SceneLinks.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/DisplaySetup.h"
#include "RendererLib/StagingInfo.h"
#include "RendererLib/SceneReferenceLogic.h"
#include "RenderExecutor.h"
#include "RenderExecutorLogger.h"
#include "RendererEventCollector.h"
#include "Platform_Base/DeviceResourceMapper.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    void RendererLogger::StartSection(const String& name, RendererLogContext& context)
    {
        context << RendererLogContext::NewLine << "+++ " << name << " +++" << RendererLogContext::NewLine << RendererLogContext::NewLine;
    }

    void RendererLogger::EndSection(const String& name, RendererLogContext& context)
    {
        context << RendererLogContext::NewLine << "--- " << name << " ---" << RendererLogContext::NewLine << RendererLogContext::NewLine;
    }

    void RendererLogger::Log(const LogContext& logContext, const RendererLogContext& context)
    {
        if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details) || context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Info))
        {
            LOG_INFO(logContext, context.getStream().c_str());
        }
        else if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Warn))
        {
            LOG_WARN(logContext, context.getStream().c_str());
        }
        else if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Error))
        {
            LOG_ERROR(logContext, context.getStream().c_str());
        }
    }

    void RendererLogger::LogTopic(const RendererSceneUpdater& updater, ERendererLogTopic topic, Bool verbose, NodeHandle nodeHandleFilter)
    {
        RendererLogContext context(verbose ? ERendererLogLevelFlag_Details : ERendererLogLevelFlag_Info);
        context.setNodeHandleFilter(nodeHandleFilter);
        switch (topic)
        {
        case ERendererLogTopic::Displays:
            LogDisplays(updater, context);
            break;
        case ERendererLogTopic::SceneStates:
            LogSceneStates(updater, context);
            break;
        case ERendererLogTopic::StreamTextures:
            LogStreamTextures(updater, context);
            break;
        case ERendererLogTopic::Resources:
            LogClientResources(updater, context);
            LogSceneResources(updater, context);
            break;
        case ERendererLogTopic::MissingResources:
            LogMissingResources(updater, context);
            break;
        case ERendererLogTopic::RenderQueue:
            LogRenderQueue(updater, context);
            break;
        case ERendererLogTopic::Links:
            LogLinks(updater.m_rendererScenes, context);
            break;
        case ERendererLogTopic::EmbeddedCompositor:
            LogEmbeddedCompositor(updater, context);
            break;
        case ERendererLogTopic::EventQueue:
            LogEventQueue(updater, context);
            break;
        case ERendererLogTopic::All:
            LogDisplays(updater, context);
            LogSceneStates(updater, context);
            LogStreamTextures(updater, context);
            LogClientResources(updater, context);
            LogSceneResources(updater, context);
            LogRenderQueue(updater, context);
            LogLinks(updater.m_rendererScenes, context);
            LogReferencedScenes(updater, context);
            LogEmbeddedCompositor(updater, context);
            LogEventQueue(updater, context);
            break;
        case ERendererLogTopic::PeriodicLog:
            LogPeriodicInfo(updater);
            break;
        default:
            context << "Log topic " << topic << " not found!" << RendererLogContext::NewLine;
            break;
        }
        if (topic != ERendererLogTopic::PeriodicLog)
        {
            Log(CONTEXT_RENDERER, context);
        }
    }

    static std::string resourcesToString(const ResourceContentHashVector& resources)
    {
        StringOutputStream str;
        str << "[";
        for (const auto& res : resources)
            str << " " << res;
        str << " ]";
        return str.release();
    };

    void RendererLogger::LogSceneStates(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        StartSection("RENDERER SCENE STATES", context);
        SceneIdVector knownSceneIds;
        updater.m_sceneStateExecutor.m_scenesStateInfo.getKnownSceneIds(knownSceneIds);
        context << knownSceneIds.size() << " known scene(s)" << RendererLogContext::NewLine << RendererLogContext::NewLine;
        context.indent();

        for(const auto sceneId : knownSceneIds)
        {
            context << "Scene [id: " << sceneId << "]" << RendererLogContext::NewLine;
            context.indent();
            {
                const ESceneState sceneState = updater.m_sceneStateExecutor.getSceneState(sceneId);
                context << "State:       " << EnumToString(sceneState) << RendererLogContext::NewLine;

                if (updater.m_rendererScenes.hasScene(sceneId))
                {
                    const auto& stagingInfo = updater.m_rendererScenes.getStagingInfo(sceneId);
                    context << "Version tag: ";
                    if (stagingInfo.lastAppliedVersionTag.isValid())
                        context << stagingInfo.lastAppliedVersionTag;
                    else
                        context << "<none>";
                    context << RendererLogContext::NewLine;

                    if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
                    {
                        const auto& sceneName = updater.m_rendererScenes.getScene(sceneId).getName();
                        context << "Name:     \"" << sceneName << "\"" << RendererLogContext::NewLine;
                        context << "Assigned to buffer (device handle): " << updater.m_renderer.getBufferSceneIsAssignedTo(sceneId) << RendererLogContext::NewLine;
                    }

                    if (updater.hasPendingFlushes(sceneId))
                    {
                        const auto& pendingData = stagingInfo.pendingData;
                        const auto& pendingFlushes = pendingData.pendingFlushes;
                        context << "Pending flushes: " << pendingFlushes.size() << RendererLogContext::NewLine;
                        context.indent();
                        for (const auto& flushInfo : pendingFlushes)
                        {
                            context << "Flush " << flushInfo.flushIndex << RendererLogContext::NewLine;
                            context << "[ sceneActions: " << flushInfo.sceneActions.numberOfActions() << " (" << flushInfo.sceneActions.collectionData().size() << " bytes) ]" << RendererLogContext::NewLine;
                            context << "[ " << stagingInfo.sizeInformation << " ]" << RendererLogContext::NewLine;
                            context << "[ addedResources: "<< resourcesToString(flushInfo.resourcesAdded) << " ]" << RendererLogContext::NewLine;
                            context << "[ removedResources: " << resourcesToString(flushInfo.resourcesRemoved) << " ]" << RendererLogContext::NewLine;
                        }
                        context.unindent();
                    }
                }
            }
            context << RendererLogContext::NewLine;
            context.unindent();
        }
        context.unindent();
        EndSection("RENDERER SCENE STATES", context);
    }

    void RendererLogger::LogDisplays(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        if (!updater.m_renderer.hasDisplayController())
        {
            context << RendererLogContext::NewLine << "Skipping display section due to missing display controller!" << RendererLogContext::NewLine;
            return;
        }

        StartSection("RENDERER DISPLAY", context);

        const auto& displayController = updater.m_renderer.getDisplayController();
        const UInt32 height = displayController.getDisplayHeight();
        const UInt32 width = displayController.getDisplayWidth();
        const WaylandIviSurfaceId surfaceId = displayController.getRenderBackend().getWindow().getWaylandIviSurfaceID();

        context.indent();
        {
            context << "Width:     " << width << RendererLogContext::NewLine;
            context << "Height:    " << height << RendererLogContext::NewLine;
            context << "IviSurfaceId: " << surfaceId.getValue() << RendererLogContext::NewLine;
            if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
            {
                const auto& renderer = updater.m_renderer;
                context << "Frame Buffer:" << RendererLogContext::NewLine;
                context.indent();
                LogDisplayBuffer(updater, renderer.m_frameBufferDeviceHandle, renderer.m_displayBuffersSetup.getDisplayBuffer(renderer.m_frameBufferDeviceHandle), context);
                context.unindent();

                const auto& resMgr = static_cast<const RendererResourceManager&>(*updater.m_displayResourceManager);
                context << "Offscreen Buffers:" << RendererLogContext::NewLine;
                context.indent();
                for (const auto& ob : resMgr.m_offscreenBuffers)
                {
                    context << "OB handle: " << ob.first << RendererLogContext::NewLine;
                    const auto deviceHandle = resMgr.getOffscreenBufferDeviceHandle(ob.first);
                    const auto& obInfo = renderer.m_displayBuffersSetup.getDisplayBuffer(deviceHandle);
                    LogDisplayBuffer(updater, deviceHandle, obInfo, context);
                }
                context.unindent();

                context << "Stream Buffers:" << RendererLogContext::NewLine;
                context.indent();
                for (const auto& sb : resMgr.m_streamBuffers)
                    context << "Handle: " << sb.first << "   SourceId: " << *sb.second << RendererLogContext::NewLine;
                context.unindent();
            }
        }
        context << RendererLogContext::NewLine;
        context.unindent();

        EndSection("RENDERER DISPLAY", context);
    }

    void RendererLogger::LogDisplayBuffer(const RendererSceneUpdater& updater, DeviceResourceHandle bufferHandle, const DisplayBufferInfo& dispBufferInfo, RendererLogContext& context)
    {
        context << "Display Buffer deviceHandle=" << bufferHandle.asMemoryHandle()
            << " GPUhandle=" << updater.m_renderer.getDisplayController().getRenderBackend().getDevice().getGPUHandle(bufferHandle)
            << (dispBufferInfo.isInterruptible ? " [interruptible]" : "") << RendererLogContext::NewLine;

        context.indent();
        const auto& vp = dispBufferInfo.viewport;
        context << "[ Width x Height: " << vp.width << " x " << vp.height << " ]" << RendererLogContext::NewLine;
        context << "[ Clear color: (" << dispBufferInfo.clearColor.r << "," << dispBufferInfo.clearColor.g << "," << dispBufferInfo.clearColor.b << "," << dispBufferInfo.clearColor.a << " ]" << RendererLogContext::NewLine;
        context << "Scenes in Rendering Order:" << RendererLogContext::NewLine;
        context.indent();
        for (const auto& sceneInfo : dispBufferInfo.scenes)
        {
            const String& sceneName = updater.m_rendererScenes.getScene(sceneInfo.sceneId).getName();
            const Bool shown = ( updater.m_sceneStateExecutor.getSceneState(sceneInfo.sceneId) == ESceneState::Rendered );
            context << "[ SceneId: " << sceneInfo.sceneId << "; order: " << sceneInfo.globalSceneOrder << "; shown: " << shown << "; name: \"" << sceneName << "\" ]" << RendererLogContext::NewLine;
        }
        context.unindent();

        context.unindent();
    }

    void RendererLogger::LogClientResources(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        if (!updater.m_renderer.hasDisplayController())
        {
            context << RendererLogContext::NewLine << "Skipping resources section due to missing display controller!" << RendererLogContext::NewLine;
            return;
        }

        StartSection("RENDERER CLIENT RESOURCES", context);

        const RendererResourceManager* resourceManager = static_cast<const RendererResourceManager*>(updater.m_displayResourceManager.get());
        context.indent();
        context << resourceManager->m_resourceRegistry.getAllResourceDescriptors().size() << " Resources" << RendererLogContext::NewLine << RendererLogContext::NewLine;

        // Infos by resource status
        context << "Status:" << RendererLogContext::NewLine;
        context.indent();
        for (UInt32 i = 0; i < static_cast<uint32_t>(EResourceStatus::Broken) + 1u; i++)
        {
            const EResourceStatus status = static_cast<EResourceStatus>(i);

            UInt32 count = 0;
            for(const auto& descriptorIt : resourceManager->m_resourceRegistry.getAllResourceDescriptors())
            {
                const ResourceDescriptor& resourceDescriptor = descriptorIt.value;
                if (resourceDescriptor.status == status)
                {
                    ++count;
                }
            }
            context << count << " " << status << RendererLogContext::NewLine;
        }
        context.unindent();

        context << RendererLogContext::NewLine;
        context << "Overview:" << RendererLogContext::NewLine;
        context.indent();
        {
            UInt32 totalCount = 0;
            UInt32 totalCompressedSize = 0;
            UInt32 totalDecompressedSize = 0;
            UInt32 totalVRAMSize = 0;

            //Infos by resource type
            for (UInt32 i = 0; i < EResourceType_NUMBER_OF_ELEMENTS; i++)
            {
                const EResourceType type = static_cast<EResourceType>(i);

                UInt32 count = 0;
                UInt32 compressedSize = 0;
                UInt32 decompressedSize = 0;
                UInt32 VRAMSize = 0;
                context << EnumToString(type) << RendererLogContext::NewLine;

                ResourceContentHashVector resources;
                for (const auto& resourceIt : resourceManager->m_resourceRegistry.getAllResourceDescriptors())
                    if (resourceIt.value.type == type)
                        resources.push_back(resourceIt.value.hash);

                std::sort(resources.begin(), resources.end(), [&](ResourceContentHash r1, ResourceContentHash r2)
                {
                    return resourceManager->m_resourceRegistry.getResourceDescriptor(r1).vramSize > resourceManager->m_resourceRegistry.getResourceDescriptor(r2).vramSize;
                });

                context.indent();
                for(const auto& resource : resources)
                {
                    const ResourceDescriptor& resourceDescriptor = resourceManager->m_resourceRegistry.getResourceDescriptor(resource);
                    if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
                    {
                        context << "[";
                        context << "handle: " << resourceDescriptor.deviceHandle << "; ";
                        context << "hash: " << resourceDescriptor.hash << "; ";
                        context << "scene usage: (";
                        for (const auto sceneId : resourceDescriptor.sceneUsage)
                        {
                            context << sceneId << ", ";
                        }
                        context << "); ";
                        context << "status: " << resourceDescriptor.status << "; ";
                        context << "sizeKB (compressed/decompressed/vram): " << resourceDescriptor.compressedSize / 1024 << "/" << resourceDescriptor.decompressedSize / 1024 << "/" << resourceDescriptor.vramSize / 1024;
                        context << "]" << RendererLogContext::NewLine;
                    }

                    compressedSize += resourceDescriptor.compressedSize;
                    decompressedSize += resourceDescriptor.decompressedSize;
                    VRAMSize += resourceDescriptor.vramSize;
                    count++;
                }
                context << count << " resources of type " << EnumToString(type)
                    << " total size KB (compressed/decompressed/vram): " << compressedSize / 1024 << "/" << decompressedSize / 1024 << "/" << VRAMSize / 1024
                    << RendererLogContext::NewLine;
                context.unindent();

                totalCount += count;
                totalCompressedSize += compressedSize;
                totalDecompressedSize += decompressedSize;
                totalVRAMSize += VRAMSize;
            }

            context << RendererLogContext::NewLine << totalCount << " client resources in total, size KB (compressed/decompressed/vram): "
                << totalCompressedSize / 1024 << "/" << totalDecompressedSize / 1024 << "/" << totalVRAMSize / 1024
                << RendererLogContext::NewLine;
        }
        context.unindent();

        EndSection("RENDERER CLIENT RESOURCES", context);
    }

    void RendererLogger::LogSceneResources(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        if (!updater.m_renderer.hasDisplayController())
        {
            context << RendererLogContext::NewLine << "Skipping scene resources section due to missing display controller!" << RendererLogContext::NewLine;
            return;
        }

        StartSection("RENDERER SCENE RESOURCES", context);
        context << "Resources for " << updater.m_rendererScenes.size() << " Scene(s)" << RendererLogContext::NewLine << RendererLogContext::NewLine;

        const RendererResourceManager& resourceManager = static_cast<const RendererResourceManager&>(*updater.m_displayResourceManager);
        for (const auto& sceneResRegistryIt : resourceManager.m_sceneResourceRegistryMap)
        {
            context.indent();
            context << "Scene resources in scene " << sceneResRegistryIt.key << ":" << RendererLogContext::NewLine;
            const RendererSceneResourceRegistry& resRegistry = sceneResRegistryIt.value;
            const RendererCachedScene& scene = updater.m_rendererScenes.getScene(sceneResRegistryIt.key);
            const auto& device = updater.m_renderer.getDisplayController().getRenderBackend().getDevice();

            context.indent();

            const auto& renderBuffers = scene.getRenderBuffers();
            const auto renderBufferCount = std::count_if(renderBuffers.cbegin(), renderBuffers.cend(), [](const auto&) {return true; });
            context << "Render buffers: " << renderBufferCount << RendererLogContext::NewLine;
            {
                UInt32 size = 0u;
                context.indent();
                for (const auto& rb : renderBuffers)
                {
                    const RenderBuffer& rbDesc = *rb.second;
                    if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
                    {
                        const auto deviceHandle = resRegistry.getRenderBufferDeviceHandle(rb.first);
                        context << rb.first << " (deviceHandle=" << deviceHandle << " GPUhandle=" << device.getGPUHandle(deviceHandle) << ") ";
                        context << "[" << rbDesc.width << "x" << rbDesc.height << "; " << EnumToString(rbDesc.type) << "; " << EnumToString(rbDesc.format) << "; " << EnumToString(rbDesc.accessMode) << "; " << rbDesc.sampleCount << " samples] ";
                        context << resRegistry.getRenderBufferByteSize(rb.first) / 1024 << " KB";
                        context << RendererLogContext::NewLine;
                    }
                    size += resRegistry.getRenderBufferByteSize(rb.first);
                }
                context << "Total KB: " << size / 1024 << RendererLogContext::NewLine;
                context.unindent();
            }

            const auto& renderTargets = scene.getRenderTargets();
            const auto renderTargetCount = std::count_if(renderTargets.cbegin(), renderTargets.cend(), [](const auto&) {return true; });
            context << "Render targets: " << renderTargetCount << RendererLogContext::NewLine;
            if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
            {
                context.indent();
                for (const auto& rt : renderTargets)
                {
                    const auto deviceHandle = resRegistry.getRenderTargetDeviceHandle(rt.first);
                    context << rt.first << " (deviceHandle=" << deviceHandle << " GPUhandle=" << device.getGPUHandle(deviceHandle) << ") ";
                    context << "renderBuffer handles: [ ";
                    for (UInt32 i = 0u; i < scene.getRenderTargetRenderBufferCount(rt.first); ++i)
                        context << scene.getRenderTargetRenderBuffer(rt.first, i) << " ";
                    context << "]" << RendererLogContext::NewLine;
                }
                context.unindent();
            }

            const auto& blitPasses = scene.getBlitPasses();
            const auto blitPassCount = std::count_if(blitPasses.cbegin(), blitPasses.cend(), [](const auto&) {return true; });
            context << "Blit passes: " << blitPassCount << RendererLogContext::NewLine;
            if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
            {
                context.indent();
                for (const auto& bp : blitPasses)
                {
                    DeviceResourceHandle bpSrc;
                    DeviceResourceHandle bpDst;
                    resRegistry.getBlitPassDeviceHandles(bp.first, bpSrc, bpDst);
                    context << bp.first << " [renderBuffer deviceHandles: src " << bpSrc << " -> dst " << bpDst << "]";
                    context << RendererLogContext::NewLine;
                }
                context.unindent();
            }

            const auto& textureBuffers = scene.getTextureBuffers();
            const auto textureBufferCount = std::count_if(textureBuffers.cbegin(), textureBuffers.cend(), [](const auto&) {return true; });
            context << "Texture buffers: " << textureBufferCount << RendererLogContext::NewLine;
            {
                UInt32 size = 0u;
                context.indent();
                for (const auto& tb : textureBuffers)
                {
                    const TextureBuffer& tbDesc = *tb.second;
                    if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
                    {
                        context << tb.first << " (deviceHandle " << resRegistry.getTextureBufferDeviceHandle(tb.first) << ") " << EnumToString(tbDesc.textureFormat);
                        context << " mips: ";
                        for (const auto& mip : tbDesc.mipMaps)
                            context << "[" << mip.width << "x" << mip.height << "] ";
                        context << resRegistry.getTextureBufferByteSize(tb.first) / 1024 << " KB";
                        context << RendererLogContext::NewLine;
                    }
                    size += resRegistry.getTextureBufferByteSize(tb.first);
                }
                context << "Total KB: " << size / 1024 << RendererLogContext::NewLine;
                context.unindent();
            }

            const auto& dataBuffers = scene.getDataBuffers();
            const auto dataBufferCount = std::count_if(dataBuffers.cbegin(), dataBuffers.cend(), [](const auto&) {return true; });
            context << "Data buffers: " << dataBufferCount << RendererLogContext::NewLine;
            {
                UInt32 size = 0u;
                context.indent();
                for (const auto& db : dataBuffers)
                {
                    const GeometryDataBuffer& dbDesc = *db.second;
                    if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
                    {
                        context << db.first << " (deviceHandle " << resRegistry.getDataBufferDeviceHandle(db.first) << ") ";
                        context << EnumToString(dbDesc.bufferType) << " " << EnumToString(dbDesc.dataType);
                        context << " size used/allocated in B: " << dbDesc.usedSize << "/" << dbDesc.data.size();
                        context << RendererLogContext::NewLine;
                    }
                    size += UInt32(dbDesc.data.size());
                }
                context << "Total KB: " << size / 1024 << RendererLogContext::NewLine;
                context.unindent();
            }

            const auto& streamTextures = scene.getStreamTextures();
            const auto streamTextureCount = std::count_if(streamTextures.cbegin(), streamTextures.cend(), [](const auto&) {return true; });
            context << "Stream textures: " << streamTextureCount << RendererLogContext::NewLine;
            if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
            {
                context.indent();
                for (const auto& st : streamTextures)
                    context << st.first << " [sourceId " << st.second->source << "]" << RendererLogContext::NewLine;
                context.unindent();
            }

            context.unindent();

            context.unindent();
        }

        // gather and report shared client resources
        uint64_t numSharedResources = 0;
        uint64_t transferSizeSharedResources = 0;
        uint64_t savedTransferSizeBySharedResources = 0;
        for (const auto& p : resourceManager.m_resourceRegistry.m_resources)
        {
            const ResourceDescriptor& rd = p.value;
            if (rd.sceneUsage.size() > 1 &&
                (rd.status == EResourceStatus::Provided || rd.status == EResourceStatus::Uploaded))
            {
                numSharedResources += 1;
                const uint32_t transferSize = (rd.compressedSize > 0) ? rd.compressedSize : rd.decompressedSize;
                transferSizeSharedResources += transferSize;
                savedTransferSizeBySharedResources += transferSize * (rd.sceneUsage.size() - 1);
            }
        }

        context << RendererLogContext::NewLine;
        context << "Client resources shared by multiple scenes:" << RendererLogContext::NewLine;
        context.indent();
        context << "Number resources: " << numSharedResources << RendererLogContext::NewLine;
        context << "Est. transfer size: " << transferSizeSharedResources << RendererLogContext::NewLine;
        context << "Est. transfer size saved by sharing: " << savedTransferSizeBySharedResources << RendererLogContext::NewLine;
        context.unindent();

        EndSection("RENDERER SCENE RESOURCES", context);
    }

    void RendererLogger::LogMissingResources(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        if (!updater.m_renderer.hasDisplayController())
        {
            context << RendererLogContext::NewLine << "Skipping missing resources section due to missing display controller!" << RendererLogContext::NewLine;
            return;
        }

        StartSection("RENDERER MISSING RESOURCES", context);
        HashMap<SceneId, std::pair<size_t, ResourceContentHashVector>> missingResourcesPerScene;

        const RendererResourceManager* resourceManager = static_cast<const RendererResourceManager*>(updater.m_displayResourceManager.get());
        const auto& resourceDescriptors = resourceManager->m_resourceRegistry.getAllResourceDescriptors();
        for (const auto& descriptorIt : resourceDescriptors)
        {
            const ResourceDescriptor& rd = descriptorIt.value;
            for (const auto sceneId : rd.sceneUsage)
            {
                auto& resCounter = missingResourcesPerScene[sceneId];
                resCounter.first++;
                if (rd.status != EResourceStatus::Uploaded)
                    resCounter.second.push_back(descriptorIt.key);
            }
        }

        for (const auto& missingResIt : missingResourcesPerScene)
        {
            const SceneId sceneId = missingResIt.key;
            const ResourceContentHashVector& missingResources = missingResIt.value.second;
            context << "Scene " << sceneId << ": " << missingResources.size() << " resources not uploaded (" << missingResIt.value.first << " total)" << RendererLogContext::NewLine;
            context.indent();
            if (!missingResources.empty())
            {
                for (const auto& hash : missingResources)
                {
                    const auto& resourceDescriptor = resourceManager->m_resourceRegistry.getResourceDescriptor(hash);
                    context << "[";
                    context << "hash: " << resourceDescriptor.hash << "; ";
                    context << "handle: " << resourceDescriptor.deviceHandle << "; ";
                    context << "scene usage:";
                    for (const auto sId : resourceDescriptor.sceneUsage)
                    {
                        context << " " << sId.getValue();
                    }
                    context << "; ";
                    context << "status: " << resourceDescriptor.status << "; ";
                    context << EnumToString(resourceDescriptor.type);
                    context << "]" << RendererLogContext::NewLine;
                }
            }
            context.unindent();
        }

        context << "Resource cached lists:" << RendererLogContext::NewLine;
        context.indent();
        const RendererResourceRegistry& resRegistry = resourceManager->m_resourceRegistry;
        context << "ToUpload  = " << resourcesToString(resRegistry.getAllProvidedResources()) << RendererLogContext::NewLine;
        context << "ToUnload  = " << resourcesToString(resRegistry.getAllResourcesNotInUseByScenes()) << RendererLogContext::NewLine;
        context.unindent();

        for (const auto& sceneIt : updater.m_rendererScenes)
        {
            context << "Scene " << sceneIt.key << RendererLogContext::NewLine;
            const auto& pendingData = sceneIt.value.stagingInfo->pendingData;

            context.indent();
            context << "Pending flush(es) : [ ";
            for (const auto& pendingFlush : pendingData.pendingFlushes)
            {
                context << pendingFlush.flushIndex << " ";
            }
            context << "]" << RendererLogContext::NewLine;

            context.unindent();
        }
        EndSection("RENDERER MISSING RESOURCES", context);
    }

    void RendererLogger::LogRenderQueue(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        if (!updater.m_renderer.hasDisplayController())
        {
            context << RendererLogContext::NewLine << "Skipping render queue section due to missing display controller!" << RendererLogContext::NewLine;
            return;
        }

        StartSection("RENDERER QUEUE", context);

        const Renderer& renderer = updater.m_renderer;
        const auto& displayBuffers = renderer.m_displayBuffersSetup.getDisplayBuffers();

        // 1. OBs
        context << RendererLogContext::NewLine;
        context << "<<< Skipping of frames is NOT considered! Logging here as if buffer was fully re-rendered! >>>" << RendererLogContext::NewLine;
        context << "Offscreen Buffers:" << RendererLogContext::NewLine;

        for (const auto& bufferInfo : displayBuffers)
        {
            if (bufferInfo.second.isOffscreenBuffer && !bufferInfo.second.isInterruptible)
            {
                context.indent();
                LogRenderQueueOfScenesRenderedToBuffer(updater, context, bufferInfo.first);
                context.unindent();
            }
        }

        // 2. FB
        context << RendererLogContext::NewLine;
        context << "<<< Skipping of frames is NOT considered! Logging here as if buffer was fully re-rendered! >>>" << RendererLogContext::NewLine;
        context << "Framebuffer:" << RendererLogContext::NewLine;

        context.indent();
        LogRenderQueueOfScenesRenderedToBuffer(updater, context, renderer.m_frameBufferDeviceHandle);
        context.unindent();

        // 3. Interruptible OBs
        context << RendererLogContext::NewLine;
        context << "<<< Interrupted rendering and skipping of frames are NOT considered! Logging here as if buffer and scenes were fully re-rendered! >>>" << RendererLogContext::NewLine;
        context << "Interruptible Offscreen Buffers:" << RendererLogContext::NewLine;

        for (const auto& bufferInfo : displayBuffers)
        {
            if (bufferInfo.second.isInterruptible)
            {
                context.indent();
                LogRenderQueueOfScenesRenderedToBuffer(updater, context, bufferInfo.first);
                context.unindent();
            }
        }

        EndSection("RENDERER QUEUE", context);
    }

    void RendererLogger::LogRenderQueueOfScenesRenderedToBuffer(const RendererSceneUpdater& updater, RendererLogContext& context, DeviceResourceHandle buffer)
    {
        context << "Display Buffer device handle: " << buffer << RendererLogContext::NewLine;

        const DisplayController& displayController = static_cast<const DisplayController&>(*updater.m_renderer.m_displayController);
        const auto& displayBuffers = updater.m_renderer.m_displayBuffersSetup.getDisplayBuffers();
        const DisplayBufferInfo& bufferInfo = displayBuffers.find(buffer)->second;

        const auto& mappedScenes = bufferInfo.scenes;
        for (const auto& sceneInfo : mappedScenes)
        {
            if (sceneInfo.shown)
            {
                const RendererCachedScene& renderScene = updater.m_renderer.m_rendererScenes.getScene(sceneInfo.sceneId);
                LoggingDevice logDevice(displayController.getRenderBackend().getDevice(), context);
                RenderingContext renderContext{
                    displayController.m_device.getFramebufferRenderTarget(),
                    displayController.getDisplayWidth(), displayController.getDisplayHeight(),
                    SceneRenderExecutionIterator{},
                    EClearFlags_All,
                    Vector4{ 0.f },
                    false
                };
                RenderExecutorLogger executor(logDevice, renderContext, context);
                executor.logScene(renderScene);
            }
        }
    }

    void RendererLogger::LogStreamTextures(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        if (!updater.m_renderer.hasDisplayController())
        {
            context << RendererLogContext::NewLine << "Skipping stream textures section due to missing display controller!" << RendererLogContext::NewLine;
            return;
        }

        StartSection("RENDERER STREAM TEXTURES", context);
        SceneIdVector knownSceneIds;
        updater.m_sceneStateExecutor.m_scenesStateInfo.getKnownSceneIds(knownSceneIds);

        for(const auto sceneId : knownSceneIds)
        {
            if (updater.m_rendererScenes.hasScene(sceneId))
            {
                RendererCachedScene& scene = updater.m_rendererScenes.getScene(sceneId);

                context << "Scene [id: " << scene.getSceneId() << "; Name: " << scene.getName() << "]" << RendererLogContext::NewLine << RendererLogContext::NewLine;
                context.indent();

                const UInt32 numberOfStreamTextures = scene.getStreamTextureCount();
                for (StreamTextureHandle streamTextureHandle(0); streamTextureHandle < numberOfStreamTextures; ++streamTextureHandle )
                {
                    if(scene.isStreamTextureAllocated(streamTextureHandle))
                    {
                        const StreamTexture& streamTex = scene.getStreamTexture(streamTextureHandle);
                        const WaylandIviSurfaceId streamSource(streamTex.source);
                        Bool contentAvailable = false;
                        DeviceResourceHandle streamDeviceHandle;
                        DeviceResourceHandle fallbackDeviceHandle;

                        const IRendererResourceManager& resourceManager = *updater.m_displayResourceManager;
                        const IEmbeddedCompositingManager& embeddedCompositingManager = updater.m_renderer.getDisplayController().getEmbeddedCompositingManager();
                        const IEmbeddedCompositor& embeddedCompositor                 = updater.m_renderer.getDisplayController().getRenderBackend().getEmbeddedCompositor();

                        contentAvailable |= embeddedCompositor.isContentAvailableForStreamTexture(streamSource);
                        DeviceResourceHandle tempResourceHandle = embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamSource);
                        if(tempResourceHandle.isValid())
                        {
                            streamDeviceHandle = tempResourceHandle;
                        }

                        tempResourceHandle = resourceManager.getResourceDeviceHandle(streamTex.fallbackTexture);
                        if(tempResourceHandle.isValid())
                        {
                            fallbackDeviceHandle = tempResourceHandle;
                        }

                        std::vector<TextureSamplerHandle> samplerList;
                        for (TextureSamplerHandle i(0); i < scene.getTextureSamplerCount(); i++)
                        {
                            if (scene.isTextureSamplerAllocated(i) && scene.getTextureSampler(i).contentType == TextureSampler::ContentType::StreamTexture && scene.getTextureSampler(i).contentHandle == streamTextureHandle.asMemoryHandle())
                            {
                                samplerList.push_back(i);
                            }
                        }

                        context << "StreamTexture [handle: " << streamTextureHandle << "]" << RendererLogContext::NewLine;
                        context.indent();
                        {
                            context << "Source:               " << streamSource.getValue() << RendererLogContext::NewLine;
                            if(streamDeviceHandle.isValid())
                            {
                                context << "Device handle:        " << streamDeviceHandle.asMemoryHandle() << RendererLogContext::NewLine;
                            }
                            else
                            {
                                context << "Device handle:        invalid" << RendererLogContext::NewLine;
                            }
                            context << "Content is available: " << (contentAvailable ? "yes" : "no") << RendererLogContext::NewLine;
                            context << RendererLogContext::NewLine;

                            // Fallback section
                            context << "FallbackTexture" << RendererLogContext::NewLine;
                            context.indent();
                            context << "Fallback is forced: " << (streamTex.forceFallbackTexture ? "yes" : "no") << RendererLogContext::NewLine;
                            if(fallbackDeviceHandle.isValid())
                            {
                                context << "Device handle:      " << fallbackDeviceHandle.asMemoryHandle() << RendererLogContext::NewLine;
                            }
                            else
                            {
                                context << "Device handle:      invalid" << RendererLogContext::NewLine;
                            }
                            context << "Hash:               " << streamTex.fallbackTexture << RendererLogContext::NewLine;
                            context.unindent();
                        }
                        context << RendererLogContext::NewLine;

                        // Sampler section
                        if(samplerList.empty())
                        {
                            context << "No sampler associated with this stream texture" << RendererLogContext::NewLine;
                        }
                        else
                        {
                            context << "Sampler using this stream texture" << RendererLogContext::NewLine;
                            context.indent();
                            for(const auto& sampler: samplerList)
                            {
                                DeviceResourceHandle samplerDeviceHandle = scene.getCachedHandlesForTextureSamplers()[sampler.asMemoryHandle()];
                                context << RendererLogContext::NewLine;
                                context << "Sampler [handle: " << sampler.asMemoryHandle() << "]" << RendererLogContext::NewLine;
                                context.indent();
                                if(samplerDeviceHandle.isValid())
                                {
                                    context << "Device handle:    " << samplerDeviceHandle.asMemoryHandle() << RendererLogContext::NewLine;
                                }
                                else
                                {
                                    context << "Device handle:    invalid" << RendererLogContext::NewLine;
                                }
                                context.unindent();
                            }
                            context.unindent();
                        }
                        context << RendererLogContext::NewLine;

                        context.unindent();
                    }
                }
                context.unindent();
            }
        }
        EndSection("RENDERER STREAM TEXTURES", context);
    }

    void RendererLogger::LogReferencedScenes(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        StartSection("SCENE REFERENCING", context);

        assert(updater.m_sceneReferenceLogic);
        const auto& sceneRefLogic = static_cast<const SceneReferenceLogic&>(*updater.m_sceneReferenceLogic);
        context << sceneRefLogic.m_masterScenes.size() << " master scene(s)" << RendererLogContext::NewLine << RendererLogContext::NewLine;

        context.indent();
        for (const auto& masterSceneIt : sceneRefLogic.m_masterScenes)
        {
            const auto masterSceneId = masterSceneIt.first;
            const auto& masterInfo = masterSceneIt.second;
            context << "Master Scene [id: " << masterSceneId << "] " << RendererLogContext::NewLine << RendererLogContext::NewLine;
            context.indent();

            context << "- referenced scenes (Ids): ";
            if (!masterInfo.sceneReferences.empty())
            {
                context << " [ ";
                for (const auto& refScene : masterInfo.sceneReferences)
                    context << refScene.first << " ";
                context << "]";
            }
            context << RendererLogContext::NewLine;

            context << "- pending actions (action types): ";
            if (!masterInfo.pendingActions.empty())
            {
                context << " [ ";
                for (const auto& action : masterInfo.pendingActions)
                    context << action.type << " ";
                context << "]";
            }
            context << RendererLogContext::NewLine;

            context << "- flush notification enabled for scenes (Ids): ";
            if (!masterInfo.sceneReferencesWithFlushNotification.empty())
            {
                context << " [ ";
                for (const auto& scene : masterInfo.sceneReferencesWithFlushNotification)
                    context << scene << " ";
                context << "]";
            }
            context << RendererLogContext::NewLine;

            context << "- expiration states (Id/state): ";
            if (!masterInfo.expirationStates.empty())
            {
                context << " [ ";
                for (const auto& scene : masterInfo.expirationStates)
                    context << scene.first << "/" << scene.second << " ";
                context << "]";
            }
            context << RendererLogContext::NewLine;

            context << "- consolidated expiration state: " << masterInfo.consolidatedExpirationState << RendererLogContext::NewLine;
            context << "- master destroyed: " << masterInfo.destroyed << RendererLogContext::NewLine;

            context.unindent();
        }
        context.unindent();

        EndSection("SCENE REFERENCING", context);
    }

    void RendererLogger::LogLinks(const RendererScenes& scenes, RendererLogContext& context)
    {
        StartSection("RENDERER LINKS", context);
        context << scenes.size() << " known Scene(s)" << RendererLogContext::NewLine << RendererLogContext::NewLine;

        context.indent();

        // Iterate through all scenes and list all consumers & providers with type and connections!!
        for(const auto& sceneIt : scenes)
        {
            const RendererCachedScene& scene = *(sceneIt.value.scene);
            const SceneId sceneId = sceneIt.key;

            context << "Scene [id: " << sceneId << "]" << RendererLogContext::NewLine << RendererLogContext::NewLine;
            context.indent();
            context << "Provider(s)" << RendererLogContext::NewLine;
            context.indent();

            UInt32 slotCount = 0u;
            for (const auto& slot : scene.getDataSlots())
            {
                const EDataSlotType slotType = slot.second->type;
                if (slotType == EDataSlotType_TransformationProvider || slotType == EDataSlotType_DataProvider || slotType == EDataSlotType_TextureProvider)
                {
                    LogProvider(scenes, context, scene, slot.first);
                    slotCount++;
                }
            }
            context.unindent();
            context << slotCount << " provider(s) found" << RendererLogContext::NewLine << RendererLogContext::NewLine;

            context << "Consumer(s)" << RendererLogContext::NewLine;
            context.indent();
            slotCount = 0u;
            for (const auto& slot : scene.getDataSlots())
            {
                const EDataSlotType slotType = slot.second->type;
                if (slotType == EDataSlotType_TransformationConsumer || slotType == EDataSlotType_DataConsumer || slotType == EDataSlotType_TextureConsumer)
                {
                    LogConsumer(scenes, context, scene, slot.first);
                    slotCount++;
                }
            }
            context.unindent();
            context << slotCount << " consumer(s) found" << RendererLogContext::NewLine << RendererLogContext::NewLine;
            context.unindent();
        }
        context.unindent();

        EndSection("RENDERER LINKS", context);
    }

    void RendererLogger::LogProvider(const RendererScenes& scenes, RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle, bool asLinked)
    {
        const SceneId providerSceneId = scene.getSceneId();
        const EDataSlotType slotType = scene.getDataSlot(slotHandle).type;

        const SceneLinks& sceneLinks = GetSceneLinks(scenes.getSceneLinksManager(), slotType);
        const Bool isLinked = sceneLinks.hasLinkedConsumers(providerSceneId, slotHandle);
        const DataSlotId providerId = scene.getDataSlot(slotHandle).id;

        context << "[";
        if (asLinked)
        {
            context << "sceneId: " << scene.getSceneId() << "; ";
        }
        context << "providerId: " << providerId.getValue() << "; ";
        LogSpecialSlotInfo(context, scene, slotHandle);
        context << "linked: " << (isLinked ? "yes" : "no");
        context << "]" << RendererLogContext::NewLine;

        if (!asLinked && isLinked && context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            context.indent();
            context << "linked to" << RendererLogContext::NewLine;
            context.indent();

            SceneLinkVector consumerLinks;
            sceneLinks.getLinkedConsumers(scene.getSceneId(), slotHandle, consumerLinks);
            for(const auto& consumerLink : consumerLinks)
            {
                LogConsumer(scenes, context, scenes.getScene(consumerLink.consumerSceneId), consumerLink.consumerSlot, true);
            }

            context.unindent();
            context.unindent();
        }
    }

    void RendererLogger::LogConsumer(const RendererScenes& scenes, RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle, bool asLinked)
    {
        const SceneId consumerSceneId = scene.getSceneId();
        const EDataSlotType slotType = scene.getDataSlot(slotHandle).type;
        const SceneLinks& sceneLinks = GetSceneLinks(scenes.getSceneLinksManager(), slotType);
        const OffscreenBufferHandle linkedOB = GetOffscreenBufferLinkedToConsumer(scenes, consumerSceneId, slotHandle);
        const StreamBufferHandle linkedSB = GetStreamBufferLinkedToConsumer(scenes, consumerSceneId, slotHandle);
        const Bool isLinked = sceneLinks.hasLinkedProvider(consumerSceneId, slotHandle) || linkedOB.isValid();
        const DataSlotId consumerId = scene.getDataSlot(slotHandle).id;

        context << "[";
        if (asLinked)
        {
            context << "sceneId: " << consumerSceneId << "; ";
        }
        context << "consumerId: " << consumerId.getValue() << "; ";
        LogSpecialSlotInfo(context, scene, slotHandle);
        context << "linked: " << (isLinked ? "yes" : "no");
        context << "]" << RendererLogContext::NewLine;

        if (!asLinked && isLinked && context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            context.indent();
            context << "linked to" << RendererLogContext::NewLine;
            context.indent();

            if (linkedOB.isValid())
            {
                context << "[ offscreen buffer handle " << linkedOB.asMemoryHandle() << " ]" << RendererLogContext::NewLine;
            }
            else if (linkedSB.isValid())
            {
                context << "[ stream buffer handle " << linkedSB.asMemoryHandle() << " ]" << RendererLogContext::NewLine;
            }
            else
            {
                const SceneLink& sceneLink = sceneLinks.getLinkedProvider(consumerSceneId, slotHandle);
                LogProvider(scenes, context, scenes.getScene(sceneLink.providerSceneId), sceneLink.providerSlot, true);
            }

            context.unindent();
            context.unindent();
        }
    }

    void RendererLogger::LogSpecialSlotInfo(RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle)
    {
        const DataSlot& dataSlot = scene.getDataSlot(slotHandle);
        switch (dataSlot.type)
        {
        case EDataSlotType_TransformationProvider:
        case EDataSlotType_TransformationConsumer:
        {
            context << "type: transformation; ";
            context << "node: \"" << dataSlot.attachedNode.asMemoryHandle() << "\"; ";
        }
        break;
        case EDataSlotType_DataProvider:
        case EDataSlotType_DataConsumer:
        {
            const EDataType datatype = GetDataTypeForSlot(scene, slotHandle);
            context << "type: data; ";
            context << "datatype: " << EnumToString(datatype) << "; ";
        }
        break;
        case EDataSlotType_TextureProvider:
        {
            const ResourceContentHash& hash = dataSlot.attachedTexture;
            context << "type: texture; ";
            context << "hash: " << hash << "; ";
        }
        break;
        case EDataSlotType_TextureConsumer:
        {
            const TextureSamplerHandle& handle = dataSlot.attachedTextureSampler;
            context << "type: texture; ";
            context << "handle: " << handle << "; ";
        }
        break;
        default:
            assert(false);
        }
    }

    const SceneLinks& RendererLogger::GetSceneLinks(const SceneLinksManager& linkManager, const EDataSlotType type)
    {
        switch (type)
        {
        case EDataSlotType_TransformationProvider:
        case EDataSlotType_TransformationConsumer:
            return linkManager.getTransformationLinkManager().getSceneLinks();
        case EDataSlotType_DataProvider:
        case EDataSlotType_DataConsumer:
            return linkManager.getDataReferenceLinkManager().getSceneLinks();
        case EDataSlotType_TextureProvider:
        case EDataSlotType_TextureConsumer:
            return linkManager.getTextureLinkManager().getSceneLinks();
        default:
            assert(false);
            static SceneLinks links;
            return links;
        }
    }

    EDataType RendererLogger::GetDataTypeForSlot(const RendererCachedScene& scene, const DataSlotHandle slotHandle)
    {
        const DataInstanceHandle dataReference = scene.getDataSlot(slotHandle).attachedDataReference;
        assert(dataReference.isValid());

        const DataLayoutHandle dataLayoutHandle = scene.getLayoutOfDataInstance(dataReference);
        assert(dataLayoutHandle.isValid());

        return scene.getDataLayout(dataLayoutHandle).getField(DataFieldHandle(0u)).dataType;
    }

    OffscreenBufferHandle RendererLogger::GetOffscreenBufferLinkedToConsumer(const RendererScenes& scenes, SceneId consumerScene, DataSlotHandle consumerSlot)
    {
        assert(scenes.hasScene(consumerScene));
        const IScene& scene = scenes.getScene(consumerScene);
        const TextureSamplerHandle sampler = scene.getDataSlot(consumerSlot).attachedTextureSampler;
        const TextureLinkManager& linkManager = scenes.getSceneLinksManager().getTextureLinkManager();
        if (linkManager.hasLinkedOffscreenBuffer(consumerScene, sampler))
            return linkManager.getLinkedOffscreenBuffer(consumerScene, sampler);

        return OffscreenBufferHandle::Invalid();
    }

    StreamBufferHandle RendererLogger::GetStreamBufferLinkedToConsumer(const RendererScenes& scenes, SceneId consumerScene, DataSlotHandle consumerSlot)
    {
        assert(scenes.hasScene(consumerScene));
        const IScene& scene = scenes.getScene(consumerScene);
        const TextureSamplerHandle sampler = scene.getDataSlot(consumerSlot).attachedTextureSampler;
        const TextureLinkManager& linkManager = scenes.getSceneLinksManager().getTextureLinkManager();
        if (linkManager.hasLinkedStreamBuffer(consumerScene, sampler))
            return linkManager.getLinkedStreamBuffer(consumerScene, sampler);

        return StreamBufferHandle::Invalid();
    }

    void RendererLogger::LogEmbeddedCompositor(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        if (!updater.m_renderer.hasDisplayController())
        {
            context << RendererLogContext::NewLine << "Skipping EC section due to missing display controller!" << RendererLogContext::NewLine;
            return;
        }

        StartSection("EMBEDDED COMPOSITOR", context);

        const WaylandIviSurfaceId iviSurfaceId = updater.m_renderer.getDisplayController().getRenderBackend().getWindow().getWaylandIviSurfaceID();

        context << "IviSurfaceId: " << iviSurfaceId.getValue() << RendererLogContext::NewLine << RendererLogContext::NewLine;

        context.indent();
        const IEmbeddedCompositor& compositor = updater.m_renderer.getDisplayController().getRenderBackend().getEmbeddedCompositor();
        compositor.logInfos(context);
        context.unindent();

        EndSection("EMBEDDED COMPOSITOR", context);
    }

    void RendererLogger::LogEventQueue(const RendererSceneUpdater& updater, RendererLogContext& context)
    {
        StartSection("RENDERER EVENTS", context);

        const auto rendererEvents = updater.m_rendererEventCollector.getRendererEvents();
        const auto sceneControlEvents = updater.m_rendererEventCollector.getSceneControlEvents();

        context << rendererEvents.size() << " renderer event(s) pending to be dispatched" << RendererLogContext::NewLine;
        context << sceneControlEvents.size() << " scene control event(s) pending to be dispatched" << RendererLogContext::NewLine;
        context.indent();

        if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            for (size_t i = 0; i < rendererEvents.size(); ++i)
                context << i << ". " << rendererEvents[i].eventType << RendererLogContext::NewLine;
            for (size_t i = 0; i < sceneControlEvents.size(); ++i)
                context << i << ". " << sceneControlEvents[i].eventType << RendererLogContext::NewLine;
        }

        context.unindent();
        EndSection("RENDERER EVENTS", context);
    }

    void RendererLogger::LogPeriodicInfo(const RendererSceneUpdater& updater)
    {
        LOG_INFO_F(CONTEXT_PERIODIC, ([&](StringOutputStream& sos)
        {
            sos.reserve(2048);

            SceneIdVector knownSceneIds;
            updater.m_sceneStateExecutor.m_scenesStateInfo.getKnownSceneIds(knownSceneIds);
            sos << "Renderer: " << knownSceneIds.size() << " scene(s):";
            for(const auto& sceneId : knownSceneIds)
            {
                const ESceneState sceneState = updater.m_sceneStateExecutor.getSceneState(sceneId);
                sos << "  " << sceneId << " " << EnumToString(sceneState);
                if (updater.m_rendererScenes.hasScene(sceneId))
                {
                    const auto& stagingInfo = updater.m_rendererScenes.getStagingInfo(sceneId);
                    if (stagingInfo.lastAppliedVersionTag.isValid())
                        sos << " (versionTag " << stagingInfo.lastAppliedVersionTag << ")";

                    const auto& pendingFlushes = stagingInfo.pendingData.pendingFlushes;
                    if (!pendingFlushes.empty())
                        sos << " (" << pendingFlushes.size() << " pending flushes)";
                }
            }

            // scene referencing rper
            assert(updater.m_sceneReferenceLogic);
            const auto& sceneRefLogic = static_cast<const SceneReferenceLogic&>(*updater.m_sceneReferenceLogic);
            if (!sceneRefLogic.m_masterScenes.empty())
            {
                sos << "\nSRef:";
                for (const auto& masterSceneIt : sceneRefLogic.m_masterScenes)
                {
                    sos << " " << masterSceneIt.first << "->[";
                    for (const auto& refScene : masterSceneIt.second.sceneReferences)
                        sos << " " << refScene.first;
                    sos << " ] ";
                }
            }

            sos << "\n";
            updater.m_renderer.getStatistics().writeStatsToStream(sos);
            sos << "\nTime budgets:"
                << " sceneResourceUpload " << Int64(updater.m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload).count()) << "us"
                << " resourceUpload " << Int64(updater.m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ResourcesUpload).count()) << "us"
                << " obRender " << Int64(updater.m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender).count()) << "us"
                << " skub=" << updater.m_skipUnmodifiedScenes;
            sos << "\n";
            updater.m_renderer.getProfilerStatistics().writeLongestFrameTimingsToStream(sos);
            sos << "\n";
            updater.m_renderer.getMemoryStatistics().writeMemoryUsageSummaryToString(sos);
        }));

        updater.m_renderer.getStatistics().reset();
        updater.m_renderer.getProfilerStatistics().resetFrameTimings();
        updater.m_renderer.getMemoryStatistics().reset();
    }
}
