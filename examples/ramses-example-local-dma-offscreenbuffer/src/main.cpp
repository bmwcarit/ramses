//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-utils.h"
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <cstring>

#include <sys/mman.h>
#include <unistd.h>
#include "linux/dma-buf.h"
#include "linux/ioctl.h"
#include <sys/ioctl.h>
#include <drm_fourcc.h>
#include <gbm.h>

/**
 * @example ramses-example-local-dma-offscreenbuffer/src/main.cpp
 * @brief DMA Offscreen Buffer Example
 * @details The example demonstrates a realistic usecase of DMA offscreen buffers,
 *          where a stream texture provided by a wayland client to the embedded compositor
 *          gets processed on CPU side and the result image is calculated on CPU.
 *
 *          The example creates the following setup:
 *          A renderer with four DMA OBs and two local scenes:
 *          * Two OBs are used for "reading" input coming to embedded compositor from wayland client
 *          * Two OBs are used for "writing" output of processing the contents of the "reading OBs"
 *          * The reason two buffers are created for both reading and writing, is to allow double buffering input and output
 *          * Main scene has two quads with textures:
 *              * left quad/texture: a stream texture that consumes stream id 789 and shows it as is
 *              * right quad/texture: result of "processing" OBs, linked to the texture
 *              * This scene is mapped to the framebuffer, i.e., rendered to display
 *          * Source scene:
 *              * Has a quad filling whole viewport that renders the input stream texture coming to embedded compositor on stream 789
 *              * This scene gets mapped to the "read OBs", alternating between them every loop
 *          * The example does some "magic" processing to the input, just for demo purpose
 *
 *          The example additionally demonstates the following points :
 *          * How to make the necessary calls to ioctl for syncing. See startBufferAccess() and endBufferAccess()
 *          * How to double buffer reading and writing to DMA OBs to avoid stalling both image processing and rendering
 */

//parameters to control the example
static constexpr uint32_t OffscreenBufferWidth = 200u;
static constexpr uint32_t OffscreenBufferHeight = 200u;
static constexpr uint32_t DisplayWidth = 800u;
static constexpr uint32_t DisplayHeight = 800u;
static constexpr uint32_t DmaBufferFourccFormat = DRM_FORMAT_ARGB8888;
static constexpr std::chrono::seconds ExampleRunDuration{ 60u };
static constexpr bool ExampleTakeScreenshots = false;

constexpr const char* const vertexShader = R"##(
#version 300 es

in lowp vec3 a_position;
in lowp vec2 a_texcoord;
out lowp vec2 v_texcoord;
uniform lowp mat4 mvpMatrix;
void main()
{
    v_texcoord = a_texcoord;
    gl_Position = mvpMatrix * vec4(a_position, 1.0);
}
)##";

constexpr const char* const fragmentShader = R"##(
#version 300 es

uniform sampler2D textureSampler;
in lowp vec2 v_texcoord;
out lowp vec4 fragColor;
void main(void)
{
    fragColor = texture(textureSampler, v_texcoord);
}
)##";

/** \cond HIDDEN_SYMBOLS */

