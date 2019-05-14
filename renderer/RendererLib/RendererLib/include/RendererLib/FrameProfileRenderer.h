//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEPROFILERENDERER_H
#define RAMSES_FRAMEPROFILERENDERER_H

#include "RendererAPI/Types.h"
#include "SceneAPI/RenderState.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector4.h"
#include "Math3d/Matrix44f.h"
#include "FrameProfilerStatistics.h"

#include <functional>

namespace ramses_internal
{
    class IDevice;
    class EffectResource;
    class Renderer;

    class FrameProfileRenderer
    {
    public:
        FrameProfileRenderer(IDevice& device, UInt32 displayWidth, UInt32 displayHeight);
        ~FrameProfileRenderer();

        void enable(Bool state);
        Bool isEnabled() const;

        void setCounterGraphHeight(UInt32 height);
        void setTimingGraphHeight(UInt32 height);

        void renderStatistics(FrameProfilerStatistics& statistics);

        using RendererFunc = std::function<void(FrameProfileRenderer&)>;
        static void ForAllFrameProfileRenderer(Renderer& renderer, const RendererFunc& rendererFunc);

    private:

        struct Geometry
        {
            DeviceResourceHandle vertexBufferHandle = DeviceResourceHandle::Invalid();
            DeviceResourceHandle indexBufferHandle = DeviceResourceHandle::Invalid();
            EDrawMode drawMode = EDrawMode::Triangles;
            UInt32 indexCount = 0;
        };

        struct Look
        {
            DataFieldHandle positionHandle = DataFieldHandle::Invalid();
            DataFieldHandle colorHandle = DataFieldHandle::Invalid();
            DataFieldHandle mvpHandle = DataFieldHandle::Invalid();
            DeviceResourceHandle shaderHandle = DeviceResourceHandle::Invalid();
        };

        struct Renderable
        {
            Geometry geometry;
            Look look;
            Matrix44f mvpMatrix;
            Vector4 color;
        };

        Bool m_doReset;
        Bool m_initialized;
        Bool m_enabled;
        IDevice* m_device;
        Matrix44f m_projectionMatrix;
        Float m_displayWidth;
        UInt32 m_counterGraphHeight = 500u;
        UInt32 m_timingGraphHeight = 16u; // milliseconds

        using RenderableCall = std::function<void(const FrameProfilerStatistics&)>;
        using RenderableList = std::vector<RenderableCall>;
        RenderableList m_renderables;

        DeviceResourceHandle m_timingLinesIndexBuffer;
        DeviceResourceHandle m_stackedTimingLineIndexBuffer;

        DeviceResourceHandle m_timingLineShaderHandle;
        DeviceResourceHandle m_stackedTimingLineShaderHandle;
        DeviceResourceHandle m_singleColorShaderHandle;
        EffectResource* m_timingLineEffect;
        EffectResource* m_stackedTimingLineEffect;
        EffectResource* m_singleColorEffect;

        Geometry m_unitCubeGeometry;
        Geometry m_filledUnitCubeGeometry;
        Geometry m_horizontalLineGeometry;
        Geometry m_verticalLineGeometry;

        void destroyEffects();
        void destroyGeometry(Geometry& geometry);
        void destroyResources();

        void createResources();
        void createGeometries();
        void createEffects();
        DeviceResourceHandle createLineIndexBuffer(UInt32 segmentCount);
        DeviceResourceHandle createStackedLinesIndexBuffer(UInt32 lineCount, UInt32 segmentPerLineCount);
        Look createLookFromEffect(const EffectResource& effect, DeviceResourceHandle shaderHandle);
        Geometry createGraphGeometry();
        Geometry createStackedGraphGeometry();
        Geometry createBoxGeometry(bool filled);
        Geometry createHorizontalLineGeometry();
        Geometry createVerticalLineGeometry();
        Renderable createRenderable(const Geometry& geometry, const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale);
        Renderable createRenderable(const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale);
        Matrix44f createMVPMatrix(const Vector2& translation, const Vector2& scale);

        void addStaticRenderable(const Geometry& geometry, const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale);
        void addDynamicRenderable(const Geometry& geometry, const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale);
        void addStackedGraphRenderable(const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale);
        void addGraphRenderableForCounter(const Look& look, const Vector4& color, const Vector2& translation, const Vector2& scale, FrameProfilerStatistics::ECounter counter);
        void updateTimingLineVertexBuffer(Geometry& geometry, const FrameProfilerStatistics::RegionTimings& timingData);
        void updateCounterLineVertexBuffer(Geometry& geometry, const FrameProfilerStatistics::CounterValues& counterValues);
        void updateTranslationForCurrentFrame(Matrix44f& mvpMatrix, UInt32 currentFrameId);

        void render(const Renderable& drawable);
    };
}

#endif
