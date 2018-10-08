//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTSTRESSTESTS_RESOURCESTRESSTESTS_H
#define RAMSES_CLIENTSTRESSTESTS_RESOURCESTRESSTESTS_H

#include "StressTest.h"

namespace ramses
{
    class ResourceFileDescription;
    class Resource;
}

namespace ramses_internal
{
    class ResourceStressTestBase : public StressTest
    {
    public:
        ResourceStressTestBase(int32_t argc, const char* argv[], const ramses_internal::String& name);
        virtual ~ResourceStressTestBase() override;

    protected:
        void destroyResource();
        void cleanupResourceFile();
        void cleanupResourceFile(const ramses::ResourceFileDescription& fileDescription);
        void saveResource(const ramses::Resource* resource = nullptr);
        void saveResource(ramses::ResourceFileDescription fileDescription, const ramses::Resource* resource = nullptr);
        void loadResource(ramses::resourceId_t id = ramses::InvalidResourceId);
        void loadResource(const ramses::ResourceFileDescription& fileDescription, ramses::resourceId_t id = ramses::InvalidResourceId);
        void loadResourceAsync(ramses::resourceId_t id = ramses::InvalidResourceId);
        void loadResourceAsync(const ramses::ResourceFileDescription& fileDescription, ramses::resourceId_t id = ramses::InvalidResourceId);

        void createFloatArray(uint32_t size);
        void createVector2fArray(uint32_t size);
        void createVector3fArray(uint32_t size);
        void createVector4fArray(uint32_t size);
        void createUInt16Array(uint32_t size);
        void createUInt32Array(uint32_t size);
        void createTexture2D(uint32_t size);
        void createTexture3D(uint32_t size);
        void createTextureCube(uint32_t size);
        void createEffect();

        ramses::ResourceFileDescription createFileDescription();
        uint32_t roundUpToPowOf2(uint32_t value);

        const ramses::Resource*       m_resource     = nullptr;
        ramses::resourceId_t          m_resourceId   = ramses::InvalidResourceId;
        const ramses_internal::String m_testBaseName = "ResourceStressTest-";

    private:
        template< typename ArrayType >
        void populateArray(ArrayType* array, uint32_t numberOfElements);
    };

