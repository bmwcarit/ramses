//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector3i.h"
#include "ramses-client-api/DataVector4i.h"
#include "ramses-client-api/DataMatrix22f.h"
#include "ramses-client-api/DataMatrix33f.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"

// internal
#include "SceneImpl.h"
#include "EffectImpl.h"
#include "TextureSamplerImpl.h"
#include "Utils/StringUtils.h"

namespace ramses
{
    Scene::Scene(SceneImpl& pimpl)
        : ClientObject(pimpl)
        , impl(pimpl)
    {
    }

    Scene::~Scene()
    {
    }

    RemoteCamera* Scene::createRemoteCamera(const char* name)
    {
        RemoteCamera* remoteCamera = impl.createRemoteCamera(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(remoteCamera), name);
        return remoteCamera;
    }

    PerspectiveCamera* Scene::createPerspectiveCamera(const char* name)
    {
        PerspectiveCamera* perspectiveCamera = impl.createPerspectiveCamera(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(perspectiveCamera), name);
        return perspectiveCamera;
    }

    OrthographicCamera* Scene::createOrthographicCamera(const char* name)
    {
        OrthographicCamera* orthographicCamera = impl.createOrthographicCamera(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(orthographicCamera), name);
        return orthographicCamera;
    }

    Appearance* Scene::createAppearance(const Effect& effect, const char* name)
    {
        Appearance* appearance = impl.createAppearance(effect, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(appearance), LOG_API_RAMSESOBJECT_STRING(effect), name);
        return appearance;
    }

    GeometryBinding* Scene::createGeometryBinding(const Effect& effect, const char* name)
    {
        GeometryBinding* geomBinding = impl.createGeometryBinding(effect, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(geomBinding), LOG_API_RAMSESOBJECT_STRING(effect), name);
        return geomBinding;
    }

    StreamTexture* Scene::createStreamTexture(const Texture2D& fallbackTexture, streamSource_t source, const char* name)
    {
        StreamTexture* tex = impl.createStreamTexture(fallbackTexture, source, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(tex), LOG_API_RAMSESOBJECT_STRING(fallbackTexture), source.getValue(), name);
        return tex;
    }

    Node* Scene::createNode(const char* name)
    {
        Node* node = impl.createNode(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(node), name);
        return node;
    }

    MeshNode* Scene::createMeshNode(const char* name)
    {
        MeshNode* meshNode = impl.createMeshNode(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(meshNode), name);
        return meshNode;
    }

    status_t Scene::publish(EScenePublicationMode publicationMode)
    {
        const status_t status = impl.publish(publicationMode);
        LOG_HL_CLIENT_API1(status, publicationMode);
        return status;
    }

    status_t Scene::unpublish()
    {
        const status_t status = impl.unpublish();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool Scene::isPublished() const
    {
        return impl.isPublished();
    }

    sceneId_t Scene::getSceneId() const
    {
        return impl.getSceneId();
    }

    status_t Scene::destroy(SceneObject& object)
    {
        const status_t status = impl.destroy(object);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(object));
        return status;
    }

