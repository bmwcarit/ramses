//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderExecutor.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererAPI/IDevice.h"
#include "SceneAPI/BlitPass.h"

namespace ramses_internal
{
    UInt32 RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks = RenderExecutor::DefaultNumRenderablesToRenderInBetweenTimeBudgetChecks;

    RenderExecutor::RenderExecutor(IDevice& device, const TargetBufferInfo& bufferInfo, const SceneRenderExecutionIterator& renderFrom, const FrameTimer* frameTimer)
        : m_state(device, bufferInfo, renderFrom, frameTimer)
    {
    }

    SceneRenderExecutionIterator RenderExecutor::executeScene(const RendererCachedScene& scene) const
    {
        setGlobalInternalStates(scene);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        for ( ; m_state.m_currentRenderIterator.getRenderPassIdx() < orderedPasses.size(); m_state.m_currentRenderIterator.incrementRenderPassIdx())
        {
            const auto& passInfo = orderedPasses[m_state.m_currentRenderIterator.getRenderPassIdx()];
            switch (passInfo.getType())
            {
            case ERenderingPassType::RenderPass:
                if (!executeRenderPass(scene, passInfo.getRenderPassHandle()))
                {
                    assert(m_state.m_currentRenderIterator.getFlattenedRenderableIdx() > 0);
                    return m_state.m_currentRenderIterator;
                }
                break;
            case ERenderingPassType::BlitPass:
                executeBlitPass(scene, passInfo.getBlitPassHandle());
                break;
            default:
                assert(false);
            }
        }

        return {};
    }

    Bool RenderExecutor::executeRenderPass(const RendererCachedScene& scene, const RenderPassHandle pass) const
    {
        const RenderPass& renderPass = scene.getRenderPass(pass);
        executeRenderTarget(renderPass.renderTarget);
        executeCamera(renderPass.camera);

        // do not clear if render pass is not executed from beginning
        const Bool renderPassIsExecutedFromBeginning = (m_state.m_currentRenderIterator.getRenderableIdx() == 0u);
        m_state.renderPassState.setState(pass);
        if (m_state.renderPassState.hasChanged() && renderPassIsExecutedFromBeginning)
        {
            // Clear only if rendering into render target, not framebuffer
            const RenderTargetHandle renderTarget = m_state.renderTargetState.getState();
            const UInt32 clearFlags = renderPass.clearFlags;
            if (renderTarget.isValid() && clearFlags != EClearFlags_None)
            {
                if (clearFlags & EClearFlags_Color)
                {
                    m_state.getDevice().colorMask(true, true, true, true);
                    m_state.getDevice().clearColor(renderPass.clearColor);
                }
                if (clearFlags & EClearFlags_Depth)
                {
                    m_state.getDevice().depthWrite(EDepthWrite::Enabled);
                }

                m_state.getDevice().scissorTest(EScissorTest::Disabled, {});

                m_state.getDevice().clear(clearFlags);

                //reset cached render states that were updated on device before clearing
                m_state.depthStencilState.reset();
                m_state.blendState.reset();
                m_state.rasterizerState.reset();
            }
        }

        const RenderableVector& orderedRenderables = scene.getOrderedRenderablesForPass(pass);
        while (m_state.m_currentRenderIterator.getRenderableIdx() < orderedRenderables.size())
        {
            const RenderableHandle renderableHandle = orderedRenderables[m_state.m_currentRenderIterator.getRenderableIdx()];
            if (!scene.renderableResourcesDirty(renderableHandle))
            {
                setRenderableInternalStates(renderableHandle);
                setSemanticDataFields();
                executeRenderable();
            }
            m_state.m_currentRenderIterator.incrementRenderableIdx();

            if ((m_state.m_currentRenderIterator.getFlattenedRenderableIdx() % NumRenderablesToRenderInBetweenTimeBudgetChecks == 0u) && m_state.hasExceededTimeBudgetForRendering())
                return false;
        }

        return true;
    }

    void RenderExecutor::executeRenderable() const
    {
        executeRenderStates();

        executeEffectAndInputs();

        executeDrawCall();
    }

    void RenderExecutor::executeRenderTarget(RenderTargetHandle renderTarget) const
    {
        m_state.renderTargetState.setState(renderTarget);
        if (m_state.renderTargetState.hasChanged())
        {
            activateRenderTarget(renderTarget);
        }
    }

