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
                m_state.depthWriteState.reset();
                m_state.colorWriteMaskState.reset();
                m_state.scissorState.reset();
            }
        }

        const RenderableVector& orderedRenderables = scene.getOrderedRenderablesForPass(pass);
        while (m_state.m_currentRenderIterator.getRenderableIdx() < orderedRenderables.size())
        {
            const RenderableHandle renderableHandle = orderedRenderables[m_state.m_currentRenderIterator.getRenderableIdx()];
            if (!scene.renderableResourcesDirty(renderableHandle))
            {
                assert(!scene.isRenderableVertexArrayDirty(renderableHandle));
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

        if(m_state.scissorState.hasChanged())
            device.scissorTest(m_state.scissorState.getState().m_scissorTest, m_state.scissorState.getState().m_scissorRegion);

        if (m_state.depthFuncState.hasChanged())
        {
            const EDepthFunc depthFunc = m_state.depthFuncState.getState();
            device.depthFunc(depthFunc);
        }

        if (m_state.depthWriteState.hasChanged())
        {
            const EDepthWrite depthWrite = m_state.depthWriteState.getState();
            device.depthWrite(depthWrite);
        }

        if (m_state.stencilState.hasChanged())
        {
            const auto& state = m_state.stencilState.getState();
            device.stencilFunc(state.m_stencilFunc, state.m_stencilRefValue, state.m_stencilMask);
            device.stencilOp(state.m_stencilOpFail, state.m_stencilOpDepthFail, state.m_stencilOpDepthPass);
        }

        if (m_state.blendOperationsState.hasChanged())
        {
            const auto& state = m_state.blendOperationsState.getState();
            device.blendOperations(state.m_blendOperationColor, state.m_blendOperationAlpha);
        }

        if (m_state.blendFactorsState.hasChanged())
        {
            const auto& state = m_state.blendFactorsState.getState();
            device.blendFactors(state.m_blendFactorSrcColor, state.m_blendFactorDstColor, state.m_blendFactorSrcAlpha, state.m_blendFactorDstAlpha);
        }

        if (m_state.blendColorState.hasChanged())
        {
            const auto& blendColor = m_state.blendColorState.getState();
            device.blendColor(blendColor);
        }

        if (m_state.colorWriteMaskState.hasChanged())
        {
            const ColorWriteMask colorMask = m_state.colorWriteMaskState.getState();
            const Bool writeR = (colorMask & EColorWriteFlag_Red) != 0u;
            const Bool writeG = (colorMask & EColorWriteFlag_Green) != 0u;
            const Bool writeB = (colorMask & EColorWriteFlag_Blue) != 0u;
            const Bool writeA = (colorMask & EColorWriteFlag_Alpha) != 0u;
            device.colorMask(writeR, writeG, writeB, writeA);
        }

        if (m_state.cullModeState.hasChanged())
        {
            const ECullMode cullMode = m_state.cullModeState.getState();
            device.cullMode(cullMode);
        }

        device.drawMode(m_state.drawMode);
    }

    void RenderExecutor::executeEffectAndInputs() const
    {
        const RendererCachedScene& renderScene = m_state.getScene();
        IDevice& device = m_state.getDevice();
        const Renderable& renderable = renderScene.getRenderable(m_state.getRenderable());
        const DataInstanceHandle uniformData = renderable.dataInstances[ERenderableDataSlotType_Uniforms];
        assert(uniformData.isValid());

        if (m_state.shaderDeviceHandle.hasChanged())
            device.activateShader(m_state.shaderDeviceHandle.getState());

        device.activateVertexArray(m_state.vertexArrayDeviceHandle);

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
            device.activateTextureSamplerObject(samplerStates, dataInstancefield);
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

        if (m_state.vertexArrayUsesIndices)
            device.drawIndexedTriangles(renderable.startIndex, renderable.indexCount, renderable.instanceCount);
        else
            device.drawTriangles(renderable.startIndex, renderable.indexCount, renderable.instanceCount);
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

        const auto& vertexArray = renderScene.getCachedHandlesForVertexArrays()[m_state.getRenderable().asMemoryHandle()];
        m_state.vertexArrayDeviceHandle = vertexArray.deviceHandle;
        m_state.vertexArrayUsesIndices = vertexArray.usesIndexArray;

        const RenderState& renderState = renderScene.getRenderState(renderable.renderState);
        ScissorState scissorState;
        scissorState.m_scissorTest = renderState.scissorTest;
        scissorState.m_scissorRegion = renderState.scissorRegion;
        m_state.scissorState.setState(scissorState);

        m_state.depthFuncState.setState(renderState.depthFunc);
        m_state.depthWriteState.setState(renderState.depthWrite);

        StencilState stencilState;
        stencilState.m_stencilFunc              = renderState.stencilFunc;
        stencilState.m_stencilMask              = renderState.stencilMask;
        stencilState.m_stencilOpDepthFail       = renderState.stencilOpDepthFail;
        stencilState.m_stencilOpDepthPass       = renderState.stencilOpDepthPass;
        stencilState.m_stencilOpFail            = renderState.stencilOpFail;
        stencilState.m_stencilRefValue          = renderState.stencilRefValue;
        m_state.stencilState.setState(stencilState);

        BlendFactorsState blendFactorsState;
        blendFactorsState.m_blendFactorSrcColor = renderState.blendFactorSrcColor;
        blendFactorsState.m_blendFactorDstColor = renderState.blendFactorDstColor;
        blendFactorsState.m_blendFactorSrcAlpha = renderState.blendFactorSrcAlpha;
        blendFactorsState.m_blendFactorDstAlpha = renderState.blendFactorDstAlpha;
        m_state.blendFactorsState.setState(blendFactorsState);

        BlendOperationsState blendOperationsState;
        blendOperationsState.m_blendOperationColor = renderState.blendOperationColor;
        blendOperationsState.m_blendOperationAlpha = renderState.blendOperationAlpha;
        m_state.blendOperationsState.setState(blendOperationsState);

        m_state.blendColorState.setState(renderState.blendColor);
        m_state.colorWriteMaskState.setState(renderState.colorWriteMask);
        m_state.cullModeState.setState(renderState.cullMode);

        m_state.drawMode = renderState.drawMode;
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
