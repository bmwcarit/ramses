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
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/ArrayResource.h"

// internal
#include "SceneImpl.h"
#include "EffectImpl.h"
#include "TextureSamplerImpl.h"
#include "RamsesFrameworkTypesImpl.h"
#include "RamsesClientTypesImpl.h"

namespace ramses
{
    Scene::Scene(std::unique_ptr<SceneImpl> impl)
        : ClientObject{ std::move(impl) }
        , m_impl{ static_cast<SceneImpl&>(ClientObject::m_impl) }
    {
    }

    PerspectiveCamera* Scene::createPerspectiveCamera(std::string_view name)
    {
        PerspectiveCamera* perspectiveCamera = m_impl.createPerspectiveCamera(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(perspectiveCamera), name);
        return perspectiveCamera;
    }

    OrthographicCamera* Scene::createOrthographicCamera(std::string_view name)
    {
        OrthographicCamera* orthographicCamera = m_impl.createOrthographicCamera(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(orthographicCamera), name);
        return orthographicCamera;
    }

    Appearance* Scene::createAppearance(const Effect& effect, std::string_view name)
    {
        Appearance* appearance = m_impl.createAppearance(effect, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(appearance), LOG_API_RAMSESOBJECT_STRING(effect), name);
        return appearance;
    }

    GeometryBinding* Scene::createGeometryBinding(const Effect& effect, std::string_view name)
    {
        GeometryBinding* geomBinding = m_impl.createGeometryBinding(effect, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(geomBinding), LOG_API_RAMSESOBJECT_STRING(effect), name);
        return geomBinding;
    }

    Node* Scene::createNode(std::string_view name)
    {
        Node* node = m_impl.createNode(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(node), name);
        return node;
    }

    MeshNode* Scene::createMeshNode(std::string_view name)
    {
        MeshNode* meshNode = m_impl.createMeshNode(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(meshNode), name);
        return meshNode;
    }

    status_t Scene::publish(EScenePublicationMode publicationMode)
    {
        const status_t status = m_impl.publish(publicationMode);
        LOG_HL_CLIENT_API1(status, publicationMode);
        return status;
    }

    status_t Scene::unpublish()
    {
        const status_t status = m_impl.unpublish();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool Scene::isPublished() const
    {
        return m_impl.isPublished();
    }

    sceneId_t Scene::getSceneId() const
    {
        return m_impl.getSceneId();
    }

    status_t Scene::saveToFile(std::string_view fileName, bool compress) const
    {
        const auto status = m_impl.saveToFile(fileName, compress);
        LOG_HL_CLIENT_API2(status, fileName, compress);
        return status;
    }

    status_t Scene::destroy(SceneObject& object)
    {
        const status_t status = m_impl.destroy(object);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(object));
        return status;
    }

