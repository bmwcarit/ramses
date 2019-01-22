//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_FRUSTUM_H
#define RAMSES_CITYMODEL_FRUSTUM_H

#include "BoundingBox.h"
#include "Math3d/Vector3.h"
#include "Math3d/Matrix44f.h"

/// Class for representing the viewing frustum and do overlap checks against bounding boxes.
class Frustum
{
public:
    /// Initializes the frustum.
    /** @param fovy Field of view in Y direction in degrees.
     *  @param aspect Aspect ratio of the frustum (width/height).
     *  @param far Distance to the far plane. */
    void init(float fovy, float aspect, float far);

    /// Positions/transforms the viewing frustum to the camera position/orientation.
    /** @param m Matrix, that transforms the viewing frustum from it's local coordinate system
     *  to world coordinates.*/
    void transform(const ramses_internal::Matrix44f& m);

    /// Checks whether the viewing frustum overlaps with a bounding box or not.
    /** @param clipMask Masks the clipping planes of the frustum, which have to be checked.
     *  Each of the five planes is represented by a bit in the mask. If the box is inside of a plane
     *  the corresponding bit is masked out, since further child boxes are then also inside.
     *  @param bb The bounding box the be checked.
     *  @return "true", when the bounding box overlaps with the frustum and "false" otherwise. */
    bool overlap(uint32_t& clipMask, const BoundingBox& bb) const;

    /// A clipping plane of the frustum.
    class Plane
    {
    public:
        /// Initializes the plane.
        /** @param p Point on the plane in transformed world coordinates (Frustum::mP[..])
         *  @param normalUntransformed Normal of the plane in local coordinates. */
        void init(ramses_internal::Vector3* p, const ramses_internal::Vector3& normalUntransformed);

        /// Checks whether a bounding box is complete outside of the clipping plane.
        /** @param bb The bounding box to be checked.
         *  @return "true", when the box is outside and "false" otherwise. */
        bool isCompleteOutside(const BoundingBox& bb) const;

        /// Checks whether a bounding box is complete outside of the clipping plane.
        /** @param bb The bounding box to be checked.
         *  @return "true", when the box is outside and "false" otherwise. */
        bool isCompleteInside(const BoundingBox& bb) const;

        /// Transforms the clipping plane from the frustum local coordinate system to world coordinates.
        /** Transforms the normal and computes the near and far point index.
         *  @param m The matrix to transform to world coordinates. */
        void transform(const ramses_internal::Matrix44f& m);

    private:
        /// Pointer to the transformed plane point in world coordinates.
        ramses_internal::Vector3* m_p;

        /// Normal in local/untransformed coordinates.
        ramses_internal::Vector3 m_normalUntransformed;

        /// Transformed normal in world coordinates.
        ramses_internal::Vector3 m_normal;

        /// Index of the bounding box point (used with CBoundingBox::getPoint()), that is nearest to this plane.
        uint32_t m_nearestPointIndex;

        /// Index of the bounding box point (used with CBoundingBox::getPoint()), that is farest to this plane.
        uint32_t m_farthestPointIndex;
    };

private:
    /// Initialies the i-th clipping plane, by three points on the plane.
    /** @param i Index of the plane (0-4).
     *  @param a Index of the first point in the mPoint array.
     *  @param b Index of the second point in the mPoint array.
     *  @param c Index of the third point in the mPoint array. */
    void setPlane(uint32_t i, uint32_t a, uint32_t b, uint32_t c);

    /// Point array for the clipping planes in untransformed local coordinates.
    ramses_internal::Vector3 m_pointUntransformed[5];

    /// Transformed clipping plane point.
    ramses_internal::Vector3 m_point[5];

    /// The five clipping planes.
    Plane m_plane[5];

    /// Bounding box of the five plane points mPlane[].
    BoundingBox m_boundingBox;
};

#endif
