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

        void createVector4fArray(uint32_t size);
        void createTexture2D(uint32_t size);
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
