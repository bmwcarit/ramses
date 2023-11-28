//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <deque>

// client api
#include "ramses/framework/TextureEnums.h"
#include "ramses/client/MipLevelData.h"
#include "ramses/client/IClientEventHandler.h"
#include "ramses/client/TextureSwizzle.h"
#include "ramses/client/Scene.h"

// RAMSES framework
#include "ramses/framework/EFeatureLevel.h"
#include "internal/Core/Utils/LogContext.h"
#include "impl/SceneFactory.h"
#include "internal/ClientApplicationLogic.h"
#include "impl/RamsesObjectImpl.h"
#include "impl/SceneObjectRegistry.h"
#include "ResourceObjects.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "impl/RamsesObjectVector.h"
#include "internal/ClientCommands/ValidateCommand.h"
#include "internal/ClientCommands/DumpSceneToFile.h"
#include "internal/ClientCommands/LogResourceMemoryUsage.h"
#include "internal/PlatformAbstraction/PlatformLock.h"
#include "internal/Core/TaskFramework/ITask.h"
#include "internal/Core/TaskFramework/EnqueueOnlyOneAtATimeQueue.h"
#include "internal/Core/TaskFramework/TaskForwardingQueue.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "impl/SceneImpl.h"
#include "impl/SceneConfigImpl.h"

#include <memory>
#include <string_view>

namespace ramses
{
    class Effect;
    class Texture3D;
    class Texture2D;
    class ArrayResource;
    class Texture;
    class TextureCube;
    class EffectDescription;
    class RamsesClient;
}

namespace ramses::internal
{
    class IInputStream;
    class PrintSceneList;
    class FlushSceneVersion;
    class BinaryFileOutputStream;
    class BinaryFileInputStream;
    class ClientScene;
    class ArrayResourceImpl;
    class Texture2DImpl;
    class ClientObjectImpl;
    class SceneImpl;
    class SceneConfigImpl;
    class ResourceImpl;
    class RamsesFrameworkImpl;
    class SceneConfigImpl;

    using SceneOwningPtr = std::unique_ptr<Scene, std::function<void(Scene*)>>;
    using SceneVector = std::vector<SceneOwningPtr>;
    using InternalSceneOwningPtr = std::unique_ptr<ramses::internal::ClientScene>;
    using ResourceVector = std::vector<Resource *>; // resources are owned by Scene's object registry

    class RamsesClientImpl final : public RamsesObjectImpl
    {
    public:
        RamsesClientImpl(RamsesFrameworkImpl& ramsesFramework, std::string_view applicationName);
        ~RamsesClientImpl() override;

        void setHLObject(RamsesClient* hlClient);

        void deinitializeFrameworkData() override;

        virtual ramses::internal::ManagedResource getResource(ramses::internal::ResourceContentHash hash) const;
        template <typename T>
        const T* getResourceData(const ramses::internal::ResourceContentHash& hash) const
        {
            ramses::internal::ManagedResource managedResource = getResource(hash);
            const ramses::internal::IResource* untypedResource = managedResource.get();
            return untypedResource->convertTo<T>();
        }
        const ramses::internal::ClientApplicationLogic& getClientApplication() const;
        ramses::internal::ClientApplicationLogic& getClientApplication();

        Scene* createScene(const SceneConfigImpl& sceneConfig, std::string_view name);
        bool destroy(ramses::Scene& scene);

        Scene* loadSceneFromFile(std::string_view fileName, const SceneConfigImpl& config);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        Scene* loadSceneFromMemory(std::unique_ptr<std::byte[], void (*)(const std::byte*)> data, size_t size, const SceneConfigImpl& config);
        Scene* loadSceneFromFileDescriptor(int fd, size_t offset, size_t length, const SceneConfigImpl& config);

        bool loadSceneFromFileAsync(std::string_view fileName, const SceneConfigImpl& config);

        bool dispatchEvents(IClientEventHandler& clientEventHandler);

        const SceneVector& getListOfScenes() const;
        const Scene* findSceneByName(std::string_view name) const;
        Scene* findSceneByName(std::string_view name);
        const Scene* getScene(sceneId_t sceneId) const;
        Scene* getScene(sceneId_t sceneId);

        const RamsesFrameworkImpl& getFramework() const;
        RamsesFrameworkImpl& getFramework();
        static RamsesClientImpl& createImpl(std::string_view name, RamsesFrameworkImpl& components);

        void onValidate(ValidationReportImpl& report) const override;

        template <typename T>
        void enqueueSceneCommand(sceneId_t sceneId, T cmd);

        // special wrappers for known thread safe function
        ramses::internal::ResourceHashUsage getHashUsage_ThreadSafe(const ramses::internal::ResourceContentHash& hash) const;
        ramses::internal::ManagedResource getResource_ThreadSafe(ramses::internal::ResourceContentHash hash) const;
        ramses::internal::ManagedResource loadResource_ThreadSafe(const ramses::internal::ResourceContentHash& hash) const;

