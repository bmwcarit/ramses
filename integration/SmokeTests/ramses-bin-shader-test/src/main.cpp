//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/BinaryShaderCache.h"
#include "ramses-renderer-api/IBinaryShaderCache.h"
#include "ramses-renderer-api/IRendererEventHandler.h"

#include "ramses-framework-api/RamsesFramework.h"

#include "TestScenes/TransformationLinkScene.h"
#include "RamsesFrameworkImpl.h"
#include "Utils/Argument.h"
#include "Ramsh/RamshCommandExit.h"
#include "RendererAPI/Types.h"
#include "Math3d/Vector3.h"
#include "Collections/StringOutputStream.h"

#include <sstream>

class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty
{
public:

    SceneStateEventHandler(ramses::RamsesRenderer& renderer)
        : m_renderer(renderer)
    {
    }

    virtual void scenePublished(ramses::sceneId_t sceneId) override
    {
        m_publishedScenes.put(sceneId);
    }

    virtual void sceneUnpublished(ramses::sceneId_t sceneId) override
    {
        m_publishedScenes.remove(sceneId);
    }

    virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_subscribedScenes.put(sceneId);
        }
    }

    virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_subscribedScenes.remove(sceneId);
        }
    }

    virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_mappedScenes.put(sceneId);
        }
    }

    virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_mappedScenes.remove(sceneId);
        }
    }

    virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_shownScenes.put(sceneId);
        }
    }

    virtual void sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_shownScenes.remove(sceneId);
        }
    }

    void waitForPublication(const ramses::sceneId_t sceneId)
    {
        waitForSceneInSet(sceneId, m_publishedScenes);
    }

    void waitForSubscription(const ramses::sceneId_t sceneId)
    {
        waitForSceneInSet(sceneId, m_subscribedScenes);
    }

    void waitForMapped(const ramses::sceneId_t sceneId)
    {
        waitForSceneInSet(sceneId, m_mappedScenes);
    }

    void waitForShown(const ramses::sceneId_t sceneId)
    {
        waitForSceneInSet(sceneId, m_shownScenes);
    }

private:
    typedef ramses_capu::HashSet<ramses::sceneId_t> SceneSet;

    void waitForSceneInSet(const ramses::sceneId_t sceneId, const SceneSet& sceneSet)
    {
        while (!sceneSet.hasElement(sceneId))
        {
            m_renderer.doOneLoop();
            m_renderer.dispatchEvents(*this);
        }
    }

    SceneSet m_publishedScenes;
    SceneSet m_subscribedScenes;
    SceneSet m_mappedScenes;
    SceneSet m_shownScenes;

    ramses::RamsesRenderer& m_renderer;
};

static ramses_internal::String EffectIdToString(ramses::effectId_t effectId)
{
    ramses_internal::ResourceContentHash effectHash(effectId.lowPart, effectId.highPart);
    return ramses_internal::StringUtils::HexFromResourceContentHash(effectHash);
}

ramses_internal::StringOutputStream& operator<< (ramses_internal::StringOutputStream& outputStream, const ramses::effectId_t& id)
{
    return outputStream << EffectIdToString(id);
}

enum EBinaryShaderRequestStatus
{
    EBinaryShaderRequestStatus_NeverRequested = 0,
    EBinaryShaderRequestStatus_RequestedWithWrongEffectId,
    EBinaryShaderRequestStatus_ExistenceRequested,
    EBinaryShaderRequestStatus_SizeRequested,
    EBinaryShaderRequestStatus_FormatRequested,
    EBinaryShaderRequestStatus_DataRequested
};

class TestBinaryShaderCache : public ramses::IBinaryShaderCache
{
public:
    TestBinaryShaderCache()
    : m_binaryShaderRequestStatus(EBinaryShaderRequestStatus_NeverRequested)
    {

    }

    void init(const ramses_internal::String& effectIdFile, const ramses_internal::String& binaryShaderFile)
    {
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Parsing binary shader cache files...");

        loadEffectID(effectIdFile);
        loadBinaryShaderData(binaryShaderFile);
        loadBinaryShaderFormat(binaryShaderFile + ramses_internal::String(".bf"));
    }

