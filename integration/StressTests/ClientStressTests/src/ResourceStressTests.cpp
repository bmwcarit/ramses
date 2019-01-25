//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceStressTests.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/Texture2D.h"
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
    if (m_resource)
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

void ResourceStressTestBase::createVector4fArray(uint32_t size)
{
    auto data =  std::unique_ptr<float[]>(new float[size*4]);
    populateArray(data.get(),size*4);
    m_resource = m_client->createConstVector4fArray(size,data.get());
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
