//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <array>

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineStepFloat.h"
#include "ramses-client-api/AnimatedProperty.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/DataMatrix22f.h"
#include "ramses-client-api/DataMatrix33f.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector3i.h"
#include "ramses-client-api/DataVector4i.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/ArrayResource.h"

#include "ScenePersistationTest.h"
#include "CameraNodeImpl.h"
#include "AppearanceImpl.h"
#include "DataObjectImpl.h"
#include "GeometryBindingImpl.h"
#include "ramses-client-api/Effect.h"
#include "EffectImpl.h"
#include "TestEffects.h"
#include "AnimationSystemImpl.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/BlitPass.h"
#include "SceneAPI/RenderGroupUtils.h"
#include "SceneAPI/RenderPass.h"
#include "SceneImpl.h"
#include "Texture2DImpl.h"
#include "TextureSamplerImpl.h"
#include "RenderGroupImpl.h"
#include "RenderPassImpl.h"
#include "BlitPassImpl.h"
#include "PickableObjectImpl.h"
#include "RenderBufferImpl.h"
#include "RenderTargetImpl.h"
#include "StreamTextureImpl.h"
#include "MeshNodeImpl.h"
#include "SplineImpl.h"
#include "AnimatedPropertyImpl.h"
#include "AnimationImpl.h"
#include "AnimationSequenceImpl.h"
#include "Utils/File.h"
#include "ramses-utils.h"

#include "ArrayBufferImpl.h"
#include "Texture2DBufferImpl.h"
#include "RamsesObjectTestTypes.h"
#include "ramses-client-api/UniformInput.h"
#include "Scene/ESceneActionId.h"
#include "Scene/SceneResourceChanges.h"
#include "Scene/SceneActionApplier.h"
#include "ramses-hmi-utils.h"

#include <fstream>

