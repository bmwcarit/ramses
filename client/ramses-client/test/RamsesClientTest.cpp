//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include <gtest/gtest.h>
#include "ramses-client-api/EffectDescription.h"
#include "ramses-utils.h"

#include "RamsesClientImpl.h"
#include "SceneConfigImpl.h"
#include "Utils/File.h"
#include "gtest/gtest-typed-test.h"
#include "RamsesObjectTestTypes.h"
#include "EffectImpl.h"
#include "ClientTestUtils.h"
#include "ClientApplicationLogic.h"
#include "SceneAPI/SceneId.h"
#include "Collections/String.h"

namespace ramses
{
    class ALocalRamsesClient : public LocalTestClient, public ::testing::Test
    {
    public:
        ALocalRamsesClient()
        {
            effectDescriptionEmpty.setVertexShader("void main(void) {gl_Position=vec4(0);}");
            effectDescriptionEmpty.setFragmentShader("void main(void) {gl_FragColor=vec4(0);}");
        }

    protected:
        ramses::EffectDescription effectDescriptionEmpty;
    };

    class MockRamsh : public ramses_internal::Ramsh
    {
    public:
        MOCK_METHOD1(execute, bool(ramses_internal::RamshInput& input));
    };

    class ARamsesClient : public ::testing::Test
    {
    public:
        ARamsesClient()
            : m_client(m_framework.impl, "client")
        {
        }

    protected:
        RamsesFramework m_framework;
        RamsesClientImpl m_client;
    };

    TEST_F(ARamsesClient, canBeValidated)
    {
        EXPECT_EQ(StatusOK, m_client.validate(0));
    }

    TEST_F(ARamsesClient, failsValidationWhenContainsSceneWithInvalidRenderPass)
    {
        ramses::Scene* scene = m_client.createScene(1, ramses::SceneConfigImpl(), "");
        ASSERT_TRUE(NULL != scene);

        RenderPass* passWithoutCamera = scene->createRenderPass();
        ASSERT_TRUE(NULL != passWithoutCamera);

        EXPECT_NE(StatusOK, scene->validate());
        EXPECT_NE(StatusOK, m_client.validate(0));
    }

    TEST_F(ARamsesClient, failsValidationWhenContainsSceneWithInvalidCamera)
    {
        ramses::Scene* scene = m_client.createScene(1, ramses::SceneConfigImpl(), "");
        ASSERT_TRUE(NULL != scene);

        Camera* cameraWithoutValidValues = scene->createPerspectiveCamera();
        ASSERT_TRUE(NULL != cameraWithoutValidValues);

        EXPECT_NE(StatusOK, scene->validate());
        EXPECT_NE(StatusOK, m_client.validate(0));
    }

    TEST_F(ARamsesClient, isNotConnectedInitially)
    {
        EXPECT_FALSE(m_framework.isConnected());
    }

    TEST_F(ARamsesClient, connectLifeCycleOK)
    {
        EXPECT_EQ(StatusOK, m_framework.connect());
        EXPECT_TRUE(m_framework.isConnected());

        EXPECT_EQ(StatusOK, m_framework.disconnect());
    }

    TEST_F(ARamsesClient, reportsErrorWhenConnectingSecondTime)
    {
        m_framework.connect();
        EXPECT_NE(StatusOK, m_framework.connect());
        EXPECT_TRUE(m_framework.isConnected());
    }

    TEST_F(ARamsesClient, disconnectsViaApplication)
    {
        m_framework.connect();
        EXPECT_EQ(StatusOK, m_framework.disconnect());
    }

    TEST_F(ARamsesClient, reportsErrorWhenDisconnectingSecondTime)
    {
        m_framework.connect();
        EXPECT_EQ(StatusOK, m_framework.disconnect());
        EXPECT_NE(StatusOK, m_framework.disconnect());
    }

    // Not really useful and behavior is not defined, but should not crash at least
    TEST_F(ARamsesClient, canLiveParallelToAnotherClientUsingTheSameFramework)
    {
        RamsesClientImpl secondClient(m_framework.impl, "client");

        m_framework.connect();
        EXPECT_TRUE(m_framework.isConnected());
    }

