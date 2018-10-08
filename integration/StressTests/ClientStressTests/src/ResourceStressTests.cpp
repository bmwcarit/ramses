//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceStressTests.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/IClientEventHandler.h"
#include "SceneAPI/TextureEnums.h"
#include "TextureUtils.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "ramses-client-api/Resource.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "MemoryLogger.h"
#include "RamsesClientImpl.h"
#include "Utils/File.h"
#include "TestRandom.h"
#include <cmath>
#include <memory>
#include <thread>

using namespace ramses_internal;

class ResourceLoadedEventHandler : public ramses::IClientEventHandler
{
public:
    virtual void resourceFileLoadFailed(const char*) override
    {}

    virtual void resourceFileLoadSucceeded(const char*) override
    {
        m_fileLoadSucceeded = true;
    }

    virtual void sceneFileLoadFailed(const char*) override
    {}
    virtual void sceneFileLoadSucceeded(const char*, ramses::Scene*) override
    {}

    bool fileHasBeenLoaded()
    {
        return m_fileLoadSucceeded;
    }
private:
    bool m_fileLoadSucceeded = false;
};


ResourceStressTestBase::ResourceStressTestBase(int32_t argc, const char* argv[], const String& name)
: StressTest(argc, argv, name)
{
    // These stress tests should check client behaviour with lots of
    // resource creation and deletion happening very fast.
    // To really trigger resource deletion when we want it, we
    // deactivate ResourceCaching on client side, i.e. set Timeout to 0
    //
    // When the renderer is involved this might cause a race condition
    // if the renderer needs resources which are already deleted.
    m_client->impl.setClientResourceCacheTimeout(std::chrono::milliseconds{ 0u });
}

ResourceStressTestBase::~ResourceStressTestBase()
{
    cleanupResourceFile();
}

void ResourceStressTestBase::destroyResource()
{
    m_client->destroy(*m_resource);
    m_resource = nullptr;
}

void ResourceStressTestBase::cleanupResourceFile()
{
    cleanupResourceFile(createFileDescription());
}

void ResourceStressTestBase::cleanupResourceFile(const ramses::ResourceFileDescription& fileDescription)
{
    File resourceFile(fileDescription.getFilename());
    if(resourceFile.exists())
    {
        resourceFile.remove();
    }
}

void ResourceStressTestBase::saveResource(const ramses::Resource* resource)
{
    saveResource(createFileDescription(), resource);
}

void ResourceStressTestBase::saveResource(ramses::ResourceFileDescription fileDescription, const ramses::Resource* resource)
{
    if(!resource)
    {
        resource = m_resource;
    }

    fileDescription.add(resource);
    m_client->saveResources(fileDescription,false);
}

void ResourceStressTestBase::loadResource(ramses::resourceId_t id)
{
    loadResource(createFileDescription(), id);
}

void ResourceStressTestBase::loadResourceAsync(ramses::resourceId_t id)
{
    loadResourceAsync(createFileDescription(), id);
}

void ResourceStressTestBase::loadResource(const ramses::ResourceFileDescription& fileDescription, ramses::resourceId_t id)
{
    if( id != ramses::InvalidResourceId )
    {
        m_resourceId = id;
    }

    m_client->loadResources(fileDescription);
    m_client->forceCloseResourceFileAsync(fileDescription);
    m_resource = m_client->findResourceById(m_resourceId);
}

void ResourceStressTestBase::loadResourceAsync(const ramses::ResourceFileDescription& fileDescription, ramses::resourceId_t id)
{
    if( id != ramses::InvalidResourceId )
    {
        m_resourceId = id;
    }

    ResourceLoadedEventHandler eventHandler;
    m_client->loadResourcesAsync(fileDescription);
    while(!eventHandler.fileHasBeenLoaded())
    {
        m_client->dispatchEvents(eventHandler);
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }
    m_client->forceCloseResourceFileAsync(fileDescription);
    m_resource = m_client->findResourceById(m_resourceId);
}