namespace ramses
{
    using namespace testing;

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAScene)
    {
        const status_t status = m_scene.saveToFile("someTemporaryFile.ram", false);
        EXPECT_EQ(StatusOK, status);

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        ObjectTypeHistogram origSceneNumbers;
        ObjectTypeHistogram loadedSceneNumbers;
        fillObjectTypeHistogramFromScene(origSceneNumbers, m_scene);
        fillObjectTypeHistogramFromScene(loadedSceneNumbers, *m_sceneLoaded);
        EXPECT_PRED_FORMAT2(AssertHistogramEqual, origSceneNumbers, loadedSceneNumbers);

        ramses_internal::SceneSizeInformation origSceneSizeInfo = m_scene.impl.getIScene().getSceneSizeInformation();
        ramses_internal::SceneSizeInformation loadedSceneSizeInfo = m_sceneLoaded->impl.getIScene().getSceneSizeInformation();
        EXPECT_EQ(origSceneSizeInfo, loadedSceneSizeInfo);

        const auto animationSystemLoaded = RamsesUtils::TryConvert<AnimationSystem>(*m_sceneLoaded->findObjectByName("animation system"));
        ASSERT_TRUE(nullptr != animationSystemLoaded);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteARemoteCamera)
    {
        Camera* camera = this->m_scene.createRemoteCamera("my cam");

        doWriteReadCycle();

        const Camera* loadedCamera = this->getObjectForTesting<Camera>("my cam");
        EXPECT_EQ(camera->impl.getCameraHandle(), loadedCamera->impl.getCameraHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAPerspectiveCamera)
    {
        PerspectiveCamera* camera = this->m_scene.createPerspectiveCamera("my cam");
        camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        camera->setViewport(1, -2, 3, 4);

        doWriteReadCycle();

        PerspectiveCamera* loadedCamera = this->getObjectForTesting<PerspectiveCamera>("my cam");
        EXPECT_EQ(camera->impl.getCameraHandle(), loadedCamera->impl.getCameraHandle());
        EXPECT_EQ(0.1f, loadedCamera->getLeftPlane());
        EXPECT_EQ(0.2f, loadedCamera->getRightPlane());
        EXPECT_EQ(0.3f, loadedCamera->getBottomPlane());
        EXPECT_EQ(0.4f, loadedCamera->getTopPlane());
        EXPECT_EQ(0.5f, loadedCamera->getNearPlane());
        EXPECT_EQ(0.6f, loadedCamera->getFarPlane());

        EXPECT_EQ(1, loadedCamera->getViewportX());
        EXPECT_EQ(-2, loadedCamera->getViewportY());
        EXPECT_EQ(3u, loadedCamera->getViewportWidth());
        EXPECT_EQ(4u, loadedCamera->getViewportHeight());

        m_sceneLoaded->destroy(*loadedCamera);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteStreamTexture)
    {
        uint8_t data[4] = { 0u };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* fallbackTexture = this->m_scene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "fallbackTexture");

        StreamTexture* streamTexture                   = this->m_scene.createStreamTexture(*fallbackTexture, waylandIviSurfaceId_t(3), "resourceName");
        StreamTexture* streamTextureWithForcedFallback = this->m_scene.createStreamTexture(*fallbackTexture, waylandIviSurfaceId_t(4), "resourceName2");
        streamTextureWithForcedFallback->forceFallbackImage(true);

        this->doWriteReadCycle();

        const StreamTexture* loadedStreamTexture                   = this->getObjectForTesting<StreamTexture>("resourceName");
        const StreamTexture* loadedStreamTextureWithForcedFallback = this->getObjectForTesting<StreamTexture>("resourceName2");

        EXPECT_EQ(streamTexture->impl.getStreamSource(), loadedStreamTexture->impl.getStreamSource());
        EXPECT_EQ(streamTexture->impl.getFallbackTextureHash(), loadedStreamTexture->impl.getFallbackTextureHash());
        EXPECT_EQ(fallbackTexture->impl.getLowlevelResourceHash(), loadedStreamTexture->impl.getFallbackTextureHash());
        EXPECT_FALSE(loadedStreamTexture->impl.getForceFallbackImage());
        EXPECT_TRUE(loadedStreamTextureWithForcedFallback->impl.getForceFallbackImage());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAnOrthographicCamera)
    {
        OrthographicCamera* camera = this->m_scene.createOrthographicCamera("my cam");
        camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        camera->setViewport(1, -2, 3, 4);

        doWriteReadCycle();

        OrthographicCamera* loadedCamera = this->getObjectForTesting<OrthographicCamera>("my cam");

        EXPECT_EQ(camera->impl.getCameraHandle(), loadedCamera->impl.getCameraHandle());

        EXPECT_FLOAT_EQ(0.1f, loadedCamera->getLeftPlane());
        EXPECT_FLOAT_EQ(0.2f, loadedCamera->getRightPlane());
        EXPECT_FLOAT_EQ(0.3f, loadedCamera->getBottomPlane());
        EXPECT_FLOAT_EQ(0.4f, loadedCamera->getTopPlane());
        EXPECT_FLOAT_EQ(0.5f, loadedCamera->getNearPlane());
        EXPECT_FLOAT_EQ(0.6f, loadedCamera->getFarPlane());

        EXPECT_EQ(1, loadedCamera->getViewportX());
        EXPECT_EQ(-2, loadedCamera->getViewportY());
        EXPECT_EQ(3u, loadedCamera->getViewportWidth());
        EXPECT_EQ(4u, loadedCamera->getViewportHeight());

        m_sceneLoaded->destroy(*loadedCamera);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAnAppearance)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        doWriteReadCycle();

        Appearance* loadedAppearance = this->getObjectForTesting<Appearance>("appearance");

        EXPECT_EQ(appearance->getEffect().impl.getLowlevelResourceHash(), loadedAppearance->getEffect().impl.getLowlevelResourceHash());
        EXPECT_EQ(appearance->impl.getRenderStateHandle(), loadedAppearance->impl.getRenderStateHandle());
        EXPECT_EQ(appearance->impl.getUniformDataInstance(), loadedAppearance->impl.getUniformDataInstance());
        EXPECT_EQ(appearance->impl.getIScene().getSceneId(), loadedAppearance->impl.getIScene().getSceneId());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, keepingTrackOfsceneObjectIdsAndFindObjectByIdWork)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);

        const Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");
        const GeometryBinding* geometry = this->m_scene.createGeometryBinding(*effect, "geometry");

        const sceneObjectId_t appearanceIdBeforeSaveAndLoad = appearance->getSceneObjectId();
        const sceneObjectId_t geometryIdBeforeSaveAndLoad = geometry->getSceneObjectId();
        EXPECT_NE(appearanceIdBeforeSaveAndLoad, geometryIdBeforeSaveAndLoad);

        doWriteReadCycle();

        const Appearance& appearanceLoaded = RamsesObjectTypeUtils::ConvertTo<Appearance>(*this->m_scene.findObjectById(appearanceIdBeforeSaveAndLoad));
        EXPECT_EQ(appearanceIdBeforeSaveAndLoad, appearanceLoaded.getSceneObjectId());

        const GeometryBinding& geometryLoaded = RamsesObjectTypeUtils::ConvertTo<GeometryBinding>(*this->m_scene.findObjectById(geometryIdBeforeSaveAndLoad));
        EXPECT_EQ(geometryIdBeforeSaveAndLoad, geometryLoaded.getSceneObjectId());

        const Camera* camera = this->m_scene.createRemoteCamera("camera");
        EXPECT_NE(appearanceIdBeforeSaveAndLoad, camera->getSceneObjectId());
        EXPECT_NE(geometryIdBeforeSaveAndLoad, camera->getSceneObjectId());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAnAppearanceWithUniformValuesSetOrBound)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        UniformInput fragColorR;
        UniformInput fragColorG;
        effect->findUniformInput("u_FragColorR", fragColorR);
        effect->findUniformInput("u_FragColorG", fragColorG);

        DataFloat* fragColorDataObject = this->m_scene.createDataFloat();
        fragColorDataObject->setValue(123.f);

        appearance->setInputValueFloat(fragColorR, 567.f);
        appearance->bindInput(fragColorG, *fragColorDataObject);

        doWriteReadCycle();

        Appearance* loadedAppearance = this->getObjectForTesting<Appearance>("appearance");
        const Effect& loadedEffect = loadedAppearance->getEffect();

        UniformInput fragColorROut;
        UniformInput fragColorGOut;
        loadedEffect.findUniformInput("u_FragColorR", fragColorROut);
        loadedEffect.findUniformInput("u_FragColorG", fragColorGOut);

        float resultR = 0.f;
        loadedAppearance->getInputValueFloat(fragColorROut, resultR);
        EXPECT_FLOAT_EQ(567.f, resultR);

        ASSERT_TRUE(loadedAppearance->isInputBound(fragColorGOut));
        float resultG = 0.f;
        const DataObject& fragColorDataObjectOut = *loadedAppearance->getDataObjectBoundToInput(fragColorGOut);
        RamsesObjectTypeUtils::ConvertTo<DataFloat>(fragColorDataObjectOut).getValue(resultG);
        EXPECT_EQ(123.f, resultG);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, multipleAppearancesSharingSameEffectAreCorrectlyWrittenAndLoaded)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        const Appearance* appearance1 = m_scene.createAppearance(*effect, "appearance1");
        const Appearance* appearance2 = m_scene.createAppearance(*effect, "appearance2");

        // check data layout ref count on LL
        EXPECT_EQ(appearance1->impl.getUniformDataLayout(), appearance2->impl.getUniformDataLayout());
        const uint32_t numReferences = m_scene.impl.getIScene().getNumDataLayoutReferences(appearance1->impl.getUniformDataLayout());
        EXPECT_EQ(2u, numReferences);

        doWriteReadCycle();

        Appearance* loadedAppearance1 = this->getObjectForTesting<Appearance>("appearance1");
        Appearance* loadedAppearance2 = this->getObjectForTesting<Appearance>("appearance2");

        // check data layout ref count on LL in loaded scene
        EXPECT_EQ(loadedAppearance1->impl.getUniformDataLayout(), loadedAppearance2->impl.getUniformDataLayout());
        const uint32_t numReferencesLoaded = m_scene.impl.getIScene().getNumDataLayoutReferences(loadedAppearance1->impl.getUniformDataLayout());
        EXPECT_EQ(numReferences, numReferencesLoaded);

        m_sceneLoaded->destroy(*loadedAppearance2);
        m_sceneLoaded->destroy(*loadedAppearance1);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteGeometryBinding)
    {
        static const uint16_t inds[3] = { 0, 1, 2 };
        ArrayResource* const indices = this->m_scene.createArrayResource(EDataType::UInt16, 3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");

        Effect* effect = TestEffects::CreateTestEffectWithAttribute(this->m_scene);

        GeometryBinding* geometry = this->m_scene.createGeometryBinding(*effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));

        doWriteReadCycle();

        const GeometryBinding* loadedGeometry = this->getObjectForTesting<GeometryBinding>("geometry");

        EXPECT_EQ(geometry->impl.getEffectHash(), loadedGeometry->impl.getEffectHash());
        EXPECT_EQ(geometry->impl.getAttributeDataLayout(), loadedGeometry->impl.getAttributeDataLayout());
        EXPECT_EQ(geometry->impl.getAttributeDataInstance(), loadedGeometry->impl.getAttributeDataInstance());
        EXPECT_EQ(geometry->impl.getIndicesCount(), loadedGeometry->impl.getIndicesCount());

        const Effect& loadedEffect = loadedGeometry->getEffect();
        EXPECT_EQ(effect->getResourceId(), loadedEffect.getResourceId());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), loadedEffect.impl.getLowlevelResourceHash());
        EXPECT_EQ(effect->impl.getObjectRegistryHandle(), loadedEffect.impl.getObjectRegistryHandle());
        EXPECT_EQ(4u, loadedEffect.getAttributeInputCount());
        AttributeInput attributeInputOut;
        EXPECT_EQ(StatusOK, loadedEffect.findAttributeInput("a_position", attributeInputOut));
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAMeshNode)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        doWriteReadCycle();

        MeshNode* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");

        EXPECT_EQ(meshNode->getAppearance(), loadedMeshNode->getAppearance());
        EXPECT_EQ(meshNode->getGeometryBinding(),   loadedMeshNode->getGeometryBinding());
        EXPECT_EQ(meshNode->getStartIndex(), loadedMeshNode->getStartIndex());
        EXPECT_EQ(meshNode->getIndexCount(), loadedMeshNode->getIndexCount());
        EXPECT_EQ(meshNode->impl.getFlattenedVisibility(), loadedMeshNode->impl.getFlattenedVisibility());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAMeshNode_withVisibilityParent)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        Node* visibilityParent = this->m_scene.createNode("vis node");
        visibilityParent->setVisibility(EVisibilityMode::Invisible);
        visibilityParent->addChild(*meshNode);

        // Apply visibility state only with flush, not before
        EXPECT_EQ(meshNode->impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        this->m_scene.flush();
        EXPECT_EQ(meshNode->impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        doWriteReadCycle();

        MeshNode* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");
        EXPECT_EQ(loadedMeshNode->impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAMeshNode_withVisibilityParentOff)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        Node* visibilityParent = this->m_scene.createNode("vis node");
        visibilityParent->setVisibility(EVisibilityMode::Off);
        visibilityParent->addChild(*meshNode);

        // Apply visibility state only with flush, not before
        EXPECT_EQ(meshNode->impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        this->m_scene.flush();
        EXPECT_EQ(meshNode->impl.getFlattenedVisibility(), EVisibilityMode::Off);

        doWriteReadCycle();

        MeshNode* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");
        EXPECT_EQ(loadedMeshNode->impl.getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAMeshNode_withValues)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        GeometryBinding* geometry = this->m_scene.createGeometryBinding(*effect, "geometry");
        const uint16_t data = 0u;
        ArrayResource* indices = this->m_scene.createArrayResource(EDataType::UInt16, 1u, &data, ramses::ResourceCacheFlag_DoNotCache, "indices");
        geometry->setIndices(*indices);

        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        //EXPECT_EQ(StatusOK, meshNode.setIndexArray(indices));
        EXPECT_EQ(StatusOK, meshNode->setAppearance(*appearance));
        EXPECT_EQ(StatusOK, meshNode->setGeometryBinding(*geometry));
        EXPECT_EQ(StatusOK, meshNode->setStartIndex(456));
        EXPECT_EQ(StatusOK, meshNode->setIndexCount(678u));
        EXPECT_EQ(StatusOK, meshNode->impl.setFlattenedVisibility(EVisibilityMode::Off));
        doWriteReadCycle();

        MeshNode* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");

        EXPECT_STREQ(meshNode->getAppearance()->getName(), loadedMeshNode->getAppearance()->getName());
        EXPECT_EQ(meshNode->getAppearance()->getSceneObjectId(), loadedMeshNode->getAppearance()->getSceneObjectId());
        EXPECT_STREQ(meshNode->getGeometryBinding()->getName(), loadedMeshNode->getGeometryBinding()->getName());
        EXPECT_EQ(meshNode->getGeometryBinding()->getSceneObjectId(), loadedMeshNode->getGeometryBinding()->getSceneObjectId());
        EXPECT_EQ(meshNode->getSceneObjectId(), loadedMeshNode->getSceneObjectId());
        EXPECT_EQ(456u, loadedMeshNode->getStartIndex());
        EXPECT_EQ(678u, loadedMeshNode->getIndexCount());
        EXPECT_EQ(loadedMeshNode->impl.getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteANodeWithVisibility)
    {
        Node* visibilityNode = this->m_scene.createNode("a visibilitynode");

        visibilityNode->setVisibility(EVisibilityMode::Invisible);

        doWriteReadCycle();

        Node* loadedVisibilityNode = this->getObjectForTesting<Node>("a visibilitynode");

        EXPECT_EQ(loadedVisibilityNode->getVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteARenderGroup)
    {
        RenderGroup* renderGroup = this->m_scene.createRenderGroup("a rendergroup");

        MeshNode* meshA = this->m_scene.createMeshNode("meshA");
        MeshNode* meshB = this->m_scene.createMeshNode("meshB");

        renderGroup->addMeshNode(*meshA, 1);
        renderGroup->addMeshNode(*meshB, 2);

        doWriteReadCycle();

        RenderGroup* loadedRenderGroup = this->getObjectForTesting<RenderGroup>("a rendergroup");
        const MeshNode* loadedMeshA = this->getObjectForTesting<MeshNode>("meshA");
        const MeshNode* loadedMeshB = this->getObjectForTesting<MeshNode>("meshB");

        EXPECT_STREQ(renderGroup->getName(), loadedRenderGroup->getName());
        EXPECT_EQ(renderGroup->getSceneObjectId(), loadedRenderGroup->getSceneObjectId());

        EXPECT_EQ(2u, loadedRenderGroup->impl.getAllMeshes().size());
        EXPECT_EQ(&loadedMeshA->impl, loadedRenderGroup->impl.getAllMeshes()[0]);
        EXPECT_EQ(&loadedMeshB->impl, loadedRenderGroup->impl.getAllMeshes()[1]);

        const auto& internalRg = m_sceneLoaded->impl.getIScene().getRenderGroup(renderGroup->impl.getRenderGroupHandle());
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(meshA->impl.getRenderableHandle(), internalRg));
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(meshB->impl.getRenderableHandle(), internalRg));
        EXPECT_EQ(1, ramses_internal::RenderGroupUtils::FindRenderableEntry(meshA->impl.getRenderableHandle(), internalRg)->order);
        EXPECT_EQ(2, ramses_internal::RenderGroupUtils::FindRenderableEntry(meshB->impl.getRenderableHandle(), internalRg)->order);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteANestedRenderGroup)
    {
        RenderGroup* renderGroup = this->m_scene.createRenderGroup("a rendergroup");
        RenderGroup* nestedRenderGroup = this->m_scene.createRenderGroup("a nested rendergroup");

        MeshNode* meshA = this->m_scene.createMeshNode("meshA");
        MeshNode* meshB = this->m_scene.createMeshNode("meshB");

        ASSERT_EQ(StatusOK, renderGroup->addMeshNode(*meshA, 1));
        ASSERT_EQ(StatusOK, nestedRenderGroup->addMeshNode(*meshB, 2));
        ASSERT_EQ(StatusOK, renderGroup->addRenderGroup(*nestedRenderGroup, 1));

        doWriteReadCycle();

        RenderGroup* loadedRenderGroup = this->getObjectForTesting<RenderGroup>("a rendergroup");
        RenderGroup* loadedNestedRenderGroup = this->getObjectForTesting<RenderGroup>("a nested rendergroup");
        const MeshNode* loadedMeshA = this->getObjectForTesting<MeshNode>("meshA");
        const MeshNode* loadedMeshB = this->getObjectForTesting<MeshNode>("meshB");

        EXPECT_STREQ(nestedRenderGroup->getName(), loadedNestedRenderGroup->getName());
        EXPECT_EQ(nestedRenderGroup->getSceneObjectId(), loadedNestedRenderGroup->getSceneObjectId());

        EXPECT_EQ(1u, loadedRenderGroup->impl.getAllMeshes().size());
        EXPECT_EQ(1u, loadedRenderGroup->impl.getAllRenderGroups().size());
        EXPECT_EQ(1u, loadedNestedRenderGroup->impl.getAllMeshes().size());

        EXPECT_EQ(&loadedMeshA->impl, loadedRenderGroup->impl.getAllMeshes()[0]);
        EXPECT_EQ(&loadedMeshB->impl, loadedNestedRenderGroup->impl.getAllMeshes()[0]);

        EXPECT_EQ(&loadedNestedRenderGroup->impl, loadedRenderGroup->impl.getAllRenderGroups()[0]);

        const auto& internalRg = m_sceneLoaded->impl.getIScene().getRenderGroup(renderGroup->impl.getRenderGroupHandle());
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(meshA->impl.getRenderableHandle(), internalRg));
        EXPECT_EQ(1, ramses_internal::RenderGroupUtils::FindRenderableEntry(meshA->impl.getRenderableHandle(), internalRg)->order);

        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(nestedRenderGroup->impl.getRenderGroupHandle(), internalRg));
        EXPECT_EQ(1, ramses_internal::RenderGroupUtils::FindRenderGroupEntry(nestedRenderGroup->impl.getRenderGroupHandle(), internalRg)->order);

        const auto& internalRgNested = m_sceneLoaded->impl.getIScene().getRenderGroup(nestedRenderGroup->impl.getRenderGroupHandle());
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(meshB->impl.getRenderableHandle(), internalRgNested));
        EXPECT_EQ(2, ramses_internal::RenderGroupUtils::FindRenderableEntry(meshB->impl.getRenderableHandle(), internalRgNested)->order);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteABasicRenderPass)
    {
        const int32_t renderOrder = 1;

        RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");
        EXPECT_EQ(StatusOK, renderPass->setRenderOrder(renderOrder));
        EXPECT_EQ(StatusOK, renderPass->setEnabled(false));
        EXPECT_EQ(StatusOK, renderPass->setRenderOnce(true));

        doWriteReadCycle();

        RenderPass* loadedRenderPass = this->getObjectForTesting<RenderPass>("a renderpass");

        EXPECT_STREQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(renderOrder, loadedRenderPass->getRenderOrder());
        EXPECT_FALSE(loadedRenderPass->isEnabled());
        EXPECT_TRUE(loadedRenderPass->isRenderOnce());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteARenderPassWithACamera)
    {
        RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");

        Camera* camera = this->m_scene.createRemoteCamera("camera");
        renderPass->setCamera(*camera);

        doWriteReadCycle();

        RenderPass* loadedRenderPass = this->getObjectForTesting<RenderPass>("a renderpass");
        const Camera* loadedCamera = this->getObjectForTesting<Camera>("camera");

        EXPECT_STREQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(loadedCamera, loadedRenderPass->getCamera());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteARenderPassWhichHasRenderGroups)
    {
        RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");
        RenderGroup* groupA = this->m_scene.createRenderGroup("groupA");
        RenderGroup* groupB = this->m_scene.createRenderGroup("groupB");
        renderPass->addRenderGroup(*groupA, 1);
        renderPass->addRenderGroup(*groupB, 2);

        doWriteReadCycle();

        RenderPass* loadedRenderPass = this->getObjectForTesting<RenderPass>("a renderpass");
        const RenderGroup* loadedMeshA = this->getObjectForTesting<RenderGroup>("groupA");
        const RenderGroup* loadedMeshB = this->getObjectForTesting<RenderGroup>("groupB");

        EXPECT_STREQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(2u, loadedRenderPass->impl.getAllRenderGroups().size());
        EXPECT_EQ(&loadedMeshA->impl, loadedRenderPass->impl.getAllRenderGroups()[0]);
        EXPECT_EQ(&loadedMeshB->impl, loadedRenderPass->impl.getAllRenderGroups()[1]);

        const ramses_internal::RenderPass& internalRP = m_sceneLoaded->impl.getIScene().getRenderPass(renderPass->impl.getRenderPassHandle());
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(groupA->impl.getRenderGroupHandle(), internalRP));
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(groupB->impl.getRenderGroupHandle(), internalRP));
        EXPECT_EQ(groupA->impl.getRenderGroupHandle(), ramses_internal::RenderGroupUtils::FindRenderGroupEntry(groupA->impl.getRenderGroupHandle(), internalRP)->renderGroup);
        EXPECT_EQ(groupB->impl.getRenderGroupHandle(), ramses_internal::RenderGroupUtils::FindRenderGroupEntry(groupB->impl.getRenderGroupHandle(), internalRP)->renderGroup);
        EXPECT_EQ(1, ramses_internal::RenderGroupUtils::FindRenderGroupEntry(groupA->impl.getRenderGroupHandle(), internalRP)->order);
        EXPECT_EQ(2, ramses_internal::RenderGroupUtils::FindRenderGroupEntry(groupB->impl.getRenderGroupHandle(), internalRP)->order);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteBlitPass)
    {
        const int32_t renderOrder = 1;
        const RenderBuffer* srcRenderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferType_Depth, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_WriteOnly, 0u, "src renderBuffer");
        const RenderBuffer* dstRenderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferType_Depth, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_WriteOnly, 0u, "dst renderBuffer");

        BlitPass* blitPass = this->m_scene.createBlitPass(*srcRenderBuffer, *dstRenderBuffer, "a blitpass");
        EXPECT_EQ(StatusOK, blitPass->setRenderOrder(renderOrder));
        EXPECT_EQ(StatusOK, blitPass->setEnabled(false));

        doWriteReadCycle();

        const BlitPass* loadedBlitPass = this->getObjectForTesting<BlitPass>("a blitpass");

        EXPECT_STREQ(blitPass->getName(), loadedBlitPass->getName());
        EXPECT_EQ(blitPass->getSceneObjectId(), loadedBlitPass->getSceneObjectId());
        EXPECT_EQ(renderOrder, loadedBlitPass->getRenderOrder());
        EXPECT_FALSE(loadedBlitPass->isEnabled());

        const ramses_internal::BlitPassHandle loadedBlitPassHandle = loadedBlitPass->impl.getBlitPassHandle();
        const ramses_internal::BlitPass& blitPassInternal = m_sceneLoaded->impl.getIScene().getBlitPass(loadedBlitPassHandle);
        EXPECT_EQ(renderOrder, blitPassInternal.renderOrder);
        EXPECT_FALSE(blitPassInternal.isEnabled);
        EXPECT_EQ(srcRenderBuffer->impl.getRenderBufferHandle(), blitPassInternal.sourceRenderBuffer);
        EXPECT_EQ(dstRenderBuffer->impl.getRenderBufferHandle(), blitPassInternal.destinationRenderBuffer);
        EXPECT_EQ(renderOrder, blitPassInternal.renderOrder);

        const ramses_internal::PixelRectangle& sourceRegion = blitPassInternal.sourceRegion;
        EXPECT_EQ(0u, sourceRegion.x);
        EXPECT_EQ(0u, sourceRegion.y);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(srcRenderBuffer->getWidth()), sourceRegion.width);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(srcRenderBuffer->getHeight()), sourceRegion.height);

        const ramses_internal::PixelRectangle& destinationRegion = blitPassInternal.destinationRegion;
        EXPECT_EQ(0u, destinationRegion.x);
        EXPECT_EQ(0u, destinationRegion.y);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(dstRenderBuffer->getWidth()), destinationRegion.width);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(dstRenderBuffer->getHeight()), destinationRegion.height);

        //client HL api
        {
            uint32_t sourceXOut;
            uint32_t sourceYOut;
            uint32_t destinationXOut;
            uint32_t destinationYOut;
            uint32_t widthOut;
            uint32_t heightOut;
            loadedBlitPass->getBlittingRegion(sourceXOut, sourceYOut, destinationXOut, destinationYOut, widthOut, heightOut);
            EXPECT_EQ(0u, sourceXOut);
            EXPECT_EQ(0u, sourceYOut);
            EXPECT_EQ(0u, destinationXOut);
            EXPECT_EQ(0u, destinationYOut);
            EXPECT_EQ(dstRenderBuffer->getWidth(), widthOut);
            EXPECT_EQ(dstRenderBuffer->getHeight(), heightOut);
        }

        EXPECT_EQ(srcRenderBuffer->impl.getRenderBufferHandle(), loadedBlitPass->getSourceRenderBuffer().impl.getRenderBufferHandle());
        EXPECT_EQ(srcRenderBuffer->impl.getObjectRegistryHandle(), loadedBlitPass->getSourceRenderBuffer().impl.getObjectRegistryHandle());

        EXPECT_EQ(dstRenderBuffer->impl.getRenderBufferHandle(), loadedBlitPass->getDestinationRenderBuffer().impl.getRenderBufferHandle());
        EXPECT_EQ(dstRenderBuffer->impl.getObjectRegistryHandle(), loadedBlitPass->getDestinationRenderBuffer().impl.getObjectRegistryHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWritePickableObject)
    {
        const EDataType geometryBufferDataType = EDataType::Vector3F;
        const ArrayBuffer* geometryBuffer = this->m_scene.createArrayBuffer(geometryBufferDataType, 3u, "geometryBuffer");

        const int32_t viewPort_x = 1;
        const int32_t viewPort_y = 2;
        const uint32_t viewPort_width = 200;
        const uint32_t viewPort_height = 300;
        PerspectiveCamera* pickableCamera = m_scene.createPerspectiveCamera("pickableCamera");
        pickableCamera->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
        pickableCamera->setViewport(viewPort_x, viewPort_y, viewPort_width, viewPort_height);

        const pickableObjectId_t id(2);
        PickableObject* pickableObject = this->m_scene.createPickableObject(*geometryBuffer, id, "PickableObject");
        EXPECT_EQ(StatusOK, pickableObject->setCamera(*pickableCamera));
        EXPECT_EQ(StatusOK, pickableObject->setEnabled(false));

        doWriteReadCycle();

        const PickableObject* loadedPickableObject = this->getObjectForTesting<PickableObject>("PickableObject");

        EXPECT_STREQ(pickableObject->getName(), loadedPickableObject->getName());
        EXPECT_EQ(pickableObject->getSceneObjectId(), loadedPickableObject->getSceneObjectId());
        EXPECT_EQ(id, loadedPickableObject->getPickableObjectId());
        EXPECT_FALSE(loadedPickableObject->isEnabled());
        EXPECT_EQ(this->getObjectForTesting<PerspectiveCamera>("pickableCamera"), loadedPickableObject->getCamera());
        EXPECT_EQ(this->getObjectForTesting<ArrayBuffer>("geometryBuffer"), &loadedPickableObject->getGeometryBuffer());

        const ramses_internal::PickableObjectHandle loadedPickableObjectPassHandle = loadedPickableObject->impl.getPickableObjectHandle();
        const ramses_internal::PickableObject& pickableObjectInternal = m_sceneLoaded->impl.getIScene().getPickableObject(loadedPickableObjectPassHandle);
        EXPECT_EQ(id.getValue(), pickableObjectInternal.id.getValue());
        EXPECT_FALSE(pickableObjectInternal.isEnabled);
        EXPECT_EQ(geometryBuffer->impl.getDataBufferHandle(), pickableObjectInternal.geometryHandle);
        EXPECT_EQ(pickableCamera->impl.getCameraHandle(), pickableObjectInternal.cameraHandle);

        EXPECT_EQ(geometryBuffer->impl.getDataBufferHandle(), loadedPickableObject->getGeometryBuffer().impl.getDataBufferHandle());
        EXPECT_EQ(geometryBuffer->impl.getObjectRegistryHandle(), loadedPickableObject->getGeometryBuffer().impl.getObjectRegistryHandle());

        EXPECT_EQ(pickableCamera->impl.getCameraHandle(), loadedPickableObject->getCamera()->impl.getCameraHandle());
        EXPECT_EQ(pickableCamera->impl.getObjectRegistryHandle(), loadedPickableObject->getCamera()->impl.getObjectRegistryHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteRenderBuffer)
    {
        RenderBuffer* renderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferType_Depth, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_WriteOnly, 4u, "a renderTarget");

        doWriteReadCycle();

        RenderBuffer* loadedRenderBuffer = this->getObjectForTesting<RenderBuffer>("a renderTarget");

        EXPECT_STREQ(renderBuffer->getName(), loadedRenderBuffer->getName());
        EXPECT_EQ(renderBuffer->getSceneObjectId(), loadedRenderBuffer->getSceneObjectId());
        EXPECT_EQ(renderBuffer->getWidth(), loadedRenderBuffer->getWidth());
        EXPECT_EQ(renderBuffer->getHeight(), loadedRenderBuffer->getHeight());
        EXPECT_EQ(renderBuffer->getBufferType(), loadedRenderBuffer->getBufferType());
        EXPECT_EQ(renderBuffer->getBufferFormat(), loadedRenderBuffer->getBufferFormat());
        EXPECT_EQ(renderBuffer->getAccessMode(), loadedRenderBuffer->getAccessMode());
        EXPECT_EQ(renderBuffer->getSampleCount(), loadedRenderBuffer->getSampleCount());

        EXPECT_EQ(renderBuffer->impl.getRenderBufferHandle(), loadedRenderBuffer->impl.getRenderBufferHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteARenderPassWithARenderTargetAndCamera)
    {
        RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");

        RenderBuffer* renderBuffer = this->m_scene.createRenderBuffer(23u, 42u, ERenderBufferType_Depth, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_ReadWrite, 0u, "a renderBuffer");
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(*renderBuffer);
        RenderTarget* renderTarget = this->m_scene.createRenderTarget(rtDesc, "target");

        OrthographicCamera* orthoCam = this->m_scene.createOrthographicCamera("camera");
        orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        orthoCam->setViewport(0, 0, 100, 200);

        renderPass->setCamera(*orthoCam);
        renderPass->setRenderTarget(renderTarget);

        doWriteReadCycle();

        const RenderPass* loadedRenderPass = this->getObjectForTesting<RenderPass>("a renderpass");
        const RenderTarget* loadedRenderTarget = this->getObjectForTesting<RenderTarget>("target");
        const OrthographicCamera* loadedCamera = this->getObjectForTesting<OrthographicCamera>("camera");

        EXPECT_EQ(loadedRenderTarget, loadedRenderPass->getRenderTarget());
        EXPECT_EQ(loadedCamera, loadedRenderPass->getCamera());
        EXPECT_EQ(StatusOK, loadedRenderTarget->validate());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteRenderTarget)
    {
        const RenderBuffer& rb = *m_scene.createRenderBuffer(16u, 8u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(rb);

        RenderTarget* renderTarget = this->m_scene.createRenderTarget(rtDesc, "a renderTarget");

        doWriteReadCycle();

        RenderTarget* loadedRenderTarget = this->getObjectForTesting<RenderTarget>("a renderTarget");

        EXPECT_STREQ(renderTarget->getName(), loadedRenderTarget->getName());
        EXPECT_EQ(renderTarget->getSceneObjectId(), loadedRenderTarget->getSceneObjectId());
        EXPECT_EQ(renderTarget->getWidth(), loadedRenderTarget->getWidth());
        EXPECT_EQ(renderTarget->getHeight(), loadedRenderTarget->getHeight());

        EXPECT_EQ(renderTarget->impl.getRenderTargetHandle(), loadedRenderTarget->impl.getRenderTargetHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteIndexDataBuffer)
    {
        ArrayBuffer& buffer = *m_scene.createArrayBuffer(EDataType::UInt32, 6u, "indexDB");
        buffer.updateData(3u, 2u, std::array<uint32_t, 2>{ {6, 7} }.data());

        doWriteReadCycle();

        const ArrayBuffer* loadedBuffer = this->getObjectForTesting<ArrayBuffer>("indexDB");

        EXPECT_STREQ(buffer.getName(), loadedBuffer->getName());
        EXPECT_EQ(buffer.impl.getDataBufferHandle(), loadedBuffer->impl.getDataBufferHandle());
        EXPECT_EQ(6 * sizeof(uint32_t), m_scene.impl.getIScene().getDataBuffer(loadedBuffer->impl.getDataBufferHandle()).data.size());
        EXPECT_EQ(5 * sizeof(uint32_t), m_scene.impl.getIScene().getDataBuffer(loadedBuffer->impl.getDataBufferHandle()).usedSize);

        const uint32_t* loadedDataBufferData = reinterpret_cast<const uint32_t*>(m_scene.impl.getIScene().getDataBuffer(loadedBuffer->impl.getDataBufferHandle()).data.data());
        EXPECT_EQ(6u, loadedDataBufferData[3]);
        EXPECT_EQ(7u, loadedDataBufferData[4]);

        EXPECT_EQ(6u, loadedBuffer->getMaximumNumberOfElements());
        EXPECT_EQ(5u, loadedBuffer->getUsedNumberOfElements());
        EXPECT_EQ(EDataType::UInt32, loadedBuffer->getDataType());
        std::array<uint32_t, 6> bufferDataOut;
        EXPECT_EQ(StatusOK, loadedBuffer->getData(bufferDataOut.data(), static_cast<uint32_t>(bufferDataOut.size() * sizeof(uint32_t))));
        EXPECT_EQ(6u, bufferDataOut[3]);
        EXPECT_EQ(7u, bufferDataOut[4]);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteTexture2DBuffer)
    {
        Texture2DBuffer& buffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2, "textureBuffer");
        buffer.updateData(0, 0, 0, 2, 2, std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data());
        buffer.updateData(1, 0, 0, 1, 1, std::array<uint32_t, 1>{ {78} }.data());

        doWriteReadCycle();

        const Texture2DBuffer* loadedBuffer = this->getObjectForTesting<Texture2DBuffer>("textureBuffer");

        EXPECT_STREQ(buffer.getName(), loadedBuffer->getName());
        EXPECT_EQ(buffer.getSceneObjectId(), loadedBuffer->getSceneObjectId());
        EXPECT_EQ(buffer.impl.getTextureBufferHandle(), loadedBuffer->impl.getTextureBufferHandle());

        //iscene
        const ramses_internal::TextureBuffer& loadedInternalBuffer = m_scene.impl.getIScene().getTextureBuffer(loadedBuffer->impl.getTextureBufferHandle());
        ASSERT_EQ(2u, loadedInternalBuffer.mipMaps.size());
        EXPECT_EQ(3u, loadedInternalBuffer.mipMaps[0].width);
        EXPECT_EQ(4u, loadedInternalBuffer.mipMaps[0].height);
        EXPECT_EQ(1u, loadedInternalBuffer.mipMaps[1].width);
        EXPECT_EQ(2u, loadedInternalBuffer.mipMaps[1].height);
        EXPECT_EQ(56u, ramses_internal::TextureBuffer::GetMipMapDataSizeInBytes(loadedInternalBuffer));
        EXPECT_EQ(ramses_internal::ETextureFormat::RGBA8, loadedInternalBuffer.textureFormat);

        const uint32_t* loadedBufferDataMip0 = reinterpret_cast<const uint32_t*>(loadedInternalBuffer.mipMaps[0].data.data());
        const uint32_t* loadedBufferDataMip1 = reinterpret_cast<const uint32_t*>(loadedInternalBuffer.mipMaps[1].data.data());
        EXPECT_EQ(12u, loadedBufferDataMip0[0]);
        EXPECT_EQ(23u, loadedBufferDataMip0[1]);
        EXPECT_EQ(34u, loadedBufferDataMip0[3 * 1 + 0]);
        EXPECT_EQ(56u, loadedBufferDataMip0[3 * 1 + 1]);
        EXPECT_EQ(78u, loadedBufferDataMip1[0]);

        //client API
        EXPECT_EQ(2u, loadedBuffer->getMipLevelCount());
        uint32_t mipLevelWidthOut;
        uint32_t mipLevelHeightOut;
        EXPECT_EQ(StatusOK, loadedBuffer->getMipLevelSize(0u, mipLevelWidthOut, mipLevelHeightOut));
        EXPECT_EQ(3u, mipLevelWidthOut);
        EXPECT_EQ(4u, mipLevelHeightOut);
        EXPECT_EQ(StatusOK, loadedBuffer->getMipLevelSize(1u, mipLevelWidthOut, mipLevelHeightOut));
        EXPECT_EQ(1u, mipLevelWidthOut);
        EXPECT_EQ(2u, mipLevelHeightOut);

        EXPECT_EQ(ETextureFormat::RGBA8, loadedBuffer->getTexelFormat());
        std::array<uint32_t, 12> bufferForMip0;
        std::array<uint32_t, 2> bufferForMip1;
        EXPECT_EQ(StatusOK, loadedBuffer->getMipLevelData(0u, reinterpret_cast<char*>(bufferForMip0.data()), static_cast<uint32_t>(bufferForMip0.size() * sizeof(uint32_t))));
        EXPECT_EQ(StatusOK, loadedBuffer->getMipLevelData(1u, reinterpret_cast<char*>(bufferForMip1.data()), static_cast<uint32_t>(bufferForMip1.size() * sizeof(uint32_t))));
        EXPECT_EQ(12u, bufferForMip0[0]);
        EXPECT_EQ(23u, bufferForMip0[1]);
        EXPECT_EQ(34u, bufferForMip0[3 * 1 + 0]);
        EXPECT_EQ(56u, bufferForMip0[3 * 1 + 1]);
        EXPECT_EQ(78u, bufferForMip1[0]);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAnAnimationSystem)
    {
        AnimationSystem* animSystem = this->m_scene.createAnimationSystem(ramses::EAnimationSystemFlags_Default, "anim system");

        doWriteReadCycle();

        AnimationSystem* animSystemLoaded = this->getObjectForTesting<AnimationSystem>("anim system");
        EXPECT_EQ(animSystem->impl.getAnimationSystemHandle(), animSystemLoaded->impl.getAnimationSystemHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAnAnimationSystemRealTime)
    {
        AnimationSystemRealTime* animSystem = this->m_scene.createRealTimeAnimationSystem(ramses::EAnimationSystemFlags_Default, "anim system");

        doWriteReadCycle();

        AnimationSystemRealTime* animSystemLoaded = this->getObjectForTesting<AnimationSystemRealTime>("anim system");
        EXPECT_EQ(animSystem->impl.getAnimationSystemHandle(), animSystemLoaded->impl.getAnimationSystemHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteANode)
    {
        //generic node cannot be created, therefore using group node
        Node* grandParent = this->m_scene.createNode("node1");
        Node* parent = this->m_scene.createNode("node2");
        Node* child = this->m_scene.createNode("node3");

        grandParent->addChild(*parent);
        child->setParent(*parent);

        child->setTranslation(1, 2, 3);
        child->setVisibility(EVisibilityMode::Invisible);
        child->setRotation(1, 2, 3);
        child->setScaling(1, 2, 3);

        doWriteReadCycle();

        Node* loadedGrandParent = this->getObjectForTesting<Node>("node1");
        Node* loadedParent = this->getObjectForTesting<Node>("node2");
        Node* loadedChild = this->getObjectForTesting<Node>("node3");
        ASSERT_TRUE(nullptr != loadedGrandParent);
        ASSERT_TRUE(nullptr != loadedParent);
        ASSERT_TRUE(nullptr != loadedChild);
        EXPECT_EQ(loadedParent, loadedChild->getParent());
        EXPECT_EQ(loadedGrandParent, loadedParent->getParent());
        EXPECT_EQ(loadedParent, loadedGrandParent->getChild(0u));
        EXPECT_EQ(1u, loadedGrandParent->getChildCount());
        EXPECT_EQ(1u, loadedParent->getChildCount());
        EXPECT_EQ(0u, loadedChild->getChildCount());

        float tx = 0;
        float ty = 0;
        float tz = 0;
        EXPECT_EQ(StatusOK, loadedChild->getTranslation(tx, ty, tz));
        EXPECT_FLOAT_EQ(1, tx);
        EXPECT_FLOAT_EQ(2, ty);
        EXPECT_FLOAT_EQ(3, tz);

        EXPECT_EQ(loadedChild->getVisibility(), EVisibilityMode::Invisible);

        float rx = 0;
        float ry = 0;
        float rz = 0;
        EXPECT_EQ(StatusOK, loadedChild->getRotation(rx, ry, rz));
        EXPECT_FLOAT_EQ(1, rx);
        EXPECT_FLOAT_EQ(2, ry);
        EXPECT_FLOAT_EQ(3, rz);

        float sx = 0;
        float sy = 0;
        float sz = 0;
        EXPECT_EQ(StatusOK, loadedChild->getScaling(sx, sy, sz));
        EXPECT_FLOAT_EQ(1, sx);
        EXPECT_FLOAT_EQ(2, sy);
        EXPECT_FLOAT_EQ(3, sz);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteANodeWithTranslation)
    {
        Node* node = this->m_scene.createNode("translate node 1");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setTranslation(1, 2, 3);
        node->addChild(*child);

        doWriteReadCycle();

        Node* loadedTranslateNode = getObjectForTesting<Node>("translate node 1");
        Node* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedTranslateNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, node->getChildCount());
        EXPECT_EQ(loadedChild, loadedTranslateNode->getChild(0u));
        float x = 0;
        float y = 0;
        float z = 0;
        EXPECT_EQ(StatusOK, node->getTranslation(x, y, z));
        EXPECT_FLOAT_EQ(1, x);
        EXPECT_FLOAT_EQ(2, y);
        EXPECT_FLOAT_EQ(3, z);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteANodeWithRotation)
    {
        Node* node = this->m_scene.createNode("rotate node 1");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setRotation(1, 2, 3);
        child->setParent(*node);
        doWriteReadCycle();

        Node* loadedRotateNode = getObjectForTesting<Node>("rotate node 1");
        Node* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedRotateNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, loadedRotateNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedRotateNode->getChild(0u));
        float x = 0;
        float y = 0;
        float z = 0;
        EXPECT_EQ(StatusOK, loadedRotateNode->getRotation(x, y, z));
        EXPECT_FLOAT_EQ(1, x);
        EXPECT_FLOAT_EQ(2, y);
        EXPECT_FLOAT_EQ(3, z);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteANodeWithScaling)
    {
        Node* node = this->m_scene.createNode("scale node");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setScaling(1, 2, 3);
        child->setParent(*node);
        doWriteReadCycle();

        Node* loadedScaleNode = getObjectForTesting<Node>("scale node");
        Node* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedScaleNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, loadedScaleNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedScaleNode->getChild(0u));
        float x = 0;
        float y = 0;
        float z = 0;
        EXPECT_EQ(StatusOK, loadedScaleNode->getScaling(x, y, z));
        EXPECT_FLOAT_EQ(1, x);
        EXPECT_FLOAT_EQ(2, y);
        EXPECT_FLOAT_EQ(3, z);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteATextureSampler)
    {
        const ETextureAddressMode wrapUMode = ETextureAddressMode_Mirror;
        const ETextureAddressMode wrapVMode = ETextureAddressMode_Repeat;
        const ETextureSamplingMethod minSamplingMethod = ETextureSamplingMethod_Linear_MipMapNearest;
        const ETextureSamplingMethod magSamplingMethod = ETextureSamplingMethod_Linear;
        const uint8_t data[4] = { 0u };
        const MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = this->m_scene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "texture");

        TextureSampler* sampler = this->m_scene.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, *texture, 8u, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        doWriteReadCycle();

        TextureSampler* loadedSampler = getObjectForTesting<TextureSampler>("sampler");
        ASSERT_TRUE(nullptr != loadedSampler);

        EXPECT_EQ(wrapUMode, loadedSampler->getWrapUMode());
        EXPECT_EQ(wrapVMode, loadedSampler->getWrapVMode());
        EXPECT_EQ(minSamplingMethod, loadedSampler->getMinSamplingMethod());
        EXPECT_EQ(magSamplingMethod, loadedSampler->getMagSamplingMethod());
        EXPECT_EQ(8u, loadedSampler->getAnisotropyLevel());
        EXPECT_EQ(texture->impl.getLowlevelResourceHash(), this->m_sceneLoaded->impl.getIScene().getTextureSampler(loadedSampler->impl.getTextureSamplerHandle()).textureResource);
        EXPECT_EQ(ERamsesObjectType_Texture2D, loadedSampler->impl.getTextureType());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteSceneId)
    {
        const sceneId_t sceneId = ramses::sceneId_t(1ULL << 63);
        ramses::Scene& mScene(*client.createScene(sceneId));

        const status_t status = mScene.saveToFile("someTempararyFile.ram", false);
        EXPECT_EQ(StatusOK, status);

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        EXPECT_EQ(sceneId, m_sceneLoaded->getSceneId());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, defaultsToLocalAndRemotePublicationMode)
    {
        const sceneId_t sceneId(81);
        EXPECT_EQ(StatusOK, client.createScene(sceneId)->saveToFile("someTempararyFile.ram", false));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode_LocalAndRemote, m_sceneLoaded->impl.getPublicationModeSetFromSceneConfig());
    }

    // TODO(tobias) add to store this option to file format as soon as changes are allowed
    TEST_F(ASceneAndAnimationSystemLoadedFromFile, DISABLED_respectsPublicationModeSetOnCreatingFileBeforeSave)
    {
        const sceneId_t sceneId(81);
        SceneConfig config;
        config.setPublicationMode(ramses::EScenePublicationMode_LocalOnly);
        EXPECT_EQ(StatusOK, client.createScene(sceneId, config)->saveToFile("someTempararyFile.ram", false));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode_LocalOnly, m_sceneLoaded->impl.getPublicationModeSetFromSceneConfig());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canOverwritePublicationModeForLoadedFiles)
    {
        const sceneId_t sceneId(80);
        EXPECT_EQ(StatusOK, client.createScene(sceneId)->saveToFile("someTempararyFile.ram", false));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram", true);
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode_LocalOnly, m_sceneLoaded->impl.getPublicationModeSetFromSceneConfig());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, reportsErrorWhenSavingSceneToFileWithInvalidFileName)
    {
        ramses::status_t status = m_scene.saveToFile("?XYZ:/dummyFile", false);
        EXPECT_NE(ramses::StatusOK, status);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, reportsErrorWhenSavingSceneToFileWithNoFileName)
    {
        ramses::status_t status = m_scene.saveToFile(nullptr, false);
        EXPECT_NE(ramses::StatusOK, status);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, overwritesExistingFileWhenSavingSceneToIt)
    {
        {
            ramses_internal::File existingFile("dummyFile.dat");
            existingFile.createFile();
        }

        ramses::status_t status = m_scene.saveToFile("dummyFile.dat", false);
        EXPECT_EQ(ramses::StatusOK, status);

        {
            ramses_internal::File fileShouldBeOverwritten("dummyFile.dat");
            EXPECT_TRUE(fileShouldBeOverwritten.open(ramses_internal::File::Mode::ReadOnly));
            ramses_internal::UInt fileSize = 0;
            EXPECT_TRUE(fileShouldBeOverwritten.getSizeInBytes(fileSize));
            EXPECT_NE(0u, fileSize);
        }

        EXPECT_TRUE(ramses_internal::File("dummyFile.dat").remove());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, doesNotLoadSceneFromFileWithInvalidFileName)
    {
        ramses::Scene* scene = client.loadSceneFromFile("?XYZ:/dummyFile");
        EXPECT_TRUE(nullptr == scene);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, doesNotLoadSceneFromFileWithoutFileName)
    {
        ramses::Scene* scene = client.loadSceneFromFile(nullptr);
        EXPECT_TRUE(nullptr == scene);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, doesNotLoadSceneFromUnexistingFile)
    {
        ramses::Scene* scene = client.loadSceneFromFile("ZEGETWTWAGTGSDGEg_thisfilename_in_this_directory_should_not_exist_DSAFDSFSTEZHDXHB");
        EXPECT_TRUE(nullptr == scene);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canHandleAllZeroFileOnSceneLoad)
    {
        const char* filename = "allzerofile.dat";
        {
            ramses_internal::File file(filename);
            EXPECT_TRUE(file.open(ramses_internal::File::Mode::WriteNew));
            std::vector<ramses_internal::Char> zerovector(4096);
            EXPECT_TRUE(file.write(&zerovector[0], zerovector.size()));
            file.close();
        }

        ramses::Scene* scene = client.loadSceneFromFile(filename);
        EXPECT_TRUE(scene == nullptr);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canHandleAllZeroFileOnResourceLoad)
    {
        const char* filename = "allzerofile.dat";
        {
            ramses_internal::File file(filename);
            EXPECT_TRUE(file.open(ramses_internal::File::Mode::WriteNew));
            std::vector<ramses_internal::Char> zerovector(4096);
            EXPECT_TRUE(file.write(&zerovector[0], zerovector.size()));
            file.close();
        }

        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).addResourceDataFile(filename));
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteSpline)
    {
        SplineStepBool* spline = this->animationSystem.createSplineStepBool("spline");

        doWriteReadCycle();

        const SplineStepBool* splineLoaded = this->getAnimationObjectForTesting<SplineStepBool>("spline");
        EXPECT_EQ(spline->getNumberOfKeys(), splineLoaded->getNumberOfKeys());
        EXPECT_EQ(spline->impl.getSplineHandle(), splineLoaded->impl.getSplineHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteMultipleSplinesOfSameType)
    {
        const SplineStepBool* spline1 = this->animationSystem.createSplineStepBool("spline1");
        const SplineStepBool* spline2 = this->animationSystem.createSplineStepBool("spline2");
        const SplineStepBool* spline3 = this->animationSystem.createSplineStepBool("spline3");

        doWriteReadCycle();

        const SplineStepBool* splineLoaded1 = this->getAnimationObjectForTesting<SplineStepBool>("spline1");
        const SplineStepBool* splineLoaded2 = this->getAnimationObjectForTesting<SplineStepBool>("spline2");
        const SplineStepBool* splineLoaded3 = this->getAnimationObjectForTesting<SplineStepBool>("spline3");
        EXPECT_EQ(spline1->getNumberOfKeys(), splineLoaded1->getNumberOfKeys());
        EXPECT_EQ(spline2->getNumberOfKeys(), splineLoaded2->getNumberOfKeys());
        EXPECT_EQ(spline3->getNumberOfKeys(), splineLoaded3->getNumberOfKeys());
        EXPECT_EQ(spline1->impl.getSplineHandle(), splineLoaded1->impl.getSplineHandle());
        EXPECT_EQ(spline2->impl.getSplineHandle(), splineLoaded2->impl.getSplineHandle());
        EXPECT_EQ(spline3->impl.getSplineHandle(), splineLoaded3->impl.getSplineHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAnimatedProperty)
    {
        Node* node = this->m_scene.createNode();
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X, "property");

        doWriteReadCycle();

        const AnimatedProperty* propLoaded = this->getAnimationObjectForTesting<AnimatedProperty>("property");
        EXPECT_EQ(prop->impl.getDataBindHandle(), propLoaded->impl.getDataBindHandle());
        EXPECT_EQ(prop->impl.getVectorComponent(), propLoaded->impl.getVectorComponent());
        EXPECT_EQ(prop->impl.getDataTypeID(), propLoaded->impl.getDataTypeID());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAnimation)
    {
        SplineStepFloat* spline = this->animationSystem.createSplineStepFloat("spline");
        Node* node = this->m_scene.createNode();
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X);
        Animation* anim = this->animationSystem.createAnimation(*prop, *spline, "animation");

        doWriteReadCycle();

        const Animation* animLoaded = this->getAnimationObjectForTesting<Animation>("animation");
        EXPECT_EQ(anim->impl.getAnimationInstanceHandle(), animLoaded->impl.getAnimationInstanceHandle());
        EXPECT_EQ(anim->impl.getAnimationHandle(), animLoaded->impl.getAnimationHandle());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteAnimationSequence)
    {
        SplineStepFloat* spline = this->animationSystem.createSplineStepFloat("spline");
        Node* node = this->m_scene.createNode();
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X);
        Animation* anim = this->animationSystem.createAnimation(*prop, *spline, "animation");
        AnimationSequence* seq = this->animationSystem.createAnimationSequence("seq");
        seq->addAnimation(*anim, 50u, 100u);
        seq->setPlaybackSpeed(2.0f);
        seq->setAnimationLooping(*anim, 50);
        seq->setAnimationAbsolute(*anim);

        doWriteReadCycle();

        const AnimationSequence* seqLoaded = this->getAnimationObjectForTesting<AnimationSequence>("seq");
        EXPECT_EQ(seq->getNumberOfAnimations(), seqLoaded->getNumberOfAnimations());
        EXPECT_EQ(seq->getPlaybackSpeed(), seqLoaded->getPlaybackSpeed());
        EXPECT_EQ(seq->isAnimationLooping(*anim), seqLoaded->isAnimationLooping(*anim));
        EXPECT_EQ(seq->getAnimationLoopDuration(*anim), seqLoaded->getAnimationLoopDuration(*anim));
        EXPECT_EQ(seq->isAnimationRelative(*anim), seqLoaded->isAnimationRelative(*anim));
        EXPECT_EQ(25u, seq->getAnimationStartTimeInSequence(*anim));
        EXPECT_EQ(50u, seq->getAnimationStopTimeInSequence(*anim));
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteTransformDataSlot)
    {
        Node* node = this->m_scene.createNode("node");

        EXPECT_EQ(StatusOK, this->m_scene.impl.createTransformationDataConsumer(*node, dataConsumerId_t(2u)));
        ASSERT_EQ(1u, this->m_scene.impl.getIScene().getDataSlotCount());

        ramses_internal::DataSlotHandle slotHandle(0u);
        EXPECT_TRUE(this->m_scene.impl.getIScene().isDataSlotAllocated(slotHandle));

        doWriteReadCycle();

        const Node* nodeLoaded = this->getObjectForTesting<Node>("node");
        ramses_internal::NodeHandle nodeHandle = nodeLoaded->impl.getNodeHandle();
        ASSERT_EQ(1u, this->m_sceneLoaded->impl.getIScene().getDataSlotCount());

        EXPECT_TRUE(this->m_sceneLoaded->impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(nodeHandle, this->m_sceneLoaded->impl.getIScene().getDataSlot(slotHandle).attachedNode);
        EXPECT_EQ(ramses_internal::DataSlotId(2u), this->m_sceneLoaded->impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_TransformationConsumer, this->m_sceneLoaded->impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataFloat)
    {
        float setValue = 5.0f;
        DataFloat* data = this->m_scene.createDataFloat("floatData");

        EXPECT_EQ(StatusOK, data->setValue(setValue));

        doWriteReadCycle();

        const DataFloat* loadedData = this->getObjectForTesting<DataFloat>("floatData");
        ASSERT_TRUE(loadedData);
        float loadedValue = 0.0f;
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue));
        EXPECT_EQ(setValue, loadedValue);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataVector2f)
    {
        float setValue[] = { 1.0f, 2.0f };
        DataVector2f* data = this->m_scene.createDataVector2f("vec2fData");

        EXPECT_EQ(StatusOK, data->setValue(setValue[0], setValue[1]));

        doWriteReadCycle();

        const DataVector2f* loadedData = this->getObjectForTesting<DataVector2f>("vec2fData");
        ASSERT_TRUE(loadedData);
        float loadedValue[] = { 0.0f, 0.0f };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue[0], loadedValue[1]));

        for (uint32_t i = 0; i < 2; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataVector3f)
    {
        float setValue[] = { 1.0f, 2.0f, 3.0f };
        DataVector3f* data = this->m_scene.createDataVector3f("vec3fData");

        EXPECT_EQ(StatusOK, data->setValue(setValue[0], setValue[1], setValue[2]));

        doWriteReadCycle();

        const DataVector3f* loadedData = this->getObjectForTesting<DataVector3f>("vec3fData");
        ASSERT_TRUE(loadedData);
        float loadedValue[] = { 0.0f, 0.0f, 0.0f };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue[0], loadedValue[1], loadedValue[2]));

        for (uint32_t i = 0; i < 3; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataVector4f)
    {
        float setValue[] = { 1.0f, 2.0f, 3.0f, 4.0f };
        DataVector4f* data = this->m_scene.createDataVector4f("vec4fData");

        EXPECT_EQ(StatusOK, data->setValue(setValue[0], setValue[1], setValue[2], setValue[3]));

        doWriteReadCycle();

        const DataVector4f* loadedData = this->getObjectForTesting<DataVector4f>("vec4fData");
        ASSERT_TRUE(loadedData);
        float loadedValue[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue[0], loadedValue[1], loadedValue[2], loadedValue[3]));
        for (uint32_t i = 0; i < 4; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataMatrix22f)
    {
        const float setValue[] = { 1.0f, 2.0f, 3.0f, 4.0f };
        DataMatrix22f* data = this->m_scene.createDataMatrix22f("matrix22Data");

        EXPECT_EQ(StatusOK, data->setValue(setValue));

        doWriteReadCycle();

        const DataMatrix22f* loadedData = this->getObjectForTesting<DataMatrix22f>("matrix22Data");
        ASSERT_TRUE(loadedData);
        float loadedValue[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue));
        for (uint32_t i = 0; i < 4u; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataMatrix33f)
    {
        const float setValue[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };
        DataMatrix33f* data = this->m_scene.createDataMatrix33f("matrix33Data");

        EXPECT_EQ(StatusOK, data->setValue(setValue));

        doWriteReadCycle();

        const DataMatrix33f* loadedData = this->getObjectForTesting<DataMatrix33f>("matrix33Data");
        ASSERT_TRUE(loadedData);
        float loadedValue[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue));
        for (uint32_t i = 0; i < 9u; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataMatrix44f)
    {
        float setValue[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };
        DataMatrix44f* data = this->m_scene.createDataMatrix44f("matrix44Data");

        EXPECT_EQ(StatusOK, data->setValue(setValue));

        doWriteReadCycle();

        const DataMatrix44f* loadedData = this->getObjectForTesting<DataMatrix44f>("matrix44Data");
        ASSERT_TRUE(loadedData);
        float loadedValue[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue));
        for (uint32_t i = 0; i < 16; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataInt32)
    {
        int32_t setValue = 5;
        DataInt32* data = this->m_scene.createDataInt32("int32Data");

        EXPECT_EQ(StatusOK, data->setValue(setValue));

        doWriteReadCycle();

        const DataInt32* loadedData = this->getObjectForTesting<DataInt32>("int32Data");
        ASSERT_TRUE(loadedData);
        int32_t loadedValue = 0;
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue));
        EXPECT_EQ(setValue, loadedValue);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataVector2i)
    {
        int32_t setValue[] = { 1, 2 };
        DataVector2i* data = this->m_scene.createDataVector2i("vec2iData");

        EXPECT_EQ(StatusOK, data->setValue(setValue[0], setValue[1]));

        doWriteReadCycle();

        const DataVector2i* loadedData = this->getObjectForTesting<DataVector2i>("vec2iData");
        ASSERT_TRUE(loadedData);
        int32_t loadedValue[] = { 0, 0 };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue[0], loadedValue[1]));

        for (uint32_t i = 0; i < 2; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataVector3i)
    {
        int32_t setValue[] = { 1, 2, 3 };
        DataVector3i* data = this->m_scene.createDataVector3i("vec3iData");

        EXPECT_EQ(StatusOK, data->setValue(setValue[0], setValue[1], setValue[2]));

        doWriteReadCycle();

        const DataVector3i* loadedData = this->getObjectForTesting<DataVector3i>("vec3iData");
        ASSERT_TRUE(loadedData);
        int32_t loadedValue[] = { 0, 0, 0 };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue[0], loadedValue[1], loadedValue[2]));

        for (uint32_t i = 0; i < 3; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteDataVector4i)
    {
        int32_t setValue[] = { 1, 2, 3, 4 };
        DataVector4i* data = this->m_scene.createDataVector4i("vec4iData");

        EXPECT_EQ(StatusOK, data->setValue(setValue[0], setValue[1], setValue[2], setValue[3]));

        doWriteReadCycle();

        const DataVector4i* loadedData = this->getObjectForTesting<DataVector4i>("vec4iData");
        ASSERT_TRUE(loadedData);
        int32_t loadedValue[] = { 0, 0, 0, 0 };
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue[0], loadedValue[1], loadedValue[2], loadedValue[3]));
        for (uint32_t i = 0; i < 4; i++)
        {
            EXPECT_EQ(setValue[i], loadedValue[i]);
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, canReadWriteSceneReferences)
    {
        constexpr ramses::sceneId_t referencedSceneId(123);
        auto sr1 = this->m_scene.createSceneReference(referencedSceneId, "scene ref");
        sr1->requestState(RendererSceneState::Ready);

        constexpr ramses::sceneId_t referencedSceneId2(555);
        auto sr2 = this->m_scene.createSceneReference(referencedSceneId2, "scene ref2");
        sr2->requestState(RendererSceneState::Rendered);

        doWriteReadCycle();

        const SceneReference* loadedSceneRef = this->getObjectForTesting<SceneReference>("scene ref");
        ASSERT_TRUE(loadedSceneRef);
        EXPECT_EQ(referencedSceneId, loadedSceneRef->getReferencedSceneId());
        EXPECT_EQ(ramses::RendererSceneState::Ready, loadedSceneRef->getRequestedState());

        const SceneReference* loadedSceneRef2 = this->getObjectForTesting<SceneReference>("scene ref2");
        ASSERT_TRUE(loadedSceneRef2);
        EXPECT_EQ(referencedSceneId2, loadedSceneRef2->getReferencedSceneId());
        EXPECT_EQ(ramses::RendererSceneState::Rendered, loadedSceneRef2->getRequestedState());
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, savesLLResourceOnlyOnceIfTwoHLResourcesReferToIt)
    {
        std::vector<uint16_t> inds(300);
        std::iota(inds.begin(), inds.end(), static_cast<uint16_t>(0u));
        this->m_scene.createArrayResource(EDataType::UInt16, 300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices");
        this->m_scene.createArrayResource(EDataType::UInt16, 300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices");
        this->m_scene.createArrayResource(EDataType::UInt16, 300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices");
        this->m_scene.createArrayResource(EDataType::UInt16, 300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices2");
        this->m_scene.createArrayResource(EDataType::UInt16, 300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices2");
        this->m_scene.createArrayResource(EDataType::UInt16, 300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices2");

        doWriteReadCycle();
        std::ifstream in("someTemporaryFile.ram", std::ifstream::ate | std::ifstream::binary);
        auto size = in.tellg();
        EXPECT_GT(1600, size) << "scene file size exceeds allowed max size. verify that LL resource is saved only once before adapting this number";
    }

    template <typename T>
    struct TestHelper
    {
        static T* create(ASceneAndAnimationSystemLoadedFromFileTemplated<T>* fixture, ramses::RamsesClient&, ramses::Scene&)
        {
            return &fixture->template createObject<T>("a node");
        }
    };

    TYPED_TEST_SUITE(ASceneAndAnimationSystemLoadedFromFileTemplated, NodeTypes);
    TYPED_TEST(ASceneAndAnimationSystemLoadedFromFileTemplated, canReadWriteAllNodes)
    {
        auto node = TestHelper<TypeParam>::create(this, this->client, this->m_scene);

        node->setVisibility(EVisibilityMode::Invisible);

        auto child = &this->template createObject<ramses::Node>("child");
        auto parent = &this->template createObject<ramses::Node>("parent");

        node->setTranslation(1, 2, 3);
        node->setRotation(4, 5, 6);
        node->setScaling(7, 8, 9);
        node->addChild(*child);
        node->setParent(*parent);

        this->m_scene.flush();

        this->doWriteReadCycle();

        const auto loadedSuperNode = this->template getObjectForTesting<TypeParam>("a node");
        const auto loadedChild = this->template getObjectForTesting<ramses::Node>("child");
        const auto loadedParent = this->template getObjectForTesting<ramses::Node>("parent");

        ASSERT_TRUE(nullptr != loadedSuperNode);
        ASSERT_TRUE(nullptr != loadedChild);
        ASSERT_TRUE(nullptr != loadedParent);

        ASSERT_EQ(1u, loadedSuperNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedSuperNode->getChild(0u));
        EXPECT_EQ(loadedParent, loadedSuperNode->getParent());
        float x = 0;
        float y = 0;
        float z = 0;
        EXPECT_EQ(StatusOK, loadedSuperNode->getTranslation(x, y, z));
        EXPECT_FLOAT_EQ(1, x);
        EXPECT_FLOAT_EQ(2, y);
        EXPECT_FLOAT_EQ(3, z);
        EXPECT_EQ(StatusOK, loadedSuperNode->getRotation(x, y, z));
        EXPECT_FLOAT_EQ(4, x);
        EXPECT_FLOAT_EQ(5, y);
        EXPECT_FLOAT_EQ(6, z);
        EXPECT_EQ(StatusOK, loadedSuperNode->getScaling(x, y, z));
        EXPECT_FLOAT_EQ(7, x);
        EXPECT_FLOAT_EQ(8, y);
        EXPECT_FLOAT_EQ(9, z);

        EXPECT_EQ(loadedSuperNode->getVisibility(), EVisibilityMode::Invisible);
    }


    TEST_F(ASceneAndAnimationSystemLoadedFromFile, compressedFileIsSmallerThanUncompressedWhenUsingSaveSceneToFile)
    {
        Scene* scene = client.createScene(sceneId_t(1));
        const std::vector<uint16_t> data(1000u, 0u);
        EXPECT_TRUE(scene->createArrayResource(EDataType::UInt16, static_cast<uint32_t>(data.size()), data.data()));

        EXPECT_EQ(StatusOK, scene->saveToFile("testscene.ramscene", false));

        ramses_internal::File file("testscene.ramscene");
        EXPECT_TRUE(file.exists());
        ramses_internal::UInt uncompressedFileSize = 0;
        EXPECT_TRUE(file.getSizeInBytes(uncompressedFileSize));

        EXPECT_EQ(StatusOK, scene->saveToFile("testscene.ramscene", true));

        ramses_internal::File file2("testscene.ramscene");
        EXPECT_TRUE(file2.exists());
        ramses_internal::UInt compressedFileSize = 0;
        EXPECT_TRUE(file2.getSizeInBytes(compressedFileSize));

        EXPECT_GT(uncompressedFileSize, compressedFileSize);
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, savedFilesAreConsistent)
    {
        for (ramses_internal::String name : { "ts1.ramscene", "ts2.ramscene", "ts3.ramscene", "ts4.ramscene", "ts5.ramscene", "ts6.ramscene" })
        {
            EXPECT_EQ(StatusOK, this->m_scene.saveToFile(name.c_str(), false));
        }

        for (ramses_internal::String name : { "ts2.ramscene", "ts3.ramscene", "ts4.ramscene", "ts5.ramscene", "ts6.ramscene" })
        {
            EXPECT_TRUE(ClientTestUtils::CompareBinaryFiles("ts1.ramscene", name.c_str()));
        }
    }

    TEST_F(ASceneAndAnimationSystemLoadedFromFile, closesSceneFileAndLowLevelResourceWhenDestroyed)
    {
        const status_t status = m_scene.saveToFile("someTemporaryFile.ram", false);
        EXPECT_EQ(StatusOK, status);

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        EXPECT_TRUE(m_clientForLoading.impl.getClientApplication().hasResourceFile("someTemporaryFile.ram"));
        m_clientForLoading.destroy(*m_sceneLoaded);

        // scene gets destroyed asynchronously, so we can't just test after the destroy
        // unfortunately there is no callback, but I don't want to skip the test
        // => wait for it to happen in finite time, we don't test for performance here
        uint32_t ticks = 60000u;
        for (; ticks > 0; --ticks)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (!m_clientForLoading.impl.getClientApplication().hasResourceFile("someTemporaryFile.ram"))
                break;
        }
        EXPECT_GT(ticks, 0u);
    }

}
