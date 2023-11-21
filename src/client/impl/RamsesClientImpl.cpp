//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/Scene.h"
#include "ramses/client/Resource.h"
#include "ramses/client/EffectDescription.h"
#include "ramses-sdk-build-config.h"

// internal
#include "impl/SceneImpl.h"
#include "impl/SceneConfigImpl.h"
#include "impl/ResourceImpl.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "impl/SerializationHelper.h"
#include "internal/RamsesVersion.h"
#include "impl/SceneReferenceImpl.h"
#include "impl/SaveFileConfigImpl.h"

// framework
#include "internal/SceneGraph/SceneAPI/SceneCreationInformation.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"
#include "internal/SceneGraph/Scene/ScenePersistation.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/Components/ResourcePersistation.h"
#include "internal/Components/ManagedResource.h"
#include "internal/Components/ResourceTableOfContents.h"
#include "internal/Components/FileInputStreamContainer.h"
#include "internal/Components/MemoryInputStreamContainer.h"
#include "internal/Components/OffsetFileInputStreamContainer.h"
#include "internal/SceneGraph/Resource/IResource.h"
#include "internal/ClientCommands/PrintSceneList.h"
#include "internal/ClientCommands/FlushSceneVersion.h"
#include "impl/SerializationContext.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/Core/Utils/LogContext.h"
#include "internal/Core/Utils/File.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/RamsesLogger.h"
#include "ClientFactory.h"
#include "impl/FrameworkFactoryRegistry.h"
#include "internal/Ramsh/Ramsh.h"
#include "impl/DataTypeUtils.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/glslEffectBlock/GlslEffect.h"
#include "impl/EffectDescriptionImpl.h"
#include "impl/TextureUtils.h"

#include <cstdint>
#include <array>

namespace ramses::internal
{
    static const bool clientRegisterSuccess = ClientFactory::RegisterClientFactory();

    RamsesClientImpl::RamsesClientImpl(RamsesFrameworkImpl& framework,  std::string_view applicationName)
        : RamsesObjectImpl(ERamsesObjectType::Client, applicationName)
        , m_appLogic(framework.getParticipantAddress().getParticipantId(), framework.getFrameworkLock())
        , m_framework(framework)
        , m_loadFromFileTaskQueue(framework.getTaskQueue())
        , m_deleteSceneQueue(framework.getTaskQueue())
    {
        assert(!framework.isConnected());

        m_appLogic.init(framework.getResourceComponent(), framework.getScenegraphComponent());
        m_cmdPrintSceneList = std::make_shared<ramses::internal::PrintSceneList>(*this);
        m_cmdPrintValidation = std::make_shared<ramses::internal::ValidateCommand>(*this);
        m_cmdFlushSceneVersion = std::make_shared<ramses::internal::FlushSceneVersion>(*this);
        m_cmdDumpSceneToFile = std::make_shared<ramses::internal::DumpSceneToFile>(*this);
        m_cmdLogResourceMemoryUsage = std::make_shared<ramses::internal::LogResourceMemoryUsage>(*this);
        framework.getRamsh().add(m_cmdPrintSceneList);
        framework.getRamsh().add(m_cmdPrintValidation);
        framework.getRamsh().add(m_cmdFlushSceneVersion);
        framework.getRamsh().add(m_cmdDumpSceneToFile);
        framework.getRamsh().add(m_cmdLogResourceMemoryUsage);
        m_framework.getPeriodicLogger().registerPeriodicLogSupplier(&m_framework.getScenegraphComponent());
    }

    RamsesClientImpl::~RamsesClientImpl()
    {
        LOG_INFO(CONTEXT_CLIENT, "RamsesClientImpl::~RamsesClientImpl");
        m_deleteSceneQueue.disableAcceptingTasksAfterExecutingCurrentQueue();
        m_loadFromFileTaskQueue.disableAcceptingTasksAfterExecutingCurrentQueue();

        // delete async loaded  scenes that were never collected via calling dispatchEvents
        ramses::internal::PlatformGuard g(m_clientLock);
        m_asyncSceneLoadStatusVec.clear();

        LOG_INFO(CONTEXT_CLIENT, "RamsesClientImpl::~RamsesClientImpl deleting scenes");
        m_scenes.clear();

        m_framework.getPeriodicLogger().removePeriodicLogSupplier(&m_framework.getScenegraphComponent());
    }

    void RamsesClientImpl::setHLObject(RamsesClient* hlClient)
    {
        assert(hlClient);
        m_hlClient = hlClient;
    }


