//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_BOUNDINGBOX_H
#define RAMSES_CITYMODEL_BOUNDINGBOX_H

#include "Math3d/Vector3.h"

/// Bounding box class.
class BoundingBox
{
public:
    /// Constructor for an empty box.
    BoundingBox();

    /// Constructor.
    /** @param min The minimum corner in X,Y and Z direction.
     *  @param max The maximum corner in X,Y and Z direction. */
    BoundingBox(const ramses_internal::Vector3& min, const ramses_internal::Vector3& max);

    /// Sets the minimum and maximum corner.
    /** @param min The minimum corner in X,Y and Z direction.
     *  @param max The maximum corner in X,Y and Z direction. */
    void set(const ramses_internal::Vector3& min, const ramses_internal::Vector3& max);

    /// Extends the bounding box to contain a point.
    /** @param p Point to be added. */
    void add(const ramses_internal::Vector3& p);

    /// Extends the bounding box by a second box.
    /** @param box The second box. */
    void add(const BoundingBox& box);

    /// Intersects this bounding box with a second one.
    /** @param other The second box.
     *  @return The intersection box. */
    BoundingBox intersect(const BoundingBox& other) const;

    /// Returns the minimum corner of the box.
    /** @return The minimum corner. */
    const ramses_internal::Vector3& getMinimumBoxCorner() const;

    /// Returns the maximum corner of the box.
    /** @return The maximum corner. */
    const ramses_internal::Vector3& getMaximumBoxCorner() const;

    /// Resets the box to be empty.
    void reset();

    /// Checks if this box overlaps with a second one.
    /** @param other The second box.
     *  @return "true", when the boxes overlap, and "false" otherwise. */
    bool checkOverlap(const BoundingBox& other) const;

    /// Returns the i-th corner point of the box.
    /** @param index The index (0-7). Min/Max value, bit 0: X, bit 1: Y, bit 2: Z.
     *  @return The corner point. */
    ramses_internal::Vector3 getPoint(uint32_t index) const;

    /// Returns "true", if this box is empty.
    /** @return "true," if empty, and "false" otherwise. */
    bool isEmpty() const;

private:
    /// The minimum corner of the box.
    ramses_internal::Vector3 m_min;

    /// The maximum corner of the box.
    ramses_internal::Vector3 m_max;
};

#endif