        ramses::SceneReference* findSceneReference(sceneId_t masterSceneId, sceneId_t referencedSceneId);

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        ramses::internal::ManagedResource createManagedArrayResource(size_t numElements, ramses::EDataType type, const void* arrayData, std::string_view name);
        template <typename MipDataStorageType>
        ramses::internal::ManagedResource createManagedTexture(ramses::internal::EResourceType textureType, uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, const std::vector<MipDataStorageType>& mipLevelData, bool generateMipChain, const TextureSwizzle& swizzle, std::string_view name);
        ramses::internal::ManagedResource createManagedEffect(const EffectDescription& effectDesc, std::string_view name, std::string& errorMessages);

        void writeLowLevelResourcesToStream(const ResourceObjects& resources, ramses::internal::IOutputStream& resourceOutputStream, bool compress) const;
        static bool ReadRamsesVersionAndPrintWarningOnMismatch(ramses::internal::IInputStream& inputStream, std::string_view verboseFileName, EFeatureLevel featureLevel);
        static void WriteCurrentBuildVersionToStream(ramses::internal::IOutputStream& stream, EFeatureLevel featureLevel);
        static bool GetFeatureLevelFromFile(std::string_view fileName, EFeatureLevel& detectedFeatureLevel);
        static bool GetFeatureLevelFromFile(int fd, size_t offset, size_t length, EFeatureLevel& detectedFeatureLevel);

    private:
        struct SceneCreationConfig
        {
            std::string caller;
            std::string dataSource;
            ramses::internal::InputStreamContainerSPtr streamContainer;
            bool prefetchData;
            SceneConfigImpl config;
        };

        class LoadSceneRunnable : public ramses::internal::ITask
        {
        public:
            LoadSceneRunnable(RamsesClientImpl& client, SceneCreationConfig&& cconfig);
            void execute() override;

        private:
            RamsesClientImpl& m_client;
            SceneCreationConfig m_cconfig;
        };

        class DeleteSceneRunnable : public ramses::internal::ITask
        {
        public:
            DeleteSceneRunnable(SceneOwningPtr scene, InternalSceneOwningPtr llscene);
            void execute() override;

        private:
            SceneOwningPtr m_scene;
            InternalSceneOwningPtr m_lowLevelScene;
        };

        struct SceneLoadStatus
        {
            SceneOwningPtr scene;
            std::string sceneFilename;
        };

        friend class LoadSceneRunnable;

        ramses::internal::ManagedResource manageResource(const ramses::internal::IResource* res);

        Scene* loadSceneSynchonousCommon(const SceneCreationConfig& cconf);
        SceneOwningPtr loadSceneFromCreationConfig(const SceneCreationConfig& cconf);
        SceneOwningPtr loadSceneObjectFromStream(const std::string& caller,
                                         std::string const& filename,
                                         ramses::internal::IInputStream& inputStream, const SceneConfigImpl& config);
        void finalizeLoadedScene(SceneOwningPtr scene);

        void validateScenes(ValidationReportImpl& report) const;

        static bool GetFeatureLevelFromStream(ramses::internal::IInputStreamContainer& streamContainer, const std::string& desc, EFeatureLevel& detectedFeatureLevel);

        RamsesClient* m_hlClient = nullptr;
        ramses::internal::ClientApplicationLogic m_appLogic;
        ramses::internal::SceneFactory m_sceneFactory;

        SceneVector m_scenes;

        std::shared_ptr<ramses::internal::PrintSceneList> m_cmdPrintSceneList;
        std::shared_ptr<ramses::internal::ValidateCommand> m_cmdPrintValidation;
        std::shared_ptr<ramses::internal::FlushSceneVersion> m_cmdFlushSceneVersion;
        std::shared_ptr<ramses::internal::DumpSceneToFile> m_cmdDumpSceneToFile;
        std::shared_ptr<ramses::internal::LogResourceMemoryUsage> m_cmdLogResourceMemoryUsage;

        RamsesFrameworkImpl& m_framework;
        mutable ramses::internal::PlatformLock m_clientLock;

        ramses::internal::TaskForwardingQueue m_loadFromFileTaskQueue;
        ramses::internal::EnqueueOnlyOneAtATimeQueue m_deleteSceneQueue;

        std::vector<SceneLoadStatus> m_asyncSceneLoadStatusVec;
    };

    template <typename T>
    void RamsesClientImpl::enqueueSceneCommand(sceneId_t sceneId, T cmd)
    {
        ramses::internal::PlatformGuard guard(m_clientLock);
        auto it = std::find_if(m_scenes.begin(), m_scenes.end(), [&](const auto& scene) { return scene->getSceneId() == sceneId; });
        if (it != m_scenes.end())
            (*it)->m_impl.enqueueSceneCommand(std::move(cmd));
    }
}
