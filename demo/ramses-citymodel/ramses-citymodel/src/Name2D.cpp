//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Name2D.h"
#include "ramses-citymodel/NamingManager.h"

Name2D::Name2D(const std::string&              text,
               ramses::Scene&                  scene,
               ramses::RamsesClient&           client,
               ramses::RenderGroup&            renderGroup,
               const ramses_internal::Vector3& p0,
               const ramses_internal::Vector3& p1)
    : Name(text, scene, client, renderGroup)
    , m_p0(p0)
    , m_p1(p1)
    , m_namingManager(0)
    , m_alpha(0.0)
    , m_validForRendering(false)
    , m_destinationVisible(false)
    , m_useFading(true)
{
}

void Name2D::setNamingManager(NamingManager* namingRoot)
{
    m_namingManager = namingRoot;
}

void Name2D::useFading(bool doFading)
{
    m_useFading = doFading;
}

void Name2D::update()
{
    updatePosition();
    checkCollision();
    updateAlpha();
}

void Name2D::updatePosition()
{
    ramses_internal::Vector2 p0;
    ramses_internal::Vector2 p1;
    bool                     xyVisible = false;
    bool                     visibleP0 = m_namingManager->projectCheckXY(m_p0, p0, xyVisible);
    bool                     visibleP1 = m_namingManager->project(m_p1, p1);

    m_validForRendering = visibleP0 && visibleP1;

    m_destinationVisible = xyVisible && m_validForRendering;

    ramses_internal::Vector2 x = p1 - p0;

    if (std::abs(x.length()) > std::numeric_limits<float>::epsilon())
    {
        x = x.normalize();

        if (x.x < 0.0)
        {
            // Flip names that are wrong way round
            x = -x;
        }

        float angle = ramses_internal::PlatformMath::Rad2Deg(atan2(x.y, x.x));

        if (m_translateNode != nullptr && m_rotateNode != nullptr && (m_alpha > 0.0 || m_destinationVisible))
        {
            m_translateNode->setTranslation(p0.x, p0.y, 0.0);
            m_rotateNode->setRotation(0.0, 0.0, -angle);
        }
    }

    computeBounding();
}

void Name2D::checkCollision()
{
    if (m_destinationVisible)
    {
        if (m_namingManager->checkCollision(this))
        {
            m_destinationVisible = false;
        }
        else
        {
            m_namingManager->addToColliders(this);
        }
    }
}

void Name2D::updateAlpha()
{
    const float alphaDelta = 1.0f / (60.0f * 1.0f);
    if (m_destinationVisible)
    {
        if (m_alpha < 1.0)
        {
            if (m_alpha == 0.0)
            {
                m_namingManager->addToVisibleNames(this);
            }
            if (m_useFading)
            {
                m_alpha += alphaDelta;
                if (m_alpha > 1.0)
                {
                    m_alpha = 1.0;
                }
            }
            else
            {
                m_alpha = 1.0;
            }
        }
    }
    else
    {
        if (m_alpha > 0.0)
        {
            if (m_useFading)
            {
                m_alpha -= alphaDelta;
                if (m_alpha <= 0.0)
                {
                    m_alpha = 0.0;
                    m_namingManager->addToInvisibleNames(this);
                }
            }
            else
            {
                m_alpha = 0.0;
                m_namingManager->addToInvisibleNames(this);
            }
        }
    }
    setAlpha(m_alpha);
}

void Name2D::computeBounding()
{
    if (m_visibilityNode != nullptr)
    {
        // convert mMin/MaxBounding from object space of mMesh to common world space

        ramses_internal::Matrix44f toWorldSpace = Name2D::GetWorldSpaceMatrixOfNode(*m_visibilityNode);

        m_boundingPoints[0] = toWorldSpace * ramses_internal::Vector4(m_minBounding.x, m_minBounding.y, 0.0, 1.0);
        m_boundingPoints[1] = toWorldSpace * ramses_internal::Vector4(m_minBounding.x, m_maxBounding.y, 0.0, 1.0);
        m_boundingPoints[2] = toWorldSpace * ramses_internal::Vector4(m_maxBounding.x, m_minBounding.y, 0.0, 1.0);
        m_boundingPoints[3] = toWorldSpace * ramses_internal::Vector4(m_maxBounding.x, m_maxBounding.y, 0.0, 1.0);
    }
}

bool Name2D::checkCollision(Name2D* name)
{
    return checkCollisionThisToOther(name) && name->checkCollisionThisToOther(this);
}

bool Name2D::checkCollisionThisToOther(Name2D* name)
{
    if (m_visibilityNode != nullptr)
    {
        // convert bounding points of other to object space of this, allows comparison to mMin/MaxBounding of this (are
        // in object space of this)
        ramses_internal::Matrix44f toObjectSpace = GetObjectSpaceMatrixOfNode(*m_visibilityNode);

        ramses_internal::Vector4 p[4];
        for (ramses_internal::UInt32 i = 0; i < 4; i++)
        {
            p[i] = toObjectSpace * name->boundingPoint(i);
        }

        // Check left:
        if (p[0].x < m_minBounding.x && p[1].x < m_minBounding.x && p[2].x < m_minBounding.x &&
            p[3].x < m_minBounding.x)
        {
            return false;
        }

        // Check right:
        if (p[0].x > m_maxBounding.x && p[1].x > m_maxBounding.x && p[2].x > m_maxBounding.x &&
            p[3].x > m_maxBounding.x)
        {
            return false;
        }

        // Check bottom:
        if (p[0].y < m_minBounding.y && p[1].y < m_minBounding.y && p[2].y < m_minBounding.y &&
            p[3].y < m_minBounding.y)
        {
            return false;
        }

        // Check top:
        if (p[0].y > m_maxBounding.y && p[1].y > m_maxBounding.y && p[2].y > m_maxBounding.y &&
            p[3].y > m_maxBounding.y)
        {
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

ramses_internal::Matrix44f Name2D::GetWorldSpaceMatrixOfNode(ramses::Node& node)
{
    float mat44[16] = {0.f};
    node.getModelMatrix(mat44);
    return ramses_internal::Matrix44f(mat44).transpose();
}

ramses_internal::Matrix44f Name2D::GetObjectSpaceMatrixOfNode(ramses::Node& node)
{
    float mat44[16] = {0.f};
    node.getInverseModelMatrix(mat44);
    return ramses_internal::Matrix44f(mat44).transpose();
}

const ramses_internal::Vector4& Name2D::boundingPoint(ramses_internal::UInt32 i) const
{
    return m_boundingPoints[i];
}