    void RamsesClientImpl::deinitializeFrameworkData()
    {
    }

    const ramses::internal::ClientApplicationLogic& RamsesClientImpl::getClientApplication() const
    {
        return m_appLogic;
    }

    ramses::internal::ClientApplicationLogic& RamsesClientImpl::getClientApplication()
    {
        return m_appLogic;
    }

    ramses::Scene* RamsesClientImpl::createScene(const SceneConfigImpl& sceneConfig, std::string_view name)
    {
        if (!sceneConfig.getSceneId().isValid())
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::createScene: invalid sceneId");
            return nullptr;
        }

        ramses::internal::PlatformGuard g(m_clientLock);
        ramses::internal::SceneInfo sceneInfo;
        sceneInfo.friendlyName = name;
        sceneInfo.sceneID = ramses::internal::SceneId(sceneConfig.getSceneId().getValue());

        ramses::internal::ClientScene* internalScene = m_sceneFactory.createScene(sceneInfo);
        if (nullptr == internalScene)
        {
            return nullptr;
        }

        auto impl = std::make_unique<SceneImpl>(*internalScene, sceneConfig, *m_hlClient);
        impl->initializeFrameworkData();
        m_scenes.push_back(SceneOwningPtr{ new ramses::Scene{ std::move(impl) }, [](ramses::Scene* s) { delete s; } });

