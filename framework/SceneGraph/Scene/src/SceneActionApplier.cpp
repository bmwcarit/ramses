//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneActionApplier.h"
#include "Scene/ResourceChanges.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/PixelRectangle.h"
#include "SceneAPI/TextureSampler.h"
#include "SceneAPI/Viewport.h"
#include "SceneAPI/Camera.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/ERotationType.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"
#include "Components/SingleResourceSerialization.h"
#include "Components/FlushTimeInformation.h"
#include "Resource/IResource.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/LogMacros.h"
#include "glm/gtx/range.hpp"

#include <string>

#define ALLOCATE_AND_ASSERT_HANDLE(_allocateExpr, _handleToCheck) \
{ \
    assert(_handleToCheck.isValid()); \
    const auto _actualHandle = _allocateExpr; \
    assert(_handleToCheck == _actualHandle); \
    UNUSED(_actualHandle); \
}

namespace ramses_internal
{
    void SceneActionApplier::ApplySingleActionOnScene(IScene& scene, SceneActionCollection::SceneActionReader& action)
    {
        switch (action.type())
        {
        case ESceneActionId::AllocateNode:
        {
            UInt32 childrenCount = 0u;
            NodeHandle nodeHandle;
            std::string objectName;
            UInt64 objectId;
            action.read(childrenCount);
            action.read(nodeHandle);
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateNode(childrenCount, nodeHandle), nodeHandle);
            break;
        }
        case ESceneActionId::AddChildToNode:
        {
            NodeHandle node;
            NodeHandle child;
            action.read(node);
            action.read(child);
            scene.addChildToNode(node, child);
            break;
        }
        case ESceneActionId::AllocateTransform:
        {
            NodeHandle nodeHandle;
            TransformHandle transformHandle;
            action.read(nodeHandle);
            action.read(transformHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateTransform(nodeHandle, transformHandle), transformHandle);
            break;
        }
        case ESceneActionId::SetTranslation:
        {
            TransformHandle transform;
            glm::vec3 vec;
            action.read(transform);
            action.read(vec);
            scene.setTranslation(transform, vec);
            break;
        }
        case ESceneActionId::SetRotation:
        {
            TransformHandle transform;
            glm::vec4 vec;
            ERotationType rotationType;
            action.read(transform);
            action.read(vec);
            action.read(rotationType);
            scene.setRotation(transform, vec, rotationType);
            break;
        }
        case ESceneActionId::SetScaling:
        {
            TransformHandle transform;
            glm::vec3 vec;
            action.read(transform);
            action.read(vec);
            scene.setScaling(transform, vec);
            break;
        }
        case ESceneActionId::AllocateDataInstance:
        {
            DataLayoutHandle dataLayout;
            DataInstanceHandle diHandle;
            std::string objectName;
            UInt64 objectId;
            action.read(dataLayout);
            action.read(diHandle);
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateDataInstance(dataLayout, diHandle), diHandle);
            break;
        }
        case ESceneActionId::SetDataFloatArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            float* const array = const_cast<float*>(scene.getDataFloatArray(handle, field));
            action.read(array, elementCount);
            scene.setDataFloatArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataVector2fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::vec2*>(scene.getDataVector2fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataVector2fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataVector3fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::vec3*>(scene.getDataVector3fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataVector3fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataVector4fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::vec4*>(scene.getDataVector4fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataVector4fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataMatrix22fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::mat2*>(scene.getDataMatrix22fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataMatrix22fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataMatrix33fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::mat3*>(scene.getDataMatrix33fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataMatrix33fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataMatrix44fArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::mat4*>(scene.getDataMatrix44fArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataMatrix44fArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataIntegerArray:
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
        case ESceneActionId::SetDataVector2iArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::ivec2*>(scene.getDataVector2iArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataVector2iArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataVector3iArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::ivec3*>(scene.getDataVector3iArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataVector3iArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataVector4iArray:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            UInt32 elementCount = 0;
            action.read(handle);
            action.read(field);
            action.read(elementCount);
            auto* array = const_cast<glm::ivec4*>(scene.getDataVector4iArray(handle, field));
            for (UInt32 i = 0; i < elementCount; ++i)
                action.read(array[i]);
            scene.setDataVector4iArray(handle, field, elementCount, array);
            break;
        }
        case ESceneActionId::SetDataResource:
        {
            DataInstanceHandle handle;
            DataFieldHandle field;
            ResourceContentHash hash;
            DataBufferHandle dataBuffer;
            UInt32 instancingDivisor;
            UInt16 offset;
            UInt16 stride;
            action.read(handle);
            action.read(field);
            action.read(hash);
            action.read(dataBuffer);
            action.read(instancingDivisor);
            action.read(offset);
            action.read(stride);
            scene.setDataResource(handle, field, hash, dataBuffer, instancingDivisor, offset, stride);
            break;
        }
        case ESceneActionId::SetDataTextureSamplerHandle:
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
        case ESceneActionId::SetDataReference:
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
        case ESceneActionId::AllocateRenderable:
        {
            NodeHandle node;
            RenderableHandle handle;
            action.read(node);
            action.read(handle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderable(node, handle), handle);
            break;
        }
        case ESceneActionId::RemoveChildFromNode:
        {
            NodeHandle parent;
            NodeHandle child;
            action.read(parent);
            action.read(child);
            scene.removeChildFromNode(parent, child);
            break;
        }
        case ESceneActionId::AllocateDataLayout:
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
            ResourceContentHash effectHash;
            action.read(effectHash);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateDataLayout(dataFields, effectHash, layoutHandle), layoutHandle);
            break;
        }
        case ESceneActionId::ReleaseDataLayout:
        {
            DataLayoutHandle layout;
            action.read(layout);
            scene.releaseDataLayout(layout);
            break;
        }
        case ESceneActionId::ReleaseDataInstance:
        {
            DataInstanceHandle instance;
            action.read(instance);
            scene.releaseDataInstance(instance);
            break;
        }
        case ESceneActionId::SetRenderableStartIndex:
        {
            RenderableHandle renderable;
            UInt32 startIndex = 0;
            action.read(renderable);
            action.read(startIndex);
            scene.setRenderableStartIndex(renderable, startIndex);
            break;
        }
        case ESceneActionId::SetRenderableIndexCount:
        {
            RenderableHandle renderable;
            UInt32 count = 0;
            action.read(renderable);
            action.read(count);
            scene.setRenderableIndexCount(renderable, count);
            break;
        }
        case ESceneActionId::SetRenderableVisibility:
        {
            RenderableHandle renderable;
            EVisibilityMode visibility = EVisibilityMode::Visible;
            action.read(renderable);
            action.read(visibility);
            scene.setRenderableVisibility(renderable, visibility);
            break;
        }
        case ESceneActionId::SetRenderableDataInstance:
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
        case ESceneActionId::SetRenderableInstanceCount:
        {
            RenderableHandle renderable;
            UInt32 numInstances = 1;
            action.read(renderable);
            action.read(numInstances);
            scene.setRenderableInstanceCount(renderable, numInstances);
            break;
        }
        case ESceneActionId::SetRenderableStartVertex:
        {
            RenderableHandle renderable;
            UInt32 startVertex = 0;
            action.read(renderable);
            action.read(startVertex);
            scene.setRenderableStartVertex(renderable, startVertex);
            break;
        }
        case ESceneActionId::AllocateRenderGroup:
        {
            UInt32 renderableCount = 0u;
            UInt32 nestedGroupCount = 0u;
            std::string objectName;
            UInt64 objectId;
            RenderGroupHandle renderGroup;
            action.read(renderableCount);
            action.read(nestedGroupCount);
            action.read(renderGroup);
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderGroup(renderableCount, nestedGroupCount, renderGroup), renderGroup);
            break;
        }
        case ESceneActionId::ReleaseRenderGroup:
        {
            RenderGroupHandle renderGroup;
            action.read(renderGroup);
            scene.releaseRenderGroup(renderGroup);
            break;
        }
        case ESceneActionId::AddRenderableToRenderGroup:
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
        case ESceneActionId::RemoveRenderableFromRenderGroup:
        {
            RenderGroupHandle renderGroup;
            RenderableHandle renderable;
            action.read(renderGroup);
            action.read(renderable);
            scene.removeRenderableFromRenderGroup(renderGroup, renderable);
            break;
        }
        case ESceneActionId::AddRenderGroupToRenderGroup:
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
        case ESceneActionId::RemoveRenderGroupFromRenderGroup:
        {
            RenderGroupHandle renderGroupParent;
            RenderGroupHandle renderGroupChild;
            action.read(renderGroupParent);
            action.read(renderGroupChild);
            scene.removeRenderGroupFromRenderGroup(renderGroupParent, renderGroupChild);
            break;
        }
        case ESceneActionId::ReleaseRenderable:
        {
            RenderableHandle renderable;
            action.read(renderable);
            scene.releaseRenderable(renderable);
            break;
        }
        case ESceneActionId::ReleaseTransform:
        {
            TransformHandle transformHandle;
            action.read(transformHandle);
            scene.releaseTransform(transformHandle);
            break;
        }
        case ESceneActionId::ReleaseNode:
        {
            NodeHandle nodeHandle;
            action.read(nodeHandle);
            scene.releaseNode(nodeHandle);
            break;
        }
        case ESceneActionId::AllocateRenderState:
        {
            RenderStateHandle stateHandle;
            action.read(stateHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderState(stateHandle), stateHandle);
            break;
        }
        case ESceneActionId::ReleaseState:
        {
            RenderStateHandle stateHandle;
            action.read(stateHandle);
            scene.releaseRenderState(stateHandle);
            break;
        }
        case ESceneActionId::SetRenderableState:
        {
            RenderableHandle renderable;
            RenderStateHandle stateHandle;
            action.read(renderable);
            action.read(stateHandle);
            scene.setRenderableRenderState(renderable, stateHandle);
            break;
        }
        case ESceneActionId::SetStateStencilOps:
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
        case ESceneActionId::SetStateStencilFunc:
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
        case ESceneActionId::SetStateDepthWrite:
        {
            RenderStateHandle stateHandle;
            EDepthWrite flag = EDepthWrite::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(flag);
            scene.setRenderStateDepthWrite(stateHandle, flag);
            break;
        }
        case ESceneActionId::SetStateScissorTest:
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
        case ESceneActionId::SetStateDepthFunc:
        {
            RenderStateHandle stateHandle;
            EDepthFunc func = EDepthFunc::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(func);
            scene.setRenderStateDepthFunc(stateHandle, func);
            break;
        }
        case ESceneActionId::SetStateCullMode:
        {
            RenderStateHandle stateHandle;
            ECullMode mode = ECullMode::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(mode);
            scene.setRenderStateCullMode(stateHandle, mode);
            break;
        }
        case ESceneActionId::SetStateDrawMode:
        {
            RenderStateHandle stateHandle;
            EDrawMode mode = EDrawMode::NUMBER_OF_ELEMENTS;
            action.read(stateHandle);
            action.read(mode);
            scene.setRenderStateDrawMode(stateHandle, mode);
            break;
        }
        case ESceneActionId::SetStateBlendOperations:
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
        case ESceneActionId::SetStateBlendFactors:
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
        case ESceneActionId::SetStateBlendColor:
        {
            RenderStateHandle stateHandle;
            glm::vec4 color{ 0.f, 0.f, 0.f, 0.f };
            action.read(stateHandle);
            action.read(color.r);
            action.read(color.g);
            action.read(color.b);
            action.read(color.a);
            scene.setRenderStateBlendColor(stateHandle, color);
            break;
        }
        case ESceneActionId::SetStateColorWriteMask:
        {
            RenderStateHandle stateHandle;
            ColorWriteMask colorMask;
            action.read(stateHandle);
            action.read(colorMask);
            scene.setRenderStateColorWriteMask(stateHandle, colorMask);
            break;
        }
        case ESceneActionId::AllocateCamera:
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
        case ESceneActionId::ReleaseCamera:
        {
            CameraHandle cameraHandle;
            action.read(cameraHandle);
            scene.releaseCamera(cameraHandle);
            break;
        }
        case ESceneActionId::AllocateRenderPass:
        {
            UInt32 renderGroupCount = 0u;
            RenderPassHandle passHandle;
            std::string objectName;
            UInt64 objectId;
            action.read(renderGroupCount);
            action.read(passHandle);
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderPass(renderGroupCount, passHandle), passHandle);
            break;
        }
        case ESceneActionId::ReleaseRenderPass:
        {
            RenderPassHandle passHandle;
            action.read(passHandle);
            scene.releaseRenderPass(passHandle);
            break;
        }
        case ESceneActionId::SetRenderPassCamera:
        {
            RenderPassHandle passHandle;
            CameraHandle cameraHandle;
            action.read(passHandle);
            action.read(cameraHandle);
            scene.setRenderPassCamera(passHandle, cameraHandle);
            break;
        }
        case ESceneActionId::SetRenderPassRenderTarget:
        {
            RenderPassHandle passHandle;
            RenderTargetHandle targetHandle;
            action.read(passHandle);
            action.read(targetHandle);
            scene.setRenderPassRenderTarget(passHandle, targetHandle);
            break;
        }
        case ESceneActionId::SetRenderPassRenderOrder:
        {
            RenderPassHandle passHandle;
            Int32 renderOrder;
            action.read(passHandle);
            action.read(renderOrder);
            scene.setRenderPassRenderOrder(passHandle, renderOrder);
            break;
        }
        case ESceneActionId::SetRenderPassEnabled:
        {
            RenderPassHandle passHandle;
            bool isEnabled;
            action.read(passHandle);
            action.read(isEnabled);
            scene.setRenderPassEnabled(passHandle, isEnabled);
            break;
        }
        case ESceneActionId::SetRenderPassRenderOnce:
        {
            RenderPassHandle passHandle;
            bool enabled;
            action.read(passHandle);
            action.read(enabled);
            scene.setRenderPassRenderOnce(passHandle, enabled);
            break;
        }
        case ESceneActionId::RetriggerRenderPassRenderOnce:
        {
            RenderPassHandle passHandle;
            action.read(passHandle);
            scene.retriggerRenderPassRenderOnce(passHandle);
            break;
        }
        case ESceneActionId::AddRenderGroupToRenderPass:
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
        case ESceneActionId::RemoveRenderGroupFromRenderPass:
        {
            RenderPassHandle passHandle;
            RenderGroupHandle groupHandle;
            action.read(passHandle);
            action.read(groupHandle);
            scene.removeRenderGroupFromRenderPass(passHandle, groupHandle);
            break;
        }
        case ESceneActionId::AllocatePickableObject:
        {
            DataBufferHandle geometryHandle;
            NodeHandle nodeHandle;
            PickableObjectId id;
            PickableObjectHandle pickableHandle;
            action.read(geometryHandle);
            action.read(nodeHandle);
            action.read(id);
            action.read(pickableHandle);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocatePickableObject(geometryHandle, nodeHandle, id, pickableHandle), pickableHandle);
            break;
        }
        case ESceneActionId::ReleasePickableObject:
        {
            PickableObjectHandle pickableHandle;
            action.read(pickableHandle);
            scene.releasePickableObject(pickableHandle);
            break;
        }
        case ESceneActionId::SetPickableObjectId:
        {
            PickableObjectHandle pickableHandle;
            PickableObjectId id;
            action.read(pickableHandle);
            action.read(id);
            scene.setPickableObjectId(pickableHandle, id);
            break;
        }
        case ESceneActionId::SetPickableObjectCamera:
        {
            PickableObjectHandle pickableHandle;
            CameraHandle cameraHandle;
            action.read(pickableHandle);
            action.read(cameraHandle);
            scene.setPickableObjectCamera(pickableHandle, cameraHandle);
            break;
        }
        case ESceneActionId::SetPickableObjectEnabled:
        {
            PickableObjectHandle pickableHandle;
            bool           isEnabled;
            action.read(pickableHandle);
            action.read(isEnabled);
            scene.setPickableObjectEnabled(pickableHandle, isEnabled);
            break;
        }
        case ESceneActionId::AllocateBlitPass:
        {
            BlitPassHandle passHandle;
            RenderBufferHandle sourceRenderbufferHandle;
            RenderBufferHandle destinationRenderbufferHandle;
            std::string objectName;
            UInt64 objectId;
            action.read(sourceRenderbufferHandle);
            action.read(destinationRenderbufferHandle);
            action.read(passHandle);
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateBlitPass(sourceRenderbufferHandle, destinationRenderbufferHandle, passHandle), passHandle);
            break;
        }
        case ESceneActionId::ReleaseBlitPass:
        {
            BlitPassHandle passHandle;
            action.read(passHandle);
            scene.releaseBlitPass(passHandle);
            break;
        }
        case ESceneActionId::SetBlitPassRenderOrder:
        {
            BlitPassHandle passHandle;
            Int32 renderOrder;
            action.read(passHandle);
            action.read(renderOrder);
            scene.setBlitPassRenderOrder(passHandle, renderOrder);
            break;
        }
        case ESceneActionId::SetBlitPassEnabled:
        {
            BlitPassHandle passHandle;
            bool isEnabled;
            action.read(passHandle);
            action.read(isEnabled);
            scene.setBlitPassEnabled(passHandle, isEnabled);
            break;
        }
        case ESceneActionId::SetBlitPassRegions:
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
        case ESceneActionId::AllocateTextureSampler:
        {
            TextureSamplerHandle samplerHandle;
            TextureSampler sampler;
            std::string objectName;
            UInt64 objectId;

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

            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);

            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateTextureSampler(sampler, samplerHandle), samplerHandle);
            break;
        }
        case ESceneActionId::ReleaseTextureSampler:
        {
            TextureSamplerHandle samplerHandle;
            action.read(samplerHandle);
            scene.releaseTextureSampler(samplerHandle);
            break;
        }
        case ESceneActionId::AllocateRenderTarget:
        {
            RenderTargetHandle renderTargetHandle;
            std::string objectName;
            UInt64 objectId;
            action.read(renderTargetHandle);
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderTarget(renderTargetHandle), renderTargetHandle);
            break;
        }
        case ESceneActionId::ReleaseRenderTarget:
        {
            RenderTargetHandle renderTargetHandle;
            action.read(renderTargetHandle);
            scene.releaseRenderTarget(renderTargetHandle);
            break;
        }
        case ESceneActionId::AddRenderTargetRenderBuffer:
        {
            RenderTargetHandle renderTargetHandle;
            RenderBufferHandle renderBufferHandle;
            action.read(renderTargetHandle);
            action.read(renderBufferHandle);
            scene.addRenderTargetRenderBuffer(renderTargetHandle, renderBufferHandle);
            break;
        }
        case ESceneActionId::AllocateRenderBuffer:
        {
            RenderBufferHandle handle;
            RenderBuffer renderBuffer;
            UInt32 enumInt;
            std::string objectName;
            UInt64 objectId;

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
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderBuffer(renderBuffer, handle), handle);
            break;
        }
        case ESceneActionId::ReleaseRenderBuffer:
        {
            RenderBufferHandle handle;
            action.read(handle);
            scene.releaseRenderBuffer(handle);
            break;
        }
        case ESceneActionId::AllocateDataSlot:
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
        case ESceneActionId::SetDataSlotTexture:
        {
            DataSlotHandle dataSlotHandle(0);
            ResourceContentHash texture = ResourceContentHash::Invalid();

            action.read(dataSlotHandle);
            action.read(texture);
            scene.setDataSlotTexture(dataSlotHandle, texture);
            break;
        }
        case ESceneActionId::ReleaseDataSlot:
        {
            DataSlotHandle dataSlotHandle(0);
            action.read(dataSlotHandle);
            scene.releaseDataSlot(dataSlotHandle);
            break;
        }
        case ESceneActionId::SetRenderPassClearColor:
        {
            glm::vec4 clearColor;
            RenderPassHandle renderPassHandle;
            action.read(renderPassHandle);
            action.read(clearColor);
            scene.setRenderPassClearColor(renderPassHandle, clearColor);
            break;
        }
        case ESceneActionId::SetRenderPassClearFlag:
        {
            UInt32 clearFlag;
            RenderPassHandle renderPassHandle;
            action.read(renderPassHandle);
            action.read(clearFlag);
            scene.setRenderPassClearFlag(renderPassHandle, clearFlag);
            break;
        }
        case ESceneActionId::AllocateDataBuffer:
        {
            UInt32 dataBufferType;
            UInt32 dataType;
            UInt32 maximumSizeInBytes;
            DataBufferHandle handle;
            std::string objectName;
            UInt64 objectId;
            action.read(dataBufferType);
            action.read(dataType);
            action.read(maximumSizeInBytes);
            action.read(handle.asMemoryHandleReference());
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateDataBuffer(static_cast<EDataBufferType>(dataBufferType), static_cast<EDataType>(dataType), maximumSizeInBytes, handle), handle);
            break;
        }
        case ESceneActionId::ReleaseDataBuffer:
        {
            DataBufferHandle handle;
            action.read(handle.asMemoryHandleReference());
            scene.releaseDataBuffer(handle);
            break;
        }
        case ESceneActionId::UpdateDataBuffer:
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
        case ESceneActionId::AllocateTextureBuffer:
        {
            UInt32 textureFormat;
            UInt32 mipLevelCount;
            MipMapDimensions mipMapDimensions;
            TextureBufferHandle handle;
            std::string objectName;
            UInt64 objectId;

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
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);

            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateTextureBuffer(static_cast<ETextureFormat>(textureFormat), mipMapDimensions, handle), handle);
            break;
        }
        case ESceneActionId::ReleaseTextureBuffer:
        {
            TextureBufferHandle handle;
            action.read(handle);

            scene.releaseTextureBuffer(handle);
            break;
        }
        case ESceneActionId::UpdateTextureBuffer:
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
        case ESceneActionId::AllocateSceneReference:
        {
            SceneReferenceHandle handle;
            SceneId sceneId;
            std::string objectName;
            UInt64 objectId;
            action.read(handle);
            action.read(sceneId);
            action.read(objectName);
            action.read(objectId);
            UNUSED(objectName);
            UNUSED(objectId);
            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateSceneReference(sceneId, handle), handle);
            break;
        }
        case ESceneActionId::ReleaseSceneReference:
        {
            SceneReferenceHandle handle;
            action.read(handle);
            scene.releaseSceneReference(handle);
            break;
        }
        case ESceneActionId::RequestSceneReferenceState:
        {
            SceneReferenceHandle handle;
            RendererSceneState state;
            action.read(handle);
            action.read(state);
            scene.requestSceneReferenceState(handle, state);
            break;
        }
        case ESceneActionId::SetSceneReferenceRenderOrder:
        {
            SceneReferenceHandle handle;
            int32_t renderOrder;
            action.read(handle);
            action.read(renderOrder);
            scene.setSceneReferenceRenderOrder(handle, renderOrder);
            break;
        }
        case ESceneActionId::RequestSceneReferenceFlushNotifications:
        {
            SceneReferenceHandle handle;
            bool enable;
            action.read(handle);
            action.read(enable);
            scene.requestSceneReferenceFlushNotifications(handle, enable);
            break;
        }
        case ESceneActionId::PreallocateSceneSize:
        {
            SceneSizeInformation sizeInfos;
            GetSceneSizeInformation(action, sizeInfos);
            scene.preallocateSceneSize(sizeInfos);
            break;
        }
        case ESceneActionId::TestAction:
        {
            break;
        }
        case ESceneActionId::CompoundRenderableEffectData:
        {
            RenderableHandle renderable;
            RenderStateHandle stateHandle;
            DataInstanceHandle uniformInstanceHandle;

            action.read(renderable);
            action.read(uniformInstanceHandle);
            action.read(stateHandle);

            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformInstanceHandle);
            scene.setRenderableRenderState(renderable, stateHandle);

            break;
        }
        case ESceneActionId::CompoundRenderable:
        {
            // due to no setter in scene, leaving out
            // renderOrder
            // effectAttributeLayout
            RenderableHandle renderable;
            NodeHandle node;
            UInt32 startIndex;
            UInt32 indexCount;
            RenderStateHandle stateHandle;
            EVisibilityMode visible;
            UInt32 instanceCount;
            UInt32 startVertex;
            DataInstanceHandle geoInstanceHandle;
            DataInstanceHandle uniformInstanceHandle;

            action.read(renderable);
            action.read(node);
            action.read(startIndex);
            action.read(indexCount);
            action.read(stateHandle);
            action.read(visible);
            action.read(instanceCount);
            action.read(startVertex);
            action.read(geoInstanceHandle);
            action.read(uniformInstanceHandle);

            ALLOCATE_AND_ASSERT_HANDLE(scene.allocateRenderable(node, renderable), renderable);

            if(startIndex != 0u)
            {
                scene.setRenderableStartIndex(renderable, startIndex);
            }
            scene.setRenderableIndexCount(renderable, indexCount);
            scene.setRenderableRenderState(renderable, stateHandle);

            if(visible != EVisibilityMode::Visible)
            {
                scene.setRenderableVisibility(renderable, visible);
            }

            if(instanceCount != 1u)
            {
                scene.setRenderableInstanceCount(renderable, instanceCount);
            }

            if (startVertex != 0u)
            {
                scene.setRenderableStartVertex(renderable, startVertex);
            }

            static_assert(ERenderableDataSlotType_MAX_SLOTS==2u
                , "Expected ERenderableDataSlotType containing 2 elements, adjust ESceneActionId::CompoundRenderable SceneAction handling");

            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geoInstanceHandle);
            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformInstanceHandle);

            break;
        }
        case ESceneActionId::CompoundState:
        {
            RenderStateHandle stateHandle;
            RenderState::ScissorRegion scissorRegion;
            EBlendFactor bfSrcColor;
            EBlendFactor bfDstColor;
            EBlendFactor bfSrcAlpha;
            EBlendFactor bfDstAlpha;
            EBlendOperation boColor;
            EBlendOperation boAlpha;
            glm::vec4 blendColor;
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
            action.read(blendColor.r);
            action.read(blendColor.g);
            action.read(blendColor.b);
            action.read(blendColor.a);
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
            scene.setRenderStateBlendColor(     stateHandle, blendColor);
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

        default:
        {
            assert(false && "unhandled scene message id");
            break;
        }
        }

        assert(action.isFullyRead());
    }

    void SceneActionApplier::ApplyActionsOnScene(IScene& scene, const SceneActionCollection& actions)
    {
        for (auto& reader : actions)
        {
            ApplySingleActionOnScene(scene, reader);
        }
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
        action.read(sizeInfo.dataSlotCount);
        action.read(sizeInfo.dataBufferCount);
        action.read(sizeInfo.textureBufferCount);
        action.read(sizeInfo.pickableObjectCount);
        action.read(sizeInfo.sceneReferenceCount);
    }
}
