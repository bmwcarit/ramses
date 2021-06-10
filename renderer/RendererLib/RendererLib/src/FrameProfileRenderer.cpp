//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/FrameProfileRenderer.h"

#include "RendererLib/Renderer.h"
#include "RendererAPI/IDevice.h"
#include "Resource/EffectInputInformation.h"
#include "Resource/ArrayResource.h"
#include "Resource/EffectResource.h"
#include "Math3d/CameraMatrixHelper.h"
#include "Math3d/ProjectionParams.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    static const Float TimingAreaHeight(300.f);
    static const Float TimingGridlinePixelDistance(200.f);

    static const Float CounterAreaHeight(150.f);
    static const Float CounterGridlinePixelDistance(120.f);

    static const char* SimpleColorFragmentShader =  R"GLSL(
        #version 300 es
        precision highp float;

        uniform vec4 color;
        out vec4 fragColor;

        void main(void)
        {
            fragColor = color;
        })GLSL";

    static const char* IndexedColorFragmentShader = R"GLSL(
        #version 300 es
        precision highp float;

        flat in int colorId;
        const vec4 colors[16] = vec4[16](
            vec4(0.0, 0.0, 0.0, 1.0),
            vec4(0.5, 1.0, 0.0, 0.5),        // RendererCommands
            vec4(1.0, 0.0, 0.0, 0.5),        // UpdateClientResources
            vec4(0.0, 0.0, 1.0, 0.5),        // ApplySceneActions
            vec4(1.0, 0.0, 1.0, 0.5),        // UpdateSceneResources
            vec4(0.0, 1.0, 1.0, 0.5),        // UpdateEmbeddedCompositingResources
            vec4(1.0, 0.5, 0.0, 0.5),        // UpdateStreamTextures
            vec4(0.5, 0.0, 1.0, 0.25),       // UpdateScenesToBeMapped
            vec4(0.25, 0.75, 0.25, 0.5),     // UpdateResourceCache
            vec4(0.75, 0.25, 0.25, 0.5),     // UpdateAnimations
            vec4(0.25, 0.75, 0.75, 0.5),     // UpdateTransformations
            vec4(1.0, 1.0, 0.0, 0.5),        // UpdateDataLinks
            vec4(0.0, 0.0, 0.0, 0.5),        // HandleDisplayEvents
            vec4(0.0, 1.0, 0.0, 0.5),        // DrawScenes
            vec4(1.0, 1.0, 0.0, 0.25),       // SwapBuffersAndNotifyClients
            vec4(1.0, 1.0, 1.0, 0.2) );      // MaxFramerateSleep
        out vec4 fragColor;

        void main(void)
        {
            fragColor = colors[colorId];
        })GLSL";

    static const char* StackedTimingLineVertexShader = R"GLSL(
        #version 300 es

        uniform mat4 mvpMatrix;

        in float a_position;
        flat out int colorId;

        void main()
        {
            vec4 pos;
            pos.x = float(gl_VertexID / 16);
            pos.y = a_position;
            pos.z = 0.0;
            pos.w = 1.0;
            gl_Position = mvpMatrix * pos;
            colorId = gl_VertexID % 16;
        })GLSL";

    static const char* TimingLineVertexShader = R"GLSL(
        #version 300 es

        uniform mat4 mvpMatrix;

        in float a_position;

        void main()
        {
            vec4 pos;
            pos.x = float(gl_VertexID);
            pos.y = a_position;
            pos.z = 0.0;
            pos.w = 1.0;
            gl_Position = mvpMatrix * pos;
        })GLSL";

    static const char* SimpleVertexShader = R"GLSL(
        #version 300 es

        uniform mat4 mvpMatrix;

        in vec2 a_position;

        void main()
        {
            gl_Position = mvpMatrix * vec4(a_position, 0.0, 1.0);
        })GLSL";

    FrameProfileRenderer::FrameProfileRenderer(IDevice& device, UInt32 displayWidth, UInt32 displayHeight)
        : m_device(&device)
        , m_displayWidth(static_cast<Float>(displayWidth))
    {
        const ProjectionParams params = ProjectionParams::Frustum(ECameraProjectionType::Orthographic, 0.0f, m_displayWidth, 0.0f, static_cast<Float>(displayHeight), 0.0f, 1.0f);
        m_projectionMatrix = CameraMatrixHelper::ProjectionMatrix(params);
    }

    FrameProfileRenderer::~FrameProfileRenderer()
    {
        destroyResources();
    }

    void FrameProfileRenderer::createEffects()
    {
        EffectInputInformation color("color", 1, EDataType::Vector4F, EFixedSemantics::Invalid);
        EffectInputInformation mvpMatrix("mvpMatrix", 1, EDataType::Matrix44F, EFixedSemantics::ModelViewProjectionMatrix);

        EffectInputInformationVector uniforms;
        uniforms.push_back(color);
        uniforms.push_back(mvpMatrix);

        EffectInputInformationVector attributes;
        EffectInputInformation position("a_position", 1, EDataType::Float, EFixedSemantics::Invalid);
        attributes.push_back(position);
        m_timingLineEffect = new EffectResource(TimingLineVertexShader, SimpleColorFragmentShader, "", absl::nullopt, uniforms, attributes, "overlayShader", ResourceCacheFlag_DoNotCache);
        m_singleColorEffect = new EffectResource(SimpleVertexShader, SimpleColorFragmentShader, "", absl::nullopt, uniforms, attributes, "overlayShader", ResourceCacheFlag_DoNotCache);


        EffectInputInformationVector iuniforms;
        iuniforms.push_back(mvpMatrix);

        EffectInputInformationVector iAttributes;
        EffectInputInformation iposition("a_position", FrameProfilerStatistics::NumberOfEntries, EDataType::Float, EFixedSemantics::Invalid);
        iAttributes.push_back(iposition);

        m_stackedTimingLineEffect = new EffectResource(StackedTimingLineVertexShader, IndexedColorFragmentShader, "", absl::nullopt, iuniforms, iAttributes, "overlayShader", ResourceCacheFlag_DoNotCache);


        auto singleColorShaderResource = m_device->uploadShader(*m_singleColorEffect);
        auto timingLineShaderResource = m_device->uploadShader(*m_timingLineEffect);
        auto stackedTimingLineShaderResource = m_device->uploadShader(*m_stackedTimingLineEffect);

        m_singleColorShaderHandle = m_device->registerShader(std::move(singleColorShaderResource));
        m_timingLineShaderHandle = m_device->registerShader(std::move(timingLineShaderResource));
        m_stackedTimingLineShaderHandle = m_device->registerShader(std::move(stackedTimingLineShaderResource));
    }

    void FrameProfileRenderer::destroyEffects()
    {
        m_device->deleteShader(m_singleColorShaderHandle);
        m_device->deleteShader(m_timingLineShaderHandle);
        m_device->deleteShader(m_stackedTimingLineShaderHandle);

        m_singleColorShaderHandle = DeviceResourceHandle::Invalid();
        m_timingLineShaderHandle = DeviceResourceHandle::Invalid();
        m_stackedTimingLineShaderHandle = DeviceResourceHandle::Invalid();

        delete m_timingLineEffect;
        delete m_singleColorEffect;
        delete m_stackedTimingLineEffect;
    }

    void FrameProfileRenderer::createGeometries()
    {
        m_timingLinesIndexBuffer = createLineIndexBuffer(FrameProfilerStatistics::NumberOfFrames - 1);
        m_stackedTimingLineIndexBuffer = createStackedLinesIndexBuffer(FrameProfilerStatistics::NumberOfFrames, FrameProfilerStatistics::NumberOfRegions);
        m_filledUnitCubeGeometry = createBoxGeometry(true);
        m_unitCubeGeometry = createBoxGeometry(false);
        m_horizontalLineGeometry = createHorizontalLineGeometry();
        m_verticalLineGeometry = createVerticalLineGeometry();
    }

    void FrameProfileRenderer::destroyGeometry(Geometry& geometry)
    {
        m_device->deleteVertexBuffer(geometry.vertexBufferHandle);
        m_device->deleteIndexBuffer(geometry.indexBufferHandle);

        geometry.vertexBufferHandle = DeviceResourceHandle::Invalid();
        geometry.indexBufferHandle = DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle FrameProfileRenderer::createLineIndexBuffer(UInt32 segmentCount)
    {
        const UInt32 indexCount = 2 * segmentCount;
        std::vector<UInt16> indices(indexCount);
        UInt16 id = 0;
        for (UInt32 i = 0; i < indexCount;)
        {
            indices[i++] = id++;
            indices[i++] = id;
        }

        const ArrayResource indexArray(EResourceType_IndexArray, indexCount, EDataType::UInt16, indices.data(), ResourceCacheFlag_DoNotCache, String());
        const DeviceResourceHandle deviceHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(deviceHandle, indexArray.getResourceData().data(), indexArray.getDecompressedDataSize());

        return deviceHandle;
    }

    DeviceResourceHandle FrameProfileRenderer::createStackedLinesIndexBuffer(UInt32 lineCount, UInt32 segmentPerLineCount)
    {
        const UInt32 indexCount = 2 * segmentPerLineCount * lineCount;
        std::vector<UInt16> indices(indexCount);
        UInt16 id = 0;
        UInt32 i = 0;
        for (UInt32 l = 0; l < lineCount; l++)
        {
            for (UInt32 s = 0; s < segmentPerLineCount; s++)
            {
                indices[i++] = id++;
                indices[i++] = id;
            }
            id++;
        }

        const ArrayResource indexArray(EResourceType_IndexArray, indexCount, EDataType::UInt16, indices.data(), ResourceCacheFlag_DoNotCache, String());
        const DeviceResourceHandle deviceHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(deviceHandle, indexArray.getResourceData().data(), indexArray.getDecompressedDataSize());

        return deviceHandle;
    }

    DeviceResourceHandle FrameProfileRenderer::createVertexArray(const Geometry& geometry, const Look& look)
    {
        assert(look.shaderHandle.isValid());
        assert(geometry.indexBufferHandle.isValid());
        assert(geometry.vertexBufferHandle.isValid());
        assert(IsBufferDataType(geometry.dataType));
        VertexArrayInfo vaInfo;
        vaInfo.shader = look.shaderHandle;
        vaInfo.indexBuffer = geometry.indexBufferHandle;
        vaInfo.vertexBuffers.push_back({ geometry.vertexBufferHandle, look.positionHandle, 0u, 0u, geometry.dataType, 0u, 0u });

        const auto vertexArrayHandle = m_device->allocateVertexArray(vaInfo);
        m_vertexArrayHandles.push_back(vertexArrayHandle);

        return vertexArrayHandle;
    }

    void FrameProfileRenderer::updateTimingLineVertexBuffer(Renderable& renderable, const FrameProfilerStatistics::RegionTimings& timingData)
    {
        const auto& look = renderable.look;
        auto& geometry = renderable.geometry;
        const ArrayResource res(EResourceType_VertexArray, static_cast<UInt32>(timingData.size()), EDataType::Float, timingData.data(), ResourceCacheFlag_DoNotCache, String());
        if (!geometry.vertexBufferHandle.isValid())
        {
            geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getDecompressedDataSize());
            geometry.dataType = EDataType::FloatBuffer;

            assert(!renderable.vertexArrayHandle.isValid());
            renderable.vertexArrayHandle = createVertexArray(geometry, look);
        }
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData().data(), res.getDecompressedDataSize());
    }

    void FrameProfileRenderer::updateCounterLineVertexBuffer(Renderable& renderable, const FrameProfilerStatistics::CounterValues& counterValues)
    {
        const auto& look = renderable.look;
        auto& geometry = renderable.geometry;
        const ArrayResource res(EResourceType_VertexArray, static_cast<UInt32>(counterValues.size()), EDataType::Float, counterValues.data(), ResourceCacheFlag_DoNotCache, String());
        if (!geometry.vertexBufferHandle.isValid())
        {
            geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getDecompressedDataSize());
            geometry.dataType = EDataType::FloatBuffer;

            assert(!renderable.vertexArrayHandle.isValid());
            renderable.vertexArrayHandle = createVertexArray(geometry, look);
        }
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData().data(), res.getDecompressedDataSize());
    }

    FrameProfileRenderer::Geometry FrameProfileRenderer::createBoxGeometry(bool filled = true)
    {
        const Float vertices[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 1.0f
        };

        const UInt16 indicesFilled[] = { 0,1,2,3 };
        const UInt16 indicesLines[] = { 0,1,3,2 };

        Geometry geometry;
        geometry.drawMode = filled ? EDrawMode::TriangleStrip : EDrawMode::LineLoop;
        geometry.indexCount = 4;

        const ArrayResource res(EResourceType_VertexArray, 4, EDataType::Vector2F, vertices, ResourceCacheFlag_DoNotCache, String());
        geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getDecompressedDataSize());
        geometry.dataType = EDataType::Vector2Buffer;
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData().data(), res.getDecompressedDataSize());

        const ArrayResource indexArray(EResourceType_IndexArray, geometry.indexCount, EDataType::UInt16, filled ? indicesFilled : indicesLines, ResourceCacheFlag_DoNotCache, String());
        geometry.indexBufferHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(geometry.indexBufferHandle, indexArray.getResourceData().data(), indexArray.getDecompressedDataSize());

        return geometry;
    }

    FrameProfileRenderer::Geometry FrameProfileRenderer::createVerticalLineGeometry()
    {
        const Float vertices[] = {
            0.0f, 0.0f,
            0.0f, 1.0f
        };

        const UInt16 indices[] = { 0, 1 };

        Geometry geometry;
        geometry.indexCount = 2;
        geometry.drawMode = EDrawMode::Lines;

        const ArrayResource res(EResourceType_VertexArray, 2, EDataType::Vector2F, vertices, ResourceCacheFlag_DoNotCache, String());
        geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getDecompressedDataSize());
        geometry.dataType = EDataType::Vector2Buffer;
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData().data(), res.getDecompressedDataSize());

        const ArrayResource indexArray(EResourceType_IndexArray, 2, EDataType::UInt16, indices, ResourceCacheFlag_DoNotCache, String());
        geometry.indexBufferHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(geometry.indexBufferHandle, indexArray.getResourceData().data(), indexArray.getDecompressedDataSize());

        return geometry;
    }

    FrameProfileRenderer::Geometry FrameProfileRenderer::createHorizontalLineGeometry()
    {
        const Float vertices[] = {
            0.0f, 0.0f,
            1.0f, 0.0f
        };

        const UInt16 indices[] = { 0, 1 };

        Geometry geometry;
        geometry.indexCount = 2;
        geometry.drawMode = EDrawMode::Lines;

        const ArrayResource res(EResourceType_VertexArray, 2, EDataType::Vector2F, vertices, ResourceCacheFlag_DoNotCache, String());
        geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getDecompressedDataSize());
        geometry.dataType = EDataType::Vector2Buffer;
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData().data(), res.getDecompressedDataSize());

        const ArrayResource indexArray(EResourceType_IndexArray, 2, EDataType::UInt16, indices, ResourceCacheFlag_DoNotCache, String());
        geometry.indexBufferHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(geometry.indexBufferHandle, indexArray.getResourceData().data(), indexArray.getDecompressedDataSize());

        return geometry;
    }

    FrameProfileRenderer::Look FrameProfileRenderer::createLookFromEffect(const EffectResource& effect, DeviceResourceHandle shaderHandle)
    {
        Look look;
        look.shaderHandle = shaderHandle;
        look.positionHandle = effect.getAttributeDataFieldHandleByName("a_position");
        look.colorHandle = effect.getUniformDataFieldHandleByName("color");
        look.mvpHandle = effect.getUniformDataFieldHandleByName("mvpMatrix");
        return look;
    }

    Matrix44f FrameProfileRenderer::createMVPMatrix(const Vector2& translation, const Vector2& scale)
    {
        Matrix44f modelView = Matrix44f::Identity;
        modelView.m11 = scale.x;
        modelView.m22 = scale.y;
        modelView.m14 = translation.x;
        modelView.m24 = translation.y;
        return m_projectionMatrix * modelView;
    }

    FrameProfileRenderer::Renderable FrameProfileRenderer::createRenderable(const Geometry& geometry, const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale)
    {
        Renderable renderable;
        renderable.geometry = geometry;
        renderable.look = look;
        renderable.color = color;
        renderable.mvpMatrix = createMVPMatrix(translation, scale);

        renderable.vertexArrayHandle = {};

        return renderable;
    }

    FrameProfileRenderer::Geometry FrameProfileRenderer::createGraphGeometry()
    {
        Geometry geometry;
        geometry.drawMode = EDrawMode::Lines;
        geometry.indexBufferHandle = m_timingLinesIndexBuffer;
        geometry.indexCount = 2 * (FrameProfilerStatistics::NumberOfFrames - 1);
        return geometry;
    }

    FrameProfileRenderer::Geometry FrameProfileRenderer::createStackedGraphGeometry()
    {
        Geometry geometry;
        geometry.drawMode = EDrawMode::Lines;
        geometry.indexBufferHandle = m_stackedTimingLineIndexBuffer;
        geometry.indexCount = 2 * FrameProfilerStatistics::NumberOfRegions * FrameProfilerStatistics::NumberOfFrames;
        return geometry;
    }

    void FrameProfileRenderer::addStackedGraphRenderable(const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale)
    {
        Geometry geometry = createStackedGraphGeometry();
        Renderable renderable = createRenderable(geometry, look, color, translation, scale);

        m_renderables.push_back([=](const FrameProfilerStatistics& statistics) mutable {

            // using scissor test to prevent graph rendering outside its box
            m_device->scissorTest(EScissorTest::Enabled, { static_cast<Int16>(translation.x), static_cast<Int16>(translation.y), UInt16(FrameProfilerStatistics::NumberOfFrames), static_cast<UInt16>(TimingAreaHeight) });

            updateTimingLineVertexBuffer(renderable, statistics.getRegionTimings());
            render(renderable);

            m_device->scissorTest(EScissorTest::Disabled, {});
        });
    }

    void FrameProfileRenderer::addGraphRenderableForCounter(const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale, FrameProfilerStatistics::ECounter counter)
    {
        Geometry geometry = createGraphGeometry();
        Renderable renderable = createRenderable(geometry, look, color, translation, scale);

        m_renderables.push_back([=](const FrameProfilerStatistics& statistics) mutable {
            // using scissor test to prevent graph rendering outside its box
            m_device->scissorTest(EScissorTest::Enabled, { static_cast<Int16>(translation.x), static_cast<Int16>(translation.y), UInt16(FrameProfilerStatistics::NumberOfFrames), static_cast<UInt16>(CounterAreaHeight) });

            const FrameProfilerStatistics::CounterValues& counterValues = statistics.getCounterValues(counter);
            updateCounterLineVertexBuffer(renderable, counterValues);
            render(renderable);

            m_device->scissorTest(EScissorTest::Disabled, {});
        });
    }

    void FrameProfileRenderer::addStaticRenderable(const Geometry& geometry, const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale)
    {
        Renderable renderable = createRenderable(geometry, look, color, translation, scale);

        m_renderables.push_back([this, renderable](const FrameProfilerStatistics&) {
            render(renderable);
        });
    }

    void FrameProfileRenderer::addDynamicRenderable(const Geometry& geometry, const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale)
    {
        Renderable renderable = createRenderable(geometry, look, color, translation, scale);

        m_renderables.push_back([this, renderable](const FrameProfilerStatistics& statistics) mutable {
            updateTranslationForCurrentFrame(renderable.mvpMatrix, statistics.getCurrentFrameId());
            render(renderable);
        });
    }

    void FrameProfileRenderer::updateTranslationForCurrentFrame(Matrix44f& mvpMatrix, UInt32 currentFrameId)
    {
        mvpMatrix.m14 = (static_cast<Float>(currentFrameId + 10.f) / (m_displayWidth * 0.5f) - 1.0f);
    }

    void FrameProfileRenderer::destroyResources()
    {
        if (m_initialized)
        {
            m_device->deleteIndexBuffer(m_timingLinesIndexBuffer);
            m_device->deleteIndexBuffer(m_stackedTimingLineIndexBuffer);
            destroyGeometry(m_unitCubeGeometry);
            destroyGeometry(m_filledUnitCubeGeometry);
            destroyGeometry(m_verticalLineGeometry);
            destroyGeometry(m_horizontalLineGeometry);
            destroyEffects();

            for (const auto& vaHandle : m_vertexArrayHandles)
                m_device->deleteVertexArray(vaHandle);
            m_vertexArrayHandles.clear();

            m_renderables.clear();
            m_initialized = false;
        }
    }

    void FrameProfileRenderer::createResources()
    {
        assert(!m_initialized);

        createEffects();
        createGeometries();
        const Look singleColorLook = createLookFromEffect(*m_singleColorEffect, m_singleColorShaderHandle);
        const Look graphLook = createLookFromEffect(*m_timingLineEffect, m_timingLineShaderHandle);
        const Look stackedGraphLook = createLookFromEffect(*m_stackedTimingLineEffect, m_stackedTimingLineShaderHandle);
        const Vector4 backgroundColor(1.0f, 1.0f, 1.0f, 0.2f);
        const Vector4 greenColor(0.0f, 1.0f, 0.0f, 0.5f);
        const Vector4 blueColor(0.0f, 0.0f, 1.0f, 0.5f);
        const Vector4 whiteColor(1.0f, 1.0f, 1.0f, 0.5f);
        const Vector4 blackColor(0.0f, 0.0f, 0.0f, 0.5f);
        const Vector4 violetColor(1.0f, 0.0f, 1.0f, 0.5f);

        // timing graphs
        const Float VerticalTimingScale(TimingGridlinePixelDistance / static_cast<float>(m_timingGraphHeight * 1000)); // convert into microseconds
        const Vector2 timingScale(1.0f, VerticalTimingScale);
        const Vector2 timingTranslation(10.f, 10.f);

        addStaticRenderable(m_filledUnitCubeGeometry, singleColorLook, backgroundColor, timingTranslation, Vector2(static_cast<Float>(FrameProfilerStatistics::NumberOfFrames), TimingAreaHeight));
        addStaticRenderable(m_horizontalLineGeometry, singleColorLook, whiteColor, Vector2(timingTranslation.x, timingTranslation.y + TimingGridlinePixelDistance), Vector2(static_cast<Float>(FrameProfilerStatistics::NumberOfFrames), 1.0f));
        addStaticRenderable(m_unitCubeGeometry, singleColorLook, blackColor, timingTranslation, Vector2(static_cast<Float>(FrameProfilerStatistics::NumberOfFrames), TimingAreaHeight));
        addStackedGraphRenderable(stackedGraphLook, violetColor, timingTranslation, timingScale);
        addDynamicRenderable(m_verticalLineGeometry, singleColorLook, blackColor, timingTranslation, Vector2(1.f, TimingAreaHeight));

        // counter graphs
        const Float VerticalCounterScale(CounterGridlinePixelDistance / m_counterGraphHeight);
        const Vector2 counterScale(1.0f, VerticalCounterScale);
        const Vector2 counterTranslation(10.f, TimingAreaHeight + 20.f);

        addStaticRenderable(m_filledUnitCubeGeometry, singleColorLook, backgroundColor, counterTranslation, Vector2(static_cast<Float>(FrameProfilerStatistics::NumberOfFrames), CounterAreaHeight));
        addStaticRenderable(m_horizontalLineGeometry, singleColorLook, whiteColor, Vector2(counterTranslation.x, counterTranslation.y + (CounterGridlinePixelDistance)), Vector2(static_cast<Float>(FrameProfilerStatistics::NumberOfFrames), 1.0f));
        addStaticRenderable(m_unitCubeGeometry, singleColorLook, blackColor, counterTranslation, Vector2(static_cast<Float>(FrameProfilerStatistics::NumberOfFrames), CounterAreaHeight));
        addGraphRenderableForCounter(graphLook, greenColor, counterTranslation, counterScale, FrameProfilerStatistics::ECounter::DrawCalls);
        addGraphRenderableForCounter(graphLook, blueColor, counterTranslation, counterScale, FrameProfilerStatistics::ECounter::AppliedSceneActions);
        addGraphRenderableForCounter(graphLook, violetColor, counterTranslation, counterScale, FrameProfilerStatistics::ECounter::UsedGPUMemory);
        addDynamicRenderable(m_verticalLineGeometry, singleColorLook, blackColor, counterTranslation, Vector2(1.f, CounterAreaHeight));

        m_initialized = true;
    }

    void FrameProfileRenderer::renderStatistics(FrameProfilerStatistics& statistics)
    {
        if (m_enabled)
        {
            if (m_doReset)
            {
                destroyResources();
                m_doReset = false;
            }
            if (!m_initialized)
            {
                createResources();
            }

            m_device->depthFunc(EDepthFunc::Disabled);
            m_device->blendOperations(EBlendOperation::Add, EBlendOperation::Add);
            m_device->blendFactors(EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha, EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha);
            m_device->cullMode(ECullMode::Disabled);

            for(const auto& renderable : m_renderables)
            {
                renderable(statistics);
            }
        }
    }

    void FrameProfileRenderer::render(const Renderable& drawable)
    {
        if (!drawable.vertexArrayHandle.isValid())
            return;

        m_device->activateShader(drawable.look.shaderHandle);
        if (drawable.look.colorHandle != DataFieldHandle::Invalid())
        {
            m_device->setConstant(drawable.look.colorHandle, 1, &drawable.color);
        }
        m_device->setConstant(drawable.look.mvpHandle, 1, &drawable.mvpMatrix);

        m_device->drawMode(drawable.geometry.drawMode);

        m_device->activateVertexArray(drawable.vertexArrayHandle);
        m_device->drawIndexedTriangles(0, drawable.geometry.indexCount, 0);
    }

    void FrameProfileRenderer::enable(Bool state)
    {
        m_enabled = state;
    }

    Bool FrameProfileRenderer::isEnabled() const
    {
        return m_enabled;
    }

    void FrameProfileRenderer::setCounterGraphHeight(UInt32 height)
    {
        assert(height > 0);
        m_counterGraphHeight = height;
        if (m_enabled)
        {
            m_doReset = true;
        }
    }

    void FrameProfileRenderer::setTimingGraphHeight(UInt32 height)
    {
        assert(height > 0);
        m_timingGraphHeight = height;
        if (m_enabled)
        {
            m_doReset = true;
        }
    }
}