    TEST_F(ALocalRamsesClient, canGetResourceByID)
    {
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL != effectFixture);

        const resourceId_t resourceID = effectFixture->getResourceId();
        ramses::Resource* resource = client.findResourceById(resourceID);
        ASSERT_TRUE(NULL != resource);

        const ramses::Effect* effectFound = RamsesUtils::TryConvert<ramses::Effect>(*resource);
        ASSERT_TRUE(NULL != effectFound);

        ASSERT_TRUE(effectFound == effectFixture);

        const resourceId_t nonExistEffectId = { 0, 0 };
        ASSERT_TRUE(NULL == client.findResourceById(nonExistEffectId));
    }

    TEST_F(ALocalRamsesClient, returnsNULLWhenResourceWithIDCannotBeFound)
    {
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL != effectFixture);

        const resourceId_t nonExistEffectId = { 0, 0 };
        ASSERT_TRUE(NULL == client.findResourceById(nonExistEffectId));
    }

    TEST_F(ALocalRamsesClient, returnsNULLWhenTryingToFindDeletedResource)
    {
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL != effectFixture);

        const resourceId_t resourceID = effectFixture->getResourceId();
        ramses::Resource* resource = client.findResourceById(resourceID);
        ASSERT_TRUE(NULL != resource);

        client.destroy(*effectFixture);

        ramses::Resource* resourceFound = client.findResourceById(resourceID);
        ASSERT_TRUE(NULL == resourceFound);
    }

    TEST_F(ALocalRamsesClient, disconnectingWhenDisconnectedGivesError)
    {
        EXPECT_EQ(ramses::StatusOK, framework.connect());
        EXPECT_EQ(ramses::StatusOK, framework.disconnect());

        EXPECT_NE(ramses::StatusOK, framework.disconnect());
    }

    TEST_F(ALocalRamsesClient, connectingTwiceGivesError)
    {
        EXPECT_EQ(ramses::StatusOK, framework.connect());
        EXPECT_NE(ramses::StatusOK, framework.connect());
    }

    TEST_F(ALocalRamsesClient, requestNonexistantStatusMessage)
    {
        const char* msg = client.getStatusMessage(0xFFFFFFFF);
        EXPECT_TRUE(NULL != msg);
    }

    TEST(RamsesClient, canCreateClientWithNULLNameAndCmdLineArguments)
    {
        ramses::RamsesFramework framework(0, static_cast<const char**>(NULL));
        ramses::RamsesClient client(NULL, framework);
        EXPECT_FALSE(framework.isConnected());
    }

    TEST(RamsesClient, canCreateClientWithoutCmdLineArguments)
    {
        ramses::RamsesFramework framework;
        ramses::RamsesClient client(NULL, framework);
        EXPECT_FALSE(framework.isConnected());
    }

    TEST_F(ALocalRamsesClient, createsSceneWithGivenId)
    {
        const sceneId_t sceneId = 33u;
        ramses::Scene* scene = client.createScene(sceneId);
        ASSERT_TRUE(scene != nullptr);
        EXPECT_EQ(sceneId, scene->getSceneId());
    }

    TEST_F(ALocalRamsesClient, simpleSceneGetsDestroyedProperlyWithoutExplicitDestroyCall)
    {
        ramses::Scene* scene = client.createScene(1u);
        EXPECT_TRUE(scene != NULL);
        ramses::Node* node = scene->createNode("node");
        EXPECT_TRUE(node != NULL);
    }

    // effect from string: valid uses
    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_withName)
    {
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL != effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_withDefines)
    {
        effectDescriptionEmpty.addCompilerDefine("float dummy;");
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL != effectFixture);
    }

    // effect from string: invalid uses
    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_invalidVertexShader)
    {
        effectDescriptionEmpty.setVertexShader("void main(void) {dsadsadasd}");
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_emptyVertexShader)
    {
        effectDescriptionEmpty.setVertexShader("");
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_invalidFragmentShader)
    {
        effectDescriptionEmpty.setFragmentShader("void main(void) {dsadsadasd}");
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_emptyFragmentShader)
    {
        effectDescriptionEmpty.setFragmentShader("");
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_invalidDefines)
    {
        effectDescriptionEmpty.addCompilerDefine("thisisinvalidstuff\n8fd7f9ds");
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_withInputSemantics)
    {
        effectDescriptionEmpty.setVertexShader(
            "uniform mat4 someMatrix;"
            "void main(void)"
            "{"
            "gl_Position = someMatrix * vec4(1.0);"
            "}");
        effectDescriptionEmpty.setUniformSemantic("someMatrix", ramses::EEffectUniformSemantic_ProjectionMatrix);
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL != effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSLString_withInputSemanticsOfWrongType)
    {
        effectDescriptionEmpty.setVertexShader(
            "uniform mat2 someMatrix;"
            "void main(void)"
            "{"
            "gl_Position = someMatrix * vec4(1.0);"
            "}");
        effectDescriptionEmpty.setUniformSemantic("someMatrix", ramses::EEffectUniformSemantic_ProjectionMatrix);
        const ramses::Effect* effectFixture = client.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_FALSE(NULL != effectFixture);
    }

    // effect from file: valid uses
    TEST_F(ALocalRamsesClient, createEffectFromGLSL_withName)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-client-test_minimalShader.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag");
        const ramses::Effect* effectFixture = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL != effectFixture);
    }


    // effect from file: invalid uses
    TEST_F(ALocalRamsesClient, createEffectFromGLSL_nonExistantVertexShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag");
        const ramses::Effect* effectFixture = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSL_nonExistantFragmentShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-client-test_minimalShader.frag");
        effectDesc.setFragmentShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        const ramses::Effect* effectFixture = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSL_NULLVertexShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("");
        effectDesc.setFragmentShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        const ramses::Effect* effectFixture = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, createEffectFromGLSL_NULLFragmentShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("");
        const ramses::Effect* effectFixture = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);
    }

    TEST_F(ALocalRamsesClient, verifyHLAPILogCanHandleNullPtrReturnWhenEnabled)
    {
        ramses_internal::ELogLevel oldLogLevel = ramses_internal::CONTEXT_HLAPI_CLIENT.getLogLevel();
        ramses_internal::CONTEXT_HLAPI_CLIENT.setLogLevel(ramses_internal::ELogLevel::Trace);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("");
        const ramses::Effect* effectFixture = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(NULL == effectFixture);

        ramses_internal::CONTEXT_HLAPI_CLIENT.setLogLevel(oldLogLevel);
    }

    TEST_F(ALocalRamsesClient, requestValidStatusMessage)
    {
        LocalTestClient otherClient;
        Scene& scene = otherClient.createObject<Scene>();

        ramses::status_t status = client.destroy(scene);
        EXPECT_NE(ramses::StatusOK, status);
        const char* msg = client.getStatusMessage(status);
        EXPECT_TRUE(NULL != msg);
    }

    TEST_F(ALocalRamsesClient, isUnpublishedOnDestroy)
    {
        const sceneId_t sceneId = 45u;
        ramses::Scene* scene = client.createScene(sceneId);

        using ramses_internal::SceneInfoVector;
        using ramses_internal::SceneInfo;
        ramses_internal::SceneId internalSceneId(sceneId);

        EXPECT_CALL(sceneActionsCollector, handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(internalSceneId, ramses_internal::String(scene->getName()))), testing::_, testing::_));
        EXPECT_CALL(sceneActionsCollector, handleInitializeScene(testing::_, testing::_));
        EXPECT_CALL(sceneActionsCollector, handleSceneActionList_rvr(ramses_internal::SceneId(sceneId), testing::_, testing::_, testing::_));
        EXPECT_CALL(sceneActionsCollector, handleScenesBecameUnavailable(SceneInfoVector(1, SceneInfo(internalSceneId)), testing::_));

        scene->publish(ramses::EScenePublicationMode_LocalOnly);
        scene->flush();
        EXPECT_TRUE(client.impl.getClientApplication().isScenePublished(internalSceneId));

        client.destroy(*scene);
        EXPECT_FALSE(client.impl.getClientApplication().isScenePublished(internalSceneId));
    }

}
