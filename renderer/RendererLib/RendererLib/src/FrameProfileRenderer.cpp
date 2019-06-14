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

    static const char* SimpleColorFragmentShader =  "       \n\
        #version 300 es                                     \n\
        precision highp float;                              \n\
                                                            \n\
        uniform vec4 color;                                 \n\
        out vec4 fragColor;                                 \n\
                                                            \n\
        void main(void)                                     \n\
        {                                                   \n\
            fragColor = color;                              \n\
        }";

    static const char* IndexedColorFragmentShader = "       \n\
        #version 300 es                                     \n\
        precision highp float;                              \n\
                                                            \n\
        flat in int colorId;                                \n\
        const vec4 colors[17] = vec4[17](                   \n\
            vec4(0.0, 0.0, 0.0, 1.0),                       \n\
            vec4(0.5, 1.0, 0.0, 0.5),        // RendererCommands                   \n\
            vec4(0.0, 0.0, 0.0, 0.5),        // ConsolidateSceneActions            \n\
            vec4(1.0, 0.0, 0.0, 0.5),        // UpdateClientResources              \n\
            vec4(0.0, 0.0, 1.0, 0.5),        // ApplySceneActions                  \n\
            vec4(1.0, 0.0, 1.0, 0.5),        // UpdateSceneResources               \n\
            vec4(0.0, 1.0, 1.0, 0.5),        // UpdateEmbeddedCompositingResources \n\
            vec4(1.0, 0.5, 0.0, 0.5),        // UpdateStreamTextures               \n\
            vec4(0.5, 0.0, 1.0, 0.25),       // UpdateScenesToBeMapped             \n\
            vec4(0.25, 0.75, 0.25, 0.5),     // UpdateResourceCache                \n\
            vec4(0.75, 0.25, 0.25, 0.5),     // UpdateAnimations                   \n\
            vec4(0.25, 0.75, 0.75, 0.5),     // UpdateTransformations              \n\
            vec4(1.0, 1.0, 0.0, 0.5),        // UpdateDataLinks                    \n\
            vec4(0.0, 0.0, 0.0, 0.5),        // HandleDisplayEvents                \n\
            vec4(0.0, 1.0, 0.0, 0.5),        // DrawScenes                         \n\
            vec4(1.0, 1.0, 0.0, 0.25),       // SwapBuffersAndNotifyClients        \n\
            vec4(1.0, 1.0, 1.0, 0.2) );      // MaxFramerateSleep                  \n\
        out vec4 fragColor;                                 \n\
                                                            \n\
        void main(void)                                     \n\
        {                                                   \n\
            fragColor = colors[colorId];                    \n\
        }";

    static const char* StackedTimingLineVertexShader = "    \n\
        #version 300 es                                     \n\
                                                            \n\
        uniform mat4 mvpMatrix;                             \n\
                                                            \n\
        in float a_position;                                \n\
        flat out int colorId;                               \n\
                                                            \n\
        void main()                                         \n\
        {                                                   \n\
            vec4 pos;                                       \n\
            pos.x = float(gl_VertexID / 17);                \n\
            pos.y = a_position;                             \n\
            pos.z = 0.0;                                    \n\
            pos.w = 1.0;                                    \n\
            gl_Position = mvpMatrix * pos;                  \n\
            colorId = gl_VertexID % 17;                     \n\
        }";

    static const char* TimingLineVertexShader = "           \n\
        #version 300 es                                     \n\
                                                            \n\
        uniform mat4 mvpMatrix;                             \n\
                                                            \n\
        in float a_position;                                \n\
                                                            \n\
        void main()                                         \n\
        {                                                   \n\
            vec4 pos;                                       \n\
            pos.x = float(gl_VertexID);                     \n\
            pos.y = a_position;                             \n\
            pos.z = 0.0;                                    \n\
            pos.w = 1.0;                                    \n\
            gl_Position = mvpMatrix * pos;                  \n\
        }";

    static const char* SimpleVertexShader = "               \n\
        #version 300 es                                     \n\
                                                            \n\
        uniform mat4 mvpMatrix;                             \n\
                                                            \n\
        in vec2 a_position;                                 \n\
                                                            \n\
        void main()                                         \n\
        {                                                   \n\
            gl_Position = mvpMatrix * vec4(a_position, 0.0, 1.0); \n\
        }";

    FrameProfileRenderer::FrameProfileRenderer(IDevice& device, UInt32 displayWidth, UInt32 displayHeight)
        : m_doReset(false)
        , m_initialized(false)
        , m_enabled(false)
        , m_device(&device)
        , m_displayWidth(static_cast<Float>(displayWidth))
        , m_timingLinesIndexBuffer(DeviceResourceHandle::Invalid())
        , m_stackedTimingLineIndexBuffer(DeviceResourceHandle::Invalid())
        , m_timingLineShaderHandle(DeviceResourceHandle::Invalid())
        , m_stackedTimingLineShaderHandle(DeviceResourceHandle::Invalid())
        , m_singleColorShaderHandle(DeviceResourceHandle::Invalid())
        , m_timingLineEffect(nullptr)
        , m_stackedTimingLineEffect(nullptr)
        , m_singleColorEffect(nullptr)
    {
        const ProjectionParams params = ProjectionParams::Frustum(ECameraProjectionType_Orthographic, 0.0f, m_displayWidth, 0.0f, static_cast<Float>(displayHeight), 0.0f, 1.0f);
        m_projectionMatrix = CameraMatrixHelper::ProjectionMatrix(params);
    }

    FrameProfileRenderer::~FrameProfileRenderer()
    {
        destroyResources();
    }

    void FrameProfileRenderer::createEffects()
    {
        EffectInputInformation color("color", 1, EDataType_Vector4F, EFixedSemantics_Invalid, EEffectInputTextureType_Invalid);
        EffectInputInformation mvpMatrix("mvpMatrix", 1, EDataType_Matrix44F, EFixedSemantics_ModelViewProjectionMatrix, EEffectInputTextureType_Invalid);

        EffectInputInformationVector uniforms;
        uniforms.push_back(color);
        uniforms.push_back(mvpMatrix);

        EffectInputInformationVector attributes;
        EffectInputInformation position("a_position", 1, EDataType_Float, EFixedSemantics_VertexPositionAttribute, EEffectInputTextureType_Invalid);
        attributes.push_back(position);
        m_timingLineEffect = new EffectResource(TimingLineVertexShader, SimpleColorFragmentShader, uniforms, attributes, "overlayShader", ResourceCacheFlag_DoNotCache);
        m_singleColorEffect = new EffectResource(SimpleVertexShader, SimpleColorFragmentShader, uniforms, attributes, "overlayShader", ResourceCacheFlag_DoNotCache);


        EffectInputInformationVector iuniforms;
        iuniforms.push_back(mvpMatrix);

        EffectInputInformationVector iAttributes;
        EffectInputInformation iposition("a_position", FrameProfilerStatistics::NumberOfEntries, EDataType_Float, EFixedSemantics_Invalid, EEffectInputTextureType_Invalid);
        iAttributes.push_back(iposition);

        m_stackedTimingLineEffect = new EffectResource(StackedTimingLineVertexShader, IndexedColorFragmentShader, iuniforms, iAttributes, "overlayShader", ResourceCacheFlag_DoNotCache);


        m_singleColorShaderHandle = m_device->uploadShader(*m_singleColorEffect);
        m_timingLineShaderHandle = m_device->uploadShader(*m_timingLineEffect);
        m_stackedTimingLineShaderHandle = m_device->uploadShader(*m_stackedTimingLineEffect);
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

        const ArrayResource indexArray(EResourceType_IndexArray, indexCount, EDataType_UInt16, reinterpret_cast<const Byte*>(&indices[0]), ResourceCacheFlag_DoNotCache, String());
        const DeviceResourceHandle deviceHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(deviceHandle, indexArray.getResourceData()->getRawData(), indexArray.getDecompressedDataSize());

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

        const ArrayResource indexArray(EResourceType_IndexArray, indexCount, EDataType_UInt16, reinterpret_cast<const Byte*>(&indices[0]), ResourceCacheFlag_DoNotCache, String());
        const DeviceResourceHandle deviceHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(deviceHandle, indexArray.getResourceData()->getRawData(), indexArray.getDecompressedDataSize());

        return deviceHandle;
    }

    void FrameProfileRenderer::updateTimingLineVertexBuffer(Geometry& geometry, const FrameProfilerStatistics::RegionTimings& timingData)
    {
        const ArrayResource res(EResourceType_VertexArray, static_cast<UInt32>(timingData.size()), EDataType::EDataType_Float, reinterpret_cast<const Byte*>(&timingData[0]), ResourceCacheFlag_DoNotCache, String());
        if (!geometry.vertexBufferHandle.isValid())
        {
            geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize());
        }
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData()->getRawData(), res.getDecompressedDataSize());
    }

    void FrameProfileRenderer::updateCounterLineVertexBuffer(Geometry& geometry, const FrameProfilerStatistics::CounterValues& counterValues)
    {
        const ArrayResource res(EResourceType_VertexArray, static_cast<UInt32>(counterValues.size()), EDataType::EDataType_Float, reinterpret_cast<const Byte*>(&counterValues[0]), ResourceCacheFlag_DoNotCache, String());
        if (!geometry.vertexBufferHandle.isValid())
        {
            geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize());
        }
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData()->getRawData(), res.getDecompressedDataSize());
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

        const ArrayResource res(EResourceType_VertexArray, 4, EDataType::EDataType_Vector2F, reinterpret_cast<const Byte*>(vertices), ResourceCacheFlag_DoNotCache, String());
        geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize());
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData()->getRawData(), res.getDecompressedDataSize());

        const ArrayResource indexArray(EResourceType_IndexArray, geometry.indexCount, EDataType_UInt16, reinterpret_cast<const Byte*>(filled ? indicesFilled : indicesLines), ResourceCacheFlag_DoNotCache, String());
        geometry.indexBufferHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(geometry.indexBufferHandle, indexArray.getResourceData()->getRawData(), indexArray.getDecompressedDataSize());

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

        const ArrayResource res(EResourceType_VertexArray, 2, EDataType::EDataType_Vector2F, reinterpret_cast<const Byte*>(vertices), ResourceCacheFlag_DoNotCache, String());
        geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize());
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData()->getRawData(), res.getDecompressedDataSize());

        const ArrayResource indexArray(EResourceType_IndexArray, 2, EDataType_UInt16, reinterpret_cast<const Byte*>(indices), ResourceCacheFlag_DoNotCache, String());
        geometry.indexBufferHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(geometry.indexBufferHandle, indexArray.getResourceData()->getRawData(), indexArray.getDecompressedDataSize());

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

        const ArrayResource res(EResourceType_VertexArray, 2, EDataType::EDataType_Vector2F, reinterpret_cast<const Byte*>(vertices), ResourceCacheFlag_DoNotCache, String());
        geometry.vertexBufferHandle = m_device->allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize());
        m_device->uploadVertexBufferData(geometry.vertexBufferHandle, res.getResourceData()->getRawData(), res.getDecompressedDataSize());

        const ArrayResource indexArray(EResourceType_IndexArray, 2, EDataType_UInt16, reinterpret_cast<const Byte*>(indices), ResourceCacheFlag_DoNotCache, String());
        geometry.indexBufferHandle = m_device->allocateIndexBuffer(indexArray.getElementType(), indexArray.getDecompressedDataSize());
        m_device->uploadIndexBufferData(geometry.indexBufferHandle, indexArray.getResourceData()->getRawData(), indexArray.getDecompressedDataSize());

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

            updateTimingLineVertexBuffer(renderable.geometry, statistics.getRegionTimings());
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
            updateCounterLineVertexBuffer(renderable.geometry, counterValues);
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
        const Vector4 redColor(1.0f, 0.0f, 0.0f, 0.5f);
        const Vector4 greenColor(0.0f, 1.0f, 0.0f, 0.5f);
        const Vector4 blueColor(0.0f, 0.0f, 1.0f, 0.5f);
        const Vector4 whiteColor(1.0f, 1.0f, 1.0f, 0.5f);
        const Vector4 blackColor(0.0f, 0.0f, 0.0f, 0.5f);
        const Vector4 violetColor(1.0f, 0.0f, 1.0f, 0.5f);
        const Vector4 cyanColor(0.0f, 1.0f, 1.0f, 0.5f);

        // timing graphs
        const Float VerticalTimingScale(TimingGridlinePixelDistance / (m_timingGraphHeight * 1000)); // convert into microseconds
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
        m_device->activateShader(drawable.look.shaderHandle);
        if (drawable.look.colorHandle != DataFieldHandle::Invalid())
        {
            m_device->setConstant(drawable.look.colorHandle, 1, &drawable.color);
        }
        m_device->setConstant(drawable.look.mvpHandle, 1, &drawable.mvpMatrix);

        m_device->drawMode(drawable.geometry.drawMode);
        m_device->activateVertexBuffer(drawable.geometry.vertexBufferHandle, drawable.look.positionHandle, 0);
        m_device->activateIndexBuffer(drawable.geometry.indexBufferHandle);
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

    void FrameProfileRenderer::ForAllFrameProfileRenderer(Renderer& renderer, const RendererFunc& rendererFunc)
    {
        for (auto displayId = 0u; displayId < renderer.getDisplayControllerCount(); displayId++)
        {
            const auto displayHandle = DisplayHandle(displayId);
            if (renderer.hasDisplayController(displayHandle))
            {
                rendererFunc(renderer.getFrameProfileRenderer(displayHandle));
            }
        }
    }
}