    void RenderExecutor::executeCamera(CameraHandle camera) const
    {
        m_state.setCamera(camera);

        if (m_state.viewportState.hasChanged())
        {
            const Viewport& viewport = m_state.viewportState.getState();
            m_state.getDevice().setViewport(viewport.posX, viewport.posY, viewport.width, viewport.height);
        }
    }

    void RenderExecutor::executeRenderStates() const
    {
        IDevice& device = m_state.getDevice();

        // TODO Mohamed: merge with rasterizer cached state, first check wrong cached states due to explicit state change on clear
        device.scissorTest(m_state.scissorState.m_scissorTest, m_state.scissorState.m_scissorRegion);

        if (m_state.depthStencilState.hasChanged())
        {
            const DepthStencilState& depthStencilState = m_state.depthStencilState.getState();
            device.depthFunc(depthStencilState.m_depthFunc);
            device.depthWrite(depthStencilState.m_depthWrite);
            device.stencilFunc(depthStencilState.m_stencilFunc, depthStencilState.m_stencilRefValue, depthStencilState.m_stencilMask);
            device.stencilOp(depthStencilState.m_stencilOpFail, depthStencilState.m_stencilOpDepthFail, depthStencilState.m_stencilOpDepthPass);
        }

        if (m_state.blendState.hasChanged())
        {
            const BlendState& blendState = m_state.blendState.getState();
            device.blendOperations(blendState.m_blendOperationColor, blendState.m_blendOperationAlpha);
            device.blendFactors(blendState.m_blendFactorSrcColor, blendState.m_blendFactorDstColor, blendState.m_blendFactorSrcAlpha, blendState.m_blendFactorDstAlpha);
            device.blendColor(blendState.m_blendColor);
            const ColorWriteMask colorMask = blendState.m_colorWriteMask;
            const Bool writeR = (colorMask & EColorWriteFlag_Red) != 0u;
            const Bool writeG = (colorMask & EColorWriteFlag_Green) != 0u;
            const Bool writeB = (colorMask & EColorWriteFlag_Blue) != 0u;
            const Bool writeA = (colorMask & EColorWriteFlag_Alpha) != 0u;
            device.colorMask(writeR, writeG, writeB, writeA);
        }

        if (m_state.rasterizerState.hasChanged())
        {
            const RasterizerState& rasterizerState = m_state.rasterizerState.getState();
            device.cullMode(rasterizerState.m_cullMode);
            device.drawMode(rasterizerState.m_drawMode);
        }
    }

    void RenderExecutor::executeEffectAndInputs() const
    {
        const RendererCachedScene& renderScene = m_state.getScene();
        IDevice& device = m_state.getDevice();
        const Renderable& renderable = renderScene.getRenderable(m_state.getRenderable());
        const DataInstanceHandle uniformData = renderable.dataInstances[ERenderableDataSlotType_Uniforms];
        const DataInstanceHandle vertexData = renderable.dataInstances[ERenderableDataSlotType_Geometry];
        assert(uniformData.isValid());
        assert(vertexData.isValid());

        if (m_state.shaderDeviceHandle.hasChanged())
        {
            device.activateShader(m_state.shaderDeviceHandle.getState());
        }

        const auto& vtxCache = renderScene.getCachedHandlesForVertexAttributes(vertexData);
        // Vertex attributes cache contains indices as first element, therefore the vertex attributes are shifted by 1 when accessing them
        assert(vtxCache.size() > 0);
        const UInt attributesCount = vtxCache.size() - 1;
        for (DataFieldHandle attributeField(0u); attributeField < attributesCount ; ++attributeField)
        {
            const auto& attribCache = vtxCache[attributeField.asMemoryHandle() + 1];
            assert(attribCache.deviceHandle.isValid());
            const auto& resourceField = renderScene.getDataResource(vertexData, attributeField + 1);
            device.activateVertexBuffer(attribCache.deviceHandle, attributeField, resourceField.instancingDivisor, renderable.startVertex, attribCache.dataType, resourceField.offsetWithinElementInBytes, resourceField.stride);
        }

        const DataLayoutHandle dataLayoutHandle = renderScene.getLayoutOfDataInstance(uniformData);
        const DataLayout& dataLayout = renderScene.getDataLayout(dataLayoutHandle);
        const UInt32 uniformsCount = dataLayout.getFieldCount();
        for (DataFieldHandle constantField(0u); constantField < uniformsCount; ++constantField)
        {
            const DataFieldInfo& field = dataLayout.getField(constantField);
            if (field.dataType == EDataType::DataReference)
            {
                DataInstanceHandle dataRef = renderScene.getDataReference(uniformData, constantField);
                const DataLayoutHandle dataRefLayout = renderScene.getLayoutOfDataInstance(dataRef);
                const EDataType dataTypeRef = renderScene.getDataLayout(dataRefLayout).getField(DataFieldHandle(0u)).dataType;
                executeConstant(dataTypeRef, 1u, dataRef, DataFieldHandle(0u), constantField);
            }
            else
            {
                executeConstant(field.dataType, field.elementCount, uniformData, constantField, constantField);
            }
        }
    }

