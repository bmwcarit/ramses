//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextStressTests.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-renderer-api/RamsesRenderer.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-client-api/ResourceIterator.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-text-api/TextCache.h"
#include "ramses-utils.h"
#include "RamsesRendererUtils.h"

#include "RamsesRendererImpl.h"
#include "Utils/RamsesLogger.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "MemoryLogger.h"
#include "TestRandom.h"

using namespace ramses_internal;

TextStressTestBase::TextStressTestBase(int32_t argc, const char* argv[], const ramses_internal::String& name)
    : StressTest(argc, argv, name)
{
}

TextStressTestBase::~TextStressTestBase()
{
    m_textCache.reset(nullptr);
    destroyScene();
}

void TextStressTestBase::createSceneElements()
{
    assert(m_client);
    assert(m_clientScene);

    // create appearance to be used for text
    ramses::EffectDescription effectDesc;
    effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic_TextPositions);
    effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);
    effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic_TextTexture);
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    effectDesc.setVertexShaderFromFile("res/ramses-text-stress-tests.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-text-stress-tests.frag");
    m_textEffect = m_client->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "simpleTextShader");

    // create text font and style
    m_font = m_fontRegistry.createFreetype2Font("res/ramses-test-client-Roboto-Regular.ttf");
    m_fontInst = m_fontRegistry.createFreetype2FontInstance(m_font, 24u);
    m_textCache.reset(new ramses::TextCache(*m_clientScene, m_fontRegistry, 1024u, 1024u));
    m_glyphMetrics = m_textCache->getPositionedGlyphs(U"Left aligned text", m_fontInst);
}


void TextStressTestBase::destroyScene()
{
    if (m_fontInst != ramses::InvalidFontInstanceId)
    {
        m_fontRegistry.deleteFontInstance(m_fontInst);
        m_fontInst = ramses::InvalidFontInstanceId;
    }
    if (m_font != ramses::InvalidFontId)
    {
        m_fontRegistry.deleteFont(m_font);
        m_font = ramses::InvalidFontId;
    }
    if (m_client && m_textEffect)
    {
        m_client->destroy(*m_textEffect);
        m_textEffect = nullptr;
    }

    StressTest::destroyScene();
}

void TextStressTestBase::doUpdateLoop()
{
    ramses::RamsesRendererUtils::DoOneLoop(m_renderer->impl.getRenderer(), ramses_internal::ELoopMode_UpdateOnly, std::chrono::microseconds{ 0u });
}

ramses::TextLineId TextStressTestBase::createText()
{
    return m_textCache->createTextLine(m_glyphMetrics, *m_textEffect);
}

CreateDestroyTextNodes::CreateDestroyTextNodes(int32_t argc, const char* argv[])
: TextStressTestBase(argc, argv, "ETest_createDestroyTextNodes")
{}

CreateDestroyTextNodes::~CreateDestroyTextNodes()
{}

int32_t CreateDestroyTextNodes::run_loop()
{
    int32_t returnValue = 0;

    const ramses::TextLineId textLine = createText();
    ramses::MeshNode* const textMesh = m_textCache->getTextLine(textLine)->meshNode;
    m_renderGroup->addMeshNode(*textMesh);
    m_clientScene->flush();
    doUpdateLoop();

    if (m_firstRun)
    {
        if (!isScreenshotSimilar("TextStressTestSample", 1.0f))
        {
            LOG_ERROR(CONTEXT_TEST, "Test runTestCreateDestroyTextNodes: screenshot test failed");
            returnValue = -1;
        }
    }

    m_renderGroup->removeMeshNode(*textMesh);
    m_textCache->deleteTextLine(textLine);
    m_clientScene->flush();
    doUpdateLoop();

    return returnValue;
}

AddRemoveTextNodes::AddRemoveTextNodes(int32_t argc, const char* argv[])
: TextStressTestBase(argc, argv, "ETest_addRemoveTextNodes")
{}

int32_t AddRemoveTextNodes::run_loop()
{
    int32_t returnValue = 0;

    const ramses::TextLineId textLine = createText();
    ramses::MeshNode *const textMesh = m_textCache->getTextLine(textLine)->meshNode;
    m_renderGroup->addMeshNode(*textMesh);
    m_clientScene->flush();
    doUpdateLoop();

    if (m_firstRun)
    {
        if (!isScreenshotSimilar("TextStressTestSample", 1.0f))
        {
            LOG_ERROR(CONTEXT_TEST, "Test runAddRemoveTextNodes: screenshot test failed");
            returnValue = -1;
        }
    }

    m_renderGroup->removeMeshNode(*textMesh);
    m_clientScene->flush();
    doUpdateLoop();

    m_textCache->deleteTextLine(textLine);
    m_clientScene->flush();
    doUpdateLoop();

    return returnValue;
}