    status_t Scene::setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds)
    {
        const status_t status = m_impl.setExpirationTimestamp(ptpExpirationTimestampInMilliseconds);
        LOG_HL_CLIENT_API1(status, ptpExpirationTimestampInMilliseconds);
        return status;
    }

    status_t Scene::flush(sceneVersionTag_t sceneVersionTag)
    {
        const status_t status = m_impl.flush(sceneVersionTag);
        LOG_HL_CLIENT_API2(status, sceneVersionTag, m_impl.getSceneId());
        return status;
    }

    status_t Scene::resetUniformTimeMs()
    {
        const auto status = m_impl.resetUniformTimeMs();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    int32_t Scene::getUniformTimeMs() const
    {
        return m_impl.getUniformTimeMs();
    }

    ArrayBuffer* Scene::createArrayBuffer(EDataType dataType, uint32_t maxNumElements, std::string_view name /*= {}*/)
    {
        auto dataBufferObject = m_impl.createArrayBuffer(dataType, maxNumElements, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(dataBufferObject), maxNumElements, dataType, name);
        return dataBufferObject;
    }

    Texture2DBuffer* Scene::createTexture2DBuffer(ETextureFormat textureFormat, uint32_t width, uint32_t height, size_t mipLevelCount, std::string_view name /*= 0*/)
    {
        Texture2DBuffer* texture2DBuffer = m_impl.createTexture2DBuffer(mipLevelCount, width, height, textureFormat, name);
        LOG_HL_CLIENT_API5(LOG_API_RAMSESOBJECT_PTR_STRING(texture2DBuffer), mipLevelCount, width, height, toString(textureFormat), name);
        return texture2DBuffer;
    }

    SceneReference* Scene::createSceneReference(sceneId_t referencedScene, std::string_view name /*= {}*/)
    {
        SceneReference* sceneReference = m_impl.createSceneReference(referencedScene, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(sceneReference), referencedScene, name);
        return sceneReference;
    }

    status_t Scene::linkData(SceneReference* providerReference, dataProviderId_t providerId, SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        auto status = m_impl.linkData(providerReference, providerId, consumerReference, consumerId);
        LOG_HL_CLIENT_API4(status, LOG_API_RAMSESOBJECT_PTR_STRING(providerReference), providerId, LOG_API_RAMSESOBJECT_PTR_STRING(consumerReference), consumerId);
        return status;
    }

    ramses::status_t Scene::unlinkData(SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        auto status = m_impl.unlinkData(consumerReference, consumerId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_PTR_STRING(consumerReference), consumerId);
        return status;
    }

    const RamsesObject* Scene::findObjectByName(std::string_view name) const
    {
        return m_impl.findObjectByName(name);
    }

    RamsesObject* Scene::findObjectByName(std::string_view name)
    {
        return m_impl.findObjectByName(name);
    }

    const SceneObject* Scene::findObjectById(sceneObjectId_t id) const
    {
        return m_impl.findObjectById(id);
    }

    SceneObject* Scene::findObjectById(sceneObjectId_t id)
    {
        return m_impl.findObjectById(id);
    }

    RenderGroup* Scene::createRenderGroup(std::string_view name)
    {
        RenderGroup* renderGroup = m_impl.createRenderGroup(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(renderGroup), name);
        return renderGroup;
    }

    RenderPass* Scene::createRenderPass(std::string_view name)
    {
        RenderPass* renderPass = m_impl.createRenderPass(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(renderPass), name);
        return renderPass;
    }

    BlitPass* Scene::createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, std::string_view name /*= {}*/)
    {
        BlitPass* blitPass = m_impl.createBlitPass(sourceRenderBuffer, destinationRenderBuffer, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(blitPass), LOG_API_RAMSESOBJECT_STRING(sourceRenderBuffer), LOG_API_RAMSESOBJECT_STRING(destinationRenderBuffer), name);
        return blitPass;
    }

    PickableObject* Scene::createPickableObject(const ArrayBuffer& geometryBuffer, const pickableObjectId_t id, std::string_view name /* = {} */)
    {
        PickableObject* pickableObject = m_impl.createPickableObject(geometryBuffer, id, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(pickableObject), LOG_API_RAMSESOBJECT_STRING(geometryBuffer), id, name);
        return pickableObject;
    }

    RenderBuffer* Scene::createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, std::string_view name)
    {
        RenderBuffer* renderBuffer = m_impl.createRenderBuffer(width, height, bufferType, bufferFormat, accessMode, sampleCount, name);
        LOG_HL_CLIENT_API7(LOG_API_RAMSESOBJECT_PTR_STRING(renderBuffer), width, height, bufferType, bufferFormat, accessMode, sampleCount, name);
        return renderBuffer;
    }

    RenderTarget* Scene::createRenderTarget(const RenderTargetDescription& rtDesc, std::string_view name)
    {
        RenderTarget* renderTarget = m_impl.createRenderTarget(rtDesc.m_impl, name);
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
        std::string_view name)
    {
        TextureSampler* texSampler = m_impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, texture, name);
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
        std::string_view name)
    {
        TextureSampler* texSampler = m_impl.createTextureSampler(wrapUMode, wrapVMode, wrapRMode, minSamplingMethod, magSamplingMethod, texture, name);
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
        std::string_view name)
    {
        TextureSampler* texSampler = m_impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel,texture, name);
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
        std::string_view name)
    {
        TextureSampler* texSampler = m_impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, renderBuffer, name);
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
        std::string_view name)
    {
        TextureSampler* texSampler = m_impl.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, texture2DBuffer, name);
        LOG_HL_CLIENT_API7(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, anisotropyLevel, LOG_API_RAMSESOBJECT_STRING(texture2DBuffer), name);
        return texSampler;
    }

    TextureSamplerMS* Scene::createTextureSamplerMS(const RenderBuffer& renderBuffer, std::string_view name)
    {
        TextureSamplerMS* texSampler = m_impl.createTextureSamplerMS(renderBuffer, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), LOG_API_RAMSESOBJECT_STRING(renderBuffer), name);
        return texSampler;
    }

    TextureSamplerExternal* Scene::createTextureSamplerExternal(ETextureSamplingMethod minSamplingMethod, ETextureSamplingMethod magSamplingMethod, std::string_view name)
    {
        TextureSamplerExternal* texSampler = m_impl.createTextureSamplerExternal(minSamplingMethod, magSamplingMethod, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(texSampler), minSamplingMethod, magSamplingMethod, name);
        return texSampler;
    }

    status_t Scene::createTransformationDataProvider(const Node& node, dataProviderId_t dataId)
    {
        status_t status = m_impl.createTransformationDataProvider(node, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(node), dataId);
        return status;
    }

    status_t Scene::createTransformationDataConsumer(const Node& node, dataConsumerId_t dataId)
    {
        status_t status = m_impl.createTransformationDataConsumer(node, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(node), dataId);
        return status;
    }

    status_t Scene::createDataProvider(const DataObject& dataObject, dataProviderId_t dataId)
    {
        status_t status = m_impl.createDataProvider(dataObject, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(dataObject), dataId);
        return status;
    }

    status_t Scene::createDataConsumer(const DataObject& dataObject, dataConsumerId_t dataId)
    {
        status_t status = m_impl.createDataConsumer(dataObject, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(dataObject), dataId);
        return status;
    }

    status_t Scene::createTextureProvider(const Texture2D& texture, dataProviderId_t dataId)
    {
        status_t status = m_impl.createTextureProvider(texture, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(texture), dataId);
        return status;
    }

    status_t Scene::updateTextureProvider(const Texture2D& texture, dataProviderId_t dataId)
    {
        status_t status = m_impl.updateTextureProvider(texture, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(texture), dataId);
        return status;
    }

    status_t Scene::createTextureConsumer(const TextureSampler& sampler, dataConsumerId_t dataId)
    {
        status_t status = m_impl.createTextureConsumer(sampler, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(sampler), dataId);
        return status;
    }

    status_t Scene::createTextureConsumer(const TextureSamplerMS& sampler, dataConsumerId_t dataId)
    {
        status_t status = m_impl.createTextureConsumer(sampler, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(sampler), dataId);
        return status;
    }

    status_t Scene::createTextureConsumer(const TextureSamplerExternal& sampler, dataConsumerId_t dataId)
    {
        status_t status = m_impl.createTextureConsumer(sampler, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(sampler), dataId);
        return status;
    }

    DataObject* Scene::createDataObject(EDataType dataType, std::string_view name)
    {
        DataObject* dataObject = m_impl.createDataObject(dataType, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(dataObject), dataType, name);
        return dataObject;
    }

    RamsesClient& Scene::getRamsesClient()
    {
        return m_impl.getHlRamsesClient();
    }

    template <typename T>
    ArrayResource* Scene::createArrayResourceInternal(uint32_t numElements, const T* arrayData, resourceCacheFlag_t cacheFlag, std::string_view name)
    {
        auto arr = m_impl.createArrayResource<T>(numElements, arrayData, cacheFlag, name);
        LOG_HL_CLIENT_API5(LOG_API_RESOURCE_PTR_STRING(arr), numElements, GetEDataType<T>(), LOG_API_GENERIC_PTR_STRING(arrayData), cacheFlag, name);
        return arr;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    Texture2D* Scene::createTexture2D(ETextureFormat format, uint32_t width, uint32_t height, size_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, std::string_view name /* = {} */)
    {
        Texture2D* tex = m_impl.createTexture2D(width, height, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        LOG_HL_CLIENT_API9(LOG_API_RESOURCE_PTR_STRING(tex), width, height, toString(format), mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, swizzle, cacheFlag, name);
        return tex;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    Texture3D* Scene::createTexture3D(ETextureFormat format, uint32_t width, uint32_t height, uint32_t depth, size_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, std::string_view name /* = {} */)
    {
        Texture3D* tex = m_impl.createTexture3D(width, height, depth, format, mipMapCount, mipLevelData, generateMipChain, cacheFlag, name);
        LOG_HL_CLIENT_API9(LOG_API_RESOURCE_PTR_STRING(tex), width, height, depth, toString(format), mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, cacheFlag, name);
        return tex;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    TextureCube* Scene::createTextureCube(ETextureFormat format, uint32_t size, size_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, std::string_view name /* = {} */)
    {
        TextureCube* tex = m_impl.createTextureCube(size, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        LOG_HL_CLIENT_API8(LOG_API_RESOURCE_PTR_STRING(tex), size, toString(format), mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, swizzle, cacheFlag, name);
        return tex;
    }

    Effect* Scene::createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, std::string_view name)
    {
        Effect* effect = m_impl.createEffect(effectDesc, cacheFlag, name);
        LOG_HL_CLIENT_API3(LOG_API_RESOURCE_PTR_STRING(effect), LOG_API_GENERIC_OBJECT_STRING(effectDesc), cacheFlag, name);
        return effect;
    }

    std::string Scene::getLastEffectErrorMessages() const
    {
        return m_impl.getLastEffectErrorMessages();
    }

    const Resource* Scene::getResource(resourceId_t id) const
    {
        return m_impl.getResource(id);
    }

    Resource* Scene::getResource(resourceId_t id)
    {
        return m_impl.getResource(id);
    }

    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<uint16_t>(uint32_t, const uint16_t*, resourceCacheFlag_t, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<uint32_t>(uint32_t, const uint32_t*, resourceCacheFlag_t, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<float>(uint32_t, const float*, resourceCacheFlag_t, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<vec2f>(uint32_t, const vec2f*, resourceCacheFlag_t, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<vec3f>(uint32_t, const vec3f*, resourceCacheFlag_t, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<vec4f>(uint32_t, const vec4f*, resourceCacheFlag_t, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<Byte>(uint32_t, const Byte*, resourceCacheFlag_t, std::string_view);
}
