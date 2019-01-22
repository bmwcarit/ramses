//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Frustum.h"
#include "ramses-citymodel/BoundingBox.h"

void Frustum::init(float fovy, float aspect, float f)
{
    float tangent = ramses_internal::PlatformMath::Tan(ramses_internal::PlatformMath::Deg2Rad(fovy / 2.0f));

    float dy = f * tangent;
    float dx = dy * aspect;

    m_pointUntransformed[0].set(0.0, 0.0, 0.0);
    m_pointUntransformed[1].set(-dx, -dy, -f);
    m_pointUntransformed[2].set(dx, -dy, -f);
    m_pointUntransformed[3].set(dx, dy, -f);
    m_pointUntransformed[4].set(-dx, dy, -f);

    setPlane(0, 0, 1, 2);
    setPlane(1, 0, 2, 3);
    setPlane(2, 0, 3, 4);
    setPlane(3, 0, 4, 1);
    setPlane(4, 1, 3, 2);
}

void Frustum::setPlane(uint32_t i, uint32_t a, uint32_t b, uint32_t c)
{
    ramses_internal::Vector3 u = m_pointUntransformed[b] - m_pointUntransformed[a];
    ramses_internal::Vector3 v = m_pointUntransformed[c] - m_pointUntransformed[a];

    ramses_internal::Vector3 n = u.cross(v);

    m_plane[i].init(&m_point[a], n.normalize());
}

void Frustum::transform(const ramses_internal::Matrix44f& m)
{
    m_boundingBox.reset();

    for (ramses_internal::UInt32 i = 0; i < 5; i++)
    {
        const ramses_internal::Vector4 p = m * ramses_internal::Vector4(m_pointUntransformed[i]);
        m_point[i].set(p.x, p.y, p.z);

        m_boundingBox.add(m_point[i]);

        m_plane[i].transform(m);
    }
}

bool Frustum::overlap(uint32_t& clipMask, const BoundingBox& bb) const
{
    if (clipMask == 0)
    {
        /// Parent of bb is complete inside -> so bb is also complete inside.
        return true;
    }

    for (ramses_internal::UInt32 i = 0; i < 5; i++)
    {
        ramses_internal::UInt32 m = 1 << i;
        if (clipMask & m)
        {
            if (m_plane[i].isCompleteOutside(bb))
            {
                return false;
            }

            if (m_plane[i].isCompleteInside(bb))
            {
                clipMask &= ~m;
            }
        }
    }

    return bb.checkOverlap(m_boundingBox);
}

void Frustum::Plane::init(ramses_internal::Vector3* p, const ramses_internal::Vector3& normalUntransformed)
{
    m_p                   = p;
    m_normalUntransformed = normalUntransformed;
}

void Frustum::Plane::transform(const ramses_internal::Matrix44f& m)
{
    ramses_internal::Vector4 n(m_normalUntransformed);
    n.w                         = 0.0;
    ramses_internal::Vector4 nt = m * n;
    m_normal.set(nt.x, nt.y, nt.z);
    m_normal = m_normal.normalize();

    m_nearestPointIndex = 0;
    if (nt.x < 0.0)
    {
        m_nearestPointIndex |= 1;
    }

    if (nt.y < 0.0)
    {
        m_nearestPointIndex |= 2;
    }

    if (nt.z < 0.0)
    {
        m_nearestPointIndex |= 4;
    }
    m_farthestPointIndex = m_nearestPointIndex ^ 7;
}

bool Frustum::Plane::isCompleteOutside(const BoundingBox& bb) const
{
    ramses_internal::Vector3 a = bb.getPoint(m_nearestPointIndex);
    float                    d = m_normal.dot(a - *m_p);
    return d >= 0;
}

bool Frustum::Plane::isCompleteInside(const BoundingBox& bb) const
{
    ramses_internal::Vector3 a = bb.getPoint(m_farthestPointIndex);
    float                    d = m_normal.dot(a - *m_p);
    return d <= 0;
}