//Helper class to simplify handling renderer events.
//The class blocks queries waiting for events and loops until expected event is reached or timesout
class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    explicit SceneStateEventHandler(ramses::RamsesRenderer& renderer)
        : m_renderer(renderer)
    {
    }

    void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, ramses::displayId_t, ramses::displayBufferId_t, ramses::ERendererEventResult result) override
    {
        if(result == ramses::ERendererEventResult_OK)
        {
            static uint32_t filePostifx = 0u;
            const std::string fileName = "./dmaBufExampleScreenshot_" + std::to_string(filePostifx++) + ".png";
            if(!ramses::RamsesUtils::SaveImageBufferToPng(fileName, {pixelData, pixelData + pixelDataSize}, DisplayWidth, DisplayHeight))
                printf("Error: Failed saving screenshot to %s !!\n", fileName.c_str());
            else
                printf("Saved screenshot to %s\n", fileName.c_str());
        }
        else
            printf("Error: Failed read pixels!!\n");
    }

    void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override
    {
        m_scenes[sceneId].state = state;
    }

    void displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_createdDisplays.insert(displayId);
        }
    }

    void offscreenBufferCreated(ramses::displayId_t, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_createdOffscreenBuffers.insert(offscreenBufferId);
        }
    }

    void offscreenBufferLinked(ramses::displayBufferId_t, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t, bool success) override
    {
        if (success)
        {
            m_scenesConsumingOffscreenBuffer[consumerScene].state = ramses::RendererSceneState::Unavailable;
        }
    }

    void waitForDisplayCreated(const ramses::displayId_t displayId)
    {
        waitForElementInSet(displayId, m_createdDisplays);
    }

    void waitForOffscreenBufferCreated(const ramses::displayBufferId_t offscreenBufferId)
    {
        waitForElementInSet(offscreenBufferId, m_createdOffscreenBuffers);
    }

    void waitForOffscreenBufferLinkedToConsumingScene(const ramses::sceneId_t sceneId)
    {
        waitUntilOrTimeout([&] {return m_scenesConsumingOffscreenBuffer.count(sceneId) > 0; });
    }

    void waitForSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        waitUntilOrTimeout([&] { return m_scenes[sceneId].state == state; });
    }

    bool waitUntilOrTimeout(const std::function<bool()>& conditionFunction)
    {
        const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{ 5 };
        while (!conditionFunction() && std::chrono::steady_clock::now() < timeoutTS)
        {
            m_renderer.doOneLoop();
            m_renderer.dispatchEvents(*this);
            m_renderer.getSceneControlAPI()->dispatchEvents(*this);
        }

        const auto result = conditionFunction();
        if(!result)
            printf("Error: timed out waiting for state/condition!\n");

        return result;
    }

private:
    struct SceneInfo
    {
        ramses::RendererSceneState state = ramses::RendererSceneState::Unavailable;
        ramses::sceneVersionTag_t version = ramses::InvalidSceneVersionTag;
    };

    using SceneSet = std::unordered_map<ramses::sceneId_t, SceneInfo>;
    using DataConsumerSet = std::unordered_set<ramses::dataConsumerId_t>;
    using DisplaySet = std::unordered_set<ramses::displayId_t>;
    using OffscreenBufferSet = std::unordered_set<ramses::displayBufferId_t>;

    template <typename T>
    void waitForElementInSet(const T element, const std::unordered_set<T>& set)
    {
        while (set.find(element) == set.end())
        {
            m_renderer.doOneLoop();
            m_renderer.dispatchEvents(*this);
            m_renderer.getSceneControlAPI()->dispatchEvents(*this);
            std::this_thread::sleep_for(std::chrono::milliseconds(10u));
        }
    }

    SceneSet m_scenes;
    SceneSet m_scenesAssignedToOffscreenBuffer;
    SceneSet m_scenesConsumingOffscreenBuffer;
    DisplaySet m_createdDisplays;
    OffscreenBufferSet m_createdOffscreenBuffers;
    DataConsumerSet m_dataConsumers;
    ramses::RamsesRenderer& m_renderer;
};
/** \endcond */

ramses::Effect& createEffect(ramses::Scene& scene, const std::string& effectName)
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShader(vertexShader);
    effectDesc.setFragmentShader(fragmentShader);
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    return *scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, effectName.c_str());
}

ramses::MeshNode& createQuadWithTexture(ramses::Scene& scene, ramses::Effect& effect, ramses::TextureSampler& textureSampler)
{
    const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
    const ramses::ArrayResource* indices = scene.createArrayResource(6u, indicesArray);

    const std::array<ramses::vec3f, 4u> vertexPositionsArray
    {
        ramses::vec3f{ -1.f, -1.f, 0.f },
        ramses::vec3f{ 1.f, -1.f, 0.f },
        ramses::vec3f{ -1.f, 1.f, 0.f },
        ramses::vec3f{ 1.f, 1.f, 0.f }
    };
    const ramses::ArrayResource* vertexPositions = scene.createArrayResource(4u, vertexPositionsArray.data());

    const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f}, ramses::vec2f{1.f, 0.f} };
    const ramses::ArrayResource* textureCoords = scene.createArrayResource(4u, textureCoordsArray.data());

    ramses::Appearance* appearance = scene.createAppearance(effect, "quad appearance");
    ramses::GeometryBinding* geometry = scene.createGeometryBinding(effect, "quad geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect.findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);
    ramses::AttributeInput texCoordsInput;
    effect.findAttributeInput("a_texcoord", texCoordsInput);
    geometry->setInputBuffer(texCoordsInput, *textureCoords);

    ramses::MeshNode* meshNode = scene.createMeshNode();
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);

    ramses::UniformInput textureInput;
    effect.findUniformInput("textureSampler", textureInput);
    appearance->setInputTexture(textureInput, textureSampler);

    return *meshNode;
}

