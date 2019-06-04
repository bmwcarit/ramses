//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "glslEffectBlock/GlslEffect.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/Resource.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-sdk-build-config.h"

// internal
#include "SceneImpl.h"
#include "SceneConfigImpl.h"
#include "ArrayResourceImpl.h"
#include "ResourceImpl.h"
#include "AnimationSystemImpl.h"
#include "AnimatedPropertyImpl.h"
#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "TextureCubeImpl.h"
#include "EffectImpl.h"
#include "EffectDescriptionImpl.h"
#include "TextureUtils.h"
#include "RamsesFrameworkImpl.h"
#include "RamsesClientImpl.h"
#include "ResourceIteratorImpl.h"
#include "ResourceFileDescriptionImpl.h"
#include "ResourceFileDescriptionSetImpl.h"
#include "RamsesObjectRegistryIterator.h"
#include "SerializationHelper.h"
#include "RamsesVersion.h"

// framework
#include "SceneAPI/SceneCreationInformation.h"
#include "Resource/TextureResource.h"
#include "Scene/ScenePersistation.h"
#include "Scene/ClientScene.h"
#include "Components/ResourcePersistation.h"
#include "Components/ManagedResource.h"
#include "Components/ResourceTableOfContents.h"
#include "Animation/AnimationSystemFactory.h"
#include "Resource/ArrayResource.h"
#include "Resource/EffectResource.h"
#include "Resource/IResource.h"
#include "ClientCommands/PrintSceneList.h"
#include "ClientCommands/ForceFallbackImage.h"
#include "ClientCommands/FlushSceneVersion.h"
#include "SerializationContext.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "Utils/LogContext.h"
#include "Utils/File.h"
#include "Collections/IInputStream.h"
#include "Collections/String.h"
#include "Collections/HashMap.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"

#include "PlatformAbstraction/PlatformTypes.h"
#include <array>

namespace ramses
{
    RamsesClientImpl::RamsesClientImpl(RamsesFrameworkImpl& framework,  const char* applicationName)
        : RamsesObjectImpl(ERamsesObjectType_Client, applicationName)
        , m_appLogic(framework.getParticipantAddress().getParticipantId(), framework.getFrameworkLock())
        , m_sceneFactory()
        , m_framework(framework)
        , m_loadFromFileTaskQueue(framework.getTaskQueue())
        , m_deleteSceneQueue(framework.getTaskQueue())
        , m_clientResourceCacheTimeout(5000)
    {
        if (framework.isConnected())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::RamsesClient creating a RamsesClient with RamsesFramework which is already connected - this may lead to further issues! Please first create RamsesClient, then call connect()");
        }