    class CreateDestroyFloatArray : public ResourceStressTestBase
    {
    public:
        CreateDestroyFloatArray(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveFloatArray : public ResourceStressTestBase
    {
    public:
        SaveFloatArray(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadFloatArray : public ResourceStressTestBase
    {
    public:
        LoadFloatArray(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadFloatArrayAsync : public ResourceStressTestBase
    {
    public:
        LoadFloatArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadFloatArray : public ResourceStressTestBase
    {
    public:
        SaveLoadFloatArray(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadFloatArrayAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadFloatArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyVector2fArray : public ResourceStressTestBase
    {
    public:
        CreateDestroyVector2fArray(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveVector2fArray : public ResourceStressTestBase
    {
    public:
        SaveVector2fArray(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadVector2fArray : public ResourceStressTestBase
    {
    public:
        LoadVector2fArray(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadVector2fArrayAsync : public ResourceStressTestBase
    {
    public:
        LoadVector2fArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadVector2fArray : public ResourceStressTestBase
    {
    public:
        SaveLoadVector2fArray(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadVector2fArrayAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadVector2fArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyVector3fArray : public ResourceStressTestBase
    {
    public:
        CreateDestroyVector3fArray(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveVector3fArray : public ResourceStressTestBase
    {
    public:
        SaveVector3fArray(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadVector3fArray : public ResourceStressTestBase
    {
    public:
        LoadVector3fArray(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadVector3fArrayAsync : public ResourceStressTestBase
    {
    public:
        LoadVector3fArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadVector3fArray : public ResourceStressTestBase
    {
    public:
        SaveLoadVector3fArray(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadVector3fArrayAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadVector3fArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyVector4fArray : public ResourceStressTestBase
    {
    public:
        CreateDestroyVector4fArray(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveVector4fArray : public ResourceStressTestBase
    {
    public:
        SaveVector4fArray(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadVector4fArray : public ResourceStressTestBase
    {
    public:
        LoadVector4fArray(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadVector4fArrayAsync : public ResourceStressTestBase
    {
    public:
        LoadVector4fArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadVector4fArray : public ResourceStressTestBase
    {
    public:
        SaveLoadVector4fArray(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadVector4fArrayAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadVector4fArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyUInt16Array : public ResourceStressTestBase
    {
    public:
        CreateDestroyUInt16Array(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveUInt16Array : public ResourceStressTestBase
    {
    public:
        SaveUInt16Array(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadUInt16Array : public ResourceStressTestBase
    {
    public:
        LoadUInt16Array(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadUInt16ArrayAsync : public ResourceStressTestBase
    {
    public:
        LoadUInt16ArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadUInt16Array : public ResourceStressTestBase
    {
    public:
        SaveLoadUInt16Array(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadUInt16ArrayAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadUInt16ArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyUInt32Array : public ResourceStressTestBase
    {
    public:
        CreateDestroyUInt32Array(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveUInt32Array : public ResourceStressTestBase
    {
    public:
        SaveUInt32Array(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadUInt32Array : public ResourceStressTestBase
    {
    public:
        LoadUInt32Array(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadUInt32ArrayAsync : public ResourceStressTestBase
    {
    public:
        LoadUInt32ArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadUInt32Array : public ResourceStressTestBase
    {
    public:
        SaveLoadUInt32Array(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadUInt32ArrayAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadUInt32ArrayAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyTexture2D : public ResourceStressTestBase
    {
    public:
        CreateDestroyTexture2D(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveTexture2D : public ResourceStressTestBase
    {
    public:
        SaveTexture2D(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadTexture2D : public ResourceStressTestBase
    {
    public:
        LoadTexture2D(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadTexture2DAsync : public ResourceStressTestBase
    {
    public:
        LoadTexture2DAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadTexture2D : public ResourceStressTestBase
    {
    public:
        SaveLoadTexture2D(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadTexture2DAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadTexture2DAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyTexture3D : public ResourceStressTestBase
    {
    public:
        CreateDestroyTexture3D(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveTexture3D : public ResourceStressTestBase
    {
    public:
        SaveTexture3D(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadTexture3D : public ResourceStressTestBase
    {
    public:
        LoadTexture3D(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadTexture3DAsync : public ResourceStressTestBase
    {
    public:
        LoadTexture3DAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadTexture3D : public ResourceStressTestBase
    {
    public:
        SaveLoadTexture3D(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadTexture3DAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadTexture3DAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyTextureCube : public ResourceStressTestBase
    {
    public:
        CreateDestroyTextureCube(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveTextureCube : public ResourceStressTestBase
    {
    public:
        SaveTextureCube(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadTextureCube : public ResourceStressTestBase
    {
    public:
        LoadTextureCube(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadTextureCubeAsync : public ResourceStressTestBase
    {
    public:
        LoadTextureCubeAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadTextureCube : public ResourceStressTestBase
    {
    public:
        SaveLoadTextureCube(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadTextureCubeAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadTextureCubeAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class CreateDestroyEffect : public ResourceStressTestBase
    {
    public:
        CreateDestroyEffect(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveEffect : public ResourceStressTestBase
    {
    public:
        SaveEffect(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadEffect : public ResourceStressTestBase
    {
    public:
        LoadEffect(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class LoadEffectAsync : public ResourceStressTestBase
    {
    public:
        LoadEffectAsync(int32_t argc, const char* argv[]);
        int32_t run_pre() override;
        int32_t run_loop() override;
    };

    class SaveLoadEffect : public ResourceStressTestBase
    {
    public:
        SaveLoadEffect(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };

    class SaveLoadEffectAsync : public ResourceStressTestBase
    {
    public:
        SaveLoadEffectAsync(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };
}

#endif
