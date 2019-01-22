//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/NamingManager.h"
#include "ramses-citymodel/Name2D.h"

#include "ramses-utils.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RamsesClient.h"

NamingManager::NamingManager(ramses::RamsesClient& client,
                             ramses::Scene&        scene,
                             ramses::Node*         parent,
                             float                 width,
                             float                 height,
                             float                 fovy,
                             bool                  useFading)
    : m_halfSize(width * 0.5f, height * 0.5f)
    , m_scene(scene)
    , m_useFading(useFading)
    , m_fontInstance(ramses::InvalidFontInstanceId)
    , m_textCache(scene, m_fontRegistry, 1024u, 1024u)
{
    m_camera            = createOrthoCamera(width, height, fovy);
    ramses::Node* scale = m_scene.createNode();
    scale->addChild(*m_camera);
    scale->setScaling(0.25f, 0.25f, 0.25f);
    parent->addChild(*scale);

    m_permanentNames = m_scene.createNode();
    m_camera->addChild(*m_permanentNames);

    /// Create effect:
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-citymodel-text-effect.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-citymodel-text-effect.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic_TextTexture);
    effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic_TextPositions);
    effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);

    m_effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "text effect");

    ramses::FontId font = m_fontRegistry.createFreetype2Font("res/ramses-demoLib-SourceSansPro-Regular.ttf");
    m_fontInstance      = m_fontRegistry.createFreetype2FontInstance(font, 48);
}

ramses::Node* NamingManager::createOrthoCamera(float width, float height, float fovy)
{
    ramses::Node* camera  = m_scene.createNode();
    float         tangent = ramses_internal::PlatformMath::Tan(ramses_internal::PlatformMath::Deg2Rad(fovy / 2.0f));
    float         d       = height / (2.0f * tangent);
    camera->setTranslation(-width / 2.0f, -height / 2.0f, -d);
    return camera;
}

void NamingManager::add(Name2D* name)
{
    m_names.push_back(name);
    name->useFading(m_useFading);

    name->createGeometry(m_textCache, m_fontInstance, *m_effect);
    name->setNamingManager(this);

    m_camera->addChild(*name->topNode());

    addToInvisibleNames(name);
}

void NamingManager::addPermanent(Name* name)
{
    name->createGeometry(m_textCache, m_fontInstance, *m_effect);
    ramses::Node* node = name->topNode();
    if (node)
    {
        m_permanentNames->addChild(*node);
    }
}

void NamingManager::removePermanent(Name* name)
{
    ramses::Node* node = name->topNode();
    if (node)
    {
        m_permanentNames->removeChild(*node);
    }
}

void NamingManager::update(const ramses_internal::Matrix44f& viewProjectionMatrix)
{
    m_colliderNames.clear();

    ramses_internal::Int32 maxInvisibleNamesToCheck = 5;

    m_viewProjectionMatrix = viewProjectionMatrix;

    std::vector<Name2D*> names;

    for (std::list<Name2D*>::iterator i = m_visibleNames.begin(); i != m_visibleNames.end(); ++i)
    {
        names.push_back(*i);
    }

    ramses_internal::Int32 n = static_cast<ramses_internal::UInt32>(m_invisibleNames.size());
    // We only restrict number of invisible names to check when we use fading (i.e. we are in
    // animation mode). When we use a static frame (!m_useFading) we want get all street names
    // visible for the first frame and don't alter the number of names to check.
    if (m_useFading && n > maxInvisibleNamesToCheck)
    {
        n = maxInvisibleNamesToCheck;
    }

    while (n > 0)
    {
        Name2D* name = m_invisibleNames.front();
        m_invisibleNames.pop_front();
        m_invisibleNames.push_back(name);
        names.push_back(name);
        n--;
    }

    n = static_cast<ramses_internal::UInt32>(names.size());
    for (ramses_internal::Int32 i = 0; i < n; i++)
    {
        names[i]->update();
    }
}

bool NamingManager::projectCheckXY(const ramses_internal::Vector3& p, ramses_internal::Vector2& p2d, bool& xyVisible)
{
    ramses_internal::Vector4 pt = m_viewProjectionMatrix * ramses_internal::Vector4(p);

    float w = pt.w;

    // Clip:
    if (pt.z <= -w)
    {
        return false;
    }

    float oow = 1.0f / w;

    p2d.set((pt.x * oow + 1.0f) * m_halfSize.x, (pt.y * oow + 1.0f) * m_halfSize.y);

    // Clip with left, right, bottom, top plane:
    xyVisible = pt.x >= -w && pt.x <= w && pt.y >= -w && pt.y <= w;

    return true;
}

bool NamingManager::project(const ramses_internal::Vector3& p, ramses_internal::Vector2& p2d)
{
    ramses_internal::Vector4 pt = m_viewProjectionMatrix * ramses_internal::Vector4(p);

    float w = pt.w;

    // Clip:
    if (w <= 0.0)
    {
        return false;
    }

    float oow = 1.0f / w;

    p2d.set((pt.x * oow + 1.0f) * m_halfSize.x, (pt.y * oow + 1.0f) * m_halfSize.y);
    return true;
}

void NamingManager::addToVisibleNames(Name2D* name)
{
    m_invisibleNames.remove(name);
    m_visibleNames.push_back(name);
    ramses::Node* node = name->topNode();
    if (node)
    {
        name->setVisibility(true);
    }
}

void NamingManager::addToInvisibleNames(Name2D* name)
{
    m_visibleNames.remove(name);
    m_invisibleNames.push_back(name);
    ramses::Node* node = name->topNode();
    if (node)
    {
        name->setVisibility(false);
    }
}

bool NamingManager::checkCollision(Name2D* name)
{
    ramses_internal::Int32 n = static_cast<ramses_internal::Int32>(m_colliderNames.size());
    for (ramses_internal::Int32 i = 0; i < n; i++)
    {
        if (m_colliderNames[i]->checkCollision(name))
        {
            return true;
        }
    }
    return false;
}

void NamingManager::addToColliders(Name2D* name)
{
    m_colliderNames.push_back(name);
}