ramses::Scene& createMainScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const std::string& processingOutputSamplerName)
{
    //scene consists of two quads with textures
    //the left quad is stream tex input rendered as is
    //the right quad is output result from processing input (from two frames ago)
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "main scene");

    ramses::OrthographicCamera* camera = clientScene->createOrthographicCamera("main scene camera");
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    camera->setFrustum(-2.f, 2.f, -2.f, 2.f, 0.1f, 100.f);
    camera->setViewport(0, 0u, DisplayWidth, DisplayHeight);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("main scene render pass");

    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    ramses::Effect& effect = createEffect(*clientScene, "quad effect main scene");

    // Create fallback texture to show when sampler not linked to an offscreen buffer
    ramses::Texture2D& fallbackTexture = *ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-dma-offscreenbuffer-fallback.png", *clientScene);
    ramses::TextureSampler& streamtextureSampler = *clientScene->createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                                                                ramses::ETextureAddressMode_Repeat,
                                                                                ramses::ETextureSamplingMethod_Linear,
                                                                                ramses::ETextureSamplingMethod_Linear,
                                                                                fallbackTexture,
                                                                                1u,
                                                                                "mainSceneStreamTexSampler");

    auto& meshCompositedTexture = createQuadWithTexture(*clientScene, effect, streamtextureSampler);
    meshCompositedTexture.setTranslation({-1.f, 0.f, 0.f});
    renderGroup->addMeshNode(meshCompositedTexture);

    ramses::TextureSampler& outputTextureSampler = *clientScene->createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                                                                ramses::ETextureAddressMode_Repeat,
                                                                                ramses::ETextureSamplingMethod_Linear,
                                                                                ramses::ETextureSamplingMethod_Linear,
                                                                                fallbackTexture,
                                                                                1u,
                                                                                processingOutputSamplerName.c_str());

    auto& meshOutputTexture = createQuadWithTexture(*clientScene, effect, outputTextureSampler);
    meshOutputTexture.setTranslation({1.f, 0.f, 0.f});
    renderGroup->addMeshNode(meshOutputTexture);

    return *clientScene;
}

ramses::Scene& createSourceScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    //scene consists of one quad with a the stream texture that fills whole OB where scene is rendered
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "source scene");

    ramses::OrthographicCamera* camera = clientScene->createOrthographicCamera("source scene camera");
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
    camera->setViewport(0, 0u, OffscreenBufferWidth, OffscreenBufferHeight);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("source scene pass");

    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    ramses::Effect& effect = createEffect(*clientScene, "quad effect source scene");

    // Create fallback texture to show when sampler not linked to an offscreen buffer
    ramses::Texture2D& fallbackTexture = *ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-dma-offscreenbuffer-fallback.png", *clientScene);
    ramses::TextureSampler& streamtextureSampler = *clientScene->createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                                                               ramses::ETextureAddressMode_Repeat,
                                                                               ramses::ETextureSamplingMethod_Linear,
                                                                               ramses::ETextureSamplingMethod_Linear,
                                                                               fallbackTexture,
                                                                               1u,
                                                                               "source stream tex sampler");

    auto& meshCompositedTexture = createQuadWithTexture(*clientScene, effect, streamtextureSampler);
    renderGroup->addMeshNode(meshCompositedTexture);

    return *clientScene;
}

struct BufferInfo
{
    int fd = -1;
    uint32_t stride = 0u;
    std::size_t bufferSize = 0u;
    void* mappedMem = nullptr;
};
std::unordered_map<ramses::displayBufferId_t, BufferInfo> bufferInfoMap;