        m_appLogic.init(framework.getResourceComponent(), framework.getScenegraphComponent());
        m_cmdPrintSceneList.reset(new ramses_internal::PrintSceneList(*this));
        m_cmdPrintValidation.reset(new ramses_internal::ValidateCommand(*this));
        m_cmdForceFallbackImage.reset(new ramses_internal::ForceFallbackImage(*this));
        m_cmdFlushSceneVersion.reset(new ramses_internal::FlushSceneVersion(*this));
        m_cmdDumpSceneToFile.reset(new ramses_internal::DumpSceneToFile(*this));
        m_cmdLogResourceMemoryUsage.reset(new ramses_internal::LogResourceMemoryUsage(*this));
        framework.getRamsh().add(*m_cmdPrintSceneList);
        framework.getRamsh().add(*m_cmdPrintValidation);
        framework.getRamsh().add(*m_cmdForceFallbackImage);
        framework.getRamsh().add(*m_cmdFlushSceneVersion);
        framework.getRamsh().add(*m_cmdDumpSceneToFile);
        framework.getRamsh().add(*m_cmdLogResourceMemoryUsage);
        m_framework.getPeriodicLogger().registerPeriodicLogSupplier(&m_framework.getScenegraphComponent());
    }

    RamsesClientImpl::~RamsesClientImpl()
    {
        m_deleteSceneQueue.disableAcceptingTasksAfterExecutingCurrentQueue();
        m_loadFromFileTaskQueue.disableAcceptingTasksAfterExecutingCurrentQueue();

        // delete async loaded  scenes that were never collected via calling dispatchEvents
        ramses_internal::PlatformGuard g(m_clientLock);
        for (auto& loadStatus : m_asyncSceneLoadStatusVec)
        {
            delete loadStatus.scene;
        }

        for (auto& scene : m_scenes)
        {
            delete scene;
        }

        RamsesObjectVector resources;
        m_resources.getObjectsOfType(resources, ERamsesObjectType_Resource);
        for (const auto it : resources)
        {
            delete &RamsesObjectTypeUtils::ConvertTo<Resource>(*it);
        }
        m_framework.getPeriodicLogger().removePeriodicLogSupplier(&m_framework.getScenegraphComponent());
    }

    void RamsesClientImpl::deinitializeFrameworkData()
    {
    }

    const ramses_internal::ClientApplicationLogic& RamsesClientImpl::getClientApplication() const
    {
        return m_appLogic;
    }

    ramses_internal::ClientApplicationLogic& RamsesClientImpl::getClientApplication()
    {
        return m_appLogic;
    }

    Scene* RamsesClientImpl::createScene(sceneId_t sceneId, const SceneConfigImpl& sceneConfig, const char* name)
    {
        ramses_internal::PlatformGuard g(m_clientLock);
        ramses_internal::SceneInfo sceneInfo;
        sceneInfo.friendlyName = name;
        sceneInfo.sceneID = ramses_internal::SceneId(sceneId);

        ramses_internal::ClientScene* internalScene = m_sceneFactory.createScene(sceneInfo);
        if (NULL == internalScene)
        {
            return NULL;
        }

        SceneImpl& pimpl = *new SceneImpl(*internalScene, sceneConfig, *this);
        pimpl.initializeFrameworkData();
        Scene* scene = new Scene(pimpl);
        m_scenes.push_back(scene);

        return scene;
    }

    RamsesClientImpl::DeleteSceneRunnable::DeleteSceneRunnable(Scene* scene, ramses_internal::ClientScene* llscene)
        : m_scene(scene)
        , m_lowLevelScene(llscene)
    {
    }

    void RamsesClientImpl::DeleteSceneRunnable::execute()
    {
        delete m_scene;
        delete m_lowLevelScene;
    }

    status_t RamsesClientImpl::destroy(Scene& scene)
    {
        ramses_internal::PlatformGuard g(m_clientLock);
        SceneVector::iterator iter = ramses_internal::find_c(m_scenes, &scene);
        if (iter != m_scenes.end())
        {
            m_scenes.erase(iter);

            const ramses_internal::SceneId sceneID(scene.impl.getSceneId());
            auto llscene = m_sceneFactory.releaseScene(sceneID);

            getClientApplication().removeScene(sceneID);

            auto task = new DeleteSceneRunnable(&scene, llscene);
            m_deleteSceneQueue.enqueue(*task);
            task->release();

            return StatusOK;
        }

        return addErrorEntry("RamsesClient::destroy failed, scene is not in this client.");
    }

    status_t RamsesClientImpl::destroy(const Resource& resource)
    {
        if (&resource.impl.getClientImpl() != this)
        {
            return addErrorEntry("RamsesClient::destroy failed, resource is not in this client.");
        }

        ramses_internal::PlatformGuard g(m_clientLock);
        Resource& resourceNonConst = const_cast<Resource&>(resource);
        return destroyResource(&resourceNonConst);
    }

    status_t RamsesClientImpl::destroyResource(Resource* resource)
    {
        if (m_clientResourceCacheTimeout > std::chrono::milliseconds{0u})
        {
            const auto timeNow = std::chrono::steady_clock::now();
            m_clientResourceCache.push_back(std::make_pair(timeNow, m_framework.getResourceComponent().getResourceHashUsage(resource->impl.getLowlevelResourceHash())));
        }

        const resourceId_t resId = resource->impl.getResourceId();
        m_resourcesById.remove(resId);

        m_framework.getStatisticCollection().statResourcesDestroyed.incCounter(1);
        m_resources.removeObject(*resource);
        delete resource;

        return StatusOK;
    }

    template <typename MipDataStorageType>
    const ramses_internal::TextureResource* RamsesClientImpl::createTextureResource(ramses_internal::EResourceType textureType, uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipDataStorageType mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name) const
    {
        if (!TextureUtils::TextureParametersValid(width, height, depth, mipMapCount) || !TextureUtils::MipDataValid(width, height, depth, mipMapCount, mipLevelData, format))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::createTexture: invalid parameters");
            return NULL;
        }

        if (generateMipChain && (!FormatSupportsMipChainGeneration(format) || (mipMapCount > 1)))
        {
            LOG_WARN(ramses_internal::CONTEXT_CLIENT, "RamsesClient::createTexture: cannot auto generate mipmaps when custom mipmap data provided or unsupported format used");
            generateMipChain = false;
        }

        ramses_internal::TextureMetaInfo texDesc;
        texDesc.m_width = width;
        texDesc.m_height = height;
        texDesc.m_depth = depth;
        texDesc.m_format = TextureUtils::GetTextureFormatInternal(format);
        texDesc.m_generateMipChain = generateMipChain;
        TextureUtils::FillMipDataSizes(texDesc.m_dataSizes, mipMapCount, mipLevelData);

        ramses_internal::TextureResource* resource = new ramses_internal::TextureResource(textureType, texDesc, ramses_internal::ResourceCacheFlag(cacheFlag.getValue()), name);
        TextureUtils::FillMipData(static_cast<uint8_t*>(const_cast<void*>(resource->getData())), mipMapCount, mipLevelData);

        return resource;
    }

    Texture2D* RamsesClientImpl::createTexture2D(uint32_t width, uint32_t height, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name)
    {
        LOG_TRACE(ramses_internal::CONTEXT_CLIENT, "RamsesClient::createTexture2D:");

        const ramses_internal::TextureResource* resource = createTextureResource(ramses_internal::EResourceType_Texture2D, width, height, 1u, format, mipMapCount, mipLevelData, generateMipChain, cacheFlag, name);
        if (resource != nullptr)
        {
            ramses_internal::ManagedResource res = manageResource(resource);
            ramses_internal::ResourceHashUsage hashUsage = m_appLogic.getHashUsage(res.getResourceObject()->getHash());
            Texture2DImpl& pimpl = *new Texture2DImpl(hashUsage, *this, name);
            pimpl.initializeFromFrameworkData(width, height, format);
            Texture2D* texture = new Texture2D(pimpl);

            addResourceObjectToRegistry_ThreadSafe(*texture);

            return texture;
        }

        return nullptr;
    }

    Texture3D* RamsesClientImpl::createTexture3D(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name)
    {
        LOG_TRACE(ramses_internal::CONTEXT_CLIENT, "RamsesClient::createTexture3D:");

        const ramses_internal::TextureResource* resource = createTextureResource(ramses_internal::EResourceType_Texture3D, width, height, depth, format, mipMapCount, mipLevelData, generateMipChain, cacheFlag, name);
        if (resource != nullptr)
        {
            ramses_internal::ManagedResource res = manageResource(resource);
            ramses_internal::ResourceHashUsage hashUsage = m_appLogic.getHashUsage(res.getResourceObject()->getHash());
            Texture3DImpl& pimpl = *new Texture3DImpl(hashUsage, *this, name);
            pimpl.initializeFromFrameworkData(width, height, depth, format);
            Texture3D* texture = new Texture3D(pimpl);

            addResourceObjectToRegistry_ThreadSafe(*texture);

            return texture;
        }

        return nullptr;
    }

    TextureCube* RamsesClientImpl::createTextureCube(uint32_t size, ETextureFormat format, resourceCacheFlag_t cacheFlag, const char* name, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain)
    {
        LOG_TRACE(ramses_internal::CONTEXT_CLIENT, "RamsesClient::createTextureCube:");

        const ramses_internal::TextureResource* resource = createTextureResource(ramses_internal::EResourceType_TextureCube, size, 1u, 1u, format, mipMapCount, mipLevelData, generateMipChain, cacheFlag, name);
        if (resource != nullptr)
        {
            ramses_internal::ManagedResource res = manageResource(resource);
            ramses_internal::ResourceHashUsage hashUsage = m_appLogic.getHashUsage(res.getResourceObject()->getHash());
            TextureCubeImpl& pimpl = *new TextureCubeImpl(hashUsage, *this, name);
            pimpl.initializeFromFrameworkData(size, format);
            TextureCube* texture = new TextureCube(pimpl);

            addResourceObjectToRegistry_ThreadSafe(*texture);

            return texture;
        }

        return nullptr;
    }

    ArrayResourceImpl& RamsesClientImpl::createArrayResourceImpl(uint32_t count, const ramses_internal::Byte* arrayData, resourceCacheFlag_t cacheFlag, const char* name, ramses_internal::EDataType elementType, ERamsesObjectType objectType, ramses_internal::EResourceType resourceType)
    {
        const auto* resource = new ramses_internal::ArrayResource(resourceType, count, elementType, arrayData, ramses_internal::ResourceCacheFlag(cacheFlag.getValue()), name);
        ramses_internal::ManagedResource res = manageResource(resource);
        ramses_internal::ResourceHashUsage usage = m_appLogic.getHashUsage(res.getResourceObject()->getHash());
        ArrayResourceImpl& pimpl = *new ArrayResourceImpl(usage, objectType, *this, name);
        pimpl.initializeFromFrameworkData(count, elementType);

        return pimpl;
    }

    bool RamsesClientImpl::validateArray(uint32_t count, const void* arrayData) const
    {
        if (0u == count || NULL == arrayData)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::validateArray Array resource must have element count > 0 and data must not be NULL!");
            return false;
        }
        return true;
    }

    const FloatArray* RamsesClientImpl::createConstFloatArray(uint32_t count, const float *arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        if (!validateArray(count, arrayData))
        {
            return NULL;
        }

        ArrayResourceImpl& pimpl = createArrayResourceImpl(count, reinterpret_cast<const ramses_internal::Byte*>(arrayData), cacheFlag, name, ramses_internal::EDataType_Float, ERamsesObjectType_FloatArray, ramses_internal::EResourceType_VertexArray);
        FloatArray* vertexArray = new FloatArray(pimpl);
        addResourceObjectToRegistry_ThreadSafe(*vertexArray);


        return vertexArray;
    }

    const Vector2fArray* RamsesClientImpl::createConstVector2fArray(uint32_t count, const float *arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        if (!validateArray(count, arrayData))
        {
            return NULL;
        }

        ArrayResourceImpl& pimpl = createArrayResourceImpl(count, reinterpret_cast<const ramses_internal::Byte*>(arrayData), cacheFlag, name, ramses_internal::EDataType_Vector2F, ERamsesObjectType_Vector2fArray, ramses_internal::EResourceType_VertexArray);
        Vector2fArray* vertexArray = new Vector2fArray(pimpl);
        addResourceObjectToRegistry_ThreadSafe(*vertexArray);


        return vertexArray;
    }

    const Vector3fArray* RamsesClientImpl::createConstVector3fArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        if (!validateArray(count, arrayData))
        {
            return NULL;
        }

        ArrayResourceImpl& pimpl = createArrayResourceImpl(count, reinterpret_cast<const ramses_internal::Byte*>(arrayData), cacheFlag, name, ramses_internal::EDataType_Vector3F, ERamsesObjectType_Vector3fArray, ramses_internal::EResourceType_VertexArray);
        Vector3fArray* vertexArray = new Vector3fArray(pimpl);
        addResourceObjectToRegistry_ThreadSafe(*vertexArray);


        return vertexArray;
    }

    const Vector4fArray* RamsesClientImpl::createConstVector4fArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        if (!validateArray(count, arrayData))
        {
            return NULL;
        }

        ArrayResourceImpl& pimpl = createArrayResourceImpl(count, reinterpret_cast<const ramses_internal::Byte*>(arrayData), cacheFlag, name, ramses_internal::EDataType_Vector4F, ERamsesObjectType_Vector4fArray, ramses_internal::EResourceType_VertexArray);
        Vector4fArray* vertexArray = new Vector4fArray(pimpl);
        addResourceObjectToRegistry_ThreadSafe(*vertexArray);

        return vertexArray;
    }

    const UInt16Array* RamsesClientImpl::createConstUInt16Array(uint32_t count, const uint16_t* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        if (!validateArray(count, arrayData))
        {
            return NULL;
        }

        ArrayResourceImpl& pimpl = createArrayResourceImpl(count, reinterpret_cast<const ramses_internal::Byte*>(arrayData), cacheFlag, name, ramses_internal::EDataType_UInt16, ERamsesObjectType_UInt16Array, ramses_internal::EResourceType_IndexArray);
        UInt16Array* indexArray = new UInt16Array(pimpl);
        addResourceObjectToRegistry_ThreadSafe(*indexArray);

        return indexArray;
    }

    const UInt32Array* RamsesClientImpl::createConstUInt32Array(uint32_t count, const uint32_t* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        if (!validateArray(count, arrayData))
        {
            return NULL;
        }

        ArrayResourceImpl& pimpl = createArrayResourceImpl(count, reinterpret_cast<const ramses_internal::Byte*>(arrayData), cacheFlag, name, ramses_internal::EDataType_UInt32, ERamsesObjectType_UInt32Array, ramses_internal::EResourceType_IndexArray);
        UInt32Array* indexArray = new UInt32Array(pimpl);
        addResourceObjectToRegistry_ThreadSafe(*indexArray);

        return indexArray;
    }

    status_t RamsesClientImpl::writeResourcesToFile(const ResourceFileDescription& fileDescription, bool compress) const
    {
        LOG_DEBUG(ramses_internal::CONTEXT_CLIENT, "RamsesClient::writeResourcesToFile:  " << fileDescription.getFilename());

        if (getClientApplication().hasResourceFile(fileDescription.getFilename()))
        {
            return addErrorEntry("Cannot write resources to file, its already opened by the client.");
        }

        ramses_internal::File outputResources(fileDescription.getFilename());
        ramses_internal::BinaryFileOutputStream resourceOutputStream(outputResources);
        if (!outputResources.isOpen())
        {
            return addErrorEntry("Could not open file for writing resources");
        }

        WriteCurrentBuildVersionToStream(resourceOutputStream);

        ramses_internal::UInt bytesForVersion = 0;
        outputResources.getPos(bytesForVersion);

        const ResourceObjects& resources = fileDescription.impl->m_resources;

        // reserve space for offset to HL-Objects and LL-Objects
        const uint64_t bytesForOffsets = sizeof(uint64_t) * 2u;
        const uint64_t offsetHLResourcesStart = bytesForVersion + bytesForOffsets;
        outputResources.seek(static_cast<ramses_internal::Int>(offsetHLResourcesStart), ramses_internal::EFileSeekOrigin_BeginningOfFile);

        CHECK_RETURN_ERR(writeHLResourcesToStream(resourceOutputStream, resources));
        ramses_internal::UInt offsetLLResourcesStart = 0;
        outputResources.getPos(offsetLLResourcesStart);

        writeLowLevelResourcesToStream(resources, resourceOutputStream, compress);

        outputResources.seek(bytesForVersion, ramses_internal::EFileSeekOrigin_BeginningOfFile);

        resourceOutputStream << static_cast<uint64_t>(offsetHLResourcesStart);
        resourceOutputStream << static_cast<uint64_t>(offsetLLResourcesStart);

        ramses_internal::EStatus stat = outputResources.close();
        if (ramses_internal::EStatus_RAMSES_OK != stat)
        {
            return addErrorEntry("Could not close resource file");
        }

        return StatusOK;
    }

    status_t RamsesClientImpl::writeHLResourcesToStream(ramses_internal::IOutputStream& resourceOutputStream, const ResourceObjects &resources) const
    {
        // check how many resources of each type we have
        ramses_internal::HashMap<ERamsesObjectType, uint32_t> typesToSerialize;
        for (const auto& resource : resources)
        {
            typesToSerialize[resource->getType()]++;
        }

        resourceOutputStream << static_cast<uint32_t>(resources.size());
        resourceOutputStream << static_cast<uint32_t>(typesToSerialize.count());

        SerializationContext serializationContext;
        for (const auto& p : typesToSerialize)
        {
            const ERamsesObjectType type = p.key;
            resourceOutputStream << static_cast<uint32_t>(type);
            resourceOutputStream << p.value;

            for (const auto resource : resources)
            {
                if (resource->getType() == type)
                {
                    CHECK_RETURN_ERR(resource->impl.serialize(resourceOutputStream, serializationContext));
                }
            }
        }

        return StatusOK;
    }

    void RamsesClientImpl::writeLowLevelResourcesToStream(const ResourceObjects& resources, ramses_internal::BinaryFileOutputStream& resourceOutputStream, bool compress) const
    {
        //getting names for resources (names are transmitted only for debugging purposes)
        ramses_internal::ManagedResourceVector managedResources;
        for (const auto res : resources)
        {
            assert(res != NULL);
            const ramses_internal::ResourceContentHash& hash = res->impl.getLowlevelResourceHash();
            const ramses_internal::ManagedResource managedRes = getClientApplication().getResource(hash);
            if (managedRes.getResourceObject() != NULL)
            {
                managedResources.push_back(managedRes);
            }
            else
            {
                const ramses_internal::ManagedResource forceLoadedResource = getClientApplication().forceLoadResource(hash);
                assert(forceLoadedResource.getResourceObject() != NULL);
                managedResources.push_back(forceLoadedResource);
            }
        }

        // write LL-TOC and LL resources
        ramses_internal::ResourcePersistation::WriteNamedResourcesWithTOCToStream(resourceOutputStream, managedResources, compress);
    }

    status_t RamsesClientImpl::writeSceneObjectsToStream(SceneImpl& scene, ramses_internal::IOutputStream& outputStream) const
    {
        ramses_internal::ScenePersistation::WriteSceneMetadataToStream(outputStream, scene.getIScene());
        ramses_internal::ScenePersistation::WriteSceneToStream(outputStream, scene.getIScene());

        SerializationContext serializationContext;
        return scene.serialize(outputStream, serializationContext);
    }

    status_t RamsesClientImpl::saveSceneToFile(SceneImpl& scene, const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation, bool compress) const
    {
        if (&scene.getClientImpl() != this)
        {
            return addErrorEntry("RamsesClient::saveSceneToFile failed, scene is not in this client.");
        }

        ramses_internal::File outputFile(fileName);
        ramses_internal::BinaryFileOutputStream outputStream(outputFile);
        if (!outputFile.isOpen())
        {
            return addErrorEntry("RamsesClient::saveSceneToFile failed, could not open file for writing.");
        }

        WriteCurrentBuildVersionToStream(outputStream);

        const ResourceFileDescriptionVector& descriptions = resourceFileInformation.impl->descriptions;
        for (const auto& description : descriptions)
        {
            CHECK_RETURN_ERR( writeResourcesToFile(description, compress) );
        }

        const status_t status = writeSceneObjectsToStream(scene, outputStream);

        if (outputFile.close() != ramses_internal::EStatus_RAMSES_OK)
        {
            return addErrorEntry("RamsesClient::saveSceneToFile failed, close file failed.");
        }

        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::saveSceneToFile:  '" << fileName << "'.");

        return status;
    }

    template <typename T>
    T& createImplHelper(RamsesClientImpl& clientImpl, ERamsesObjectType)
    {
        return *new T(ramses_internal::ResourceHashUsage(), clientImpl, "");
    }

    template <>
    ArrayResourceImpl& createImplHelper<ArrayResourceImpl>(RamsesClientImpl& clientImpl, ERamsesObjectType type)
    {
        return *new ArrayResourceImpl(ramses_internal::ResourceHashUsage(), type, clientImpl, "");
    }

    template <typename ObjectType, typename ObjectImplType>
    status_t RamsesClientImpl::createAndDeserializeResourceImpls(ramses_internal::IInputStream& inStream, DeserializationContext& deserializationContext, uint32_t count, ResourceVector& container)
    {
        for (uint32_t i = 0u; i < count; ++i)
        {
            ObjectImplType& impl = createImplHelper<ObjectImplType>(*this, TYPE_ID_OF_RAMSES_OBJECT<ObjectType>::ID);
            ObjectIDType objectID = DeserializationContext::GetObjectIDNull();
            const status_t status = SerializationHelper::DeserializeObjectImpl(inStream, deserializationContext, impl, objectID);
            if (status != StatusOK)
            {
                delete &impl;
                return status;
            }
            ObjectType* object = new ObjectType(impl);
            container.push_back(object);
        }

        return StatusOK;
    }

    status_t RamsesClientImpl::readResourcesFromFile(const ramses_internal::String& resourceFilename)
    {
        //TODO Domotor Tobias at this point, we could check whether a stream for the file already exists, and use that.
        //That way, we would have to block the framework lock for the whole reading, which is unfavorable.
        //The current solution opens a new stream for every read operation and then closes them, except the first one.
        ramses_internal::ResourceFileInputStreamSPtr resourceFileStream(new ramses_internal::ResourceFileInputStream(resourceFilename));
        ramses_internal::BinaryFileInputStream& inputStream = resourceFileStream->resourceStream;
        if (inputStream.getState() != ramses_internal::EStatus_RAMSES_OK)
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "RamsesClient::readResourcesFromFile '" << resourceFilename << "' failed, file could not be opened.").c_str());
        }

        const ramses_internal::String fileInfo = ramses_internal::String("resource file '") + resourceFilename + ramses_internal::String("'");
        if (!ReadRamsesVersionAndPrintWarningOnMismatch(inputStream, fileInfo))
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "RamsesClient::readResourcesFromFile '" << resourceFilename << "' failed, file is invalid").c_str());
        }

        uint64_t offsetForHLResources = 0;
        uint64_t offsetForLLResourceBlock = 0;
        inputStream >> offsetForHLResources;
        inputStream >> offsetForLLResourceBlock;

        // read HL Resources
        DeserializationContext deserializationContext;
        inputStream.seek(static_cast<ramses_internal::Int>(offsetForHLResources), ramses_internal::EFileSeekOrigin_BeginningOfFile);

        uint32_t totalCount = 0u;
        uint32_t typesCount = 0u;
        SerializationHelper::DeserializeNumberOfObjectTypes(inputStream, totalCount, typesCount);

        typedef std::vector<ResourceVector> ResourcesPerType;
        ResourcesPerType resourcesPerType(typesCount);
        m_appLogic.reserveResourceCount(totalCount);

        std::array<uint32_t, ERamsesObjectType_NUMBER_OF_TYPES> objectCounts = {};

        for (uint32_t i = 0u; i < typesCount; ++i)
        {
            uint32_t count = 0u;
            const ERamsesObjectType type = SerializationHelper::DeserializeObjectTypeAndCount(inputStream, count);
            ResourceVector& resources = resourcesPerType[i];
            resources.reserve(count);
            ++objectCounts[type];

            status_t status = StatusOK;
            switch (type)
            {
            case ERamsesObjectType_Effect:
                status = createAndDeserializeResourceImpls<Effect, EffectImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_Texture2D:
                status = createAndDeserializeResourceImpls<Texture2D, Texture2DImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_Texture3D:
                status = createAndDeserializeResourceImpls<Texture3D, Texture3DImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_TextureCube:
                status = createAndDeserializeResourceImpls<TextureCube, TextureCubeImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_UInt16Array:
                status = createAndDeserializeResourceImpls<UInt16Array, ArrayResourceImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_UInt32Array:
                status = createAndDeserializeResourceImpls<UInt32Array, ArrayResourceImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_FloatArray:
                status = createAndDeserializeResourceImpls<FloatArray, ArrayResourceImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_Vector2fArray:
                status = createAndDeserializeResourceImpls<Vector2fArray, ArrayResourceImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_Vector3fArray:
                status = createAndDeserializeResourceImpls<Vector3fArray, ArrayResourceImpl>(inputStream, deserializationContext, count, resources);
                break;
            case ERamsesObjectType_Vector4fArray:
                status = createAndDeserializeResourceImpls<Vector4fArray, ArrayResourceImpl>(inputStream, deserializationContext, count, resources);
                break;
            default:
                return addErrorEntry("RamsesClient::deserialize failed, unexpected object type in file stream.");
            }

            CHECK_RETURN_ERR(status);
        }

        LOG_DEBUG_F(ramses_internal::CONTEXT_PROFILING, ([&](ramses_internal::StringOutputStream& sos) {
                    sos << "RamsesClientImpl::readResourcesFromFile: resource counts from '" << resourceFilename << "'\n";
                    for (uint32_t i = 0; i < ERamsesObjectType_NUMBER_OF_TYPES; i++)
                    {
                        if (objectCounts[i] > 0)
                        {
                            sos << "  " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(static_cast<ERamsesObjectType>(i)) << " count: " << objectCounts[i] << "\n";
                        }
                    }
                }));

        // this block must be protected by some lock to make loading resources threadsafe
        {
            ramses_internal::PlatformGuard g(m_clientLock);
            m_resources.reserveAdditionalGeneralCapacity(totalCount);
            for (ResourceVector& it : resourcesPerType)
            {
                if (!it.empty())
                {
                    m_resources.reserveAdditionalObjectCapacity(it.front()->getType(), static_cast<uint32_t>(it.size()));
                    for (Resource* resource : it)
                    {
                        const resourceId_t resId = resource->impl.getResourceId();
                        if (!m_resourcesById.contains(resId))
                        {
                            m_resources.addObject(*resource);
                            m_resourcesById.put(resId, resource);
                        }
                        else
                        {
                            delete resource;
                        }
                    }
                }
            }
        }

        // calls on m_appLogic are thread safe
        if (!m_appLogic.hasResourceFile(resourceFilename))
        {
            // register resource file for on-demand loading (LL-Resources)
            ramses_internal::ResourceTableOfContents loadedTOC;
            loadedTOC.readTOCPosAndTOCFromStream(inputStream);
            m_appLogic.addResourceFile(resourceFileStream, loadedTOC);
        }

        m_framework.getStatisticCollection().statResourcesCreated.incCounter(static_cast<uint32_t>(totalCount));

        return StatusOK;
    }

    ramses_internal::ManagedResource RamsesClientImpl::getResource(ramses_internal::ResourceContentHash hash) const
    {
        return m_appLogic.getResource(hash);
    }

    Scene* RamsesClientImpl::prepareSceneFromInputStream(const char* caller, const ramses_internal::String& filename, ramses_internal::IInputStream& inputStream)
    {
        LOG_TRACE(ramses_internal::CONTEXT_CLIENT, "RamsesClient::prepareSceneFromInputStream:  start loading scene from input stream");

        ramses_internal::SceneCreationInformation createInfo;
        ramses_internal::ScenePersistation::ReadSceneMetadataFromStream(inputStream, createInfo);
        const ramses_internal::SceneSizeInformation& sizeInformation = createInfo.m_sizeInfo;
        const ramses_internal::SceneInfo sceneInfo(createInfo.m_id, createInfo.m_name);

        LOG_DEBUG(ramses_internal::CONTEXT_CLIENT, "RamsesClient::prepareSceneFromInputStream:  scene to be loaded has " << sizeInformation.asString());

        ramses_internal::ClientScene* internalScene = nullptr;
        {
            ramses_internal::PlatformGuard g(m_clientLock);
            internalScene = m_sceneFactory.createScene(sceneInfo);
        }
        if (NULL == internalScene)
        {
            return NULL;
        }
        internalScene->preallocateSceneSize(sizeInformation);

        ramses_internal::AnimationSystemFactory animSystemFactory(ramses_internal::EAnimationSystemOwner_Client, &internalScene->getSceneActionCollection());

        // need first to create the pimpl, so that internal framework components know the new scene
        SceneConfigImpl sceneConfig;
        ramses_internal::PlatformGuard g(m_clientLock);
        {
            if (m_scenesMarkedForLoadAsLocalOnly.hasElement(createInfo.m_id))
            {
                LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::" << caller << ": Mark file loaded from " << filename << " with sceneId " << createInfo.m_id << " as local only");
                sceneConfig.setPublicationMode(EScenePublicationMode_LocalOnly);
            }
        }

        SceneImpl& pimpl = *new SceneImpl(*internalScene, sceneConfig, *this);

        // now the scene is registered, so it's possible to load the low level content into the scene
        LOG_TRACE(ramses_internal::CONTEXT_CLIENT, "    Reading low level scene from stream");
        ramses_internal::ScenePersistation::ReadSceneFromStream(inputStream, *internalScene, &animSystemFactory);

        LOG_TRACE(ramses_internal::CONTEXT_CLIENT, "    Deserializing high level scene objects from stream");
        DeserializationContext deserializationContext;
        ObjectIDType objectID = DeserializationContext::GetObjectIDNull();
        const status_t stat = SerializationHelper::DeserializeObjectImpl(inputStream, deserializationContext, pimpl, objectID);
        if (stat != StatusOK)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "    Failed to deserialize high level scene:");
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, getStatusMessage(stat));
            delete &pimpl;
            return nullptr;
        }

        LOG_TRACE(ramses_internal::CONTEXT_CLIENT, "    Done with preparing scene from input stream.");

        return new Scene(pimpl);
    }

    Scene* RamsesClientImpl::prepareSceneAndResourcesFromFiles(const char* caller, const ramses_internal::String& sceneFilename,
        const std::vector<ramses_internal::String>& resourceFilenames, std::vector<ResourceLoadStatus>& resourceloadStatus)
    {
        LOG_TRACE(ramses_internal::CONTEXT_CLIENT, "RamsesClient::" << caller << ": Reading resources from files");
        bool allResourcesLoadedSuccessfully = true;
        for (const auto& filename : resourceFilenames)
        {
            if (StatusOK != readResourcesFromFile(filename))
            {
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::" << caller << ": failed to read resources from file " << filename);
                resourceloadStatus.push_back({ false, filename });
                allResourcesLoadedSuccessfully = false;
            }
            else
            {
                resourceloadStatus.push_back({ true, filename });
            }
        }

        // if not all resources loaded scene loading fails
        if (!allResourcesLoadedSuccessfully)
        {
            return NULL;
        }

        ramses_internal::File inputFile(sceneFilename);
        ramses_internal::BinaryFileInputStream inputStream(inputFile);

        if (inputStream.getState() != ramses_internal::EStatus_RAMSES_OK)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::" << caller << ":  failed to open file");
            return NULL;
        }

        if (!ReadRamsesVersionAndPrintWarningOnMismatch(inputStream, "scene file"))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::" << caller << ": failed to read from file");
            return NULL;
        }

        Scene* scene = prepareSceneFromInputStream(caller, sceneFilename, inputStream);

        if (inputFile.close() != ramses_internal::EStatus_RAMSES_OK)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::" << caller << ":  failed to close file, continue anyway");
        }

        return scene;
    }

    Scene* RamsesClientImpl::loadSceneFromFile(const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation)
    {
        const ramses_internal::UInt64 start = ramses_internal::PlatformTime::GetMillisecondsMonotonic();
        std::vector<ResourceLoadStatus> resourceLoadStatus;
        Scene* scene = prepareSceneAndResourcesFromFiles("loadSceneFromFile", fileName, resourceFileInformation.impl->getFilenames(), resourceLoadStatus);
        if (!scene)
        {
            return nullptr;
        }
        finalizeLoadedScene(scene);

        const ramses_internal::UInt64 end = ramses_internal::PlatformTime::GetMillisecondsMonotonic();
        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::loadSceneFromFile  Scene loaded from '" << fileName << "' in " << (end - start) << " ms");

        return scene;
    }

    void RamsesClientImpl::finalizeLoadedScene(Scene* scene)
    {
        // add to the known list of scenes
        ramses_internal::PlatformGuard g(m_clientLock);
        m_scenes.push_back(scene);
    }

    void RamsesClientImpl::WriteCurrentBuildVersionToStream(ramses_internal::IOutputStream& stream)
    {
        ramses_internal::RamsesVersion::WriteToStream(stream, ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH);
    }

    bool RamsesClientImpl::ReadRamsesVersionAndPrintWarningOnMismatch(ramses_internal::BinaryFileInputStream& inputStream, const ramses_internal::String& verboseFileName)
    {
        // return false on read error only, not version mismatch
        ramses_internal::RamsesVersion::VersionInfo readVersion;
        if (!ramses_internal::RamsesVersion::ReadFromStream(inputStream, readVersion))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::ReadRamsesVersionAndPrintWarningOnMismatch: failed to read RAMSES version for " << verboseFileName << ", file probably corrupt. Loading aborted.");
            return false;
        }
        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RAMSES version in file '" << verboseFileName << "': [" << readVersion.versionString << "]; GitHash: [" << readVersion.gitHash << "]");

        if (!ramses_internal::RamsesVersion::MatchesMajorMinor(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR_INT, ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR_INT, readVersion))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::ReadRamsesVersionAndPrintWarningOnMismatch: Version of file " << verboseFileName << "does not match MAJOR.MINOR of this build. Cannot load the file.");
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "SDK version of loader: [" << ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING << "]; GitHash: [" << ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH << "]");
            return false;
        }
        return true;
    }

    status_t RamsesClientImpl::saveResources(const ResourceFileDescription& fileDescription, bool compress) const
    {
        const status_t status = writeResourcesToFile(fileDescription, compress);
        if (status != ramses_internal::EStatus_RAMSES_OK)
        {
            return addErrorEntry("RamsesClient::saveResources failed.");
        }
        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::saveResources  Saved resources as '" << fileDescription.getFilename() << "'");

        return StatusOK;
    }

    status_t RamsesClientImpl::saveResources(const ResourceFileDescriptionSet& resourceFileInformation, bool compress) const
    {
        const ResourceFileDescriptionVector& descriptions = resourceFileInformation.impl->descriptions;
        for (const auto& resourceFile : descriptions)
        {
            const status_t status = writeResourcesToFile(resourceFile, compress);
            if (status != ramses_internal::EStatus_RAMSES_OK)
            {
                return addErrorEntry("RamsesClient::saveResources failed.");
            }
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::saveResources  Saved as '" << resourceFile.getFilename() << "'");
        }

        return StatusOK;
    }

    status_t RamsesClientImpl::loadResources(const ResourceFileDescription& fileDescription)
    {
        return readResourcesFromFile(fileDescription.getFilename());
    }

    status_t RamsesClientImpl::loadResources(const ResourceFileDescriptionSet& resourceFileInformation)
    {
        for (const auto& resourceFilename : resourceFileInformation.impl->getFilenames())
        {
            CHECK_RETURN_ERR(readResourcesFromFile(resourceFilename));
        }
        return StatusOK;
    }

    status_t RamsesClientImpl::loadResourcesAsync(const ResourceFileDescription& fileDescription)
    {
        std::vector<ramses_internal::String> filenames;
        filenames.push_back(fileDescription.getFilename());
        LoadResourcesRunnable* task = new LoadResourcesRunnable(*this, filenames);
        m_loadFromFileTaskQueue.enqueue(*task);
        task->release();
        return StatusOK;
    }

    status_t RamsesClientImpl::loadResourcesAsync(const ResourceFileDescriptionSet& resourceFileInformation)
    {
        LoadResourcesRunnable* task = new LoadResourcesRunnable(*this, resourceFileInformation.impl->getFilenames());
        m_loadFromFileTaskQueue.enqueue(*task);
        task->release();
        return StatusOK;
    }

    status_t RamsesClientImpl::loadSceneFromFileAsync(const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation)
    {
        LoadSceneRunnable* task = new LoadSceneRunnable(*this, fileName, resourceFileInformation.impl->getFilenames());
        m_loadFromFileTaskQueue.enqueue(*task);
        task->release();
        return StatusOK;
    }

    status_t RamsesClientImpl::markSceneIdForLoadingAsLocalOnly(sceneId_t sceneId)
    {
        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::markSceneIdForLoadingAsLocalOnly: Add sceneId " << sceneId);

        ramses_internal::PlatformGuard g(m_clientLock);
        m_scenesMarkedForLoadAsLocalOnly.put(ramses_internal::SceneId(sceneId));
        return StatusOK;
    }

    status_t RamsesClientImpl::dispatchEvents(IClientEventHandler& clientEventHandler)
    {
        std::vector<ResourceLoadStatus> localAsyncResourcesStatus;
        std::vector<SceneLoadStatus> localAsyncSceneLoadStatus;
        {
            ramses_internal::PlatformGuard g(m_clientLock);
            localAsyncResourcesStatus.swap(m_asyncResourceLoadStatusVec);
            localAsyncSceneLoadStatus.swap(m_asyncSceneLoadStatusVec);
        }

        for (const auto& resourceStatus : localAsyncResourcesStatus)
        {
            if (resourceStatus.successful)
            {
                LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::dispatchEvents(resourceFileLoadSucceeded): " << resourceStatus.filename);
                clientEventHandler.resourceFileLoadSucceeded(resourceStatus.filename.c_str());
            }
            else
            {
                LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::dispatchEvents(resourceFileLoadFailed): " << resourceStatus.filename);
                clientEventHandler.resourceFileLoadFailed(resourceStatus.filename.c_str());
            }
        }

        for (const auto& sceneStatus : localAsyncSceneLoadStatus)
        {
            if (sceneStatus.scene)
            {
                // finalize scene
                Scene* scene = sceneStatus.scene;
                const ramses_internal::UInt64 start = ramses_internal::PlatformTime::GetMillisecondsMonotonic();
                finalizeLoadedScene(scene);
                const ramses_internal::UInt64 end = ramses_internal::PlatformTime::GetMillisecondsMonotonic();
                LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::dispatchEvents(sceneFileLoadSucceeded): Synchronous postprocessing of scene loaded from '" <<
                         sceneStatus.sceneFilename << "' (sceneName: " << scene->getName() << ", sceneId " << scene->getSceneId() << ") in " << (end - start) << " ms");

                clientEventHandler.sceneFileLoadSucceeded(sceneStatus.sceneFilename.c_str(), scene);
            }
            else
            {
                LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::dispatchEvents(sceneFileLoadFailed): " << sceneStatus.sceneFilename);
                clientEventHandler.sceneFileLoadFailed(sceneStatus.sceneFilename.c_str());
            }
        }

        return StatusOK;
    }

    RamsesClientImpl::LoadResourcesRunnable::LoadResourcesRunnable(RamsesClientImpl& client, const std::vector<ramses_internal::String>& filenames)
        : m_client(client)
        , m_filenames(filenames)
    {
    }

    void RamsesClientImpl::LoadResourcesRunnable::execute()
    {
        std::vector<ResourceLoadStatus> localResults;
        for (const auto& filename : m_filenames)
        {
            const status_t status = m_client.readResourcesFromFile(filename);
            const bool loadSuccesful = (status == StatusOK);
            localResults.push_back({ loadSuccesful, filename });

            if (!loadSuccesful)
            {
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::loadResourcesAsync: failed at " << filename << " with "  << m_client.getStatusMessage(status));
                break;
            }
        }
        ramses_internal::PlatformGuard g(m_client.m_clientLock);
        m_client.m_asyncResourceLoadStatusVec.insert(m_client.m_asyncResourceLoadStatusVec.end(), localResults.begin(), localResults.end());
    }

    RamsesClientImpl::LoadSceneRunnable::LoadSceneRunnable(RamsesClientImpl& client, const ramses_internal::String& sceneFilename, const std::vector<ramses_internal::String>& resourceFilenames)
        : m_client(client)
        , m_sceneFilename(sceneFilename)
        , m_resourceFilenames(resourceFilenames)
    {
    }

    void RamsesClientImpl::LoadSceneRunnable::execute()
    {
        const ramses_internal::UInt64 start = ramses_internal::PlatformTime::GetMillisecondsMonotonic();
        std::vector<ResourceLoadStatus> localResourceLoadStatus;
        Scene* scene = m_client.prepareSceneAndResourcesFromFiles("loadSceneFromFileAsync", m_sceneFilename, m_resourceFilenames, localResourceLoadStatus);
        const ramses_internal::UInt64 end = ramses_internal::PlatformTime::GetMillisecondsMonotonic();

        if (scene)
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesClient::loadSceneFromFileAsync: Scene loaded from '" << m_sceneFilename <<
                     "' (sceneName: " << scene->getName() << ", sceneId " << scene->getSceneId() << ") in " << (end - start) << " ms");
        }

        ramses_internal::PlatformGuard g(m_client.m_clientLock);
        m_client.m_asyncResourceLoadStatusVec.insert(m_client.m_asyncResourceLoadStatusVec.end(), localResourceLoadStatus.begin(), localResourceLoadStatus.end());
        m_client.m_asyncSceneLoadStatusVec.push_back({scene, m_sceneFilename});
    }

    RamsesObjectVector RamsesClientImpl::getListOfResourceObjects(ERamsesObjectType objType) const
    {
        ramses_internal::PlatformGuard g(m_clientLock);
        RamsesObjectVector resources;
        m_resources.getObjectsOfType(resources, objType);
        return resources;
    }

    SceneVector RamsesClientImpl::getListOfScenes() const
    {
        ramses_internal::PlatformGuard g(m_clientLock);
        return m_scenes;
    }

    const RamsesObject* RamsesClientImpl::findObjectByName(const char* name) const
    {
        ramses_internal::PlatformGuard g(m_clientLock);
        const RamsesObject* object = m_resources.findObjectByName(name);
        if (object)
            return object;

        for(const auto& scene : m_scenes)
        {
            if (ramses_internal::PlatformStringUtils::StrEqual(name, (scene->getName())))
            {
                return scene;
            }
        }

        return NULL;
    }

    ramses_internal::ResourceHashUsage RamsesClientImpl::getHashUsage_ThreadSafe(const ramses_internal::ResourceContentHash& hash) const
    {
        return m_appLogic.getHashUsage(hash);
    }

    ramses_internal::ManagedResource RamsesClientImpl::getResource_ThreadSafe(ramses_internal::ResourceContentHash hash) const
    {
        return m_appLogic.getResource(hash);
    }

    ramses_internal::ManagedResource RamsesClientImpl::forceLoadResource_ThreadSafe(const ramses_internal::ResourceContentHash& hash) const
    {
        return m_appLogic.forceLoadResource(hash);
    }

    RamsesObject* RamsesClientImpl::findObjectByName(const char* name)
    {
        // Non-const version of findObjectByName cast to its const version to avoid duplicating code
        return const_cast<RamsesObject*>((const_cast<const RamsesClientImpl&>(*this)).findObjectByName(name));
    }

    const Resource* RamsesClientImpl::scanForResourceWithHash(ramses_internal::ResourceContentHash hash) const
    {
        ramses_internal::PlatformGuard g(m_clientLock);
        RamsesObjectVector resources;
        m_resources.getObjectsOfType(resources, ERamsesObjectType_Resource);
        for (const auto& res : resources)
        {
            const Resource* resource = &RamsesObjectTypeUtils::ConvertTo<Resource>(*res);
            if (hash == resource->impl.getLowlevelResourceHash())
            {
                return resource;
            }
        }

        return NULL;
    }

    Effect* RamsesClientImpl::createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, const char* name)
    {
        //create effect using vertex and fragment shaders
        ramses_internal::String effectName(name);
        ramses_internal::GlslEffect effectBlock(effectDesc.getVertexShader(), effectDesc.getFragmentShader(), effectDesc.impl.getCompilerDefines(),
            effectDesc.impl.getSemanticsMap(), effectName);
        ramses_internal::EffectResource* effectResource = effectBlock.createEffectResource(ramses_internal::ResourceCacheFlag(cacheFlag.getValue()));
        if (!effectResource)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesClient::createEffect  Failed to create effect resource (name: '" << effectName << "') :\n    " << effectBlock.getErrorMessages());
            return NULL;
        }

        return createEffectFromResource(effectResource, name);
    }

    Effect* RamsesClientImpl::createEffectFromResource(const ramses_internal::EffectResource* res, const ramses_internal::String& name)
    {
        if (0 == res)
        {
            return NULL;
        }
        ramses_internal::ManagedResource managedRes = manageResource(res);
        ramses_internal::ResourceHashUsage hashUsage = m_appLogic.getHashUsage(managedRes.getResourceObject()->getHash());
        ramses::EffectImpl& pimpl = *new ramses::EffectImpl(hashUsage, *this, name.c_str());

        const ramses_internal::EffectResource* managedEffectRes = managedRes.getResourceObject()->convertTo<ramses_internal::EffectResource>();
        pimpl.initializeFromFrameworkData(managedEffectRes->getUniformInputs(), managedEffectRes->getAttributeInputs());
        Effect* effect = new Effect(pimpl);

        addResourceObjectToRegistry_ThreadSafe(*effect);

        return effect;
    }

    ramses_internal::ManagedResource RamsesClientImpl::manageResource(const ramses_internal::IResource* res)
    {
        ramses_internal::ManagedResource managedRes = m_appLogic.addResource(res);
        _LOG_HL_CLIENT_API_STR("Created resource with internal hash " << ramses_internal::StringUtils::HexFromResourceContentHash(managedRes.getResourceObject()->getHash()) << ", name: " << managedRes.getResourceObject()->getName());

        return managedRes;
    }

    RamsesClientImpl& RamsesClientImpl::createImpl(const char* name, RamsesFrameworkImpl& components)
    {
        return *new RamsesClientImpl(components, name);
    }

    RamsesFrameworkImpl& RamsesClientImpl::getFramework()
    {
        return m_framework;
    }

    status_t RamsesClientImpl::validate(uint32_t indent) const
    {
        status_t status = RamsesObjectImpl::validate(indent);
        indent += IndentationStep;

        const status_t scenesStatus = validateScenes(indent);
        if (StatusOK != scenesStatus)
        {
            status = scenesStatus;
        }

        const status_t resourcesStatus = validateResources(indent);
        if (StatusOK != resourcesStatus)
        {
            status = resourcesStatus;
        }

        writeResourcesInfoToValidationMessage(indent);

        return status;
    }

    status_t RamsesClientImpl::validateScenes(uint32_t indent) const
    {
        ramses_internal::PlatformGuard g(m_clientLock);

        status_t status = StatusOK;
        for(const auto& scene : m_scenes)
        {
            const status_t sceneStatus = addValidationOfDependentObject(indent, scene->impl);
            if (StatusOK != sceneStatus)
            {
                status = sceneStatus;
            }
        }

        ramses_internal::StringOutputStream msg;
        msg << "Contains " << m_scenes.size() << " scenes";
        addValidationMessage(EValidationSeverity_Info, indent, msg.c_str());

        return status;
    }

    void RamsesClientImpl::writeResourcesInfoToValidationMessage(uint32_t indent) const
    {
        addValidationMessage(EValidationSeverity_Info, indent, "Resources: ");
        indent += IndentationStep;

        for (uint32_t i = 0u; i < ERamsesObjectType_NUMBER_OF_TYPES; ++i)
        {
            const ERamsesObjectType type = static_cast<ERamsesObjectType>(i);
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType_Resource) &&
                RamsesObjectTypeUtils::IsConcreteType(type))
            {
                addValidationMessage(EValidationSeverity_Info, indent, RamsesObjectTypeUtils::GetRamsesObjectTypeName(type));

                RamsesObjectRegistryIterator iter(m_resources, type);
                while (const Resource* resource = iter.getNext<Resource>())
                {
                    const ramses_internal::String& resourceName = resource->impl.getName();
                    const resourceId_t resourceId = resource->getResourceId();
                    const ramses_internal::ResourceContentHash resourceHash = resource->impl.getLowlevelResourceHash();

                    ramses_internal::StringOutputStream msg;
                    msg << "Resource ID: " << ramses_internal::StringUtils::HexFromResourceContentHash({ resourceId.lowPart, resourceId.highPart });
                    msg << "  Resource Hash: " << ramses_internal::StringUtils::HexFromResourceContentHash(resourceHash);
                    msg << "  Name: " << resourceName;
                    addValidationMessage(EValidationSeverity_Info, indent + IndentationStep, msg.c_str());
                }
            }
        }
    }

    status_t RamsesClientImpl::validateResources(uint32_t indent) const
    {
        status_t status = StatusOK;
        ResourceIteratorImpl iter(*this, ERamsesObjectType_Resource);
        RamsesObject* ramsesObject = NULL;
        while (NULL != (ramsesObject = iter.getNext()))
        {
            const Resource& resource = RamsesObjectTypeUtils::ConvertTo<Resource>(*ramsesObject);
            const status_t resourceStatus = addValidationOfDependentObject(indent, resource.impl);
            if (StatusOK != resourceStatus)
            {
                status = resourceStatus;
            }
        }

        return status;
    }

    void RamsesClientImpl::addResourceObjectToRegistry_ThreadSafe(Resource& object)
    {
        m_framework.getStatisticCollection().statResourcesCreated.incCounter(1);

        ramses_internal::PlatformGuard g(m_clientLock);
        const resourceId_t resId = object.getResourceId();
        m_resourcesById.put(resId, &object);
        m_resources.addObject(object);
    }

    void RamsesClientImpl::enqueueSceneCommand(sceneId_t sceneId, const ramses_internal::SceneCommand& command)
    {
        ramses_internal::PlatformGuard guard(m_clientLock);

        for (const auto& scene : m_scenes)
        {
            if ( scene->impl.getSceneId() == sceneId )
            {
                scene->impl.enqueueSceneCommand(command);
            }
        }
    }

    ramses::Resource* RamsesClientImpl::getHLResource_Threadsafe(resourceId_t rid) const
    {
        ramses_internal::PlatformGuard g(m_clientLock);
        HLResourceHashMap::Iterator iter = m_resourcesById.find(rid);
        if (iter != m_resourcesById.end())
        {
            return iter->value;
        }
        return NULL;
    }

    void RamsesClientImpl::updateClientResourceCache()
    {
        const auto timeNow = std::chrono::steady_clock::now();
        while (m_clientResourceCache.size() != 0 && timeNow > m_clientResourceCache.front().first + m_clientResourceCacheTimeout)
        {
            m_clientResourceCache.pop_front();
        }
    }

    void RamsesClientImpl::setClientResourceCacheTimeout(std::chrono::milliseconds timeout)
    {
        m_clientResourceCacheTimeout = timeout;
    }

    status_t RamsesClientImpl::closeResourceFile(const ResourceFileDescription& fileDescription)
    {
        return closeResourceFile(fileDescription.getFilename());
    }

    status_t RamsesClientImpl::closeResourceFile(const ramses_internal::String& resourceFilename)
    {
        // calls on m_appLogic are thread safe
        if (m_appLogic.hasResourceFile(resourceFilename))
        {
            m_appLogic.removeResourceFile(resourceFilename);
            return StatusOK;
        }
        else
        {
            return addErrorEntry("Cannot close resource file, since it was not known");
        }
    }

    status_t RamsesClientImpl::closeResourceFiles(const ResourceFileDescriptionSet& fileDescriptions)
    {
        for (const auto& resourceFilename : fileDescriptions.impl->getFilenames())
        {
            CHECK_RETURN_ERR(closeResourceFile(resourceFilename));
        }
        return StatusOK;
    }

}