    void loadEffectID(const ramses_internal::String& effectIdFile)
    {
        ramses_internal::File file(effectIdFile);
        assert(file.exists());

        file.open(ramses_internal::EFileMode_ReadOnly);
        ramses_internal::BinaryFileInputStream inputStream(file);

        bool finishedParsing = false;
        while (!finishedParsing)
        {
            ramses_internal::Char character = 0;
            inputStream.read(&character, 1);

            if (character == '\n')
            {
                finishedParsing = true;
            }
            else
            {
                m_effectIDAsString.append(ramses_internal::String(1, character));
            }
        }

        file.close();
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Parsed effect id for binary shader cache: " << m_effectIDAsString);
    }

    void loadBinaryShaderData(const ramses_internal::String& binaryShaderFile)
    {
        ramses_internal::File file(binaryShaderFile);
        assert(file.exists());

        ramses_internal::UInt fileSize = 0u;
        file.getSizeInBytes(fileSize);

        ramses_internal::BinaryFileInputStream inputStream(file);
        m_binaryShaderData.resize(fileSize);
        inputStream.read(reinterpret_cast<ramses_internal::Char*>(&m_binaryShaderData.front()), static_cast<ramses_internal::UInt32>(fileSize));

        file.close();
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Parsed binary shader data, bytes read: " << fileSize);
    }

    void loadBinaryShaderFormat(const ramses_internal::String& binaryShaderFormatFile)
    {
        ramses_internal::File file(binaryShaderFormatFile);
        assert(file.exists());

        ramses_internal::BinaryFileInputStream inputStream(file);
        inputStream >> m_binaryShaderFormat;

        file.close();
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Parsed binary shader format: " << m_binaryShaderFormat);
    }

    virtual bool hasBinaryShader(ramses::effectId_t effectId) const override
    {
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Binary shader requested: " << effectId);
        if(m_effectIDAsString != EffectIdToString(effectId))
        {
            m_binaryShaderRequestStatus = EBinaryShaderRequestStatus_RequestedWithWrongEffectId;
            LOG_ERROR(ramses_internal::CONTEXT_SMOKETEST,
                      "Binary shader requested, but with the wrong effect id." <<
                      "Expected id: " << m_effectIDAsString <<
                      "; Provided id: " << effectId <<
                      "; Provided id converted to string: " << EffectIdToString(effectId));
        }
        else
        {
            m_binaryShaderRequestStatus = EBinaryShaderRequestStatus_ExistenceRequested;
        }

        return m_effectIDAsString == EffectIdToString(effectId);
    }

    virtual uint32_t getBinaryShaderSize(ramses::effectId_t effectId) const override
    {
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Binary shader size requested: " << effectId);
        assert(m_effectIDAsString == EffectIdToString(effectId));
        assert(m_binaryShaderRequestStatus == EBinaryShaderRequestStatus_ExistenceRequested);
        m_binaryShaderRequestStatus = EBinaryShaderRequestStatus_SizeRequested;

        return static_cast<uint32_t>(m_binaryShaderData.size());
    }

    virtual uint32_t getBinaryShaderFormat(ramses::effectId_t effectId) const override
    {
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Binary shader format requested: " << effectId);
        assert(m_effectIDAsString == EffectIdToString(effectId));
        assert(m_binaryShaderRequestStatus == EBinaryShaderRequestStatus_SizeRequested);
        m_binaryShaderRequestStatus = EBinaryShaderRequestStatus_FormatRequested;

        return m_binaryShaderFormat;
    }

    virtual bool shouldBinaryShaderBeCached(ramses::effectId_t /*effectId*/) const override
    {
        return true;
    }

    virtual void getBinaryShaderData(ramses::effectId_t effectId, uint8_t* buffer, uint32_t bufferSize) const override
    {
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Binary shader data requested: " << effectId);
        assert(m_effectIDAsString == EffectIdToString(effectId));
        assert(m_binaryShaderRequestStatus == EBinaryShaderRequestStatus_FormatRequested);
        m_binaryShaderRequestStatus = EBinaryShaderRequestStatus_DataRequested;

        const uint32_t dataSize = static_cast<uint32_t>(m_binaryShaderData.size());
        assert(bufferSize >= dataSize);
        UNUSED(bufferSize)

        ramses_internal::PlatformMemory::Copy(buffer, &m_binaryShaderData.front(), dataSize);
    }

