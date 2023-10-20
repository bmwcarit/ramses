//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/Scene.h"
#include "ramses/client/Camera.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Node.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/Effect.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LogicObject.h"

// internal
#include "impl/SceneImpl.h"
#include "impl/EffectImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "impl/SaveFileConfigImpl.h"
#include "impl/RamsesClientTypesImpl.h"

namespace ramses
{
    Scene::Scene(std::unique_ptr<internal::SceneImpl> impl)
        : ClientObject{ std::move(impl) }
        , m_impl{ static_cast<internal::SceneImpl&>(ClientObject::m_impl) }
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

    Geometry* Scene::createGeometry(const Effect& effect, std::string_view name)
    {
        Geometry* geomBinding = m_impl.createGeometry(effect, name);
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

    bool Scene::publish(EScenePublicationMode publicationMode)
    {
        const bool status = m_impl.publish(publicationMode);
        LOG_HL_CLIENT_API1(status, publicationMode);
        return status;
    }

    bool Scene::unpublish()
    {
        const bool status = m_impl.unpublish();
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

    bool Scene::saveToFile(std::string_view fileName, const SaveFileConfig& config) const
    {
        const auto status = m_impl.saveToFile(fileName, config.impl());
        LOG_HL_CLIENT_API2(status, fileName, config.impl());
        return status;
    }

    LogicEngine* Scene::createLogicEngine(std::string_view name)
    {
        auto logic = m_impl.createLogicEngine(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(logic), name);
        return logic;
    }

    bool Scene::destroy(SceneObject& object)
    {
        const bool status = m_impl.destroy(object);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(object));
        return status;
    }

    bool Scene::setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds)
    {
        const bool status = m_impl.setExpirationTimestamp(ptpExpirationTimestampInMilliseconds);
        LOG_HL_CLIENT_API1(status, ptpExpirationTimestampInMilliseconds);
        return status;
    }

    bool Scene::flush(sceneVersionTag_t sceneVersionTag)
    {
        const bool status = m_impl.flush(sceneVersionTag);
        LOG_HL_CLIENT_API2(status, sceneVersionTag, m_impl.getSceneId());
        return status;
    }

    bool Scene::resetUniformTimeMs()
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

    bool Scene::linkData(SceneReference* providerReference, dataProviderId_t providerId, SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        auto status = m_impl.linkData(providerReference, providerId, consumerReference, consumerId);
        LOG_HL_CLIENT_API4(status, LOG_API_RAMSESOBJECT_PTR_STRING(providerReference), providerId, LOG_API_RAMSESOBJECT_PTR_STRING(consumerReference), consumerId);
        return status;
    }

    bool Scene::unlinkData(SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        auto status = m_impl.unlinkData(consumerReference, consumerId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_PTR_STRING(consumerReference), consumerId);
        return status;
    }

    template <typename T> T* Scene::findObjectByNameInternal(std::string_view name)
    {
        return m_impl.findObjectByName<T>(name);
    }

    template <typename T> const T* Scene::findObjectByNameInternal(std::string_view name) const
    {
        return m_impl.findObjectByName<T>(name);
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

    RenderBuffer* Scene::createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, std::string_view name)
    {
        RenderBuffer* renderBuffer = m_impl.createRenderBuffer(width, height, bufferFormat, accessMode, sampleCount, name);
        LOG_HL_CLIENT_API6(LOG_API_RAMSESOBJECT_PTR_STRING(renderBuffer), width, height, bufferFormat, accessMode, sampleCount, name);
        return renderBuffer;
    }

