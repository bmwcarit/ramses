//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESCLIENTIMPL_H
#define RAMSES_RAMSESCLIENTIMPL_H

#include <deque>

// client api
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-client-api/IClientEventHandler.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ResourceDataPool.h"

// RAMSES framework
#include "Utils/LogContext.h"
#include "SceneFactory.h"
#include "ClientApplicationLogic.h"
#include "RamsesObjectImpl.h"
#include "RamsesObjectRegistry.h"
#include "ResourceObjects.h"
#include "Collections/Vector.h"
#include "RamsesObjectVector.h"
#include "ClientCommands/ValidateCommand.h"
#include "ClientCommands/ForceFallbackImage.h"
#include "ClientCommands/DumpSceneToFile.h"
#include "ClientCommands/LogResourceMemoryUsage.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "TaskFramework/ITask.h"
#include "TaskFramework/EnqueueOnlyOneAtATimeQueue.h"
#include "TaskFramework/TaskForwardingQueue.h"
#include "Collections/HashMap.h"
#include "RamsesFrameworkTypesImpl.h"
#include "SceneImpl.h"

#include <memory>

namespace ramses_internal
{
    class IInputStream;
    class PrintSceneList;
    class ForceFallbackImage;
    class FlushSceneVersion;
    class BinaryFileOutputStream;
    class BinaryFileInputStream;
    class ClientScene;
}

namespace ramses
{
    class Scene;
    class Effect;
    class Texture3D;
    class Texture2D;
    class ArrayResource;
    class Texture;
    class TextureCube;
    class EffectDescription;
    class ArrayResourceImpl;
    class Texture2DImpl;
    class ResourceFileDescription;
    class ResourceFileDescriptionSet;
    class RamsesFrameworkImpl;
    class ClientObjectImpl;
    class SceneImpl;
    class SceneConfigImpl;
    class ResourceImpl;
    class RamsesClient;

    using SceneVector = std::vector<Scene *>;
    using ResourceVector = std::vector<Resource *>;

    class RamsesClientImpl final : public RamsesObjectImpl
    {
    public:
        virtual ~RamsesClientImpl() override;

        void setHLObject(RamsesClient* hlClient);

        virtual void deinitializeFrameworkData() override final;

        virtual ramses_internal::ManagedResource getResource(ramses_internal::ResourceContentHash hash) const;
        template <typename T>
        const T* getResourceData(const ramses_internal::ResourceContentHash& hash) const
        {
            ramses_internal::ManagedResource managedResource = getResource(hash);
            const ramses_internal::IResource* untypedResource = managedResource.get();
            return untypedResource->convertTo<T>();
        }
        const ramses_internal::ClientApplicationLogic& getClientApplication() const;
        ramses_internal::ClientApplicationLogic& getClientApplication();

        Scene* createScene(sceneId_t sceneId, const SceneConfigImpl& sceneConfig, const char* name);
        status_t destroy(Scene& scene);

        Scene* loadSceneFromFile(const char* fileName, bool localOnly);
        Scene* loadSceneFromMemory(std::unique_ptr<unsigned char[], void(*)(const unsigned char*)> data, size_t size, bool localOnly);
        Scene* loadSceneFromFileDescriptor(int fd, size_t offset, size_t length, bool localOnly);
        Scene* loadSceneFromFileDescriptor(sceneId_t sceneId, int fd, size_t offset, size_t length, bool localOnly);

        status_t loadSceneFromFileAsync(const char* fileName, bool localOnly);

        status_t dispatchEvents(IClientEventHandler& clientEventHandler);

        SceneVector getListOfScenes() const;
        const Scene* findSceneByName(const char* name) const;
        Scene* findSceneByName(const char* name);
        const Scene* getScene(sceneId_t sceneId) const;
        Scene* getScene(sceneId_t sceneId);

        RamsesFrameworkImpl& getFramework();
        static RamsesClientImpl& createImpl(const char* name, RamsesFrameworkImpl& components);

        virtual status_t validate() const override;

        template <typename T>
        void enqueueSceneCommand(sceneId_t sceneId, T cmd);

        // special wrappers for known thread safe function
        ramses_internal::ResourceHashUsage getHashUsage_ThreadSafe(const ramses_internal::ResourceContentHash& hash) const;
        ramses_internal::ManagedResource getResource_ThreadSafe(ramses_internal::ResourceContentHash hash) const;
        ramses_internal::ManagedResource loadResource_ThreadSafe(const ramses_internal::ResourceContentHash& hash) const;

