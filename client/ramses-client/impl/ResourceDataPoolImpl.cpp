//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceDataPoolImpl.h"

#include "ramses-client-api/Resource.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/Effect.h"

#include "Utils/LogMacros.h"
#include "Components/ResourceTableOfContents.h"
#include "SerializationContext.h"
#include "RamsesClientImpl.h"
#include "ResourceImpl.h"
#include "RamsesClientTypesImpl.h"

namespace ramses
{
    class Texture3DImpl;
    class TextureCubeImpl;

    ResourceDataPoolImpl::ResourceDataPoolImpl(RamsesClientImpl& client)
        : m_client(client)
    {
    }

    resourceId_t ResourceDataPoolImpl::addResourceDataToPool(ramses_internal::ManagedResource const& res, const char* name, ERamsesObjectType type)
    {
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::addResourceDataToPool: couldn't create managed resource for type " << type);
            return {};
        }

        auto id = ResourceImpl::CreateResourceHash(res->getHash(), name, type);

        assert(!m_resourcePoolData.contains(id) || res == m_resourcePoolData.find(id)->value.managedResource);

        m_resourcePoolData[id] = { res, name ? name : "" };
        return id;
    }

    resourceId_t ResourceDataPoolImpl::addArrayResourceData(EDataType type, uint32_t numElements, const void* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        auto res = m_client.createManagedArrayResource(numElements, type, arrayData, cacheFlag, name);
        return addResourceDataToPool(res, name, ERamsesObjectType_ArrayResource);
    }

    ramses::resourceId_t ResourceDataPoolImpl::addTexture2DData(ETextureFormat format, uint32_t width, uint32_t height, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name)
    {
        auto res = m_client.createManagedTexture(ramses_internal::EResourceType_Texture2D, width, height, 1u, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        return addResourceDataToPool(res, name, ERamsesObjectType_Texture2D);
    }

    ramses::resourceId_t ResourceDataPoolImpl::addTexture3DData(ETextureFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name)
    {
        auto res = m_client.createManagedTexture(ramses_internal::EResourceType_Texture3D, width, height, depth, format, mipMapCount, mipLevelData, generateMipChain, {}, cacheFlag, name);
        return addResourceDataToPool(res, name, ERamsesObjectType_Texture3D);
    }

    ramses::resourceId_t ResourceDataPoolImpl::addTextureCubeData(ETextureFormat format, uint32_t size, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name)
    {
        auto res = m_client.createManagedTexture(ramses_internal::EResourceType_TextureCube, size, 1u, 1u, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        return addResourceDataToPool(res, name, ERamsesObjectType_TextureCube);
    }

    ramses::resourceId_t ResourceDataPoolImpl::addEffectData(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, const char* name)
    {
        auto res = m_client.createManagedEffect(effectDesc, cacheFlag, name, m_effectErrorMessages);
        return addResourceDataToPool(res, name, ERamsesObjectType_Effect);
    }

    std::string ResourceDataPoolImpl::getLastEffectErrorMessages() const
    {
        return m_effectErrorMessages;
    }

    bool ResourceDataPoolImpl::removeResourceData(resourceId_t const& id)
    {
        return m_resourcePoolData.remove(id);
    }

    bool ResourceDataPoolImpl::addResourceDataFile(std::string const& filename)
    {
        if (m_client.getClientApplication().hasResourceFile(filename.c_str()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::addResourceDataFile '" << filename << "' failed, resource file has been already added.");
            return false;
        }

        ramses_internal::ResourceFileInputStreamSPtr resourceFileStream(new ramses_internal::ResourceFileInputStream(filename.c_str()));
        ramses_internal::BinaryFileInputStream& inputStream = resourceFileStream->resourceStream;
        if (inputStream.getState() != ramses_internal::EStatus::Ok)
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::addResourceDataFile '" << filename << "' failed, file could not be opened.");
            return false;
        }

        const ramses_internal::String fileInfo = ramses_internal::String("resource data file '") + filename + ramses_internal::String("'");
        if (!RamsesClientImpl::ReadRamsesVersionAndPrintWarningOnMismatch(inputStream, fileInfo))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::addResourceDataFile '" << filename << "' failed, file is invalid");
            return false;
        }

        uint64_t offsetForHLToc = 0;
        uint64_t offsetForLLResourceBlock = 0;
        inputStream >> offsetForHLToc;
        inputStream >> offsetForLLResourceBlock;

        // read HL Resources
        inputStream.seek(static_cast<ramses_internal::Int>(offsetForHLToc), ramses_internal::File::SeekOrigin::BeginningOfFile);

        uint64_t totalCount = 0u;
        inputStream >> totalCount;

        for (uint64_t i = 0; i < totalCount; ++i)
        {
            resourceId_t id;
            uint64_t offset;
            inputStream >> id.highPart >> id.lowPart >> offset;

            m_resourceFileAddressRegister[id].push_back({ resourceFileStream, offset });
            m_resourceDataFileContent[filename].push_back(id);
        }

        inputStream.seek(static_cast<ramses_internal::Int>(offsetForLLResourceBlock), ramses_internal::File::SeekOrigin::BeginningOfFile);
        // register resource file for on-demand loading (LL-Resources)
        ramses_internal::ResourceTableOfContents loadedTOC;
        if (!loadedTOC.readTOCPosAndTOCFromStream(inputStream))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::addResourceDataFile '" << filename << "' failed, file did not contain valid resource data");
            return false;
        }

        m_client.getClientApplication().addResourceFile(resourceFileStream, loadedTOC);

        return true;
    }

    bool ResourceDataPoolImpl::forceLoadResourcesFromResourceDataFile(std::string const& filename)
    {
        m_client.getClientApplication().loadResourceFromFile(filename.c_str());
        return true;
    }

    bool ResourceDataPoolImpl::removeResourceDataFile(std::string const& filename)
    {
        if (!m_resourceDataFileContent.contains(filename))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::removeResourceDataFile: file is unknown and can't be closed.");
            return false;
        }

        assert(m_client.getClientApplication().hasResourceFile(filename.c_str()));

        // delete from file content map
        auto fileIt = m_resourceDataFileContent.find(filename);
        auto resources = std::move(fileIt->value);
        m_resourceDataFileContent.remove(fileIt);

        for (auto const& id : resources)
        {
            auto it = m_resourceFileAddressRegister.find(id);
            if (it != m_resourceFileAddressRegister.end())
            {
                auto& addresses = it->value;
                auto address = std::remove_if(addresses.begin(), addresses.end(),
                    [&filename](auto const& entry) {
                        return entry.file->getResourceFileName() == filename;
                    });
                addresses.erase(address, addresses.end());

                if (addresses.empty())
                    m_resourceFileAddressRegister.remove(it);
            }
        }

        m_client.getClientApplication().removeResourceFile(filename.c_str());
        return true;
    }

    bool ResourceDataPoolImpl::saveResourceDataFile(std::string const& filename, ResourceObjects const& resources, bool compress) const
    {
        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: " << filename << " numResources:" << resources.size());

        if (m_client.getClientApplication().hasResourceFile(filename.c_str()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: Cannot write resources to file, its already opened by the resource data pool.");
            return false;
        }

        ramses_internal::File resourceDataFileOut(filename.c_str());
        ramses_internal::BinaryFileOutputStream resourceDataFileOutStream(resourceDataFileOut);
        if (!resourceDataFileOut.isOpen())
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: Could not open file for writing resources");
            return false;
        }

        RamsesClientImpl::WriteCurrentBuildVersionToStream(resourceDataFileOutStream);

        ramses_internal::UInt offsetDataStart = 0;
        if (!resourceDataFileOut.getPos(offsetDataStart))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: File handling error.");
            return false;
        }

        // calculate offsets for hl toc and resources
        const uint64_t offsetHLTocStart = offsetDataStart + sizeof(uint64_t) * 2u;
        const uint64_t offsetHLResourcesStart =
            offsetHLTocStart +                                              // start of toc
            sizeof(uint64_t) +                                              // total resource count
            resources.size() * (sizeof(resourceId_t) + sizeof(uint64_t));   // number of entries with (resourceId_t and offset)

        if (!resourceDataFileOut.seek(static_cast<ramses_internal::Int>(offsetHLResourcesStart), ramses_internal::File::SeekOrigin::BeginningOfFile))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: File handling error.");
            return false;
        }

        std::vector<std::pair<resourceId_t, uint64_t>> fileContent;
        SerializationContext serializationContext;
        serializationContext.serializeSceneObjectIds(false); // we take the resources out of the context of its scene, so don't write sceneObjectIds
        for (const auto& res : resources)
        {
            ramses_internal::UInt filepos = 0;
            if (!resourceDataFileOut.getPos(filepos))
            {
                LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: File handling error.");
                return false;
            }

            fileContent.push_back({ res->getResourceId(), static_cast<uint64_t>(filepos) });

            resourceDataFileOutStream << static_cast<uint64_t>(res->getType());
            if (StatusOK != res->impl.serialize(resourceDataFileOutStream, serializationContext))
            {
                LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: Error serializing resource " << res->getResourceId().highPart << ":" << res->getResourceId().lowPart);
                return false;
            }
        }

        ramses_internal::UInt offsetLLResourcesStart = 0;
        if (!resourceDataFileOut.getPos(offsetLLResourcesStart))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: File handling error.");
            return false;
        }

        m_client.writeLowLevelResourcesToStream(resources, resourceDataFileOutStream, compress);

        if (!resourceDataFileOut.seek(offsetDataStart, ramses_internal::File::SeekOrigin::BeginningOfFile))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: File handling error.");
            return false;
        }

        resourceDataFileOutStream << static_cast<uint64_t>(offsetHLTocStart);
        resourceDataFileOutStream << static_cast<uint64_t>(offsetLLResourcesStart);
        resourceDataFileOutStream << static_cast<uint64_t>(fileContent.size());
        for (auto const& entry : fileContent)
            resourceDataFileOutStream << entry.first.highPart << entry.first.lowPart << entry.second;

        ramses_internal::UInt check = 0;
        if (!resourceDataFileOut.getPos(check))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: File handling error.");
            return false;
        }

        if (check != offsetHLResourcesStart)
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: Error writing resource data to file. Table of content does not fit before hl resources");
            return false;
        }

        if (!resourceDataFileOut.close())
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::saveResourceDataFile: Could not close resource file");
            return false;
        }

        return true;
    }

    ramses::Resource* ResourceDataPoolImpl::createResourceForScene(Scene& scene, resourceId_t const& id)
    {
        if (scene.getResource(id))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::createResourceForScene failed, resource already exists for scene.");
            return nullptr;
        }

        ramses::Resource* ret = nullptr;
        auto poolData = m_resourcePoolData.get(id);
        auto addresses = m_resourceFileAddressRegister.get(id);

        if (poolData) // in case resource exists in both data and file we prefer the cheaper pool data variant
            ret = createResourceForScene(scene, *poolData);

        if (!ret && addresses && !addresses->empty())
            ret = createResourceForScene(scene, addresses->front(), id);

        if (!ret)
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::createResourceForScene: Resource " << id << " not found in resource data pool");

        assert(!ret || ret->getResourceId() == id);
        return ret;
    }

    ramses::Resource* ResourceDataPoolImpl::createResourceForScene(Scene& scene, ResourceFileAddress const& address, resourceId_t const& id) const
    {
        ramses_internal::BinaryFileInputStream& inputStream = address.file->resourceStream;
        if (inputStream.seek(static_cast<ramses_internal::Int>(address.offset), ramses_internal::File::SeekOrigin::BeginningOfFile) != ramses_internal::EStatus::Ok)
        {
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::createResourceForScene: Error accessing file for Resource " << id);
            return nullptr;
        }

        uint64_t type;
        inputStream >> type;

        status_t status = StatusOK;
        DeserializationContext context;
        switch (ERamsesObjectType(type))
        {
        case ERamsesObjectType_Effect:
            status = scene.impl.createAndDeserializeObjectImpls<Effect, EffectImpl>(inputStream, context, 1);
            break;
        case ERamsesObjectType_Texture2D:
            status = scene.impl.createAndDeserializeObjectImpls<Texture2D, Texture2DImpl>(inputStream, context, 1);
            break;
        case ERamsesObjectType_Texture3D:
            status = scene.impl.createAndDeserializeObjectImpls<Texture3D, Texture3DImpl>(inputStream, context, 1);
            break;
        case ERamsesObjectType_TextureCube:
            status = scene.impl.createAndDeserializeObjectImpls<TextureCube, TextureCubeImpl>(inputStream, context, 1);
            break;
        case ERamsesObjectType_ArrayResource:
            status = scene.impl.createAndDeserializeObjectImpls<ArrayResource, ArrayResourceImpl>(inputStream, context, 1);
            break;
        default:
            LOG_ERROR(CONTEXT_CLIENT, "ResourceDataPool::createResourceForScene failed, unexpected object type in file stream " << type);
        }

        return status == StatusOK ? scene.getResource(id) : nullptr;
    }

    ramses::Resource* ResourceDataPoolImpl::createResourceForScene(Scene& scene, ResourceData const& poolData) const
    {
        switch (poolData.managedResource->getTypeID())
        {
        case ramses_internal::EResourceType_VertexArray:
        case ramses_internal::EResourceType_IndexArray:
            return scene.impl.createHLArrayResource(poolData.managedResource, poolData.name.c_str());
        case ramses_internal::EResourceType_Texture2D:
            return scene.impl.createHLTexture2D(poolData.managedResource, poolData.name.c_str());
        case ramses_internal::EResourceType_Texture3D:
            return scene.impl.createHLTexture3D(poolData.managedResource, poolData.name.c_str());
        case ramses_internal::EResourceType_TextureCube:
            return scene.impl.createHLTextureCube(poolData.managedResource, poolData.name.c_str());
        case ramses_internal::EResourceType_Effect:
            return scene.impl.createHLEffect(poolData.managedResource, poolData.name.c_str());
        default:
            assert(0);
            return nullptr;
        }
    }

    void ResourceDataPoolImpl::getAllResourceDataFileResourceIds(std::vector<resourceId_t>& resources) const
    {
        resources.reserve(m_resourceFileAddressRegister.size());
        for (auto const& entry : m_resourceFileAddressRegister)
            resources.push_back(entry.key);
    }
}