ramses::displayBufferId_t createDmaOffscreenBuffer(ramses::RamsesRenderer& renderer, ramses::displayId_t display)
{
    constexpr uint32_t usageFlags = GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT;
    constexpr uint64_t modifier = DRM_FORMAT_MOD_INVALID;
    const auto bufferId = renderer.createDmaOffscreenBuffer(display, OffscreenBufferWidth, OffscreenBufferHeight, DmaBufferFourccFormat, usageFlags, modifier);

    if(!bufferId.isValid())
        printf("Error: Failed creating offscreen buffer!\n");

    return bufferId;
}

bool mapDmaOffscreenBuffer(ramses::RamsesRenderer& renderer, ramses::displayId_t display, ramses::displayBufferId_t displayBuffer, bool readyOnly)
{
    errno = 0;
    if(bufferInfoMap.find(displayBuffer) != bufferInfoMap.cend())
    {
        printf("Error: Buffer already mapped!\n");
        return false;
    }

    int bufferFD = -1;
    uint32_t bufferStride = 0;
    if(renderer.getDmaOffscreenBufferFDAndStride(display, displayBuffer, bufferFD, bufferStride) != ramses::StatusOK)
    {
        printf("Error: Failed to get FD and stride!\n");
        return false;
    }

    const std::size_t fileSize = static_cast<std::size_t>(lseek(bufferFD, 0u, SEEK_END));
    const auto mappingPreviliges = (readyOnly? (PROT_READ) : (PROT_WRITE | PROT_READ));
    void* mappedMemory = mmap(nullptr, fileSize, mappingPreviliges, MAP_SHARED, bufferFD, 0u);
    if(mappedMemory == MAP_FAILED)
    {
        const auto errorValue = errno;
        printf("Error: Failed to map memory (errno=%i, errnoString=%s)!\n", errorValue, strerror(errorValue));
        return false;
    }

    printf("Mapped OB %u with FD %i successfully to %p\n", displayBuffer.getValue(), bufferFD, mappedMemory);

    bufferInfoMap[displayBuffer] = {bufferFD, bufferStride, fileSize, mappedMemory};
    return true;
}

bool startBufferAccess(ramses::displayBufferId_t displayBuffer)
{
    const auto bufferInfoIt = bufferInfoMap.find(displayBuffer);
    if(bufferInfoIt == bufferInfoMap.cend())
    {
        printf("Error: Failed to find buffer for start sync!\n");
        return false;
    }

    dma_buf_sync syncStartFlags = { 0 };
    syncStartFlags.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;

    const auto syncStartResult = ioctl(bufferInfoIt->second.fd, DMA_BUF_IOCTL_SYNC, &syncStartFlags);
    if(syncStartResult != 0)
    {
        printf("Error: Failed to start sync!\n");
        return false;
    }

    return true;
}

bool endBufferAccess(ramses::displayBufferId_t displayBuffer)
{
    const auto bufferInfoIt = bufferInfoMap.find(displayBuffer);
    if(bufferInfoIt == bufferInfoMap.cend())
    {
        printf("Error: Failed to find buffer for end sync!\n");
        return false;
    }

    dma_buf_sync syncEndFlags = { 0 };
    syncEndFlags.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
    const auto syncEndResult = ioctl(bufferInfoIt->second.fd, DMA_BUF_IOCTL_SYNC, &syncEndFlags);
    if(syncEndResult != 0)
    {
        printf("Failed to end sync!\n");
        return false;
    }

    return true;
}