    void RenderExecutor::executeConstant(EDataType dataType, UInt32 elementCount, DataInstanceHandle dataInstance, DataFieldHandle dataInstancefield, DataFieldHandle uniformInputField) const
    {
        IDevice& device = m_state.getDevice();
        const ResourceCachedScene& renderScene = m_state.getScene();

        switch (dataType)
        {
        case EDataType::Float:
        {
            const Float* value = renderScene.getDataFloatArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Vector2F:
        {
            const Vector2* value = renderScene.getDataVector2fArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Vector3F:
        {
            const Vector3* value = renderScene.getDataVector3fArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Vector4F:
        {
            const Vector4* value = renderScene.getDataVector4fArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Matrix22F:
        {
            const Matrix22f* value = renderScene.getDataMatrix22fArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Matrix33F:
        {
            const Matrix33f* value = renderScene.getDataMatrix33fArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Matrix44F:
        {
            const Matrix44f* value = renderScene.getDataMatrix44fArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Int32:
        {
            const Int32* value = renderScene.getDataIntegerArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Vector2I:
        {
            const Vector2i* value = renderScene.getDataVector2iArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Vector3I:
        {
            const Vector3i* value = renderScene.getDataVector3iArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::Vector4I:
        {
            const Vector4i* value = renderScene.getDataVector4iArray(dataInstance, dataInstancefield);
            device.setConstant(uniformInputField, elementCount, value);
            break;
        }

        case EDataType::DataReference:
            assert(false && "Multiple level data referencing not supported");
            break;

        case EDataType::TextureSampler2D:
        case EDataType::TextureSampler3D:
        case EDataType::TextureSamplerCube:
        {
            const TextureSamplerHandle samplerHandle = renderScene.getDataTextureSamplerHandle(dataInstance, dataInstancefield);
            assert(samplerHandle.isValid());

            const DeviceResourceHandle textureDeviceHandle = renderScene.getCachedHandlesForTextureSamplers()[samplerHandle.asMemoryHandle()];
            assert(textureDeviceHandle.isValid());

            device.activateTexture(textureDeviceHandle, uniformInputField);

            const TextureSamplerStates& samplerStates = renderScene.getTextureSampler(samplerHandle).states;
            device.setTextureSampling(uniformInputField,
                samplerStates.m_addressModeU,
                samplerStates.m_addressModeV,
                samplerStates.m_addressModeR,
                samplerStates.m_minSamplingMode,
                samplerStates.m_magSamplingMode,
                samplerStates.m_anisotropyLevel);
            break;
        }
        case EDataType::TextureSampler2DMS:
        {
            const TextureSamplerHandle samplerHandle = renderScene.getDataTextureSamplerHandle(dataInstance, dataInstancefield);
            assert(samplerHandle.isValid());

            const DeviceResourceHandle textureDeviceHandle = renderScene.getCachedHandlesForTextureSamplers()[samplerHandle.asMemoryHandle()];
            assert(textureDeviceHandle.isValid());

            device.activateTexture(textureDeviceHandle, uniformInputField);
            break;
        }

        default:
            assert(false && "Unrecognized data type");
        }
    }

    void RenderExecutor::executeDrawCall() const
    {
        IDevice& device = m_state.getDevice();
        const auto& renderScene = m_state.getScene();
        const Renderable& renderable = renderScene.getRenderable(m_state.getRenderable());

        const bool hasIndexArray = m_state.indexBufferDeviceHandle.getState() != DeviceResourceHandle::Invalid();

        if (hasIndexArray && m_state.indexBufferDeviceHandle.hasChanged())
        {
            device.activateIndexBuffer(m_state.indexBufferDeviceHandle.getState());
        }

        if (hasIndexArray)
        {
            device.drawIndexedTriangles(renderable.startIndex, renderable.indexCount, renderable.instanceCount);
        }
        else
        {
            device.drawTriangles(renderable.startIndex, renderable.indexCount, renderable.instanceCount);
        }
    }

    void RenderExecutor::setGlobalInternalStates(const RendererCachedScene& scene) const
    {
        m_state.setScene(scene);
    }

    void RenderExecutor::setRenderableInternalStates(RenderableHandle renderableHandle) const
    {
        m_state.setRenderable(renderableHandle);

        const RendererCachedScene& renderScene = m_state.getScene();
        const Renderable& renderable = renderScene.getRenderable(renderableHandle);

        const DeviceResourceHandle effectDeviceHandle = renderScene.getRenderableEffectDeviceHandle(renderableHandle);
        m_state.shaderDeviceHandle.setState(effectDeviceHandle);

        const DataInstanceHandle vertexData = renderable.dataInstances[ERenderableDataSlotType_Geometry];
        const auto& vtxCache = renderScene.getCachedHandlesForVertexAttributes(vertexData);
        m_state.indexBufferDeviceHandle.setState(vtxCache.front().deviceHandle);

        const RenderState& renderState = renderScene.getRenderState(renderable.renderState);

        m_state.scissorState.m_scissorTest = renderState.scissorTest;
        m_state.scissorState.m_scissorRegion = renderState.scissorRegion;

        DepthStencilState depthStencilState;
        depthStencilState.m_depthFunc          = renderState.depthFunc;
        depthStencilState.m_depthWrite         = renderState.depthWrite;
        depthStencilState.m_stencilFunc        = renderState.stencilFunc;
        depthStencilState.m_stencilMask        = renderState.stencilMask;
        depthStencilState.m_stencilOpDepthFail = renderState.stencilOpDepthFail;
        depthStencilState.m_stencilOpDepthPass = renderState.stencilOpDepthPass;
        depthStencilState.m_stencilOpFail      = renderState.stencilOpFail;
        depthStencilState.m_stencilRefValue    = renderState.stencilRefValue;
        m_state.depthStencilState.setState(depthStencilState);


        BlendState blendState;
        blendState.m_blendFactorSrcColor = renderState.blendFactorSrcColor;
        blendState.m_blendFactorDstColor = renderState.blendFactorDstColor;
        blendState.m_blendFactorSrcAlpha = renderState.blendFactorSrcAlpha;
        blendState.m_blendFactorDstAlpha = renderState.blendFactorDstAlpha;
        blendState.m_blendOperationColor = renderState.blendOperationColor;
        blendState.m_blendOperationAlpha = renderState.blendOperationAlpha;
        blendState.m_blendColor = renderState.blendColor;
        blendState.m_colorWriteMask      = renderState.colorWriteMask;
        m_state.blendState.setState(blendState);

        RasterizerState rasterizerState;
        rasterizerState.m_cullMode = renderState.cullMode;
        rasterizerState.m_drawMode = renderState.drawMode;
        m_state.rasterizerState.setState(rasterizerState);
    }

    void RenderExecutor::activateRenderTarget(RenderTargetHandle renderTarget) const
    {
        DeviceResourceHandle renderTargetDeviceResource;
        if (renderTarget.isValid())
        {
            const RendererCachedScene& scene = m_state.getScene();
            const DeviceHandleVector& rtHandles = scene.getCachedHandlesForRenderTargets();
            assert(renderTarget.asMemoryHandle() < rtHandles.size());
            renderTargetDeviceResource = rtHandles[renderTarget.asMemoryHandle()];
        }
        else
        {
            // Framebuffer
            renderTargetDeviceResource = m_state.getTargetBufferInfo().deviceHandle;
        }

        IDevice& device = m_state.getDevice();
        device.activateRenderTarget(renderTargetDeviceResource);
    }

    void RenderExecutor::resolveAndSetSemanticDataField(EFixedSemantics semantics, DataInstanceHandle dataInstHandle, DataFieldHandle dataFieldHandle) const
    {
        // semantic data is 'cached' directly in scene, for this special case non-const access is needed
        auto& scene = const_cast<RendererCachedScene&>(m_state.getScene());
        switch (semantics)
        {
        case EFixedSemantics::ViewMatrix:
        {
            const Matrix44f& mat = m_state.getViewMatrix();
            scene.setDataSingleMatrix44f(dataInstHandle, dataFieldHandle, mat);
            break;
        }
        case EFixedSemantics::ProjectionMatrix:
        {
            const Matrix44f& mat = m_state.getProjectionMatrix();
            scene.setDataSingleMatrix44f(dataInstHandle, dataFieldHandle, mat);
            break;
        }
        case EFixedSemantics::ModelMatrix:
        {
            const Matrix44f& mat = m_state.getModelMatrix();
            scene.setDataSingleMatrix44f(dataInstHandle, dataFieldHandle, mat);
            break;
        }
        case EFixedSemantics::ModelViewMatrix:
        {
            const Matrix44f& mat = m_state.getModelViewMatrix();
            scene.setDataSingleMatrix44f(dataInstHandle, dataFieldHandle, mat);
            break;
        }
        case EFixedSemantics::ModelViewMatrix33:
        {
            const Matrix33f mat = Matrix33f(m_state.getModelViewMatrix());
            scene.setDataSingleMatrix33f(dataInstHandle, dataFieldHandle, mat);
            break;
        }
        case EFixedSemantics::ModelViewProjectionMatrix:
        {
            const Matrix44f& mat = m_state.getModelViewProjectionMatrix();
            scene.setDataSingleMatrix44f(dataInstHandle, dataFieldHandle, mat);
            break;
        }
        case EFixedSemantics::CameraWorldPosition:
        {
            const Vector3& pos = m_state.getCameraWorldPosition();
            scene.setDataSingleVector3f(dataInstHandle, dataFieldHandle, pos);
            break;
        }
        case EFixedSemantics::NormalMatrix:
        {
            const Matrix44f& mat = m_state.getModelViewMatrix().inverse().transpose();
            scene.setDataSingleMatrix44f(dataInstHandle, dataFieldHandle, mat);
            break;
        }
        case EFixedSemantics::DisplayBufferResolution:
        {
            const Vector2 bufferRes{ float(m_state.getTargetBufferInfo().width), float(m_state.getTargetBufferInfo().height) };
            scene.setDataSingleVector2f(dataInstHandle, dataFieldHandle, bufferRes);
            break;
        }
        case EFixedSemantics::TextTexture:
            // used on client side only
            break;
        default:
            assert(false && "Unsupported semantics");
            break;
        }
    }

    void RenderExecutor::setSemanticDataFields() const
    {
        const auto& scene = m_state.getScene();
        const RenderableHandle renderable = m_state.getRenderable();
        const DataInstanceHandle dataInstance = scene.getRenderable(renderable).dataInstances[ERenderableDataSlotType_Uniforms];
        const DataLayoutHandle dataLayoutHandle = scene.getLayoutOfDataInstance(dataInstance);
        const DataLayout& dataLayout = scene.getDataLayout(dataLayoutHandle);

        const UInt32 fieldCount = dataLayout.getFieldCount();
        for (DataFieldHandle i(0u); i < fieldCount; ++i)
        {
            const EFixedSemantics semantics = dataLayout.getField(i).semantics;
            if (semantics != EFixedSemantics::Invalid)
            {
                resolveAndSetSemanticDataField(semantics, dataInstance, i);
            }
        }
    }

    void RenderExecutor::executeBlitPass(const RendererCachedScene& scene, const BlitPassHandle pass) const
    {
        //set invalid render target to state
        m_state.renderTargetState.setState(RenderTargetHandle::Invalid() - 1);

        const BlitPass& blitPass = scene.getBlitPass(pass);
        const UInt indexToCache = pass.asMemoryHandle() * 2u;
        const DeviceResourceHandle blitPassRenderTargetSrcDeviceHandle = scene.getCachedHandlesForBlitPassRenderTargets()[indexToCache];
        const DeviceResourceHandle blitPassRenderTargetDstDeviceHandle = scene.getCachedHandlesForBlitPassRenderTargets()[indexToCache + 1u];
        m_state.getDevice().blitRenderTargets(blitPassRenderTargetSrcDeviceHandle, blitPassRenderTargetDstDeviceHandle, blitPass.sourceRegion, blitPass.destinationRegion, false);
    }
}
