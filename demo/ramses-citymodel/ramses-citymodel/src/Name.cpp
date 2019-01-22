//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Name.h"

#include "PlatformAbstraction/PlatformStringUtils.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector2.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-text-api/UtfUtils.h"
#include "ramses-text-api/LayoutUtils.h"
#include "ramses-text-api/TextCache.h"

#include "vector"

Name::Name(const std::string&    text,
           ramses::Scene&        scene,
           ramses::RamsesClient& client,
           ramses::RenderGroup&  renderGroup)
    : m_text(text)
    , m_visibilityNode(0)
    , m_currentAlpha(0.0f)
    , m_ramsesClient(client)
    , m_scene(scene)
    , m_renderGroup(renderGroup)
    , m_appearance(0)
{
}

Name::~Name()
{
    if (m_textCache != nullptr && m_textId != ramses::InvalidTextLineId)
    {
        m_textCache->deleteTextLine(m_textId);
    }

    if (m_visibilityNode != nullptr)
    {
        m_scene.destroy(*m_visibilityNode);
    }
    if (m_translateNode != nullptr)
    {
        m_scene.destroy(*m_translateNode);
    }
    if (m_rotateNode != nullptr)
    {
        m_scene.destroy(*m_rotateNode);
    }
}


void Name::createGeometry(ramses::TextCache& textCache, ramses::FontInstanceId fontInstance, ramses::Effect& effect)
{
    m_textCache           = &textCache;
    std::u32string string = ramses::UtfUtils::ConvertUtf8ToUtf32(m_text);

    const ramses::GlyphMetricsVector positionedGlyphs = textCache.getPositionedGlyphs(string, fontInstance);
    m_textId                                          = textCache.createTextLine(positionedGlyphs, effect);
    ramses::TextLine* textLine                        = textCache.getTextLine(m_textId);

    m_appearance = textLine->meshNode->getAppearance();
    m_appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    m_appearance->setBlendingFactors(ramses::EBlendFactor_SrcAlpha,
                                     ramses::EBlendFactor_OneMinusSrcAlpha,
                                     ramses::EBlendFactor_SrcAlpha,
                                     ramses::EBlendFactor_OneMinusSrcAlpha);
    m_appearance->setDepthFunction(ramses::EDepthFunc_Disabled);

    ramses::LayoutUtils::StringBoundingBox bbox =
        ramses::LayoutUtils::GetBoundingBoxForString(positionedGlyphs.begin(), positionedGlyphs.end());
    const float x = static_cast<float>(bbox.offsetX);
    const float y = static_cast<float>(bbox.offsetY);
    const float w = static_cast<float>(bbox.width);
    const float h = static_cast<float>(bbox.height);

    const float dx = -ramses_internal::PlatformMath::Floor(x + w * 0.5f);
    const float dy = -ramses_internal::PlatformMath::Floor(y + h * 0.5f);

    m_minBounding.set(x, y);
    m_maxBounding.set(x + w, y + h);

    m_translateNode      = m_scene.createNode();
    m_rotateNode         = m_scene.createNode();
    m_visibilityNode     = m_scene.createNode();
    m_textLocalTranslate = m_scene.createNode();

    m_textLocalTranslate->setTranslation(dx, dy, 0);

    m_renderGroup.addMeshNode(*textLine->meshNode, 1000 - 25);

    m_translateNode->addChild(*m_rotateNode);
    m_rotateNode->addChild(*m_textLocalTranslate);
    m_textLocalTranslate->addChild(*m_visibilityNode);
    m_visibilityNode->addChild(*textLine->meshNode);


    effect.findUniformInput("u_a", m_alphaInput);
    setAlpha(1.0);
}

void Name::setAlpha(float v)
{
    if (m_currentAlpha != v)
    {
        m_appearance->setInputValueFloat(m_alphaInput, v);
        m_currentAlpha = v;
    }
}

const ramses_internal::Vector2& Name::minBounding() const
{
    return m_minBounding;
}

const ramses_internal::Vector2& Name::maxBounding() const
{
    return m_maxBounding;
}

ramses::Node* Name::topNode() const
{
    return m_translateNode;
}

void Name::setVisibility(bool value)
{
    m_visibilityNode->setVisibility(value);
}
