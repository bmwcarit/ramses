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

// RAMSES framework
#include "Utils/LogContext.h"
#include "SceneFactory.h"
#include "ClientApplicationLogic.h"
#include "RamsesObjectImpl.h"
#include "RamsesObjectRegistry.h"
#include "ResourceObjects.h"
#include "Collections/Vector.h"
#include "RamsesObjectVector.h"
#include "ClientCommands/SceneCommandTypes.h"
#include "ClientCommands/ValidateCommand.h"
#include "ClientCommands/ForceFallbackImage.h"
#include "ClientCommands/DumpSceneToFile.h"
#include "ClientCommands/LogResourceMemoryUsage.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "Utils/ScopedPointer.h"
#include "TaskFramework/ITask.h"
#include "TaskFramework/EnqueueOnlyOneAtATimeQueue.h"
#include "TaskFramework/TaskForwardingQueue.h"
#include "Collections/HashMap.h"
#include "city.h"
#include "RamsesFrameworkTypesImpl.h"
#include <chrono>


namespace ramses_internal
{
    class EffectResource;
    class TextureResource;
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
    class FloatArray;
    class Vector2fArray;
    class Vector3fArray;
    class Vector4fArray;
    class Texture3D;
    class Texture2D;
    class UInt16Array;
    class UInt32Array;
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

    typedef std::vector<Scene*> SceneVector;
    typedef std::vector<Resource*> ResourceVector;

    class RamsesClientImpl final : public RamsesObjectImpl
    {
    public:
        RamsesClientImpl(RamsesFrameworkImpl& ramsesFramework, const char* applicationName);
        virtual ~RamsesClientImpl();

        virtual void deinitializeFrameworkData() override final;

        virtual ramses_internal::ManagedResource getResource(ramses_internal::ResourceContentHash hash) const;
        template <typename T>
        const T* getResourceData(const ramses_internal::ResourceContentHash& hash) const
        {
            ramses_internal::ManagedResource managedResource = getResource(hash);
            const ramses_internal::IResource* untypedResource = managedResource.getResourceObject();
            return untypedResource->convertTo<T>();
        }
        Resource* getHLResource_Threadsafe(resourceId_t rid) const;
        const ramses_internal::ClientApplicationLogic& getClientApplication() const;
        ramses_internal::ClientApplicationLogic& getClientApplication();

        Scene* createScene(sceneId_t sceneId, const SceneConfigImpl& sceneConfig, const char* name);
        status_t destroy(Scene& scene);

