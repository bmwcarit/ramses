//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneActionApplier.h"
#include "Scene/TransformPropertyType.h"
#include "Scene/SceneResourceChanges.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/PixelRectangle.h"
#include "SceneAPI/TextureSampler.h"
#include "SceneAPI/Viewport.h"
#include "SceneAPI/Camera.h"
#include "SceneAPI/RenderBuffer.h"
#include "AnimationAPI/IAnimationSystem.h"
#include "Animation/AnimationSystemFactory.h"
#include "Animation/Animation.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"
#include "Components/SingleResourceSerialization.h"
#include "Components/FlushTimeInformation.h"
#include "Resource/IResource.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/LogMacros.h"

#define ALLOCATE_AND_ASSERT_HANDLE(_allocateExpr, _handleToCheck) \
{ \
    assert(_handleToCheck.isValid()); \
    const auto _actualHandle = _allocateExpr; \
    assert(_handleToCheck == _actualHandle); \
    UNUSED(_actualHandle); \
}

namespace ramses_internal
{
    void SceneActionApplier::ApplySingleActionOnScene(IScene& scene, SceneActionCollection::SceneActionReader& action, AnimationSystemFactory* animSystemFactory, ResourceVector* resources)
    {
        switch (action.type())
        {
        case ESceneActionId_AllocateNode:
        {
            UInt32 childrenCount = 0u;
            NodeHandle nodeHandle;
            action.read(childrenCount);
            action.read(nodeHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateNode(childrenCount, nodeHandle), nodeHandle);
            break;
        }
        case ESceneActionId_AddChildToNode:
        {
            NodeHandle node;
            NodeHandle child;
            action.read(node);
            action.read(child);
            scene.addChildToNode(node, child);
            break;
        }
        case ESceneActionId_AllocateTransform:
        {
            NodeHandle nodeHandle;
            TransformHandle transformHandle;
            action.read(nodeHandle);
            action.read(transformHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateTransform(nodeHandle, transformHandle), transformHandle);
            break;
        }
        case ESceneActionId_SetTransformComponent:
        {
            TransformHandle transform;
            UInt32 component = 0;
            Vector3 vec;
            action.read(component);
            action.read(transform);
            action.read(vec.data);
            switch (component)
            {
            case ETransformPropertyType_Rotation:
            {
                scene.setRotation(transform, vec);
                break;
            }
            case ETransformPropertyType_Scaling:
            {
                scene.setScaling(transform, vec);
                break;
            }
            case ETransformPropertyType_Translation:
            {
                scene.setTranslation(transform, vec);
                break;
            }
            };
            break;
        }
        case ESceneActionId_AllocateDataInstance:
        {
            DataLayoutHandle dataLayout;
            DataInstanceHandle diHandle;
            action.read(dataLayout);
            action.read(diHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateDataInstance(dataLayout, diHandle), diHandle);
            break;
        }
        case ESceneActionId_SetDataFloatArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            Float* const array = const_cast<Float*>(scene.getDataFloatArray(handle, field));
            action.read(array, elementCount);
            scene.setDataFloatArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataVector2fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Vector2* array = const_cast<Vector2*>(scene.getDataVector2fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataVector2fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataVector3fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Vector3* array = const_cast<Vector3*>(scene.getDataVector3fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataVector3fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataVector4fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Vector4* array = const_cast<Vector4*>(scene.getDataVector4fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataVector4fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataMatrix22fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Matrix22f* array = const_cast<Matrix22f*>(scene.getDataMatrix22fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataMatrix22fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataMatrix33fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Matrix33f* array = const_cast<Matrix33f*>(scene.getDataMatrix33fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataMatrix33fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataMatrix44fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Matrix44f* array = const_cast<Matrix44f*>(scene.getDataMatrix44fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataMatrix44fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataIntegerArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            Int32* const array = const_cast<Int32*>(scene.getDataIntegerArray(handle, field));
            action.read(array, elementCount);
            scene.setDataIntegerArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataVector2iArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Vector2i* array = const_cast<Vector2i*>(scene.getDataVector2iArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataVector2iArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataVector3iArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Vector3i* array = const_cast<Vector3i*>(scene.getDataVector3iArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataVector3iArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataVector4iArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            Vector4i* array = const_cast<Vector4i*>(scene.getDataVector4iArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i].data);
            scene.setDataVector4iArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId_SetDataResource:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            ResourceContentHash hash;
            DataBufferHandle dataBuffer;
            UInt32 instancingDivisor;
            action.read(handle);
            action.read(field);
            action.read(hash);
            action.read(dataBuffer);
            action.read(instancingDivisor);
            scene.setDataResource(handle, field, hash, dataBuffer, instancingDivisor);
            break;
        }
        case ESceneActionId_SetDataTextureSamplerHandle:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            TextureSamplerHandle samplerHandle;
            action.read(handle);
            action.read(field);
            action.read(samplerHandle);
            scene.setDataTextureSamplerHandle(handle, field, samplerHandle);
            break;
        }
        case ESceneActionId_SetDataReference:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            DataInstanceHandle dataRef;
            action.read(handle);
            action.read(field);
            action.read(dataRef);
            scene.setDataReference(handle, field, dataRef);
            break;
        }
        case ESceneActionId_AllocateRenderable:
        {
            NodeHandle node;
            RenderableHandle handle;
            action.read(node);
            action.read(handle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderable(node, handle), handle);
            break;
        }
        case ESceneActionId_SetRenderableEffect:
        {
            RenderableHandle renderable;
            ResourceContentHash effectHash;
            action.read(renderable);
            action.read(effectHash);
            scene.setRenderableEffect(renderable, effectHash);
            break;
        }
        case ESceneActionId_RemoveChildFromNode:
        {
            NodeHandle parent;
            NodeHandle child;
            action.read(parent);
            action.read(child);
            scene.removeChildFromNode(parent, child);
            break;
        }
        case ESceneActionId_AllocateDataLayout:
        {
            DataLayoutHandle layoutHandle;
            action.read(layoutHandle);

            UInt32 dataFieldCount = 0u;
            action.read(dataFieldCount);
            DataFieldInfoVector dataFields(dataFieldCount);
            UInt32 uintValue = 0;
            for (DataFieldInfo& dataField : dataFields)
            {
                action.read(uintValue);
                dataField.dataType = static_cast<EDataType>(uintValue);
                action.read(dataField.elementCount);
                action.read(uintValue);
                dataField.semantics = static_cast<EFixedSemantics>(uintValue);
            }
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateDataLayout(dataFields, layoutHandle), layoutHandle);
            break;
        }
        case ESceneActionId_ReleaseDataLayout:
        {
            DataLayoutHandle layout;
            action.read(layout);
            scene.releaseDataLayout(layout);
            break;
        }
        case ESceneActionId_ReleaseDataInstance:
        {
            DataInstanceHandle instance;
            action.read(instance);
            scene.releaseDataInstance(instance);
            break;
        }
        case ESceneActionId_SetRenderableStartIndex:
        {
            RenderableHandle renderable;
            UInt32 startIndex = 0;
            action.read(renderable);
            action.read(startIndex);
            scene.setRenderableStartIndex(renderable, startIndex);
            break;
        }
        case ESceneActionId_SetRenderableIndexCount:
        {
            RenderableHandle renderable;
            UInt32 count = 0;
            action.read(renderable);
            action.read(count);
            scene.setRenderableIndexCount(renderable, count);
            break;
        }
        case ESceneActionId_SetRenderableVisibility:
        {
            RenderableHandle renderable;
            Bool visibility = true;
            action.read(renderable);
            action.read(visibility);
            scene.setRenderableVisibility(renderable, visibility);
            break;
        }
        case ESceneActionId_SetRenderableDataInstance:
        {
            RenderableHandle renderable;
            UInt32 slot;
            DataInstanceHandle diHandle;
            action.read(renderable);
            action.read(slot);
            action.read(diHandle);
            scene.setRenderableDataInstance(renderable, static_cast<ERenderableDataSlotType>(slot), diHandle);
            break;
        }
        case ESceneActionId_SetRenderableInstanceCount:
        {
            RenderableHandle renderable;
            UInt32 numInstances = 1;
            action.read(renderable);
            action.read(numInstances);
            scene.setRenderableInstanceCount(renderable, numInstances);
            break;
        }
        case ESceneActionId_AllocateRenderGroup:
        {
            UInt32 renderableCount = 0u;
            UInt32 nestedGroupCount = 0u;
            RenderGroupHandle renderGroup;
            action.read(renderableCount);
            action.read(nestedGroupCount);
            action.read(renderGroup);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderGroup(renderableCount, nestedGroupCount, renderGroup), renderGroup);
            break;
        }
        case ESceneActionId_ReleaseRenderGroup:
        {
            RenderGroupHandle renderGroup;
            action.read(renderGroup);
            scene.releaseRenderGroup(renderGroup);
            break;
        }
        case ESceneActionId_AddRenderableToRenderGroup:
        {
            RenderGroupHandle renderGroup;
            RenderableHandle renderable;
            Int32 order = 0;
            action.read(renderGroup);
            action.read(renderable);
            action.read(order);
            scene.addRenderableToRenderGroup(renderGroup, renderable, order);
            break;
        }
        case ESceneActionId_RemoveRenderableFromRenderGroup:
        {
            RenderGroupHandle renderGroup;
            RenderableHandle renderable;
            action.read(renderGroup);
            action.read(renderable);
            scene.removeRenderableFromRenderGroup(renderGroup, renderable);
            break;
        }
        case ESceneActionId_AddRenderGroupToRenderGroup:
        {
            RenderGroupHandle renderGroupParent;
            RenderGroupHandle renderGroupChild;
            Int32 order = 0;
            action.read(renderGroupParent);
            action.read(renderGroupChild);
            action.read(order);
            scene.addRenderGroupToRenderGroup(renderGroupParent, renderGroupChild, order);
            break;
        }
        case ESceneActionId_RemoveRenderGroupFromRenderGroup:
        {
            RenderGroupHandle renderGroupParent;
            RenderGroupHandle renderGroupChild;
            action.read(renderGroupParent);
            action.read(renderGroupChild);
            scene.removeRenderGroupFromRenderGroup(renderGroupParent, renderGroupChild);
            break;
        }
        case ESceneActionId_ReleaseRenderable:
        {
            RenderableHandle renderable;
            action.read(renderable);
            scene.releaseRenderable(renderable);
            break;
        }
        case ESceneActionId_ReleaseTransform:
        {
            TransformHandle transformHandle;
            action.read(transformHandle);
            scene.releaseTransform(transformHandle);
            break;
        }
        case ESceneActionId_ReleaseNode:
        {
            NodeHandle nodeHandle;
            action.read(nodeHandle);
            scene.releaseNode(nodeHandle);
            break;
        }
        case ESceneActionId_AllocateStreamTexture:
        {
            StreamTextureHandle handle;
            uint32_t streamSource;
            ResourceContentHash fallbackTextureHash;
            action.read(handle);
            action.read(streamSource);
            action.read(fallbackTextureHash);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateStreamTexture(streamSource, fallbackTextureHash, handle), handle);
            break;
        }
        case ESceneActionId_ReleaseStreamTexture:
        {
            StreamTextureHandle handle;
            action.read(handle);
            scene.releaseStreamTexture(handle);
            break;
        }
        case ESceneActionId_SetForceFallback:
        {
            StreamTextureHandle handle;
            Bool forceFallback;
            action.read(handle);
            action.read(forceFallback);
            scene.setForceFallbackImage(handle, forceFallback);
            break;
        }
        case ESceneActionId_AllocateRenderState:
        {
            RenderStateHandle stateHandle;
            action.read(stateHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderState(stateHandle), stateHandle);
            break;
        }
        case ESceneActionId_ReleaseState:
        {
            RenderStateHandle stateHandle;
            action.read(stateHandle);
            scene.releaseRenderState(stateHandle);
            break;
        }
        case ESceneActionId_SetRenderableState:
        {
            RenderableHandle renderable;
            RenderStateHandle stateHandle;
            action.read(renderable);
            action.read(stateHandle);
            scene.setRenderableRenderState(renderable, stateHandle);
            break;
        }
        case ESceneActionId_SetStateStencilOps:
        {
            RenderStateHandle stateHandle;
            EStencilOp sfail = EStencilOp::NUMBER_OF_ELEMENTS;
            EStencilOp dpfail = EStencilOp::NUMBER_OF_ELEMENTS;
            EStencilOp dppass = EStencilOp::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(sfail);
            action.read(dpfail);
            action.read(dppass);
            scene.setRenderStateStencilOps(stateHandle, sfail, dpfail, dppass);
            break;
        }
        case ESceneActionId_SetStateStencilFunc:
        {
            RenderStateHandle stateHandle;
            EStencilFunc func = EStencilFunc::NUMBER_OF_ELEMENTS;
            uint8_t ref = 0;
            uint8_t mask = 0;
            action.read(stateHandle);
            action.read(func);
            action.read(ref);
            action.read(mask);
            scene.setRenderStateStencilFunc(stateHandle, func, ref, mask);
            break;
        }
        case ESceneActionId_SetStateDepthWrite:
        {
            RenderStateHandle stateHandle;
            EDepthWrite flag = EDepthWrite::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(flag);
            scene.setRenderStateDepthWrite(stateHandle, flag);
            break;
        }
        case ESceneActionId_SetStateScissorTest:
        {
            RenderStateHandle stateHandle;
            EScissorTest flag = EScissorTest::NUMBER_OF_ELEMENTS;
            RenderState::ScissorRegion region;
            action.read(stateHandle);
            action.read(flag);
            action.read(region.x);
            action.read(region.y);
            action.read(region.width);
            action.read(region.height);
            scene.setRenderStateScissorTest(stateHandle, flag, region);
            break;
        }
        case ESceneActionId_SetStateDepthFunc:
        {
            RenderStateHandle stateHandle;
            EDepthFunc func = EDepthFunc::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(func);
            scene.setRenderStateDepthFunc(stateHandle, func);
            break;
        }
        case ESceneActionId_SetStateCullMode:
        {
            RenderStateHandle stateHandle;
            ECullMode mode = ECullMode::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(mode);
            scene.setRenderStateCullMode(stateHandle, mode);
            break;
        }
        case ESceneActionId_SetStateDrawMode:
        {
            RenderStateHandle stateHandle;
            EDrawMode mode = EDrawMode::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(mode);
            scene.setRenderStateDrawMode(stateHandle, mode);
            break;
        }
        case ESceneActionId_SetStateBlendOperations:
        {
            RenderStateHandle stateHandle;
            EBlendOperation opColor = EBlendOperation::NUMBER_OF_ELEMENTS;
            EBlendOperation opAlpha = EBlendOperation::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(opColor);
            action.read(opAlpha);
            scene.setRenderStateBlendOperations(stateHandle, opColor, opAlpha);
            break;
        }
        case ESceneActionId_SetStateBlendFactors:
        {
            RenderStateHandle stateHandle;
            EBlendFactor srcColor = EBlendFactor::NUMBER_OF_ELEMENTS;
            EBlendFactor destColor = EBlendFactor::NUMBER_OF_ELEMENTS;
            EBlendFactor srcAlpha = EBlendFactor::NUMBER_OF_ELEMENTS;
            EBlendFactor destAlpha = EBlendFactor::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(srcColor);
            action.read(destColor);
            action.read(srcAlpha);
            action.read(destAlpha);
            scene.setRenderStateBlendFactors(stateHandle, srcColor, destColor, srcAlpha, destAlpha);
            break;
        }
        case ESceneActionId_SetStateColorWriteMask:
        {
            RenderStateHandle stateHandle;
            ColorWriteMask colorMask;
            action.read(stateHandle);
            action.read(colorMask);
            scene.setRenderStateColorWriteMask(stateHandle, colorMask);
            break;
        }
        case ESceneActionId_AllocateCamera:
        {
            UInt32 projType;
            NodeHandle nodeHandle;
            DataInstanceHandle dataInstHandle;
            CameraHandle cameraHandle;
            action.read(projType);
            action.read(nodeHandle);
            action.read(dataInstHandle);
            action.read(cameraHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateCamera(static_cast<ECameraProjectionType>(projType), nodeHandle, dataInstHandle, cameraHandle), cameraHandle);
            break;
        }
        case ESceneActionId_ReleaseCamera:
        {
            CameraHandle cameraHandle;
            action.read(cameraHandle);
            scene.releaseCamera(cameraHandle);
            break;
        }
        case ESceneActionId_SetCameraFrustum:
        {
            CameraHandle cameraHandle;
            Frustum frustum;

            action.read(cameraHandle);
            action.read(frustum.leftPlane);
            action.read(frustum.rightPlane);
            action.read(frustum.bottomPlane);
            action.read(frustum.topPlane);
            action.read(frustum.nearPlane);
            action.read(frustum.farPlane);
            scene.setCameraFrustum(cameraHandle, frustum);
            break;
        }
        case ESceneActionId_AllocateRenderPass:
        {
            UInt32 renderGroupCount = 0u;
            RenderPassHandle passHandle;
            action.read(renderGroupCount);
            action.read(passHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderPass(renderGroupCount, passHandle), passHandle);
            break;
        }
        case ESceneActionId_ReleaseRenderPass:
        {
            RenderPassHandle passHandle;
            action.read(passHandle);
            scene.releaseRenderPass(passHandle);
            break;
        }
        case ESceneActionId_SetRenderPassCamera:
        {
            RenderPassHandle passHandle;
            CameraHandle cameraHandle;
            action.read(passHandle);
            action.read(cameraHandle);
            scene.setRenderPassCamera(passHandle, cameraHandle);
            break;
        }
        case ESceneActionId_SetRenderPassRenderTarget:
        {
            RenderPassHandle passHandle;
            RenderTargetHandle targetHandle;
            action.read(passHandle);
            action.read(targetHandle);
            scene.setRenderPassRenderTarget(passHandle, targetHandle);
            break;
        }
        case ESceneActionId_SetRenderPassRenderOrder:
        {
            RenderPassHandle passHandle;
            Int32 renderOrder;
            action.read(passHandle);
            action.read(renderOrder);
            scene.setRenderPassRenderOrder(passHandle, renderOrder);
            break;
        }
        case ESceneActionId_SetRenderPassEnabled:
        {
            RenderPassHandle passHandle;
            Bool isEnabled;
            action.read(passHandle);
            action.read(isEnabled);
            scene.setRenderPassEnabled(passHandle, isEnabled);
            break;
        }
        case ESceneActionId_SetRenderPassRenderOnce:
        {
            RenderPassHandle passHandle;
            Bool enabled;
            action.read(passHandle);
            action.read(enabled);
            scene.setRenderPassRenderOnce(passHandle, enabled);
            break;
        }
        case ESceneActionId_RetriggerRenderPassRenderOnce:
        {
            RenderPassHandle passHandle;
            action.read(passHandle);
            scene.retriggerRenderPassRenderOnce(passHandle);
            break;
        }
        case ESceneActionId_AddRenderGroupToRenderPass:
        {
            RenderPassHandle passHandle;
            RenderGroupHandle groupHandle;
            Int32 order = 0;
            action.read(passHandle);
            action.read(groupHandle);
            action.read(order);
            scene.addRenderGroupToRenderPass(passHandle, groupHandle, order);
            break;
        }
        case ESceneActionId_RemoveRenderGroupFromRenderPass:
        {
            RenderPassHandle passHandle;
            RenderGroupHandle groupHandle;
            action.read(passHandle);
            action.read(groupHandle);
            scene.removeRenderGroupFromRenderPass(passHandle, groupHandle);
            break;
        }
        case ESceneActionId_AllocateBlitPass:
        {
            BlitPassHandle passHandle;
            RenderBufferHandle sourceRenderbufferHandle;
            RenderBufferHandle destinationRenderbufferHandle;
            action.read(sourceRenderbufferHandle);
            action.read(destinationRenderbufferHandle);
            action.read(passHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateBlitPass(sourceRenderbufferHandle, destinationRenderbufferHandle, passHandle), passHandle);
            break;
        }
        case ESceneActionId_ReleaseBlitPass:
        {
            BlitPassHandle passHandle;
            action.read(passHandle);
            scene.releaseBlitPass(passHandle);
            break;
        }
        case ESceneActionId_SetBlitPassRenderOrder:
        {
            BlitPassHandle passHandle;
            Int32 renderOrder;
            action.read(passHandle);
            action.read(renderOrder);
            scene.setBlitPassRenderOrder(passHandle, renderOrder);
            break;
        }
        case ESceneActionId_SetBlitPassEnabled:
        {
            BlitPassHandle passHandle;
            Bool isEnabled;
            action.read(passHandle);
            action.read(isEnabled);
            scene.setBlitPassEnabled(passHandle, isEnabled);
            break;
        }
        case ESceneActionId_SetBlitPassRegions:
        {
            BlitPassHandle passHandle;
            PixelRectangle sourceRectangle;
            PixelRectangle destinationRectangle;
            action.read(passHandle);
            action.read(sourceRectangle.x);
            action.read(sourceRectangle.y);
            action.read(sourceRectangle.width);
            action.read(sourceRectangle.height);
            action.read(destinationRectangle.x);
            action.read(destinationRectangle.y);
            action.read(destinationRectangle.width);
            action.read(destinationRectangle.height);
            scene.setBlitPassRegions(passHandle, sourceRectangle, destinationRectangle);
            break;
        }
        case ESceneActionId_AllocateTextureSampler:
        {
            TextureSamplerHandle samplerHandle;
            TextureSampler sampler;

            action.read(samplerHandle);
            action.read(sampler.states.m_addressModeU);
            action.read(sampler.states.m_addressModeV);
            action.read(sampler.states.m_addressModeR);
            action.read(sampler.states.m_minSamplingMode);
            action.read(sampler.states.m_magSamplingMode);
            action.read(sampler.states.m_anisotropyLevel);

            action.read(sampler.contentType);
            if (sampler.contentType == TextureSampler::ContentType::ClientTexture)
                action.read(sampler.textureResource);
            else
                action.read(sampler.contentHandle);

            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateTextureSampler(sampler, samplerHandle), samplerHandle);
            break;
        }
        case ESceneActionId_ReleaseTextureSampler:
        {
            TextureSamplerHandle samplerHandle;
            action.read(samplerHandle);
            scene.releaseTextureSampler(samplerHandle);
            break;
        }
        case ESceneActionId_AllocateRenderTarget:
        {
            RenderTargetHandle renderTargetHandle;
            action.read(renderTargetHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderTarget(renderTargetHandle), renderTargetHandle);
            break;
        }
        case ESceneActionId_ReleaseRenderTarget:
        {
            RenderTargetHandle renderTargetHandle;
            action.read(renderTargetHandle);
            scene.releaseRenderTarget(renderTargetHandle);
            break;
        }
        case ESceneActionId_AddRenderTargetRenderBuffer:
        {
            RenderTargetHandle renderTargetHandle;
            RenderBufferHandle renderBufferHandle;
            action.read(renderTargetHandle);
            action.read(renderBufferHandle);
            scene.addRenderTargetRenderBuffer(renderTargetHandle, renderBufferHandle);
            break;
        }
        case ESceneActionId_AllocateRenderBuffer:
        {
            RenderBufferHandle handle;
            RenderBuffer renderBuffer;
            UInt32 enumInt;

            action.read(renderBuffer.width);
            action.read(renderBuffer.height);
            action.read(handle);
            action.read(enumInt);
            renderBuffer.type = static_cast<ERenderBufferType>(enumInt);
            action.read(enumInt);
            renderBuffer.format = static_cast<ETextureFormat>(enumInt);
            action.read(enumInt);
            renderBuffer.accessMode = static_cast<ERenderBufferAccessMode>(enumInt);
            action.read(renderBuffer.sampleCount);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderBuffer(renderBuffer, handle), handle);
            break;
        }
        case ESceneActionId_ReleaseRenderBuffer:
        {
            RenderBufferHandle handle;
            action.read(handle);
            scene.releaseRenderBuffer(handle);
            break;
        }
        case ESceneActionId_AllocateDataSlot:
        {
            DataSlotHandle dataSlotHandle;
            UInt32 type = 0;
            DataSlot dataSlot;

            action.read(type);
            dataSlot.type = static_cast<EDataSlotType>(type);
            action.read(dataSlot.id);
            action.read(dataSlot.attachedNode);
            action.read(dataSlot.attachedDataReference);
            action.read(dataSlot.attachedTexture);
            action.read(dataSlot.attachedTextureSampler);
            action.read(dataSlotHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateDataSlot(dataSlot, dataSlotHandle), dataSlotHandle);
            break;
        }
        case ESceneActionId_SetDataSlotTexture:
        {
            DataSlotHandle dataSlotHandle(0);
            ResourceContentHash texture = ResourceContentHash::Invalid();

            action.read(dataSlotHandle);
            action.read(texture);
            scene.setDataSlotTexture(dataSlotHandle, texture);
            break;
        }
        case ESceneActionId_ReleaseDataSlot:
        {
            DataSlotHandle dataSlotHandle(0);
            action.read(dataSlotHandle);
            scene.releaseDataSlot(dataSlotHandle);
            break;
        }
        case ESceneActionId_SetRenderPassClearColor:
        {
            Vector4 clearColor;
            RenderPassHandle renderPassHandle;
            action.read(renderPassHandle);
            action.read(clearColor.data);
            scene.setRenderPassClearColor(renderPassHandle, clearColor);
            break;
        }
        case ESceneActionId_SetRenderPassClearFlag:
        {
            UInt32 clearFlag;
            RenderPassHandle renderPassHandle;
            action.read(renderPassHandle);
            action.read(clearFlag);
            scene.setRenderPassClearFlag(renderPassHandle, clearFlag);
            break;
        }
        case ESceneActionId_AllocateDataBuffer:
        {
            UInt32 dataBufferType;
            UInt32 dataType;
            UInt32 maximumSizeInBytes;
            DataBufferHandle handle;
            action.read(dataBufferType);
            action.read(dataType);
            action.read(maximumSizeInBytes);
            action.read(handle.asMemoryHandleReference());
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateDataBuffer(static_cast<EDataBufferType>(dataBufferType), static_cast<EDataType>(dataType), maximumSizeInBytes, handle), handle);
            break;
        }
        case ESceneActionId_ReleaseDataBuffer:
        {
            DataBufferHandle handle;
            action.read(handle.asMemoryHandleReference());
            scene.releaseDataBuffer(handle);
            break;
        }
        case ESceneActionId_UpdateDataBuffer:
        {
            DataBufferHandle handle;
            UInt32 offsetInBytes;
            UInt32 dataSizeInBytes;
            const Byte* data = nullptr;

            action.read(handle.asMemoryHandleReference());
            action.read(offsetInBytes);
            action.readWithoutCopy(data, dataSizeInBytes);
            scene.updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
            break;
        }
        case ESceneActionId_AllocateTextureBuffer:
        {
            UInt32 textureFormat;
            UInt32 mipLevelCount;
            MipMapDimensions mipMapDimensions;
            TextureBufferHandle handle;

            action.read(textureFormat);
            action.read(mipLevelCount);
            mipMapDimensions.reserve(mipLevelCount);
            for (UInt32 i = 0u; i < mipLevelCount; ++i)
            {
                UInt32 width;
                UInt32 height;
                action.read(width);
                action.read(height);
                mipMapDimensions.push_back({ width, height });
            }
            action.read(handle);

            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateTextureBuffer(static_cast<ETextureFormat>(textureFormat), mipMapDimensions, handle), handle);
            break;
        }
        case ESceneActionId_ReleaseTextureBuffer:
        {
            TextureBufferHandle handle;
            action.read(handle);

            scene.releaseTextureBuffer(handle);
            break;
        }
        case ESceneActionId_UpdateTextureBuffer:
        {
            TextureBufferHandle handle;
            UInt32 mipLevel;
            UInt32 x;
            UInt32 y;
            UInt32 width;
            UInt32 height;
            UInt32 dataSizeInBytes;
            const Byte* data = nullptr;

            action.read(handle);
            action.read(mipLevel);
            action.read(x);
            action.read(y);
            action.read(width);
            action.read(height);
            action.readWithoutCopy(data, dataSizeInBytes);

            scene.updateTextureBuffer(handle, mipLevel, x, y, width, height, data);
            break;
        }
        case ESceneActionId_AddAnimationSystem:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            UInt32 flags;
            action.read(flags);
            AnimationSystemSizeInformation sizeInfo;
            action.read(sizeInfo.splineCount);
            action.read(sizeInfo.dataBindCount);
            action.read(sizeInfo.animationInstanceCount);
            action.read(sizeInfo.animationCount);

            IAnimationSystem* animationSystem = animSystemFactory->createAnimationSystem(flags, sizeInfo);
            assert(animationSystem != NULL);

            if (animationSystem != NULL)
            {
                scene.addAnimationSystem(animationSystem, animSystemHandle);
            }

            break;
        }
        case ESceneActionId_RemoveAnimationSystem:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            scene.removeAnimationSystem(animSystemHandle);
            break;
        }
        case ESceneActionId_AnimationSystemSetTime:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationTime::TimeStamp globalTimeStamp;
            action.read(globalTimeStamp);
            const AnimationTime globalTime(globalTimeStamp);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setTime(globalTime);
            }
            break;
        }
        case ESceneActionId_AnimationSystemAllocateSpline:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            UInt32 uintData;
            action.read(uintData);
            ESplineKeyType keyType = ESplineKeyType(uintData);
            action.read(uintData);
            EDataTypeID dataTypeID = EDataTypeID(uintData);
            SplineHandle splineHandle;
            action.read(splineHandle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                ALLOCATE_AND_ASSERT_HANDLE(animSystem->allocateSpline(keyType, dataTypeID, splineHandle), splineHandle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemAllocateDataBinding:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            TDataBindID dataBindID;
            action.read(dataBindID);
            MemoryHandle handle1;
            action.read(handle1);
            MemoryHandle handle2;
            action.read(handle2);
            DataBindHandle dataBindHandle;
            action.read(dataBindHandle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                ALLOCATE_AND_ASSERT_HANDLE(animSystem->allocateDataBinding(scene, dataBindID, handle1, handle2, dataBindHandle), dataBindHandle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemAllocateAnimationInstance:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            UInt32 interpType;
            action.read(interpType);
            EInterpolationType interpolationType = EInterpolationType(interpType);
            UInt32 vecComp;
            action.read(vecComp);
            EVectorComponent vectorComponent = EVectorComponent(vecComp);
            AnimationInstanceHandle handle;
            action.read(handle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                ALLOCATE_AND_ASSERT_HANDLE(animSystem->allocateAnimationInstance(splineHandle, interpolationType, vectorComponent, handle), handle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemAllocateAnimation:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationInstanceHandle animInstHandle;
            action.read(animInstHandle);
            AnimationHandle handle;
            action.read(handle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                ALLOCATE_AND_ASSERT_HANDLE(animSystem->allocateAnimation(animInstHandle, handle), handle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemAddDataBindingToAnimationInstance:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationInstanceHandle animationInstanceHandle;
            action.read(animationInstanceHandle);
            DataBindHandle dataBindHandle;
            action.read(dataBindHandle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->addDataBindingToAnimationInstance(animationInstanceHandle, dataBindHandle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicBool:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Bool value;
            action.read(value);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicBool(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicInt32:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Int32 value;
            action.read(value);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicInt32(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicFloat:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Float value;
            action.read(value);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicFloat(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicVector2f:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector2 value;
            action.read(value.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicVector2f(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicVector3f:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector3 value;
            action.read(value.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicVector3f(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicVector4f:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector4 value;
            action.read(value.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicVector4f(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicVector2i:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector2i value;
            action.read(value.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicVector2i(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicVector3i:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector3i value;
            action.read(value.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicVector3i(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyBasicVector4i:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector4i value;
            action.read(value.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyBasicVector4i(splineHandle, timeStamp, value);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyTangentsInt32:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Int32 value;
            action.read(value);
            Vector2 tanIn;
            action.read(tanIn.data);
            Vector2 tanOut;
            action.read(tanOut.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyTangentsInt32(splineHandle, timeStamp, value, tanIn, tanOut);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyTangentsFloat:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Float value;
            action.read(value);
            Vector2 tanIn;
            action.read(tanIn.data);
            Vector2 tanOut;
            action.read(tanOut.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyTangentsFloat(splineHandle, timeStamp, value, tanIn, tanOut);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyTangentsVector2f:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector2 value;
            action.read(value.data);
            Vector2 tanIn;
            action.read(tanIn.data);
            Vector2 tanOut;
            action.read(tanOut.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyTangentsVector2f(splineHandle, timeStamp, value, tanIn, tanOut);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyTangentsVector3f:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector3 value;
            action.read(value.data);
            Vector2 tanIn;
            action.read(tanIn.data);
            Vector2 tanOut;
            action.read(tanOut.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyTangentsVector3f(splineHandle, timeStamp, value, tanIn, tanOut);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyTangentsVector4f:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector4 value;
            action.read(value.data);
            Vector2 tanIn;
            action.read(tanIn.data);
            Vector2 tanOut;
            action.read(tanOut.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyTangentsVector4f(splineHandle, timeStamp, value, tanIn, tanOut);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyTangentsVector2i:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector2i value;
            action.read(value.data);
            Vector2 tanIn;
            action.read(tanIn.data);
            Vector2 tanOut;
            action.read(tanOut.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyTangentsVector2i(splineHandle, timeStamp, value, tanIn, tanOut);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyTangentsVector3i:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector3i value;
            action.read(value.data);
            Vector2 tanIn;
            action.read(tanIn.data);
            Vector2 tanOut;
            action.read(tanOut.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyTangentsVector3i(splineHandle, timeStamp, value, tanIn, tanOut);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetSplineKeyTangentsVector4i:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineTimeStamp timeStamp;
            action.read(timeStamp);
            Vector4i value;
            action.read(value.data);
            Vector2 tanIn;
            action.read(tanIn.data);
            Vector2 tanOut;
            action.read(tanOut.data);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setSplineKeyTangentsVector4i(splineHandle, timeStamp, value, tanIn, tanOut);
            }
            break;
        }
        case ESceneActionId_AnimationSystemRemoveSplineKey:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle splineHandle;
            action.read(splineHandle);
            SplineKeyIndex keyIndex;
            action.read(keyIndex);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->removeSplineKey(splineHandle, keyIndex);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetAnimationStartTime:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationHandle handle;
            action.read(handle);
            AnimationTime::TimeStamp timeStamp;
            action.read(timeStamp);
            const AnimationTime startTime(timeStamp);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setAnimationStartTime(handle, timeStamp);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetAnimationStopTime:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationHandle handle;
            action.read(handle);
            AnimationTime::TimeStamp timeStamp;
            action.read(timeStamp);
            const AnimationTime startTime(timeStamp);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setAnimationStopTime(handle, timeStamp);
            }
            break;
        }
        case ESceneActionId_AnimationSystemSetAnimationProperties:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationHandle handle;
            action.read(handle);
            Float playbackSpeed;
            action.read(playbackSpeed);
            Animation::Flags flags;
            action.read(flags);
            AnimationTime::Duration loopDuration;
            action.read(loopDuration);
            AnimationTime::TimeStamp timeStamp;
            action.read(timeStamp);
            const AnimationTime globalTime(timeStamp);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->setAnimationProperties(handle, playbackSpeed, flags, loopDuration, globalTime);
            }
            break;
        }
        case ESceneActionId_AnimationSystemStopAnimationAndRollback:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationHandle handle;
            action.read(handle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->stopAnimationAndRollback(handle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemRemoveSpline:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            SplineHandle handle;
            action.read(handle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->removeSpline(handle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemRemoveDataBinding:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            DataBindHandle handle;
            action.read(handle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->removeDataBinding(handle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemRemoveAnimationInstance:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationInstanceHandle handle;
            action.read(handle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->removeAnimationInstance(handle);
            }
            break;
        }
        case ESceneActionId_AnimationSystemRemoveAnimation:
        {
            AnimationSystemHandle animSystemHandle;
            action.read(animSystemHandle);
            AnimationHandle handle;
            action.read(handle);

            IAnimationSystem* animSystem = scene.getAnimationSystem(animSystemHandle);
            if (animSystem != NULL)
            {
                animSystem->removeAnimation(handle);
            }
            break;
        }
        case ESceneActionId_PreallocateSceneSize:
        {
            SceneSizeInformation sizeInfos;
            GetSceneSizeInformation(action, sizeInfos);
            scene.preallocateSceneSize(sizeInfos);
            break;
        }
        case ESceneActionId_SetAckFlushState:
        {
            bool state;
            action.read(state);
            scene.setAckFlushState(state);
            break;
        }
        case ESceneActionId_Flush:
        {
            break;
        }
        case ESceneActionId_TestAction:
        {
            break;
        }
        case ESceneActionId_CompoundRenderableEffectData:
        {
            RenderableHandle renderable;
            RenderStateHandle stateHandle;
            ResourceContentHash effectHash;
            DataInstanceHandle uniformInstanceHandle;

            action.read(renderable);
            action.read(uniformInstanceHandle);
            action.read(stateHandle);
            action.read(effectHash);

            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformInstanceHandle);
            scene.setRenderableRenderState(renderable, stateHandle);
            scene.setRenderableEffect(renderable, effectHash);

            break;
        }
        case ESceneActionId_CompoundRenderable:
        {
            // due to no setter in scene, leaving out
            // renderOrder
            // effectAttributeLayout
            RenderableHandle renderable;
            NodeHandle node;
            UInt32 startIndex;
            UInt32 indexCount;
            ResourceContentHash effectHash;
            RenderStateHandle stateHandle;
            Bool visible;
            UInt32 instanceCount;
            DataInstanceHandle geoInstanceHandle;
            DataInstanceHandle uniformInstanceHandle;

            action.read(renderable);
            action.read(node);
            action.read(startIndex);
            action.read(indexCount);
            action.read(effectHash);
            action.read(stateHandle);
            action.read(visible);
            action.read(instanceCount);
            action.read(geoInstanceHandle);
            action.read(uniformInstanceHandle);

            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderable(node, renderable), renderable);

            if(startIndex != 0u)
            {
                scene.setRenderableStartIndex(renderable, startIndex);
            }
            scene.setRenderableIndexCount(renderable, indexCount);

            if(effectHash.isValid())
            {
                scene.setRenderableEffect(renderable, effectHash);
            }
            scene.setRenderableRenderState(renderable, stateHandle);

            if(!visible)
            {
                scene.setRenderableVisibility(renderable, visible);
            }

            if(instanceCount != 1u)
            {
                scene.setRenderableInstanceCount(renderable, instanceCount);
            }

            static_assert(ERenderableDataSlotType_MAX_SLOTS==2u
                , "Expected ERenderableDataSlotType containing 2 elements, adjust ESceneActionId_CompoundRenderable SceneAction handling");

            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geoInstanceHandle);
            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformInstanceHandle);

            break;
        }
        case ESceneActionId_CompoundState:
        {
            RenderStateHandle stateHandle;
            RenderState::ScissorRegion scissorRegion;
            EBlendFactor bfSrcColor;
            EBlendFactor bfDstColor;
            EBlendFactor bfSrcAlpha;
            EBlendFactor bfDstAlpha;
            EBlendOperation boColor;
            EBlendOperation boAlpha;
            ECullMode cullMode;
            EDrawMode drawMode;
            EDepthWrite depthWrite;
            EDepthFunc depthFunc;
            EScissorTest scissorTest;
            EStencilFunc stencilFunc;
            uint8_t stencilRefValue;
            uint8_t stencilMask;
            EStencilOp stencilOpFail;
            EStencilOp stencilOpDepthFail;
            EStencilOp stencilOpDepthPass;
            ColorWriteMask  colorWriteMask;

            action.read(stateHandle);
            action.read(scissorRegion.x);
            action.read(scissorRegion.y);
            action.read(scissorRegion.width);
            action.read(scissorRegion.height);
            action.read(bfSrcColor);
            action.read(bfDstColor);
            action.read(bfSrcAlpha);
            action.read(bfDstAlpha);
            action.read(boColor);
            action.read(boAlpha);
            action.read(cullMode);
            action.read(drawMode);
            action.read(depthWrite);
            action.read(depthFunc);
            action.read(scissorTest);
            action.read(stencilFunc);
            action.read(stencilRefValue);
            action.read(stencilMask);
            action.read(stencilOpFail);
            action.read(stencilOpDepthFail);
            action.read(stencilOpDepthPass);
            action.read(colorWriteMask);

            const RenderStateHandle stateHandleNew = scene.allocateRenderState(stateHandle);
            assert(stateHandle == stateHandleNew);
            UNUSED(stateHandleNew);

            scene.setRenderStateBlendFactors(   stateHandle, bfSrcColor, bfDstColor, bfSrcAlpha, bfDstAlpha);
            scene.setRenderStateBlendOperations(stateHandle, boColor, boAlpha);
            scene.setRenderStateCullMode(       stateHandle, cullMode);
            scene.setRenderStateDrawMode(       stateHandle, drawMode);
            scene.setRenderStateDepthWrite(     stateHandle, depthWrite);
            scene.setRenderStateDepthFunc(      stateHandle, depthFunc);
            scene.setRenderStateScissorTest(     stateHandle, scissorTest, scissorRegion);
            scene.setRenderStateStencilFunc(    stateHandle, stencilFunc, stencilRefValue, stencilMask);
            scene.setRenderStateStencilOps(     stateHandle, stencilOpFail, stencilOpDepthFail, stencilOpDepthPass);
            scene.setRenderStateColorWriteMask( stateHandle, colorWriteMask);

            break;
        }

        case ESceneActionId_PushResource:
        {
            ResourceContentHash hash;
            UInt32 resourceSize;
            const Byte* resourceData = nullptr;
            action.read(hash);
            action.readWithoutCopy(resourceData, resourceSize);
            BinaryInputStream stream(resourceData);
            std::unique_ptr<IResource> resource(SingleResourceSerialization::DeserializeResource(stream, hash));
            if (resources)
            {
                resources->push_back(std::move(resource));
            }
            else
            {
                LOG_DEBUG(CONTEXT_FRAMEWORK, "SceneActionApplier::ApplySingleActionOnScene: got PushResource but ignore because resource vector is nullptr");
            }
            break;
        }

        default:
        {
            assert(false && "unhandled scene message id");
            break;
        }
        }

        // check that action was fully consumed
        // (flush is never read, all others should be fully read)
        if (action.type() != ESceneActionId_Flush)
        {
            assert(action.isFullyRead());
        }
    }

    void SceneActionApplier::ApplyActionsOnScene(IScene& scene, const SceneActionCollection& actions, AnimationSystemFactory* animSystemFactory, ResourceVector* resources)
    {
        for (auto& reader : actions)
        {
            ApplySingleActionOnScene(scene, reader, animSystemFactory, resources);
        }
    }

    void SceneActionApplier::ApplyActionRangeOnScene(IScene& scene, const SceneActionCollection& actions, UInt startIdx, UInt endIdx, AnimationSystemFactory* animSystemFactory, ResourceVector* resources)
    {
        assert(startIdx <= endIdx);
        assert(endIdx <= actions.numberOfActions());

        for (UInt idx = startIdx; idx < endIdx; ++idx)
        {
            SceneActionCollection::SceneActionReader reader(actions[idx]);
            ApplySingleActionOnScene(scene, reader, animSystemFactory, resources);
        }
    }

    void SceneActionApplier::ReadParameterForFlushAction(
        SceneActionCollection::SceneActionReader action,
        UInt64& flushIndex,
        Bool& isSync,
        Bool& hasSizeInfo,
        SceneSizeInformation& sizeInfo,
        SceneResourceChanges& resourceChanges,
        FlushTimeInformation& flushTimeInfo,
        SceneVersionTag& versionTag,
        TimeStampVector* timestamps)
    {
        assert(action.type() == ESceneActionId_Flush);
        action.read(flushIndex);
        uint8_t flushFlags = 0u;
        action.read(flushFlags);
        isSync = (flushFlags & ESceneActionFlushBits_Synchronous) != 0;
        hasSizeInfo = (flushFlags & ESceneActionFlushBits_HasSizeInfo) != 0;
        const bool hasTimestamps = (flushFlags & ESceneActionFlushBits_HasTimestamps) != 0;

        if (hasSizeInfo)
        {
            GetSceneSizeInformation(action, sizeInfo);
        }
        resourceChanges.getFromSceneAction(action);

        UInt64 tsVal = 0;
        action.read(tsVal);
        flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(tsVal));
        action.read(tsVal);
        flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(tsVal));

        action.read(versionTag);

        if (hasTimestamps)
        {
            UInt32 timestampCount = 0;
            action.read(timestampCount);
            assert(timestampCount > 0u);

            if (timestamps)
            {
                timestamps->resize(timestampCount);
                for (auto& ts : *timestamps)
                {
                    action.read(ts);
                }
            }
            else
            {
                UInt64 dummyTs = 0;
                for (UInt32 i = 0; i < timestampCount; ++i)
                    action.read(dummyTs);
            }
        }
        assert(action.isFullyRead());
    }

    void SceneActionApplier::GetSceneSizeInformation(SceneActionCollection::SceneActionReader& action, SceneSizeInformation& sizeInfo)
    {
        action.read(sizeInfo.nodeCount);
        action.read(sizeInfo.cameraCount);
        action.read(sizeInfo.transformCount);
        action.read(sizeInfo.renderableCount);
        action.read(sizeInfo.renderStateCount);
        action.read(sizeInfo.datalayoutCount);
        action.read(sizeInfo.datainstanceCount);
        action.read(sizeInfo.renderGroupCount);
        action.read(sizeInfo.renderPassCount);
        action.read(sizeInfo.blitPassCount);
        action.read(sizeInfo.renderTargetCount);
        action.read(sizeInfo.renderBufferCount);
        action.read(sizeInfo.textureSamplerCount);
        action.read(sizeInfo.streamTextureCount);
        action.read(sizeInfo.dataSlotCount);
        action.read(sizeInfo.dataBufferCount);
        action.read(sizeInfo.animationSystemCount);
        action.read(sizeInfo.textureBufferCount);
    }
}