    virtual void storeBinaryShader(ramses::effectId_t effectId, const uint8_t*, uint32_t, uint32_t) override
    {
        LOG_ERROR(ramses_internal::CONTEXT_SMOKETEST, "Trying to store binary shader, should never happen in this test!! Effect id: " << effectId);
        assert(false);
    }

    virtual void binaryShaderUploaded(ramses::effectId_t effectId, bool success) const override
    {
        UNUSED(effectId);
        UNUSED(success);
    }

    EBinaryShaderRequestStatus getBinaryShaderRequestedStatus() const
    {
        return m_binaryShaderRequestStatus;
    }

private:
    mutable EBinaryShaderRequestStatus m_binaryShaderRequestStatus;
    ramses_internal::String m_effectIDAsString;
    ramses_internal::UInt8Vector m_binaryShaderData;
    uint32_t m_binaryShaderFormat;
};

bool getInputParameters(int argc, char** argv, ramses_internal::String& effectIdFileValue, ramses_internal::String& binaryShaderFileValue)
{
    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentString effectIdFile(parser, "e", "effectIdFile", "");
    ramses_internal::ArgumentString binaryShaderFile(parser, "b", "binaryShaderFile", "");

    effectIdFileValue = ramses_internal::String(effectIdFile);
    binaryShaderFileValue = ramses_internal::String(binaryShaderFile);

    if (effectIdFileValue == ramses_internal::String(""))
    {
        LOG_ERROR(ramses_internal::CONTEXT_SMOKETEST, "Effect ID file is not provided!\n");
        return false;
    }

    if (binaryShaderFileValue == ramses_internal::String(""))
    {
        LOG_ERROR(ramses_internal::CONTEXT_SMOKETEST, "Binary Shader file is not provided!\n");
        return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
    ramses_internal::String effectIDFile;
    ramses_internal::String binaryShaderFile;
    if (!getInputParameters(argc, argv, effectIDFile, binaryShaderFile))
    {
        return -1;
    }

    // create ramses framework
    ramses::RamsesFrameworkConfig frameworkConfig(argc, argv);
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RamsesFramework framework(frameworkConfig);

    ramses_internal::RamshCommandExit commandExit;
    framework.impl.getRamsh().add(commandExit);

    // create ramses client
    ramses::RamsesClient client("ramses-local-client-test", framework);

    // create ramses renderer
    ramses::RendererConfig rendererConfig(argc, argv);
    TestBinaryShaderCache testBinaryShader;
    rendererConfig.setBinaryShaderCache(testBinaryShader);
    ramses::RamsesRenderer renderer(framework, rendererConfig);

    SceneStateEventHandler eventHandler(renderer);

    // Delayed initialization, because CONTEXT_SMOKETEST is not enabled before.
    testBinaryShader.init(effectIDFile, binaryShaderFile);

    framework.connect();

    // create display
    ramses::DisplayConfig displayConfig(argc, argv);
    ramses::displayId_t display = renderer.createDisplay(displayConfig);

    // create the test scene
    const ramses::sceneId_t sceneId = 42u;
    ramses::Scene* clientScene = client.createScene(sceneId);
    ramses_internal::TransformationLinkScene redTriangleScene(client, *clientScene, ramses_internal::TransformationLinkScene::TRANSFORMATION_CONSUMER, ramses_internal::Vector3(0.0f, 0.0f, 12.0f));
    clientScene->flush();
    clientScene->publish(ramses::EScenePublicationMode_LocalOnly);

    eventHandler.waitForPublication(sceneId);

    renderer.subscribeScene(sceneId);
    renderer.flush();
    eventHandler.waitForSubscription(sceneId);

    renderer.mapScene(display, sceneId);
    renderer.flush();
    eventHandler.waitForMapped(sceneId);

    renderer.showScene(sceneId);
    renderer.flush();
    eventHandler.waitForShown(sceneId);

    LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "ramses-bin-shader-test shows scene");

    while (!commandExit.exitRequested())
    {
        renderer.doOneLoop();
    }

    if (EBinaryShaderRequestStatus_DataRequested != testBinaryShader.getBinaryShaderRequestedStatus())
    {
        LOG_ERROR(ramses_internal::CONTEXT_SMOKETEST, "Binary shader was not requested properly! Expected status " << EBinaryShaderRequestStatus_DataRequested << ", actual status: " << testBinaryShader.getBinaryShaderRequestedStatus());
        return -1;
    }

    LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "ramses-bin-shader-test ran through");

    return 0;
}
