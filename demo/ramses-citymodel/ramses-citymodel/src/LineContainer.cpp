//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/LineContainer.h"

#include "ramses-utils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/AttributeInput.h"

LineContainer::LineContainer(const ramses_internal::Vector3& color,
                             const ramses_internal::Vector3& colorInvisible,
                             float                           width,
                             const ramses_internal::Vector3& up,
                             ramses::Scene&                  scene,
                             ramses::RamsesClient&           client,
                             ramses::RenderGroup&            renderGroup)
    : m_color(color)
    , m_colorInvisible(colorInvisible)
    , m_width(width)
    , m_up(up)
    , m_ramsesClient(client)
    , m_ramsesScene(scene)
    , m_renderGroup(renderGroup)
{
    float pixelToWorldScale = 10.0f;
    m_scale.set(1.0f / pixelToWorldScale, m_width / 2.0f, pixelToWorldScale * m_width / 2.0f);

    createEffects();
    createGeometry();
}

void LineContainer::activateRenderingOfInvisibleParts(const ramses_internal::Vector3& color)
{
    m_colorInvisible  = color;
    m_renderInvisible = true;
}

void LineContainer::addPolyline(const std::vector<ramses_internal::Vector3>& points, ECapType startCap, ECapType endCap)
{
    ramses_internal::UInt pointCount = points.size();
    if (pointCount >= 2)
    {
        ramses_internal::UInt    j     = 1;
        ramses_internal::UInt    jLast = pointCount - 1;
        ramses_internal::Vector3 p2    = points[0];
        while (j <= jLast)
        {
            ramses_internal::Vector3 p1 = p2;
            p2                          = points[j];
            bool sCap                   = (j > 1) || startCap != ECapType_Flat;
            bool eCap                   = (j == jLast) && endCap != ECapType_Flat;
            addSegment(p1, p2, sCap, eCap);
            j++;
        }
    }
}

void LineContainer::addSegment(const ramses_internal::Vector3& p1,
                               const ramses_internal::Vector3& p2,
                               bool                            startCap,
                               bool                            endCap)
{
    ramses_internal::Vector3 y      = p2 - p1;
    float                    length = y.length();
    y                               = y.normalize();

    ramses_internal::Vector3 x = y.cross(m_up);

    ramses_internal::Vector3 diff[4];

    diff[0] = x;
    diff[1] = -x;
    if (startCap)
    {
        diff[0] -= y;
        diff[1] -= y;
    }
    diff[2] = -x;
    diff[3] = x;
    if (endCap)
    {
        diff[2] += y;
        diff[3] += y;
    }

    float halfLength = length / 2.0f / m_scale.z;

    float startCapValue = startCap ? -1.0f : 0.0f;
    float endCapValue   = endCap ? 1.0f : 0.0f;

    // 1
    ramses_internal::Vector3 temp = p1 + diff[1] * m_scale.z;
    m_vertex.push_back(temp.x);
    m_vertex.push_back(temp.y);
    m_vertex.push_back(temp.z);
    m_param.push_back(-halfLength);
    m_param.push_back(startCapValue);
    m_param.push_back(1.0);

    // 0
    temp = p1 + diff[0] * m_scale.z;
    m_vertex.push_back(temp.x);
    m_vertex.push_back(temp.y);
    m_vertex.push_back(temp.z);

    m_param.push_back(-halfLength);
    m_param.push_back(startCapValue);
    m_param.push_back(-1.0);

    // 2
    temp = p2 + diff[2] * m_scale.z;
    m_vertex.push_back(temp.x);
    m_vertex.push_back(temp.y);
    m_vertex.push_back(temp.z);

    m_param.push_back(halfLength);
    m_param.push_back(endCapValue);
    m_param.push_back(1.0f);

    // 0
    temp = p1 + diff[0] * m_scale.z;
    m_vertex.push_back(temp.x);
    m_vertex.push_back(temp.y);
    m_vertex.push_back(temp.z);

    m_param.push_back(-halfLength);
    m_param.push_back(startCapValue);
    m_param.push_back(-1.0);

    // 3
    temp = p2 + diff[3] * m_scale.z;
    m_vertex.push_back(temp.x);
    m_vertex.push_back(temp.y);
    m_vertex.push_back(temp.z);

    m_param.push_back(halfLength);
    m_param.push_back(endCapValue);
    m_param.push_back(-1.0f);

    // 2
    temp = p2 + diff[2] * m_scale.z;
    m_vertex.push_back(temp.x);
    m_vertex.push_back(temp.y);
    m_vertex.push_back(temp.z);

    m_param.push_back(halfLength);
    m_param.push_back(endCapValue);
    m_param.push_back(1.0f);
}