        return m_scenes.back().get();
    }

    RamsesClientImpl::DeleteSceneRunnable::DeleteSceneRunnable(SceneOwningPtr scene, InternalSceneOwningPtr llscene)
        : m_scene{ std::move(scene) }
        , m_lowLevelScene{ std::move(llscene) }
    {
    }

    void RamsesClientImpl::DeleteSceneRunnable::execute()
    {
        m_scene.reset();
        m_lowLevelScene.reset();
    }

    bool RamsesClientImpl::destroy(ramses::Scene& scene)
    {
        ramses::internal::PlatformGuard g(m_clientLock);
        auto iter = std::find_if(m_scenes.begin(), m_scenes.end(), [&scene](auto& s) { return s.get() == &scene; });
        if (iter != m_scenes.end())
        {
            auto sceneOwnPtr = std::move(*iter);
            m_scenes.erase(iter);

            const ramses::internal::SceneId sceneID(scene.getSceneId().getValue());
            auto llscene = m_sceneFactory.releaseScene(sceneID);

            getClientApplication().removeScene(sceneID);

            scene.impl().closeSceneFile();
            auto task = new DeleteSceneRunnable(std::move(sceneOwnPtr), std::move(llscene));
            m_deleteSceneQueue.enqueue(*task);
            task->release();

            return true;
        }

        m_framework.getErrorReporting().set("RamsesClient::destroy failed, scene is not in this client.");
        return false;
    }

    void RamsesClientImpl::writeLowLevelResourcesToStream(const ResourceObjects& resources, ramses::internal::IOutputStream& resourceOutputStream, bool compress) const
    {
        //getting names for resources (names are transmitted only for debugging purposes)
        ramses::internal::ManagedResourceVector managedResources;
        managedResources.reserve(resources.size());
        for (const auto res : resources)
        {
            assert(res != nullptr);
            const ramses::internal::ResourceContentHash& hash = res->impl().getLowlevelResourceHash();
            const ramses::internal::ManagedResource managedRes = getClientApplication().getResource(hash);
            if (managedRes)
            {
                managedResources.push_back(managedRes);
            }
            else
            {
                const ramses::internal::ManagedResource loadedResource = getClientApplication().loadResource(hash);
                assert(loadedResource);
                managedResources.push_back(loadedResource);
            }
        }

        // sort resources by hash to maintain a deterministic order in which we write them to file, remove duplicates
        std::sort(managedResources.begin(), managedResources.end(), [](auto const& a, auto const& b) { return a->getHash() < b->getHash(); });
        managedResources.erase(std::unique(managedResources.begin(), managedResources.end()), managedResources.end());

        // write LL-TOC and LL resources
        ramses::internal::ResourcePersistation::WriteNamedResourcesWithTOCToStream(resourceOutputStream, managedResources, compress);
    }

    ramses::internal::ManagedResource RamsesClientImpl::getResource(ramses::internal::ResourceContentHash hash) const
    {
        return m_appLogic.getResource(hash);
    }

    SceneOwningPtr RamsesClientImpl::loadSceneObjectFromStream(const std::string& caller,
                                                       std::string const& filename,
                                                       ramses::internal::IInputStream& inputStream,
                                                       const SceneConfigImpl& config)
    {
        LOG_TRACE(CONTEXT_CLIENT, "RamsesClient::prepareSceneFromInputStream:  start loading scene from input stream");

        ramses::internal::SceneCreationInformation createInfo;
        ramses::internal::ScenePersistation::ReadSceneMetadataFromStream(inputStream, createInfo);
        if (config.getSceneId().isValid())
        {
            const auto newSceneId = ramses::internal::SceneId(config.getSceneId().getValue());
            LOG_INFO(CONTEXT_CLIENT, "RamsesClient::{}: Override stored scene id: {} with user provided scene id: {}", caller, createInfo.m_id, newSceneId);
            createInfo.m_id = newSceneId;
        }
        const ramses::internal::SceneSizeInformation& sizeInformation = createInfo.m_sizeInfo;
        const ramses::internal::SceneInfo sceneInfo(createInfo.m_id, createInfo.m_name);

        LOG_DEBUG(CONTEXT_CLIENT, "RamsesClient::prepareSceneFromInputStream:  scene to be loaded has {}", sizeInformation);

        ramses::internal::ClientScene* internalScene = nullptr;
        {
            ramses::internal::PlatformGuard g(m_clientLock);
            internalScene = m_sceneFactory.createScene(sceneInfo);
        }
        if (nullptr == internalScene)
        {
            return nullptr;
        }
        internalScene->preallocateSceneSize(sizeInformation);

        // need first to create the pimpl, so that internal framework components know the new scene
        if (config.getPublicationMode() == EScenePublicationMode::LocalOnly)
        {
            LOG_INFO(CONTEXT_CLIENT, "RamsesClient::{}: Mark file loaded from {} with sceneId {} as local only", caller, filename, createInfo.m_id);
        }
        else
        {
            LOG_INFO(CONTEXT_CLIENT, "RamsesClient::{}: Mark file loaded from {} with sceneId {} as local and remote", caller, filename, createInfo.m_id);
        }

        auto impl = std::make_unique<SceneImpl>(*internalScene, config, *m_hlClient);

        // now the scene is registered, so it's possible to load the low level content into the scene
        LOG_TRACE(CONTEXT_CLIENT, "    Reading low level scene from stream");
        ramses::internal::ScenePersistation::ReadSceneFromStream(inputStream, *internalScene);

        LOG_TRACE(CONTEXT_CLIENT, "    Deserializing high level scene objects from stream");
        DeserializationContext deserializationContext(config);
        SerializationHelper::DeserializeObjectID(inputStream);
        if (!impl->deserialize(inputStream, deserializationContext))
        {
            LOG_ERROR(CONTEXT_CLIENT, "    Failed to deserialize high level scene:");
            LOG_ERROR(CONTEXT_CLIENT, m_framework.getErrorReporting().getError().value_or(Issue{}).message);
            return nullptr;
        }

        LOG_TRACE(CONTEXT_CLIENT, "    Done with preparing scene from input stream.");

        return SceneOwningPtr{ new ramses::Scene{ std::move(impl) }, [](ramses::Scene* s) { delete s; } };
    }

    SceneOwningPtr RamsesClientImpl::loadSceneFromCreationConfig(const SceneCreationConfig& cconfig)
    {
        // this stream contains scene data AND resource data and will be handed over to and held open by resource component as resource stream
        ramses::internal::IInputStream& inputStream = cconfig.streamContainer->getStream();
        if (inputStream.getState() != ramses::internal::EStatus::Ok)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::{}:  failed to open scene source {}", cconfig.caller, cconfig.dataSource);
            return nullptr;
        }

        if (!ReadRamsesVersionAndPrintWarningOnMismatch(inputStream, "scene file", getFramework().getFeatureLevel()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::{}: failed to read from scene source {}", cconfig.caller, cconfig.dataSource);
            return nullptr;
        }

        SaveFileConfigImpl::ExporterVersion exporter;
        std::string metadataString;
        inputStream >> exporter;
        inputStream >> metadataString;

        LOG_INFO(CONTEXT_CLIENT, "Metadata: '{}'", metadataString);
        LOG_INFO(CONTEXT_CLIENT, "Exporter version: {}.{}.{} (file format version {})", exporter.major, exporter.minor, exporter.patch, exporter.fileFormat);

        uint64_t sceneObjectStart = 0;
        uint64_t llResourceStart = 0;
        inputStream >> sceneObjectStart;
        inputStream >> llResourceStart;

        SceneOwningPtr scene;
        if (cconfig.prefetchData)
        {
            std::vector<std::byte> sceneData(static_cast<size_t>(llResourceStart - sceneObjectStart));
            inputStream.read(sceneData.data(), sceneData.size());

            if (inputStream.getState() != ramses::internal::EStatus::Ok)
            {
                LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::{}: Failed reading scene from file: {} ", cconfig.caller, inputStream.getState());
                return nullptr;
            }

            ramses::internal::BinaryInputStream sceneDataStream(sceneData.data());
            scene = loadSceneObjectFromStream(cconfig.caller, cconfig.dataSource, sceneDataStream, cconfig.config);
        }
        else
        {
            // this path will be used in the future when creating scene from user provided stream
            scene = loadSceneObjectFromStream(cconfig.caller, cconfig.dataSource, inputStream, cconfig.config);
        }
        if (!scene)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::{}: scene creation for '{}' failed", cconfig.caller, cconfig.dataSource);
            return nullptr;
        }

        // calls on m_appLogic are thread safe
        // register stream for on-demand resource loading (LL-Resources)
        ramses::internal::ResourceTableOfContents loadedTOC;
        loadedTOC.readTOCPosAndTOCFromStream(inputStream);
        const ramses::internal::SceneFileHandle fileHandle = m_appLogic.addResourceFile(cconfig.streamContainer, loadedTOC);
        scene->m_impl.setSceneFileHandle(fileHandle);

        LOG_INFO(CONTEXT_CLIENT, "RamsesClient::{}: Source '{}' has handle {}", cconfig.caller, cconfig.dataSource, fileHandle);

        return scene;
    }

    ramses::Scene* RamsesClientImpl::loadSceneFromFile(std::string_view fileName, const SceneConfigImpl& config)
    {
        if (fileName.empty())
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::loadSceneFromFile: filename may not be empty");
            return nullptr;
        }

        return loadSceneSynchonousCommon({
                "loadSceneFromFile",
                std::string{fileName},
                std::make_shared<ramses::internal::FileInputStreamContainer>(fileName), true, config
            });
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ramses::Scene* RamsesClientImpl::loadSceneFromMemory(std::unique_ptr<std::byte[], void (*)(const std::byte*)> data, size_t size, const SceneConfigImpl& config)
    {
        if (!data)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::loadSceneFromMemory: data may not be null");
            return nullptr;
        }
        if (size == 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::loadSceneFromMemory: size may not be 0");
            return nullptr;
        }

        return loadSceneSynchonousCommon({
                "loadSceneFromMemory",
                fmt::format("<memorybuffer size:{}>", size),
                std::make_shared<ramses::internal::MemoryInputStreamContainer>(std::move(data)),
                false,
                config
            });
    }

    ramses::Scene* RamsesClientImpl::loadSceneFromFileDescriptor(int fd, size_t offset, size_t length, const SceneConfigImpl& config)
    {
        if (fd <= 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::loadSceneFromFileDescriptor: filedescriptor must be valid {}", fd);
            return nullptr;
        }
        if (length == 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::loadSceneFromFileDescriptor: length may not be 0");
            return nullptr;
        }

        return loadSceneSynchonousCommon(SceneCreationConfig{
                "loadSceneFromFileDescriptor",
                fmt::format("<filedescriptor fd:{} offset:{} length:{}>", fd, offset, length),
                std::make_shared<ramses::internal::OffsetFileInputStreamContainer>(fd, offset, length),
                true,
                config
            });
    }

    ramses::Scene* RamsesClientImpl::loadSceneSynchonousCommon(const SceneCreationConfig& cconfig)
    {
        const uint64_t start = ramses::internal::PlatformTime::GetMillisecondsMonotonic();
        auto scene = loadSceneFromCreationConfig(cconfig);
        if (!scene)
            return nullptr;

        auto* scenePtr = scene.get();
        finalizeLoadedScene(std::move(scene));

        const uint64_t end = ramses::internal::PlatformTime::GetMillisecondsMonotonic();
        LOG_INFO(CONTEXT_CLIENT, "RamsesClient::{} ramses::Scene loaded from '{}' in {} ms", cconfig.caller, cconfig.dataSource, end - start);

        return scenePtr;
    }

    void RamsesClientImpl::finalizeLoadedScene(SceneOwningPtr scene)
    {
        // add to the known list of scenes
        ramses::internal::PlatformGuard g(m_clientLock);
        m_scenes.push_back(std::move(scene));
    }

    void RamsesClientImpl::WriteCurrentBuildVersionToStream(ramses::internal::IOutputStream& stream, EFeatureLevel featureLevel)
    {
        ramses::internal::RamsesVersion::WriteToStream(stream, ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, featureLevel);
    }

    bool RamsesClientImpl::GetFeatureLevelFromStream(ramses::internal::IInputStreamContainer& streamContainer, const std::string& desc, EFeatureLevel& detectedFeatureLevel)
    {
        ramses::internal::RamsesVersion::VersionInfo readVersion;
        if (!ramses::internal::RamsesVersion::ReadFromStream(streamContainer.getStream(), readVersion, detectedFeatureLevel))
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::GetFeatureLevelFromSceneFile: failed to read RAMSES version and feature level from '{}'.", desc);
            return false;
        }

        return true;
    }

    bool RamsesClientImpl::GetFeatureLevelFromFile(std::string_view fileName, EFeatureLevel& detectedFeatureLevel)
    {
        ramses::internal::FileInputStreamContainer streamContainer{ fileName };
        return GetFeatureLevelFromStream(streamContainer, std::string{fileName}, detectedFeatureLevel);
    }

    bool RamsesClientImpl::GetFeatureLevelFromFile(int fd, size_t offset, size_t length, EFeatureLevel& detectedFeatureLevel)
    {
        if (fd <= 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::GetFeatureLevelFromFile: filedescriptor must be valid {}", fd);
            return false;
        }
        if (length == 0u)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::GetFeatureLevelFromFile: length may not be 0");
            return false;
        }

        ramses::internal::OffsetFileInputStreamContainer streamContainer{ fd, offset, length };
        return GetFeatureLevelFromStream(streamContainer, fmt::format("fileDescriptor fd:{} offset:{} length:{}", fd, offset, length), detectedFeatureLevel);
    }

    bool RamsesClientImpl::ReadRamsesVersionAndPrintWarningOnMismatch(ramses::internal::IInputStream& inputStream, std::string_view verboseFileName, EFeatureLevel featureLevel)
    {
        // return false on read error only, not version mismatch
        ramses::internal::RamsesVersion::VersionInfo readVersion;
        EFeatureLevel featureLevelFromFile = EFeatureLevel_01;
        if (!ramses::internal::RamsesVersion::ReadFromStream(inputStream, readVersion, featureLevelFromFile))
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::ReadRamsesVersionAndPrintWarningOnMismatch: failed to read RAMSES version for {}, file probably corrupt. Loading aborted.", verboseFileName);
            return false;
        }
        LOG_INFO(CONTEXT_CLIENT, "RAMSES version in file '{}': [{}]; GitHash: [{}]; FeatureLevel: [{}];", verboseFileName, readVersion.versionString, readVersion.gitHash, featureLevelFromFile);

        if (!ramses::internal::RamsesVersion::MatchesMajorMinor(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR_INT, ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR_INT, readVersion))
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::ReadRamsesVersionAndPrintWarningOnMismatch: Version of file {} does not match MAJOR.MINOR of this build. Cannot load the file.", verboseFileName);
            LOG_ERROR(CONTEXT_CLIENT, "SDK version of loader: [{}]; GitHash: [{}]", ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH);
            return false;
        }

        if (featureLevelFromFile != featureLevel)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::ReadRamsesVersionAndPrintWarningOnMismatch: Feature level of file '{}' is {} which does not match feature level of this Ramses instance({}).Cannot load the file.",
                verboseFileName, featureLevelFromFile, featureLevel);
            return false;
        }

        return true;
    }

    bool RamsesClientImpl::loadSceneFromFileAsync(std::string_view fileName, const SceneConfigImpl& config)
    {
        const std::string stdFilename(fileName);
        if (stdFilename.empty())
        {
            m_framework.getErrorReporting().set("RamsesClient::loadSceneFromFileAsync: filename may not be empty");
            return false;
        }

        auto* task =
            new LoadSceneRunnable(*this, SceneCreationConfig{
                    "loadSceneFromFileAsync",
                    stdFilename,
                    std::make_shared<ramses::internal::FileInputStreamContainer>(stdFilename),
                    true,
                    config
                });
        m_loadFromFileTaskQueue.enqueue(*task);
        task->release();
        return true;
    }

    ramses::SceneReference* RamsesClientImpl::findSceneReference(sceneId_t masterSceneId, sceneId_t referencedSceneId)
    {
        for (auto const& scene : getListOfScenes())
        {
            if (masterSceneId == scene->getSceneId())
                return scene->m_impl.getSceneReference(referencedSceneId);
        }

        return nullptr;
    }

    bool RamsesClientImpl::dispatchEvents(IClientEventHandler& clientEventHandler)
    {
        std::vector<SceneLoadStatus> localAsyncSceneLoadStatus;
        {
            ramses::internal::PlatformGuard g(m_clientLock);
            localAsyncSceneLoadStatus.swap(m_asyncSceneLoadStatusVec);
        }

        for (auto& sceneStatus : localAsyncSceneLoadStatus)
        {
            if (sceneStatus.scene)
            {
                // finalize scene
                ramses::Scene* scene = sceneStatus.scene.get();
                const uint64_t start = ramses::internal::PlatformTime::GetMillisecondsMonotonic();
                finalizeLoadedScene(std::move(sceneStatus.scene));
                const uint64_t end = ramses::internal::PlatformTime::GetMillisecondsMonotonic();
                LOG_INFO(CONTEXT_CLIENT, "RamsesClient::dispatchEvents(sceneFileLoadSucceeded): Synchronous postprocessing of scene loaded from '{}' (sceneName: {}, sceneId {}) in {} ms",
                    sceneStatus.sceneFilename, scene->getName(), scene->getSceneId(), end - start);

                clientEventHandler.sceneFileLoadSucceeded(sceneStatus.sceneFilename.c_str(), scene);
            }
            else
            {
                LOG_INFO(CONTEXT_CLIENT, "RamsesClient::dispatchEvents(sceneFileLoadFailed): {}", sceneStatus.sceneFilename);
                clientEventHandler.sceneFileLoadFailed(sceneStatus.sceneFilename.c_str());
            }
        }

        const auto clientRendererEvents = getClientApplication().popSceneReferenceEvents();
        for (const auto& rendererEvent : clientRendererEvents)
        {
            switch (rendererEvent.type)
            {
            case ramses::internal::SceneReferenceEventType::SceneStateChanged:
            {
                auto sr = findSceneReference(sceneId_t{ rendererEvent.masterSceneId.getValue() }, sceneId_t{ rendererEvent.referencedScene.getValue() });
                if (sr)
                {
                    LOG_INFO(CONTEXT_CLIENT, "RamsesClient::dispatchEvents master:reference scene state changed: {}:{} {}",
                        rendererEvent.masterSceneId, rendererEvent.referencedScene, ramses::internal::EnumToString(rendererEvent.sceneState));

                    sr->impl().setReportedState(rendererEvent.sceneState);
                    clientEventHandler.sceneReferenceStateChanged(*sr, rendererEvent.sceneState);
                }
                else
                {
                    LOG_WARN(CONTEXT_CLIENT, "RamsesClientImpl::dispatchEvents: did not find SceneReference for a SceneStateChanged event: {} {} {}",
                        rendererEvent.masterSceneId, rendererEvent.referencedScene, ramses::internal::EnumToString(rendererEvent.sceneState));
                }
                break;
            }
            case ramses::internal::SceneReferenceEventType::SceneFlushed:
            {
                auto sr = findSceneReference(sceneId_t{ rendererEvent.masterSceneId.getValue() }, sceneId_t{ rendererEvent.referencedScene.getValue() });
                if (sr)
                {
                    clientEventHandler.sceneReferenceFlushed(*sr, sceneVersionTag_t{ rendererEvent.tag.getValue() });
                }
                else
                {
                    LOG_WARN(CONTEXT_CLIENT, "RamsesClientImpl::dispatchEvents: did not find SceneReference for a SceneFlushed event: {} {} {}",
                        rendererEvent.masterSceneId, rendererEvent.referencedScene, rendererEvent.tag);
                }
                break;
            }
            case ramses::internal::SceneReferenceEventType::DataLinked:
                clientEventHandler.dataLinked(sceneId_t{ rendererEvent.providerScene.getValue() }, dataProviderId_t{ rendererEvent.dataProvider.getValue() },
                    sceneId_t{ rendererEvent.consumerScene.getValue() }, dataConsumerId_t{ rendererEvent.dataConsumer.getValue() }, rendererEvent.status);
                break;
            case ramses::internal::SceneReferenceEventType::DataUnlinked:
                clientEventHandler.dataUnlinked(sceneId_t{ rendererEvent.consumerScene.getValue() }, dataConsumerId_t{ rendererEvent.dataConsumer.getValue() }, rendererEvent.status);
                break;
            }
        }

        return true;
    }

    RamsesClientImpl::LoadSceneRunnable::LoadSceneRunnable(RamsesClientImpl& client, SceneCreationConfig&& cconfig)
        : m_client(client)
        , m_cconfig(std::move(cconfig))
    {
    }

    void RamsesClientImpl::LoadSceneRunnable::execute()
    {
        const uint64_t start = ramses::internal::PlatformTime::GetMillisecondsMonotonic();
        auto scene = m_client.loadSceneFromCreationConfig(m_cconfig);
        const uint64_t end = ramses::internal::PlatformTime::GetMillisecondsMonotonic();

        if (scene)
        {
            LOG_INFO(CONTEXT_CLIENT, "RamsesClient::loadSceneFromFileAsync: ramses::Scene loaded from '{}' (sceneName: {}, sceneId {}) in {} ms",
                m_cconfig.dataSource, scene->getName(), scene->getSceneId(), end - start);
        }

        ramses::internal::PlatformGuard g(m_client.m_clientLock);
        // NOTE: only used for real files by name for now, not sure how to report other cases to user.
        // therefore can assume dataSource is the filename
        m_client.m_asyncSceneLoadStatusVec.push_back({std::move(scene), m_cconfig.dataSource});
    }

    const SceneVector& RamsesClientImpl::getListOfScenes() const
    {
        ramses::internal::PlatformGuard g(m_clientLock);
        return m_scenes;
    }

    ramses::internal::ResourceHashUsage RamsesClientImpl::getHashUsage_ThreadSafe(const ramses::internal::ResourceContentHash& hash) const
    {
        return m_appLogic.getHashUsage(hash);
    }

    ramses::internal::ManagedResource RamsesClientImpl::getResource_ThreadSafe(ramses::internal::ResourceContentHash hash) const
    {
        return m_appLogic.getResource(hash);
    }

    ramses::internal::ManagedResource RamsesClientImpl::loadResource_ThreadSafe(const ramses::internal::ResourceContentHash& hash) const
    {
        return m_appLogic.loadResource(hash);
    }

    const ramses::Scene* RamsesClientImpl::findSceneByName(std::string_view name) const
    {
        ramses::internal::PlatformGuard g(m_clientLock);
        for (const auto& scene : m_scenes)
        {
            if (scene->getName() == name)
                return scene.get();
        }

        return nullptr;
    }

    ramses::Scene* RamsesClientImpl::findSceneByName(std::string_view name)
    {
        // Non-const version of findObjectByName cast to its const version to avoid duplicating code
        return const_cast<ramses::Scene*>((const_cast<const RamsesClientImpl&>(*this)).findSceneByName(name));
    }

    const ramses::Scene* RamsesClientImpl::getScene(sceneId_t sceneId) const
    {
        ramses::internal::PlatformGuard g(m_clientLock);
        for (const auto& scene : m_scenes)
        {
            if (scene->getSceneId() == sceneId)
                return scene.get();
        }

        return nullptr;
    }

    ramses::Scene* RamsesClientImpl::getScene(sceneId_t sceneId)
    {
        // Non-const version of findObjectByName cast to its const version to avoid duplicating code
        return const_cast<ramses::Scene*>((const_cast<const RamsesClientImpl&>(*this)).getScene(sceneId));
    }


    ramses::internal::ManagedResource RamsesClientImpl::manageResource(const ramses::internal::IResource* res)
    {
        ramses::internal::ManagedResource managedRes = m_appLogic.addResource(res);
        LOG_HL_CLIENT_API_STR("Created resource with internal hash {}, name: {}", managedRes->getHash(), managedRes->getName());

        return managedRes;
    }

    RamsesClientImpl& RamsesClientImpl::createImpl(std::string_view name, RamsesFrameworkImpl& components)
    {
        return *new RamsesClientImpl(components, name);
    }

    const RamsesFrameworkImpl& RamsesClientImpl::getFramework() const
    {
        return m_framework;
    }

    RamsesFrameworkImpl& RamsesClientImpl::getFramework()
    {
        return m_framework;
    }

    void RamsesClientImpl::onValidate(ValidationReportImpl& report) const
    {
        RamsesObjectImpl::onValidate(report);
        validateScenes(report);
    }

    void RamsesClientImpl::validateScenes(ValidationReportImpl& report) const
    {
        ramses::internal::PlatformGuard g(m_clientLock);

        for (const auto& scene : m_scenes)
        {
            scene->impl().validate(report);
        }
    }

    ramses::internal::ManagedResource RamsesClientImpl::createManagedArrayResource(uint32_t numElements, ramses::EDataType type, const void* arrayData, std::string_view name)
    {
        if (0u == numElements || nullptr == arrayData)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClientImpl::createManagedArrayResource Array resource must have element count > 0 and data must not be nullptr!");
            return {};
        }

        ramses::internal::EDataType elementType = DataTypeUtils::ConvertDataTypeToInternal(type);
        ramses::internal::EResourceType resourceType = DataTypeUtils::DeductResourceTypeFromDataType(type);

        auto resource = new ramses::internal::ArrayResource(resourceType, numElements, elementType, arrayData, name);
        return manageResource(resource);
    }

    template <typename MipDataStorageType>
    ramses::internal::ManagedResource RamsesClientImpl::createManagedTexture(ramses::internal::EResourceType textureType,
                                                                            uint32_t width, uint32_t height, uint32_t depth,
                                                                            ETextureFormat format,
                                                                            const std::vector<MipDataStorageType>& mipLevelData, bool generateMipChain,
                                                                            const TextureSwizzle& swizzle, std::string_view name)
    {
        if (!TextureUtils::TextureParametersValid(width, height, depth, static_cast<uint32_t>(mipLevelData.size())) || !TextureUtils::MipDataValid(width, height, depth, mipLevelData, format))
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::createTexture: invalid parameters");
            return {};
        }

        if (generateMipChain && (!FormatSupportsMipChainGeneration(format) || (mipLevelData.size() > 1)))
        {
            LOG_WARN(CONTEXT_CLIENT, "RamsesClient::createTexture: cannot auto generate mipmaps when custom mipmap data provided or unsupported format used");
            generateMipChain = false;
        }

        ramses::internal::TextureMetaInfo texDesc;
        texDesc.m_width = width;
        texDesc.m_height = height;
        texDesc.m_depth = depth;
        texDesc.m_format = TextureUtils::GetTextureFormatInternal(format);
        texDesc.m_generateMipChain = generateMipChain;
        texDesc.m_swizzle = TextureUtils::GetTextureSwizzleInternal(swizzle);
        TextureUtils::FillMipDataSizes(texDesc.m_dataSizes, mipLevelData);

        auto* resource = new ramses::internal::TextureResource(textureType, texDesc, name);
        TextureUtils::FillMipData(const_cast<std::byte*>(resource->getResourceData().data()), mipLevelData);

        return manageResource(resource);
    }
    template ramses::internal::ManagedResource RamsesClientImpl::createManagedTexture(ramses::internal::EResourceType textureType,
                                                                                     uint32_t width, uint32_t height, uint32_t depth,
                                                                                     ETextureFormat format,
                                                                                     const std::vector<MipLevelData>& mipLevelData, bool generateMipChain,
                                                                                     const TextureSwizzle& swizzle, std::string_view name);
    template ramses::internal::ManagedResource RamsesClientImpl::createManagedTexture(ramses::internal::EResourceType textureType,
                                                                                     uint32_t width, uint32_t height, uint32_t depth,
                                                                                     ETextureFormat format,
                                                                                     const std::vector<CubeMipLevelData>& mipLevelData, bool generateMipChain,
                                                                                     const TextureSwizzle& swizzle, std::string_view name);

    ramses::internal::ManagedResource RamsesClientImpl::createManagedEffect(const EffectDescription& effectDesc, std::string_view name, std::string& errorMessages)
    {
        //create effect using vertex and fragment shaders
        ramses::internal::GlslEffect effectBlock(effectDesc.getVertexShader(), effectDesc.getFragmentShader(), effectDesc.getGeometryShader(), effectDesc.impl().getCompilerDefines(),
            effectDesc.impl().getSemanticsMap(), name);
        errorMessages.clear();
        ramses::internal::EffectResource* effectResource = effectBlock.createEffectResource();
        if (!effectResource)
        {
            errorMessages = effectBlock.getEffectErrorMessages();
            LOG_ERROR(CONTEXT_CLIENT, "RamsesClient::createEffect  Failed to create effect resource (name: '{}') :\n    {}", name, effectBlock.getEffectErrorMessages());
            return {};
        }
        return manageResource(effectResource);
    }
}