    RenderTarget* Scene::createRenderTarget(const RenderTargetDescription& rtDesc, std::string_view name)
    {
        RenderTarget* renderTarget = m_impl.createRenderTarget(rtDesc.impl(), name);
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

    bool Scene::createTransformationDataProvider(const Node& node, dataProviderId_t dataId)
    {
        bool status = m_impl.createTransformationDataProvider(node, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(node), dataId);
        return status;
    }

    bool Scene::createTransformationDataConsumer(const Node& node, dataConsumerId_t dataId)
    {
        bool status = m_impl.createTransformationDataConsumer(node, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(node), dataId);
        return status;
    }

    bool Scene::createDataProvider(const DataObject& dataObject, dataProviderId_t dataId)
    {
        bool status = m_impl.createDataProvider(dataObject, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(dataObject), dataId);
        return status;
    }

    bool Scene::createDataConsumer(const DataObject& dataObject, dataConsumerId_t dataId)
    {
        bool status = m_impl.createDataConsumer(dataObject, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(dataObject), dataId);
        return status;
    }

    bool Scene::createTextureProvider(const Texture2D& texture, dataProviderId_t dataId)
    {
        bool status = m_impl.createTextureProvider(texture, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(texture), dataId);
        return status;
    }

    bool Scene::updateTextureProvider(const Texture2D& texture, dataProviderId_t dataId)
    {
        bool status = m_impl.updateTextureProvider(texture, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(texture), dataId);
        return status;
    }

    bool Scene::createTextureConsumer(const TextureSampler& sampler, dataConsumerId_t dataId)
    {
        bool status = m_impl.createTextureConsumer(sampler, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(sampler), dataId);
        return status;
    }

    bool Scene::createTextureConsumer(const TextureSamplerMS& sampler, dataConsumerId_t dataId)
    {
        bool status = m_impl.createTextureConsumer(sampler, dataId);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(sampler), dataId);
        return status;
    }