void LineContainer::createGeometry()
{
    ramses::Node* groupNode = m_ramsesScene.createNode();

    /// First pass: Fill alpha buffer

    {
        m_meshFillAlpha = m_ramsesScene.createMeshNode("route pass 1");
        m_meshFillAlpha->setParent(*groupNode);
        ramses::Appearance* appearance = m_ramsesScene.createAppearance(*m_effectLine, "route pass 1 appearance");
        m_geometryFillAlpha            = m_ramsesScene.createGeometryBinding(*m_effectLine);

        m_meshFillAlpha->setAppearance(*appearance);
        m_meshFillAlpha->setGeometryBinding(*m_geometryFillAlpha);

        m_renderGroup.addMeshNode(*m_meshFillAlpha, 1);

        appearance->setBlendingFactors(
            ramses::EBlendFactor_Zero, ramses::EBlendFactor_Zero, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
        appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Min);
        appearance->setColorWriteMask(false, false, false, true);
        appearance->setDepthFunction(ramses::EDepthFunc_Disabled);
        appearance->setDepthWrite(false);
    }

    // Second pass : Render visible part

    {
        m_meshRenderVisible = m_ramsesScene.createMeshNode("route pass 2");
        m_meshRenderVisible->setParent(*groupNode);
        ramses::Appearance* appearance =
            m_ramsesScene.createAppearance(*m_effectLineFillVisible, "route pass2 appearance");
        m_geometryRenderVisible = m_ramsesScene.createGeometryBinding(*m_effectLineFillVisible);

        m_meshRenderVisible->setAppearance(*appearance);
        m_meshRenderVisible->setGeometryBinding(*m_geometryRenderVisible);

        m_renderGroup.addMeshNode(*m_meshRenderVisible, 2);

        appearance->setBlendingFactors(ramses::EBlendFactor_OneMinusDstAlpha,
                                       ramses::EBlendFactor_DstAlpha,
                                       ramses::EBlendFactor_One,
                                       ramses::EBlendFactor_Zero);
        appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        appearance->setDepthFunction(ramses::EDepthFunc_LessEqual);
        appearance->setDepthWrite(false);
    }


    /// Third pass: Render invisible part

    {
        m_meshRenderInvisible = m_ramsesScene.createMeshNode("route pass 3");
        m_meshRenderInvisible->setParent(*groupNode);
        ramses::Appearance* appearance =
            m_ramsesScene.createAppearance(*m_effectLineFillInvisible, "route pass 3 appearance");
        m_geometryRenderInvisible = m_ramsesScene.createGeometryBinding(*m_effectLineFillInvisible);

        m_meshRenderInvisible->setAppearance(*appearance);
        m_meshRenderInvisible->setGeometryBinding(*m_geometryRenderInvisible);

        m_renderGroup.addMeshNode(*m_meshRenderInvisible, 3);

        appearance->setBlendingFactors(ramses::EBlendFactor_OneMinusDstAlpha,
                                       ramses::EBlendFactor_DstAlpha,
                                       ramses::EBlendFactor_One,
                                       ramses::EBlendFactor_Zero);
        appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        appearance->setDepthFunction(ramses::EDepthFunc_GreaterEqual);
        appearance->setDepthWrite(false);
    }
}

void LineContainer::updateGeometry()
{
    ramses_internal::UInt32 n = static_cast<ramses_internal::UInt32>(m_vertex.size() / 3);

    if (n >= 2)
    {
        if (m_positions)
        {
            m_ramsesClient.destroy(*m_positions);
        }
        if (m_normals)
        {
            m_ramsesClient.destroy(*m_normals);
        }

        m_positions = m_ramsesClient.createConstVector3fArray(n, &m_vertex[0]);
        m_normals   = m_ramsesClient.createConstVector3fArray(n, &m_param[0]);

        /// First pass: Fill alpha buffer

        {
            ramses::AttributeInput positionsInput;
            ramses::AttributeInput normalsInput;
            m_effectLine->findAttributeInput("a_position", positionsInput);
            m_effectLine->findAttributeInput("a_normal", normalsInput);

            if (positionsInput.isValid())
            {
                m_geometryFillAlpha->setInputBuffer(positionsInput, *m_positions);
            }
            if (normalsInput.isValid())
            {
                m_geometryFillAlpha->setInputBuffer(normalsInput, *m_normals);
            }
        }

        // Second pass : Render visible part

        {
            ramses::AttributeInput positionsInput;
            m_effectLineFillVisible->findAttributeInput("a_position", positionsInput);
            if (positionsInput.isValid())
            {
                m_geometryRenderVisible->setInputBuffer(positionsInput, *m_positions);
            }
        }


        /// Third pass: Render invisible part

        {
            ramses::AttributeInput positionsInput;
            m_effectLineFillInvisible->findAttributeInput("a_position", positionsInput);

            if (positionsInput.isValid())
            {
                m_geometryRenderInvisible->setInputBuffer(positionsInput, *m_positions);
            }
        }
    }
    m_meshFillAlpha->setIndexCount(n);
    m_meshRenderVisible->setIndexCount(n);
    m_meshRenderInvisible->setIndexCount(n);

    m_vertex.clear();
    m_param.clear();
}

void LineContainer::createEffects()
{
    ramses::EffectDescription effectDesc;

    effectDesc.setUniformSemantic("u_mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    effectDesc.setVertexShaderFromFile("res/ramses-citymodel-line.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-citymodel-line.frag");

    m_effectLine = m_ramsesClient.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effectline");

    effectDesc.setVertexShaderFromFile("res/ramses-citymodel-lineVisible.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-citymodel-lineVisible.frag");
    m_effectLineFillVisible =
        m_ramsesClient.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect line visible");

    effectDesc.setVertexShaderFromFile("res/ramses-citymodel-lineInvisible.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-citymodel-lineInvisible.frag");
    m_effectLineFillInvisible =
        m_ramsesClient.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect line invisible");
}