        const FloatArray* createConstFloatArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name);
        const Vector2fArray* createConstVector2fArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name);
        const Vector3fArray* createConstVector3fArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name);
        const Vector4fArray* createConstVector4fArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name);
        const UInt16Array* createConstUInt16Array(uint32_t count, const uint16_t *arrayData, resourceCacheFlag_t cacheFlag, const char* name);
        const UInt32Array* createConstUInt32Array(uint32_t count, const uint32_t *arrayData, resourceCacheFlag_t cacheFlag, const char* name);
        Texture2D* createTexture2D(uint32_t width, uint32_t height, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name);
        Texture3D* createTexture3D(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name);
        TextureCube* createTextureCube(uint32_t, ETextureFormat format, resourceCacheFlag_t cacheFlag, const char* name, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain);
        status_t destroy(const Resource& resource);

        status_t saveSceneToFile(SceneImpl& scene, const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation, bool compress) const;
        Scene* loadSceneFromFile(const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation);

        status_t saveResources(const ResourceFileDescription& fileDescription, bool compress) const;
        status_t saveResources(const ResourceFileDescriptionSet& resourceFileInformation, bool compress) const;
        status_t loadResources(const ResourceFileDescription& fileDescription);
        status_t loadResources(const ResourceFileDescriptionSet& resourceFileInformation);

        status_t loadResourcesAsync(const ResourceFileDescription& fileDescription);
        status_t loadResourcesAsync(const ResourceFileDescriptionSet& resourceFileInformation);
        status_t loadSceneFromFileAsync(const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation);
        status_t dispatchEvents(IClientEventHandler& clientEventHandler);

        status_t markSceneIdForLoadingAsLocalOnly(sceneId_t sceneId);

        status_t closeResourceFile(const ResourceFileDescription& fileDescription);
        status_t closeResourceFiles(const ResourceFileDescriptionSet& fileDescriptions);

        RamsesObjectVector getListOfResourceObjects(ERamsesObjectType objType = ERamsesObjectType_Resource) const;
        SceneVector getListOfScenes() const;
        const RamsesObject* findObjectByName(const char* name) const;
        RamsesObject* findObjectByName(const char* name);
        const Resource* scanForResourceWithHash(ramses_internal::ResourceContentHash hash) const;

        Effect* createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, const char* name);
        Effect* createEffectFromResource(const ramses_internal::EffectResource* res, const ramses_internal::String& name);

        RamsesFrameworkImpl& getFramework();
        static RamsesClientImpl& createImpl(const char* name, RamsesFrameworkImpl& components);

        virtual status_t validate(uint32_t indent) const override;

        void enqueueSceneCommand(sceneId_t sceneId, const ramses_internal::SceneCommand& command);

        // special wrappers for known thread safe function
        ramses_internal::ResourceHashUsage getHashUsage_ThreadSafe(const ramses_internal::ResourceContentHash& hash) const;
        ramses_internal::ManagedResource getResource_ThreadSafe(ramses_internal::ResourceContentHash hash) const;
        ramses_internal::ManagedResource forceLoadResource_ThreadSafe(const ramses_internal::ResourceContentHash& hash) const;

        void setClientResourceCacheTimeout(std::chrono::milliseconds timeout);
        void updateClientResourceCache();

    private:
        ArrayResourceImpl& createArrayResourceImpl(uint32_t count, const ramses_internal::Byte* arrayData, resourceCacheFlag_t cacheFlag, const char* name, ramses_internal::EDataType elementType, ERamsesObjectType objectType, ramses_internal::EResourceType resourceType);

        class LoadResourcesRunnable : public ramses_internal::ITask
        {
        public:
            LoadResourcesRunnable(RamsesClientImpl& client, const std::vector<ramses_internal::String>& filenames);
            virtual void execute() override;

        private:
            RamsesClientImpl& m_client;
            std::vector<ramses_internal::String> m_filenames;
        };

        class LoadSceneRunnable : public ramses_internal::ITask
        {
        public:
            LoadSceneRunnable(RamsesClientImpl& client, const ramses_internal::String& sceneFilename, const std::vector<ramses_internal::String>& resourceFilenames);
            virtual void execute() override;

        private:
            RamsesClientImpl& m_client;
            ramses_internal::String m_sceneFilename;
            std::vector<ramses_internal::String> m_resourceFilenames;
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

        struct ResourceLoadStatus
        {
            bool successful;
            ramses_internal::String filename;
        };

        struct SceneLoadStatus
        {
            Scene* scene;
            ramses_internal::String sceneFilename;
        };

        friend class LoadResourcesRunnable;
        friend class LoadSceneRunnable;

        status_t writeResourcesToFile(const ResourceFileDescription& fileDescription, bool compress) const;
        status_t closeResourceFile(const ramses_internal::String& fileName);

        static void WriteCurrentBuildVersionToStream(ramses_internal::IOutputStream& stream);
        static bool ReadRamsesVersionAndPrintWarningOnMismatch(ramses_internal::BinaryFileInputStream& inputStream, const ramses_internal::String& verboseFileName);

        status_t writeHLResourcesToStream(ramses_internal::IOutputStream& resourceOutputStream, const ResourceObjects& resources) const;
        void writeLowLevelResourcesToStream(const ResourceObjects& resources, ramses_internal::BinaryFileOutputStream& resourceOutputStream, bool compress) const;

        status_t writeSceneObjectsToStream(SceneImpl& scene, ramses_internal::IOutputStream& outputStream) const;

        Scene* prepareSceneFromInputStream(const char* caller, const ramses_internal::String& filename, ramses_internal::IInputStream& inputStream);
        template <typename ObjectType, typename ObjectImplType>
        status_t createAndDeserializeResourceImpls(ramses_internal::IInputStream& inStream, DeserializationContext& deserializationContext, uint32_t count, ResourceVector& container);
        status_t readResourcesFromFile(const ramses_internal::String& resourceFilename);
        Scene* prepareSceneAndResourcesFromFiles(const char* caller, const ramses_internal::String& sceneFilename,
            const std::vector<ramses_internal::String>& resourceFilenames, std::vector<ResourceLoadStatus>& resourceloadStatus);
        void finalizeLoadedScene(Scene* scene);

        status_t validateScenes(uint32_t indent) const;
        status_t validateResources(uint32_t indent) const;
        void writeResourcesInfoToValidationMessage(uint32_t indent) const;

        template <typename MipDataStorageType>
        const ramses_internal::TextureResource* createTextureResource(ramses_internal::EResourceType textureType, uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipDataStorageType mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name) const;

        ramses_internal::ManagedResource manageResource(const ramses_internal::IResource* res);

        status_t destroyResource(Resource* resource);
        void addResourceObjectToRegistry_ThreadSafe(Resource& object);

        bool validateArray(uint32_t count, const void* arrayData) const;

        ramses_internal::ClientApplicationLogic m_appLogic;
        ramses_internal::SceneFactory     m_sceneFactory;

        RamsesObjectRegistry m_resources;
        typedef ramses_internal::HashMap<resourceId_t, Resource*> HLResourceHashMap;
        HLResourceHashMap    m_resourcesById;
        SceneVector          m_scenes;

        ramses_internal::ScopedPointer<ramses_internal::PrintSceneList> m_cmdPrintSceneList;
        ramses_internal::ScopedPointer<ramses_internal::ValidateCommand> m_cmdPrintValidation;
        ramses_internal::ScopedPointer<ramses_internal::ForceFallbackImage> m_cmdForceFallbackImage;
        ramses_internal::ScopedPointer<ramses_internal::FlushSceneVersion> m_cmdFlushSceneVersion;
        ramses_internal::ScopedPointer<ramses_internal::DumpSceneToFile> m_cmdDumpSceneToFile;
        ramses_internal::ScopedPointer<ramses_internal::LogResourceMemoryUsage> m_cmdLogResourceMemoryUsage;

        RamsesFrameworkImpl& m_framework;
        mutable ramses_internal::PlatformLock m_clientLock;

        ramses_internal::TaskForwardingQueue m_loadFromFileTaskQueue;
        ramses_internal::EnqueueOnlyOneAtATimeQueue m_deleteSceneQueue;

        std::vector<ResourceLoadStatus> m_asyncResourceLoadStatusVec;
        std::vector<SceneLoadStatus> m_asyncSceneLoadStatusVec;
        ramses_internal::HashSet<ramses_internal::SceneId> m_scenesMarkedForLoadAsLocalOnly;

        std::chrono::milliseconds m_clientResourceCacheTimeout;
        using ClientResourceCache = std::deque<std::pair<std::chrono::time_point<std::chrono::steady_clock>, ramses_internal::ResourceHashUsage>>;
        ClientResourceCache m_clientResourceCache;
    };
}

#endif