bool doProcessing(ramses::displayBufferId_t inputBuffer, ramses::displayBufferId_t outputBuffer)
{
    const auto& inputBufferInfo = bufferInfoMap.find(inputBuffer)->second;
    const auto& outputBufferInfo = bufferInfoMap.find(outputBuffer)->second;

    const auto bufferStride = inputBufferInfo.stride;

    if(!startBufferAccess(inputBuffer))
        return false;
    if(!startBufferAccess(outputBuffer))
        return false;

    for(uint32_t row = 0u; row < OffscreenBufferHeight; ++row)
    {
        for(uint32_t col = 0u; col < OffscreenBufferWidth; ++col)
        {
            uint8_t* inputPixelMem  = reinterpret_cast<uint8_t*>(inputBufferInfo.mappedMem)  + row * bufferStride + col * 4u;
            uint8_t* outputPixelMem = reinterpret_cast<uint8_t*>(outputBufferInfo.mappedMem) + row * bufferStride + col * 4u;

            //Magic values for demonestrating noticeable change in several color channels
            const float     magicValue1     = 1.f * col / OffscreenBufferWidth;
            const uint16_t  magicValue2     = uint8_t(255.f * col / OffscreenBufferWidth);
            const uint8_t   magicValue3     = uint8_t(255.f * row / OffscreenBufferHeight);

            switch (DmaBufferFourccFormat)
            {
            case DRM_FORMAT_ARGB8888: //ARGB little endian
                outputPixelMem[0] = inputPixelMem[0];                                                   //Blue channel
                outputPixelMem[1] = std::min<uint8_t>(uint8_t(magicValue1 * inputPixelMem[1]), 255u);   //Green channel
                outputPixelMem[2] = uint8_t(std::min<uint16_t>(magicValue2 + inputPixelMem[2], 255u));  //Red channel
                outputPixelMem[3] = magicValue3;                                                        //Alpha channel
                break;
            default:
                printf("Error: Unsupported format!\n");
                return false;
            }
        }
    }

    if(!endBufferAccess(inputBuffer))
        return false;
    if(!endBufferAccess(outputBuffer))
        return  false;

    return true;
}