    bool Scene::createTextureConsumer(const TextureSamplerExternal& sampler, dataConsumerId_t dataId)
    {
        bool status = m_impl.createTextureConsumer(sampler, dataId);
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
    ArrayResource* Scene::createArrayResourceInternal(uint32_t numElements, const T* arrayData, std::string_view name)
    {
        auto arr = m_impl.createArrayResource<T>(numElements, arrayData, name);
        LOG_HL_CLIENT_API4(LOG_API_RESOURCE_PTR_STRING(arr), numElements, GetEDataType<T>(), LOG_API_GENERIC_PTR_STRING(arrayData), name);
        return arr;
    }

    Texture2D* Scene::createTexture2D(ETextureFormat format, uint32_t width, uint32_t height, const std::vector<MipLevelData>& mipLevelData, bool generateMipChain, const TextureSwizzle& swizzle, std::string_view name /* = {} */)
    {
        Texture2D* tex = m_impl.createTexture2D(width, height, format, mipLevelData, generateMipChain, swizzle, name);
        LOG_HL_CLIENT_API8(LOG_API_RESOURCE_PTR_STRING(tex), width, height, toString(format), mipLevelData.size(), LOG_API_GENERIC_OBJECT_STRING(mipLevelData), generateMipChain, swizzle, name);
        return tex;
    }

    Texture3D* Scene::createTexture3D(ETextureFormat format, uint32_t width, uint32_t height, uint32_t depth, const std::vector<MipLevelData>& mipLevelData, bool generateMipChain, std::string_view name /* = {} */)
    {
        Texture3D* tex = m_impl.createTexture3D(width, height, depth, format, mipLevelData, generateMipChain, name);
        LOG_HL_CLIENT_API8(LOG_API_RESOURCE_PTR_STRING(tex), width, height, depth, toString(format), mipLevelData.size(), LOG_API_GENERIC_OBJECT_STRING(mipLevelData), generateMipChain, name);
        return tex;
    }

    TextureCube* Scene::createTextureCube(ETextureFormat format, uint32_t size, const std::vector<CubeMipLevelData>& mipLevelData, bool generateMipChain, const TextureSwizzle& swizzle, std::string_view name /* = {} */)
    {
        TextureCube* tex = m_impl.createTextureCube(size, format, mipLevelData, generateMipChain, swizzle, name);
        LOG_HL_CLIENT_API7(LOG_API_RESOURCE_PTR_STRING(tex), size, toString(format), mipLevelData.size(), LOG_API_GENERIC_OBJECT_STRING(mipLevelData), generateMipChain, swizzle, name);
        return tex;
    }

    Effect* Scene::createEffect(const EffectDescription& effectDesc, std::string_view name)
    {
        Effect* effect = m_impl.createEffect(effectDesc, name);
        LOG_HL_CLIENT_API2(LOG_API_RESOURCE_PTR_STRING(effect), LOG_API_GENERIC_OBJECT_STRING(effectDesc), name);
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

    internal::SceneImpl& Scene::impl()
    {
        return m_impl;
    }

    const internal::SceneImpl& Scene::impl() const
    {
        return m_impl;
    }

    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<uint16_t>(uint32_t, const uint16_t*, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<uint32_t>(uint32_t, const uint32_t*, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<float>(uint32_t, const float*, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<vec2f>(uint32_t, const vec2f*, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<vec3f>(uint32_t, const vec3f*, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<vec4f>(uint32_t, const vec4f*, std::string_view);
    template RAMSES_API ArrayResource* Scene::createArrayResourceInternal<std::byte>(uint32_t, const std::byte*, std::string_view);

    template RAMSES_API SceneObject*            Scene::findObjectByNameInternal<SceneObject>(std::string_view);
    template RAMSES_API LogicEngine*            Scene::findObjectByNameInternal<LogicEngine>(std::string_view);
    template RAMSES_API LogicObject*            Scene::findObjectByNameInternal<LogicObject>(std::string_view);
    template RAMSES_API Node*                   Scene::findObjectByNameInternal<Node>(std::string_view);
    template RAMSES_API MeshNode*               Scene::findObjectByNameInternal<MeshNode>(std::string_view);
    template RAMSES_API Camera*                 Scene::findObjectByNameInternal<Camera>(std::string_view);
    template RAMSES_API PerspectiveCamera*      Scene::findObjectByNameInternal<PerspectiveCamera>(std::string_view);
    template RAMSES_API OrthographicCamera*     Scene::findObjectByNameInternal<OrthographicCamera>(std::string_view);
    template RAMSES_API Effect*                 Scene::findObjectByNameInternal<Effect>(std::string_view);
    template RAMSES_API Appearance*             Scene::findObjectByNameInternal<Appearance>(std::string_view);
    template RAMSES_API Geometry*               Scene::findObjectByNameInternal<Geometry>(std::string_view);
    template RAMSES_API PickableObject*         Scene::findObjectByNameInternal<PickableObject>(std::string_view);
    template RAMSES_API Resource*               Scene::findObjectByNameInternal<Resource>(std::string_view);
    template RAMSES_API Texture2D*              Scene::findObjectByNameInternal<Texture2D>(std::string_view);
    template RAMSES_API Texture3D*              Scene::findObjectByNameInternal<Texture3D>(std::string_view);
    template RAMSES_API TextureCube*            Scene::findObjectByNameInternal<TextureCube>(std::string_view);
    template RAMSES_API ArrayResource*          Scene::findObjectByNameInternal<ArrayResource>(std::string_view);
    template RAMSES_API RenderGroup*            Scene::findObjectByNameInternal<RenderGroup>(std::string_view);
    template RAMSES_API RenderPass*             Scene::findObjectByNameInternal<RenderPass>(std::string_view);
    template RAMSES_API BlitPass*               Scene::findObjectByNameInternal<BlitPass>(std::string_view);
    template RAMSES_API TextureSampler*         Scene::findObjectByNameInternal<TextureSampler>(std::string_view);
    template RAMSES_API TextureSamplerMS*       Scene::findObjectByNameInternal<TextureSamplerMS>(std::string_view);
    template RAMSES_API RenderBuffer*           Scene::findObjectByNameInternal<RenderBuffer>(std::string_view);
    template RAMSES_API RenderTarget*           Scene::findObjectByNameInternal<RenderTarget>(std::string_view);
    template RAMSES_API ArrayBuffer*            Scene::findObjectByNameInternal<ArrayBuffer>(std::string_view);
    template RAMSES_API Texture2DBuffer*        Scene::findObjectByNameInternal<Texture2DBuffer>(std::string_view);
    template RAMSES_API DataObject*             Scene::findObjectByNameInternal<DataObject>(std::string_view);
    template RAMSES_API SceneReference*         Scene::findObjectByNameInternal<SceneReference>(std::string_view);
    template RAMSES_API TextureSamplerExternal* Scene::findObjectByNameInternal<TextureSamplerExternal>(std::string_view);

    template RAMSES_API const SceneObject*            Scene::findObjectByNameInternal<SceneObject>(std::string_view) const;
    template RAMSES_API const LogicEngine*            Scene::findObjectByNameInternal<LogicEngine>(std::string_view) const;
    template RAMSES_API const LogicObject*            Scene::findObjectByNameInternal<LogicObject>(std::string_view) const;
    template RAMSES_API const Node*                   Scene::findObjectByNameInternal<Node>(std::string_view) const;
    template RAMSES_API const MeshNode*               Scene::findObjectByNameInternal<MeshNode>(std::string_view) const;
    template RAMSES_API const Camera*                 Scene::findObjectByNameInternal<Camera>(std::string_view) const;
    template RAMSES_API const PerspectiveCamera*      Scene::findObjectByNameInternal<PerspectiveCamera>(std::string_view) const;
    template RAMSES_API const OrthographicCamera*     Scene::findObjectByNameInternal<OrthographicCamera>(std::string_view) const;
    template RAMSES_API const Effect*                 Scene::findObjectByNameInternal<Effect>(std::string_view) const;
    template RAMSES_API const Appearance*             Scene::findObjectByNameInternal<Appearance>(std::string_view) const;
    template RAMSES_API const Geometry*               Scene::findObjectByNameInternal<Geometry>(std::string_view) const;
    template RAMSES_API const PickableObject*         Scene::findObjectByNameInternal<PickableObject>(std::string_view) const;
    template RAMSES_API const Resource*               Scene::findObjectByNameInternal<Resource>(std::string_view) const;
    template RAMSES_API const Texture2D*              Scene::findObjectByNameInternal<Texture2D>(std::string_view) const;
    template RAMSES_API const Texture3D*              Scene::findObjectByNameInternal<Texture3D>(std::string_view) const;
    template RAMSES_API const TextureCube*            Scene::findObjectByNameInternal<TextureCube>(std::string_view) const;
    template RAMSES_API const ArrayResource*          Scene::findObjectByNameInternal<ArrayResource>(std::string_view) const;
    template RAMSES_API const RenderGroup*            Scene::findObjectByNameInternal<RenderGroup>(std::string_view) const;
    template RAMSES_API const RenderPass*             Scene::findObjectByNameInternal<RenderPass>(std::string_view) const;
    template RAMSES_API const BlitPass*               Scene::findObjectByNameInternal<BlitPass>(std::string_view) const;
    template RAMSES_API const TextureSampler*         Scene::findObjectByNameInternal<TextureSampler>(std::string_view) const;
    template RAMSES_API const TextureSamplerMS*       Scene::findObjectByNameInternal<TextureSamplerMS>(std::string_view) const;
    template RAMSES_API const RenderBuffer*           Scene::findObjectByNameInternal<RenderBuffer>(std::string_view) const;
    template RAMSES_API const RenderTarget*           Scene::findObjectByNameInternal<RenderTarget>(std::string_view) const;
    template RAMSES_API const ArrayBuffer*            Scene::findObjectByNameInternal<ArrayBuffer>(std::string_view) const;
    template RAMSES_API const Texture2DBuffer*        Scene::findObjectByNameInternal<Texture2DBuffer>(std::string_view) const;
    template RAMSES_API const DataObject*             Scene::findObjectByNameInternal<DataObject>(std::string_view) const;
    template RAMSES_API const SceneReference*         Scene::findObjectByNameInternal<SceneReference>(std::string_view) const;
    template RAMSES_API const TextureSamplerExternal* Scene::findObjectByNameInternal<TextureSamplerExternal>(std::string_view) const;
}