        SceneReference* findSceneReference(sceneId_t masterSceneId, sceneId_t referencedSceneId);

        ramses_internal::ManagedResource createManagedArrayResource(uint32_t numElements, EDataType type, const void* arrayData, resourceCacheFlag_t cacheFlag, const char* name);
        template <typename MipDataStorageType>
        ramses_internal::ManagedResource createManagedTexture(ramses_internal::EResourceType textureType, uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipDataStorageType mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name);
        ramses_internal::ManagedResource createManagedEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, const char* name, std::string& errorMessages);

        void writeLowLevelResourcesToStream(const ResourceObjects& resources, ramses_internal::BinaryFileOutputStream& resourceOutputStream, bool compress) const;
        static bool ReadRamsesVersionAndPrintWarningOnMismatch(ramses_internal::IInputStream& inputStream, const ramses_internal::String& verboseFileName);
        static void WriteCurrentBuildVersionToStream(ramses_internal::IOutputStream& stream);

        ResourceDataPool& getResourceDataPool();
        ResourceDataPool const& getResourceDataPool() const;

    private:
        friend class ClientFactory;
        RamsesClientImpl(RamsesFrameworkImpl& ramsesFramework, const char* applicationName);

        struct SceneCreationConfig
        {
            std::string caller;
            std::string dataSource;
            ramses_internal::InputStreamContainerSPtr streamContainer;
            bool prefetchData;
            bool localOnly;
            sceneId_t sceneId;
        };

        class LoadSceneRunnable : public ramses_internal::ITask
        {
        public:
            LoadSceneRunnable(RamsesClientImpl& client, SceneCreationConfig&& cconfig);
            virtual void execute() override;

        private:
            RamsesClientImpl& m_client;
            SceneCreationConfig m_cconfig;
        };

        class DeleteSceneRunnable : public ramses_internal::ITask
        {
        public:
            DeleteSceneRunnable(Scene* scene, ramses_internal::ClientScene* llscene);
            virtual void execute() override;

        private:
            Scene* m_scene;
            ramses_internal::ClientScene* m_lowLevelScene;
        };

        struct SceneLoadStatus
        {
            Scene* scene;
            std::string sceneFilename;
        };

        friend class LoadSceneRunnable;

        ramses_internal::ManagedResource manageResource(const ramses_internal::IResource* res);

        Scene* loadSceneSynchonousCommon(const SceneCreationConfig& cconf);
        Scene* loadSceneFromCreationConfig(const SceneCreationConfig& cconf);
        Scene* loadSceneObjectFromStream(const std::string& caller,
                                         std::string const& filename,
                                         ramses_internal::IInputStream& inputStream,
                                         bool localOnly,
                                         sceneId_t sceneId);
        void finalizeLoadedScene(Scene* scene);

        status_t validateScenes() const;

        RamsesClient* m_hlClient = nullptr;
        ramses_internal::ClientApplicationLogic m_appLogic;
        ramses_internal::SceneFactory     m_sceneFactory;

        SceneVector          m_scenes;

        std::shared_ptr<ramses_internal::PrintSceneList> m_cmdPrintSceneList;
        std::shared_ptr<ramses_internal::ValidateCommand> m_cmdPrintValidation;
        std::shared_ptr<ramses_internal::ForceFallbackImage> m_cmdForceFallbackImage;
        std::shared_ptr<ramses_internal::FlushSceneVersion> m_cmdFlushSceneVersion;
        std::shared_ptr<ramses_internal::DumpSceneToFile> m_cmdDumpSceneToFile;
        std::shared_ptr<ramses_internal::LogResourceMemoryUsage> m_cmdLogResourceMemoryUsage;

        RamsesFrameworkImpl& m_framework;
        mutable ramses_internal::PlatformLock m_clientLock;

        ramses_internal::TaskForwardingQueue m_loadFromFileTaskQueue;
        ramses_internal::EnqueueOnlyOneAtATimeQueue m_deleteSceneQueue;

        std::vector<SceneLoadStatus> m_asyncSceneLoadStatusVec;

        ResourceDataPool m_resourceDataPool;
    };

    template <typename T>
    void RamsesClientImpl::enqueueSceneCommand(sceneId_t sceneId, T cmd)
    {
        ramses_internal::PlatformGuard guard(m_clientLock);
        auto it = std::find_if(m_scenes.begin(), m_scenes.end(), [&](Scene* scene) { return scene->impl.getSceneId() == sceneId; });
        if (it != m_scenes.end())
            (*it)->impl.enqueueSceneCommand(std::move(cmd));
    }
}

#endif