int main()
{
    ramses::RamsesFrameworkConfig config;
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-example-local-dma-offscreenbuffer"));

    ramses::RendererConfig rendererConfig;

    //Set big enough max frame poll time in order to wait for every frame to finish
    //This is essential to avoid skipping any frames
    rendererConfig.setFrameCallbackMaxPollTime(66000);

    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    auto& sceneControlAPI = *renderer.getSceneControlAPI();
    framework.connect();

    //avoid skipping any frames
    renderer.setSkippingOfUnmodifiedBuffers(false);

    SceneStateEventHandler eventHandler(renderer);

    //Create display and OBs
    ramses::DisplayConfig displayConfig;
    displayConfig.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);
    displayConfig.setPlatformRenderNode("/dev/dri/renderD128");
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();
    eventHandler.waitForDisplayCreated(display);

    //to simulate double buffering using OBs for reading
    const ramses::displayBufferId_t dmaOffscreenBufferRead1 = createDmaOffscreenBuffer(renderer, display);
    const ramses::displayBufferId_t dmaOffscreenBufferRead2 = createDmaOffscreenBuffer(renderer, display);
    ramses::displayBufferId_t backDMAReadBufferId = dmaOffscreenBufferRead1;
    ramses::displayBufferId_t frontDmaReadBufferId = dmaOffscreenBufferRead2;

    //to simulate double/tripple buffering using OBs for writing
    //Note: double buffering for write buffers "might" not be enough depending on image processing latency,
    //because once an OB is linked as input to a texture consumer, two loops must pass before safely writing into that OB again
    //so it is possible to have a constellation where:
    // one OB is needed since it has ready output for next call to doOneLoop (image processing just finished)
    // one OB is needed since it is still in use from previous call to doOneLoop (previous output from CV that is currently rendered)
    // one OB is needed to be passed to image processing for writing
    const ramses::displayBufferId_t dmaOffscreenBufferWrite1 = createDmaOffscreenBuffer(renderer, display);
    const ramses::displayBufferId_t dmaOffscreenBufferWrite2 = createDmaOffscreenBuffer(renderer, display);
    ramses::displayBufferId_t backDMAWriteBufferId = dmaOffscreenBufferWrite1;
    ramses::displayBufferId_t frontDmaWriteBufferId = dmaOffscreenBufferWrite2;

    renderer.flush();
    eventHandler.waitForOffscreenBufferCreated(dmaOffscreenBufferRead1);
    eventHandler.waitForOffscreenBufferCreated(dmaOffscreenBufferRead2);
    eventHandler.waitForOffscreenBufferCreated(dmaOffscreenBufferWrite1);
    eventHandler.waitForOffscreenBufferCreated(dmaOffscreenBufferWrite2);

    //map DMA OBs once in beginning and keep the mapping as long as the OBs memory
    //need to be accesed on CPU
    if(!mapDmaOffscreenBuffer(renderer, display, dmaOffscreenBufferRead1, true))
        return 1;
    if(!mapDmaOffscreenBuffer(renderer, display, dmaOffscreenBufferRead2, true))
        return 1;
    if(!mapDmaOffscreenBuffer(renderer, display, dmaOffscreenBufferWrite1, false))
       return 1;
    if(!mapDmaOffscreenBuffer(renderer, display, dmaOffscreenBufferWrite2, false))
       return 1;

    printf("\n************* All OBs mapped successfully !! **********\n");

    //disable clearing for OBs in order to control when does content get overwritten
    //for example, the content of the write OBs (final result after processing) is written to memory
    //on CPU side, so GPU should not clear that content while rendering
    renderer.setDisplayBufferClearFlags(display, dmaOffscreenBufferRead1, ramses::EClearFlags_None);
    renderer.setDisplayBufferClearFlags(display, dmaOffscreenBufferRead2, ramses::EClearFlags_None);
    renderer.setDisplayBufferClearFlags(display, dmaOffscreenBufferWrite1, ramses::EClearFlags_None);
    renderer.setDisplayBufferClearFlags(display, dmaOffscreenBufferWrite2, ramses::EClearFlags_None);
    renderer.flush();

    const ramses::dataConsumerId_t samplerConsumerId(457u);

    const ramses::sceneId_t mainSceneId{1u};
    const std::string& processingOutputSamplerName = "processingOutputTexSampler";
    ramses::Scene& mainScene = createMainScene(client, mainSceneId, processingOutputSamplerName);
    const ramses::TextureSampler& processingOutputSampler = *ramses::RamsesUtils::TryConvert<ramses::TextureSampler>(*mainScene.findObjectByName(processingOutputSamplerName.c_str()));
    mainScene.createTextureConsumer(processingOutputSampler, samplerConsumerId);

    mainScene.flush();
    mainScene.publish();
    sceneControlAPI.setSceneMapping(mainSceneId, display);
    sceneControlAPI.setSceneState(mainSceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();
    eventHandler.waitForSceneState(mainSceneId, ramses::RendererSceneState::Rendered);

    const ramses::sceneId_t sourceSceneId{2u};
    ramses::Scene& sourceScene = createSourceScene(client, sourceSceneId);
    sourceScene.flush();
    sourceScene.publish();

    sceneControlAPI.setSceneMapping(sourceSceneId, display);
    sceneControlAPI.setSceneState(sourceSceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneDisplayBufferAssignment(sourceSceneId, backDMAReadBufferId);
    sceneControlAPI.flush();

    const auto exampleStartTime = std::chrono::steady_clock::now();
    while ((std::chrono::steady_clock::now() - exampleStartTime) < ExampleRunDuration)
    {
        if(ExampleTakeScreenshots)
        {
            renderer.readPixels(display, renderer.getDisplayFramebuffer(display), 0u, 0u, DisplayWidth, DisplayHeight);
            renderer.flush();
        }

        //Consume input from current "read OB" and write image processing result to current "write OB"
        doProcessing(backDMAReadBufferId, backDMAWriteBufferId);

        sceneControlAPI.setSceneDisplayBufferAssignment(sourceSceneId, backDMAReadBufferId);
        sceneControlAPI.linkOffscreenBuffer(backDMAWriteBufferId, mainSceneId, samplerConsumerId);
        sceneControlAPI.flush();

        renderer.doOneLoop();
        renderer.dispatchEvents(eventHandler);
        std::swap(backDMAReadBufferId, frontDmaReadBufferId);
        std::swap(backDMAWriteBufferId, frontDmaWriteBufferId);
    }

    //unmap buffers after they do not need to be accessed from CPU
    for(const auto& bufferInfo : bufferInfoMap)
    {
        errno = 0;

        const auto result = munmap(bufferInfo.second.mappedMem, bufferInfo.second.bufferSize);
        if(result == -1)
        {
            const auto errorValue = errno;
            printf("Error: Failed to unmap memory for OB %u (errno=%i, errnoString=%s)!\n", bufferInfo.first.getValue(), errorValue, strerror(errorValue));
            return 1;
        }

        printf("Unmapped OB %u with FD %i successfully!\n", bufferInfo.first.getValue(), bufferInfo.second.fd);
    }

    printf("\n************* All OBs unmapped successfully !! **********\n");

    return 0;
}