    status_t Scene::setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds)
    {
        const status_t status = impl.setExpirationTimestamp(ptpExpirationTimestampInMilliseconds);
        LOG_HL_CLIENT_API1(status, ptpExpirationTimestampInMilliseconds);
        return status;
    }

    status_t Scene::flush(sceneVersionTag_t sceneVersionTag)
    {
        const status_t status = impl.flush(sceneVersionTag);
        LOG_HL_CLIENT_API2(status, sceneVersionTag, impl.getSceneId());
        return status;
    }

    AnimationSystem* Scene::createAnimationSystem(uint32_t flags, const char* name)
    {
        AnimationSystem* animationSystem = impl.createAnimationSystem(flags, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(animationSystem), flags, name);
        return animationSystem;
    }

    AnimationSystemRealTime* Scene::createRealTimeAnimationSystem(uint32_t flags, const char* name)
    {
        AnimationSystemRealTime* animationSystemRealtime = impl.createRealTimeAnimationSystem(flags, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(animationSystemRealtime), flags, name);
        return animationSystemRealtime;
    }

    IndexDataBuffer* Scene::createIndexDataBuffer(uint32_t maximumSizeInBytes, EDataType dataType, const char* name /*= 0*/)
    {
        IndexDataBuffer* indexDataBuffer = impl.createIndexDataBuffer(maximumSizeInBytes, dataType, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(indexDataBuffer), maximumSizeInBytes, dataType, name);
        return indexDataBuffer;
    }

    VertexDataBuffer* Scene::createVertexDataBuffer(uint32_t maximumSizeInBytes, EDataType dataType, const char* name /*= 0*/)
    {
        VertexDataBuffer* vertexDataBuffer = impl.createVertexDataBuffer(maximumSizeInBytes, dataType, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(vertexDataBuffer), maximumSizeInBytes, dataType, name);
        return vertexDataBuffer;
    }

    Texture2DBuffer* Scene::createTexture2DBuffer(uint32_t mipLevelCount, uint32_t width, uint32_t height, ETextureFormat textureFormat, const char* name /*= 0*/)
    {
        Texture2DBuffer* texture2DBuffer = impl.createTexture2DBuffer(mipLevelCount, width, height, textureFormat, name);
        LOG_HL_CLIENT_API5(LOG_API_RAMSESOBJECT_PTR_STRING(texture2DBuffer), mipLevelCount, width, height, textureFormat, name);
        return texture2DBuffer;
    }

    const RamsesObject* Scene::findObjectByName(const char* name) const
    {
        return impl.findObjectByName(name);
    }

    RamsesObject* Scene::findObjectByName(const char* name)
    {
        return impl.findObjectByName(name);
    }

    RenderGroup* Scene::createRenderGroup(const char* name)
    {
        RenderGroup* renderGroup = impl.createRenderGroup(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(renderGroup), name);
        return renderGroup;
    }

    RenderPass* Scene::createRenderPass(const char* name)
    {
        RenderPass* renderPass = impl.createRenderPass(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(renderPass), name);
        return renderPass;
    }

    BlitPass* Scene::createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, const char* name /*= 0*/)
    {
        BlitPass* blitPass = impl.createBlitPass(sourceRenderBuffer, destinationRenderBuffer, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(blitPass), LOG_API_RAMSESOBJECT_STRING(sourceRenderBuffer), LOG_API_RAMSESOBJECT_STRING(destinationRenderBuffer), name);
        return blitPass;
    }

    RenderBuffer* Scene::createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, const char* name)
    {
        RenderBuffer* renderBuffer = impl.createRenderBuffer(width, height, bufferType, bufferFormat, accessMode, sampleCount, name);
        LOG_HL_CLIENT_API7(LOG_API_RAMSESOBJECT_PTR_STRING(renderBuffer), width, height, bufferType, bufferFormat, accessMode, sampleCount, name);
        return renderBuffer;
    }

    RenderTarget* Scene::createRenderTarget(const RenderTargetDescription& rtDesc, const char* name)
    {
        RenderTarget* renderTarget = impl.createRenderTarget(rtDesc.impl, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(renderTarget), LOG_API_GENERIC_OBJECT_STRING(rtDesc), name);
        return renderTarget;
    }

    TextureSampler* Scene::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const Texture2D& texture,
        uint32_t anisotropyLevel,
        const char* name)
    {
        TextureSampler* texSampler = impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, texture, name);
        LOG_HL_CLIENT_API7(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, LOG_API_RAMSESOBJECT_STRING(texture), name);
        return texSampler;
    }

    TextureSampler* Scene::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureAddressMode wrapRMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const Texture3D& texture,
        const char* name)
    {
        TextureSampler* texSampler = impl.createTextureSampler(wrapUMode, wrapVMode, wrapRMode, minSamplingMethod, magSamplingMethod, texture, name);
        LOG_HL_CLIENT_API7(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), wrapUMode, wrapVMode, wrapRMode, minSamplingMethod, magSamplingMethod, LOG_API_RAMSESOBJECT_STRING(texture), name);
        return texSampler;
    }

    TextureSampler* Scene::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const TextureCube& texture,
        uint32_t anisotropyLevel,
        const char* name)
    {
        TextureSampler* texSampler = impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel,texture, name);
        LOG_HL_CLIENT_API7(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, LOG_API_RAMSESOBJECT_STRING(texture), name);
        return texSampler;
    }

    TextureSampler* Scene::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const RenderBuffer& renderBuffer,
        uint32_t anisotropyLevel,
        const char* name)
    {
        TextureSampler* texSampler = impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, renderBuffer, name);
        LOG_HL_CLIENT_API7(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, LOG_API_RAMSESOBJECT_STRING(renderBuffer), name);
        return texSampler;
    }

    TextureSampler* Scene::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const Texture2DBuffer& texture2DBuffer,
        uint32_t anisotropyLevel,
        const char* name)
    {
        TextureSampler* texSampler = impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, texture2DBuffer, name);
        LOG_HL_CLIENT_API7(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, LOG_API_RAMSESOBJECT_STRING(texture2DBuffer), name);
        return texSampler;
    }

    TextureSampler* Scene::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const StreamTexture& streamTexture,
        const char* name)
    {
        TextureSampler* texSampler = impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, streamTexture, name);
        LOG_HL_CLIENT_API6(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, LOG_API_RAMSESOBJECT_STRING(streamTexture), name);
        return texSampler;
    }

    status_t Scene::createTransformationDataProvider(const Node& node, dataProviderId_t dataId)
    {
        status_t status = impl.createTransformationDataProvider(node, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(node), dataId);
        return status;
    }

    status_t Scene::createTransformationDataConsumer(const Node& node, dataConsumerId_t dataId)
    {
        status_t status = impl.createTransformationDataConsumer(node, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(node), dataId);
        return status;
    }

    status_t Scene::createDataProvider(const DataObject& dataObject, dataProviderId_t dataId)
    {
        status_t status = impl.createDataProvider(dataObject, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(dataObject), dataId);
        return status;
    }

    status_t Scene::createDataConsumer(const DataObject& dataObject, dataConsumerId_t dataId)
    {
        status_t status = impl.createDataConsumer(dataObject, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(dataObject), dataId);
        return status;
    }

    status_t Scene::createTextureProvider(const Texture2D& texture, dataProviderId_t dataId)
    {
        status_t status = impl.createTextureProvider(texture, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(texture), dataId);
        return status;
    }

    status_t Scene::updateTextureProvider(const Texture2D& texture, dataProviderId_t dataId)
    {
        status_t status = impl.updateTextureProvider(texture, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(texture), dataId);
        return status;
    }

    status_t Scene::createTextureConsumer(const TextureSampler& sampler, dataConsumerId_t dataId)
    {
        status_t status = impl.createTextureConsumer(sampler, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(sampler), dataId);
        return status;
    }

    DataFloat* Scene::createDataFloat(const char* name)
    {
        DataFloat* dataObject = impl.createDataFloat(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataVector2f* Scene::createDataVector2f(const char* name)
    {
        DataVector2f* dataObject = impl.createDataVector2f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataVector3f* Scene::createDataVector3f(const char* name)
    {
        DataVector3f* dataObject = impl.createDataVector3f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataVector4f* Scene::createDataVector4f(const char* name)
    {
        DataVector4f* dataObject = impl.createDataVector4f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataMatrix22f* Scene::createDataMatrix22f(const char* name)
    {
        DataMatrix22f* dataObject = impl.createDataMatrix22f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataMatrix33f* Scene::createDataMatrix33f(const char* name)
    {
        DataMatrix33f* dataObject = impl.createDataMatrix33f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataMatrix44f* Scene::createDataMatrix44f(const char* name)
    {
        DataMatrix44f* dataObject = impl.createDataMatrix44f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataInt32* Scene::createDataInt32(const char* name)
    {
        DataInt32* dataObject = impl.createDataInt32(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataVector2i* Scene::createDataVector2i(const char* name)
    {
        DataVector2i* dataObject = impl.createDataVector2i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataVector3i* Scene::createDataVector3i(const char* name)
    {
        DataVector3i* dataObject = impl.createDataVector3i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

    DataVector4i* Scene::createDataVector4i(const char* name)
    {
        DataVector4i* dataObject = impl.createDataVector4i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), name);
        return dataObject;
    }

}