template< typename ArrayType >
void ResourceStressTestBase::populateArray(ArrayType* array, uint32_t numberOfElements)
{
    for(uint32_t i = 0; i < numberOfElements; ++i)
    {
        array[i] = static_cast<ArrayType>(TestRandom::Get(0, std::numeric_limits<UInt32>::max()));
    }
}

void ResourceStressTestBase::createFloatArray(uint32_t size)
{
    auto data =  std::unique_ptr<float[]>(new float[size]);
    populateArray(data.get(),size);
    m_resource   = m_client->createConstFloatArray(size,data.get());
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createVector2fArray(uint32_t size)
{
    auto data =  std::unique_ptr<float[]>(new float[size*2]);
    populateArray(data.get(),size*2);
    m_resource = m_client->createConstVector2fArray(size,data.get());
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createVector3fArray(uint32_t size)
{
    auto data =  std::unique_ptr<float[]>(new float[size*3]);
    populateArray(data.get(),size*3);
    m_resource = m_client->createConstVector3fArray(size,data.get());
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createVector4fArray(uint32_t size)
{
    auto data =  std::unique_ptr<float[]>(new float[size*4]);
    populateArray(data.get(),size*4);
    m_resource = m_client->createConstVector4fArray(size,data.get());
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createUInt16Array(uint32_t size)
{
    auto data =  std::unique_ptr<uint16_t[]>(new uint16_t[size]);
    populateArray(data.get(),size);
    m_resource = m_client->createConstUInt16Array(size,data.get());
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createUInt32Array(uint32_t size)
{
    auto data =  std::unique_ptr<uint32_t[]>(new uint32_t[size]);
    populateArray(data.get(),size);
    m_resource = m_client->createConstUInt32Array(size,data.get());
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createTexture2D(uint32_t size)
{
    const ramses::ETextureFormat format = ramses::ETextureFormat_RGBA32F;
    const uint32_t sizeOfTexel          = GetTexelSizeFromFormat(ramses::TextureUtils::GetTextureFormatInternal(format));
    const uint32_t sizePowerOf2         = roundUpToPowOf2(size);
    Vector<std::unique_ptr<float[]>> rawData;
    Vector<ramses::MipLevelData> levelData;

    for(uint32_t sizeOfLevel = sizePowerOf2; sizeOfLevel > 0; sizeOfLevel = sizeOfLevel >> 1)
    {
        const uint32_t sizeOfData = sizeOfLevel * sizeOfLevel * sizeOfTexel;
        rawData.push_back(std::unique_ptr<float[]>(new float[sizeOfData]));
        populateArray(rawData.back().get(),sizeOfData);

        levelData.push_back(ramses::MipLevelData(sizeOfData * sizeof(float), reinterpret_cast<const uint8_t*>(rawData.back().get())));
    }

    m_resource   = m_client->createTexture2D(sizePowerOf2, sizePowerOf2, format, static_cast<uint32_t>(levelData.size()), &levelData[0]);
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createTexture3D(uint32_t size)
{
    const ramses::ETextureFormat format = ramses::ETextureFormat_RGBA32F;
    const uint32_t sizeOfTexel          = GetTexelSizeFromFormat(ramses::TextureUtils::GetTextureFormatInternal(format));
    const uint32_t sizePowerOf2         = roundUpToPowOf2(size);
    Vector<std::unique_ptr<float[]>> rawData;
    Vector<ramses::MipLevelData> levelData;

    for(uint32_t sizeOfLevel = sizePowerOf2; sizeOfLevel > 0; sizeOfLevel = sizeOfLevel >> 1)
    {
        const uint32_t sizeOfData = sizeOfLevel * sizeOfLevel * sizeOfLevel * sizeOfTexel;
        rawData.push_back(std::unique_ptr<float[]>(new float[sizeOfData]));
        populateArray(rawData.back().get(),sizeOfData);

        levelData.push_back(ramses::MipLevelData(sizeOfData * sizeof(float), reinterpret_cast<const uint8_t*>(rawData.back().get())));
    }

    m_resource   = m_client->createTexture3D(sizePowerOf2, sizePowerOf2, sizePowerOf2, format, static_cast<uint32_t>(levelData.size()), &levelData[0]);
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createTextureCube(uint32_t size)
{
    const ramses::ETextureFormat format = ramses::ETextureFormat_RGBA32F;
    const uint32_t sizeOfTexel          = GetTexelSizeFromFormat(ramses::TextureUtils::GetTextureFormatInternal(format));
    const uint32_t numberOfFaces        = 6u;
    const uint32_t sizePowerOf2         = roundUpToPowOf2(size);
    Vector<std::unique_ptr<float[]>> rawData;
    Vector<ramses::CubeMipLevelData> levelData;

    for(uint32_t sizeOfLevel = sizePowerOf2; sizeOfLevel > 0; sizeOfLevel = sizeOfLevel >> 1)
    {
        const uint32_t sizeOfFace = sizeOfLevel * sizeOfLevel * sizeOfTexel;
        const uint32_t sizeOfData = sizeOfFace * numberOfFaces;
        rawData.push_back(std::unique_ptr<float[]>(new float[sizeOfData]));
        populateArray(rawData.back().get(),sizeOfData);

        levelData.push_back( ramses::CubeMipLevelData(sizeOfFace * sizeof(float)
                                                    , reinterpret_cast<const uint8_t*>(rawData.back().get() + sizeOfFace * 0)
                                                    , reinterpret_cast<const uint8_t*>(rawData.back().get() + sizeOfFace * 1)
                                                    , reinterpret_cast<const uint8_t*>(rawData.back().get() + sizeOfFace * 2)
                                                    , reinterpret_cast<const uint8_t*>(rawData.back().get() + sizeOfFace * 3)
                                                    , reinterpret_cast<const uint8_t*>(rawData.back().get() + sizeOfFace * 4)
                                                    , reinterpret_cast<const uint8_t*>(rawData.back().get() + sizeOfFace * 5) ) );
    }

    m_resource   = m_client->createTextureCube(sizePowerOf2, format, static_cast<uint32_t>(levelData.size()), &levelData[0]);
    m_resourceId = m_resource->getResourceId();
}

void ResourceStressTestBase::createEffect()
{
    ramses::EffectDescription effectDescription;
    effectDescription.setVertexShader("void main(void) {gl_Position=vec4(0);}");
    effectDescription.setFragmentShader("void main(void) {gl_FragColor=vec4(1);}");
    m_resource   = m_client->createEffect(effectDescription);
    m_resourceId = m_resource->getResourceId();
}

ramses::ResourceFileDescription ResourceStressTestBase::createFileDescription()
{
    return ramses::ResourceFileDescription( (m_testBaseName + name()).c_str() );
}

uint32_t ResourceStressTestBase::roundUpToPowOf2(uint32_t value)
{
    uint32_t v = 1;
    while(v < value && v != 0)
    {
        v = v << 1;
    }
    return v;
}

CreateDestroyFloatArray::CreateDestroyFloatArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyFloatArray")
{
}

int32_t CreateDestroyFloatArray::run_loop()
{
    createFloatArray(1024);
    destroyResource();
    return 0;
}

SaveFloatArray::SaveFloatArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveFloatArray")
{}

int32_t SaveFloatArray::run_pre()
{
    createFloatArray(1024);
    return 0;
}

int32_t SaveFloatArray::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadFloatArray::LoadFloatArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadFloatArray")
{}

int32_t LoadFloatArray::run_pre()
{
    createFloatArray(1024);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadFloatArray::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadFloatArrayAsync::LoadFloatArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadFloatArrayAsync")
{}

int32_t LoadFloatArrayAsync::run_pre()
{
    createFloatArray(1024);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadFloatArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadFloatArray::SaveLoadFloatArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadFloatArray")
{}

int32_t SaveLoadFloatArray::run_loop()
{
    int32_t returnValue = 0;

    createFloatArray(1024);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadFloatArrayAsync::SaveLoadFloatArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadFloatArrayAsync")
{}

int32_t SaveLoadFloatArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    createFloatArray(1024);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyVector2fArray::CreateDestroyVector2fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyVector2fArray")
{
}

int32_t CreateDestroyVector2fArray::run_loop()
{
    createVector2fArray(1024);
    destroyResource();
    return 0;
}

SaveVector2fArray::SaveVector2fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveVector2fArray")
{}

int32_t SaveVector2fArray::run_pre()
{
    createVector2fArray(1024);
    return 0;
}

int32_t SaveVector2fArray::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadVector2fArray::LoadVector2fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadVector2fArray")
{}

int32_t LoadVector2fArray::run_pre()
{
    createVector2fArray(1024);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadVector2fArray::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadVector2fArrayAsync::LoadVector2fArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadVector2fArrayAsync")
{}

int32_t LoadVector2fArrayAsync::run_pre()
{
    createVector2fArray(1024);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadVector2fArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadVector2fArray::SaveLoadVector2fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadVector2fArray")
{}

int32_t SaveLoadVector2fArray::run_loop()
{
    int32_t returnValue = 0;

    createVector2fArray(1024);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadVector2fArrayAsync::SaveLoadVector2fArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadVector2fArrayAsync")
{}

int32_t SaveLoadVector2fArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    createVector2fArray(1024);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyVector3fArray::CreateDestroyVector3fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyVector3fArray")
{
}

int32_t CreateDestroyVector3fArray::run_loop()
{
    createVector3fArray(512);
    destroyResource();
    return 0;
}

SaveVector3fArray::SaveVector3fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveVector3fArray")
{}

int32_t SaveVector3fArray::run_pre()
{
    createVector3fArray(512);
    return 0;
}

int32_t SaveVector3fArray::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadVector3fArray::LoadVector3fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadVector3fArray")
{}

int32_t LoadVector3fArray::run_pre()
{
    createVector3fArray(512);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadVector3fArray::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadVector3fArrayAsync::LoadVector3fArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadVector3fArrayAsync")
{}

int32_t LoadVector3fArrayAsync::run_pre()
{
    createVector3fArray(512);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadVector3fArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadVector3fArray::SaveLoadVector3fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadVector3fArray")
{}

int32_t SaveLoadVector3fArray::run_loop()
{
    int32_t returnValue = 0;

    createVector3fArray(512);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadVector3fArrayAsync::SaveLoadVector3fArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadVector3fArrayAsync")
{}

int32_t SaveLoadVector3fArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    createVector3fArray(512);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyVector4fArray::CreateDestroyVector4fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyVector4fArray")
{
}

int32_t CreateDestroyVector4fArray::run_loop()
{
    createVector4fArray(512);
    destroyResource();
    return 0;
}

SaveVector4fArray::SaveVector4fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveVector4fArray")
{}

int32_t SaveVector4fArray::run_pre()
{
    createVector4fArray(512);
    return 0;
}

int32_t SaveVector4fArray::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadVector4fArray::LoadVector4fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadVector4fArray")
{}

int32_t LoadVector4fArray::run_pre()
{
    createVector4fArray(512);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadVector4fArray::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadVector4fArrayAsync::LoadVector4fArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadVector4fArrayAsync")
{}

int32_t LoadVector4fArrayAsync::run_pre()
{
    createVector4fArray(512);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadVector4fArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadVector4fArray::SaveLoadVector4fArray(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadVector4fArray")
{}

int32_t SaveLoadVector4fArray::run_loop()
{
    int32_t returnValue = 0;

    createVector4fArray(512);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadVector4fArrayAsync::SaveLoadVector4fArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadVector4fArrayAsync")
{}

int32_t SaveLoadVector4fArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    createVector4fArray(512);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyUInt16Array::CreateDestroyUInt16Array(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyUInt16Array")
{
}

int32_t CreateDestroyUInt16Array::run_loop()
{
    createUInt16Array(1024);
    destroyResource();
    return 0;
}

SaveUInt16Array::SaveUInt16Array(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveUInt16Array")
{}

int32_t SaveUInt16Array::run_pre()
{
    createUInt16Array(1024);
    return 0;
}

int32_t SaveUInt16Array::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadUInt16Array::LoadUInt16Array(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadUInt16Array")
{}

int32_t LoadUInt16Array::run_pre()
{
    createUInt16Array(1024);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadUInt16Array::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadUInt16ArrayAsync::LoadUInt16ArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadUInt16ArrayAsync")
{}

int32_t LoadUInt16ArrayAsync::run_pre()
{
    createUInt16Array(1024);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadUInt16ArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadUInt16Array::SaveLoadUInt16Array(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadUInt16Array")
{}

int32_t SaveLoadUInt16Array::run_loop()
{
    int32_t returnValue = 0;

    createUInt16Array(1024);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadUInt16ArrayAsync::SaveLoadUInt16ArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadUInt16ArrayAsync")
{}

int32_t SaveLoadUInt16ArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    createUInt16Array(1024);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyUInt32Array::CreateDestroyUInt32Array(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyUInt32Array")
{
}

int32_t CreateDestroyUInt32Array::run_loop()
{
    createUInt32Array(512);
    destroyResource();
    return 0;
}

SaveUInt32Array::SaveUInt32Array(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveUInt32Array")
{}

int32_t SaveUInt32Array::run_pre()
{
    createUInt32Array(512);
    return 0;
}

int32_t SaveUInt32Array::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadUInt32Array::LoadUInt32Array(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadUInt32Array")
{}

int32_t LoadUInt32Array::run_pre()
{
    createUInt32Array(512);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadUInt32Array::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadUInt32ArrayAsync::LoadUInt32ArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadUInt32ArrayAsync")
{}

int32_t LoadUInt32ArrayAsync::run_pre()
{
    createUInt32Array(512);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadUInt32ArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadUInt32Array::SaveLoadUInt32Array(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadUInt32Array")
{}

int32_t SaveLoadUInt32Array::run_loop()
{
    int32_t returnValue = 0;

    createUInt32Array(512);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadUInt32ArrayAsync::SaveLoadUInt32ArrayAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadUInt32ArrayAsync")
{}

int32_t SaveLoadUInt32ArrayAsync::run_loop()
{
    int32_t returnValue = 0;

    createUInt32Array(512);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyTexture2D::CreateDestroyTexture2D(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyTexture2D")
{
}

int32_t CreateDestroyTexture2D::run_loop()
{
    createTexture2D(256);
    destroyResource();
    return 0;
}

SaveTexture2D::SaveTexture2D(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveTexture2D")
{}

int32_t SaveTexture2D::run_pre()
{
    createTexture2D(256);
    return 0;
}

int32_t SaveTexture2D::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadTexture2D::LoadTexture2D(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadTexture2D")
{}

int32_t LoadTexture2D::run_pre()
{
    createTexture2D(256);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadTexture2D::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadTexture2DAsync::LoadTexture2DAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadTexture2DAsync")
{}

int32_t LoadTexture2DAsync::run_pre()
{
    createTexture2D(256);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadTexture2DAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadTexture2D::SaveLoadTexture2D(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadTexture2D")
{}

int32_t SaveLoadTexture2D::run_loop()
{
    int32_t returnValue = 0;

    createTexture2D(256);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadTexture2DAsync::SaveLoadTexture2DAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadTexture2DAsync")
{}

int32_t SaveLoadTexture2DAsync::run_loop()
{
    int32_t returnValue = 0;

    createTexture2D(256);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyTexture3D::CreateDestroyTexture3D(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyTexture3D")
{
}

int32_t CreateDestroyTexture3D::run_loop()
{
    createTexture3D(32);
    destroyResource();
    return 0;
}

SaveTexture3D::SaveTexture3D(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveTexture3D")
{}

int32_t SaveTexture3D::run_pre()
{
    createTexture3D(32);
    return 0;
}

int32_t SaveTexture3D::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadTexture3D::LoadTexture3D(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadTexture3D")
{}

int32_t LoadTexture3D::run_pre()
{
    createTexture3D(32);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadTexture3D::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadTexture3DAsync::LoadTexture3DAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadTexture3DAsync")
{}

int32_t LoadTexture3DAsync::run_pre()
{
    createTexture3D(32);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadTexture3DAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadTexture3D::SaveLoadTexture3D(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadTexture3D")
{}

int32_t SaveLoadTexture3D::run_loop()
{
    int32_t returnValue = 0;

    createTexture3D(32);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadTexture3DAsync::SaveLoadTexture3DAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadTexture3DAsync")
{}

int32_t SaveLoadTexture3DAsync::run_loop()
{
    int32_t returnValue = 0;

    createTexture3D(32);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyTextureCube::CreateDestroyTextureCube(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyTextureCube")
{
}

int32_t CreateDestroyTextureCube::run_loop()
{
    createTextureCube(128);
    destroyResource();
    return 0;
}

SaveTextureCube::SaveTextureCube(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveTextureCube")
{}

int32_t SaveTextureCube::run_pre()
{
    createTextureCube(128);
    return 0;
}

int32_t SaveTextureCube::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadTextureCube::LoadTextureCube(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadTextureCube")
{}

int32_t LoadTextureCube::run_pre()
{
    createTextureCube(128);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadTextureCube::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadTextureCubeAsync::LoadTextureCubeAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadTextureCubeAsync")
{}

int32_t LoadTextureCubeAsync::run_pre()
{
    createTextureCube(128);
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadTextureCubeAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadTextureCube::SaveLoadTextureCube(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadTextureCube")
{}

int32_t SaveLoadTextureCube::run_loop()
{
    int32_t returnValue = 0;

    createTextureCube(128);
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadTextureCubeAsync::SaveLoadTextureCubeAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadTextureCubeAsync")
{}

int32_t SaveLoadTextureCubeAsync::run_loop()
{
    int32_t returnValue = 0;

    createTextureCube(128);
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

CreateDestroyEffect::CreateDestroyEffect(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_CreateDestroyEffect")
{
}

int32_t CreateDestroyEffect::run_loop()
{
    createEffect();
    destroyResource();
    return 0;
}

SaveEffect::SaveEffect(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveEffect")
{}

int32_t SaveEffect::run_pre()
{
    createEffect();
    return 0;
}

int32_t SaveEffect::run_loop()
{
    saveResource();
    cleanupResourceFile();

    return 0;
}

LoadEffect::LoadEffect(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadEffect")
{}

int32_t LoadEffect::run_pre()
{
    createEffect();
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadEffect::run_loop()
{
    int32_t returnValue = 0;

    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

LoadEffectAsync::LoadEffectAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_LoadEffectAsync")
{}

int32_t LoadEffectAsync::run_pre()
{
    createEffect();
    saveResource();
    destroyResource();
    return 0;
}

int32_t LoadEffectAsync::run_loop()
{
    int32_t returnValue = 0;

    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();

    return returnValue;
}

SaveLoadEffect::SaveLoadEffect(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadEffect")
{}

int32_t SaveLoadEffect::run_loop()
{
    int32_t returnValue = 0;

    createEffect();
    saveResource();
    destroyResource();
    loadResource();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}

SaveLoadEffectAsync::SaveLoadEffectAsync(int32_t argc, const char* argv[])
: ResourceStressTestBase(argc, argv, "ETest_SaveLoadEffectAsync")
{}

int32_t SaveLoadEffectAsync::run_loop()
{
    int32_t returnValue = 0;

    createEffect();
    saveResource();
    destroyResource();
    loadResourceAsync();
    returnValue = (m_resource) ? 0 : -1;
    destroyResource();
    cleanupResourceFile();

    return returnValue;
}
