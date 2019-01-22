//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_NAME2D_H
#define RAMSES_CITYMODEL_NAME2D_H

#include "Math3d/Vector4.h"
#include "Collections/String.h"
#include "Name.h"

class NamingManager;

namespace ramses
{
    class Node;
}

/// Name class for tangential 2d names
class Name2D : public Name
{
public:
    /// Constructor.
    /** @param text The label text.
     *  @param p0 Anchor point in 3D.
     *  @param p1 Second point to define the tangent direction. */
    Name2D(const std::string&              text,
           ramses::Scene&                  scene,
           ramses::RamsesClient&           client,
           ramses::RenderGroup&            renderGroup,
           const ramses_internal::Vector3& p0,
           const ramses_internal::Vector3& p1);

    /// Updates the position on the screen, computes whether the name collides and update the transparency.
    void update();

    /// Sets the naming root manager.
    /** @param namingRoot The naming manager. */
    void setNamingManager(NamingManager* namingRoot);

    /// Returns the i-th bounding box corner point.
    /** @param i Index (0-3).
     *  @return The i-th bounding box point. */
    const ramses_internal::Vector4& boundingPoint(uint32_t i) const;

    /// Checks if this name collides with another name.
    /** @param name The second name to check with.
     *  @return "true", when the names colide and "false" otherwise. */
    bool checkCollision(Name2D* name);

    /**
     * Changes behaviour whether name will fade in/out over several frames or
     * is directly visible.
     *
     * @param doFading If \c true the name will slowly fade in/out over several
     * frames, otherwise \c false the name will be switch visibility directly
     */
    void useFading(bool doFading);

    static ramses_internal::Matrix44f GetWorldSpaceMatrixOfNode(ramses::Node& node);
    static ramses_internal::Matrix44f GetObjectSpaceMatrixOfNode(ramses::Node& node);

protected:
    /// Computes translation and rotation of the label for the current view matrix.
    void updatePosition();

    /// Checks this name, if currently visible against all current colliders and makes it invisible, when it does
    /// collide.
    void checkCollision();

    /// Smoothly increments/decrements the current alpha value towards it's current destination value.
    void updateAlpha();

    /// Computes the four bounding points for the current view matrix.
    void computeBounding();

    /// Checks if one of this bounding box sides is a dividing plane against a second name label.
    /** @param name The name to check with.
     *  @param "false", dividing plane found, so there is no collision, and "true", if no plane divides the boxes, so
     * there might be a collision. */
    bool checkCollisionThisToOther(Name2D* name);

    /// The anchor point in 3D for this name.
    ramses_internal::Vector3 m_p0;

    /// Second point to define the tangent direction
    ramses_internal::Vector3 m_p1;

    /// The naming manager.
    NamingManager* m_namingManager;

    /// Current transparency value for smooth fading in/out.
    float m_alpha;

    /// Flag, if the label is valid for rendering.
    /** It is valid, when both points can be projected on the screen. */
    bool m_validForRendering;

    /// Current computed visible state.
    bool m_destinationVisible;

    /// The for points of the bounding box projected on the screen.
    ramses_internal::Vector4 m_boundingPoints[4];

    /// Setting if fading in/out should be used
    bool m_useFading;
};

#endif
