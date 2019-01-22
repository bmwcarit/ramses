//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_NAMINGMANAGER_H
#define RAMSES_CITYMODEL_NAMINGMANAGER_H

#include "Math3d/Vector2.h"

#include "Math3d/Matrix44f.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/TextCache.h"

#include "vector"
#include "list"

class Name2D;
class Name;

namespace ramses
{
    class RamsesClient;
    class Scene;
    class Node;
}

/// Naming manager.
/** Computes position and overlapping of text labels per frame. */
class NamingManager
{
public:
    /// Constructor.
    /*
     *  @param client RamsesClient containing the scene and resources.
     *  @param scene Scene containing the nodes.
     *  @param parent The ramses parent node, where to hang in the visible names.
     *  @param width Width of the window in pixel.
     *  @param height Height of the window in pixel.
     *  @param fovy Vertical field of view angle in degrees.
     *  @param useFading If "true" street names fade in over a couple of frames, otherwise they
     *  are shown already in the first frame.
     */
    NamingManager(ramses::RamsesClient& client,
                  ramses::Scene&        scene,
                  ramses::Node*         parent,
                  float                 width,
                  float                 height,
                  float                 fovy,
                  bool                  useFading = true);

    /// Adds a naming object to the manager
    /// Position, collision and visibility for the naming object is computed by the naming manager.
    /** @param name The naming object. */
    void add(Name2D* name);

    /// Adds a naming object to the manager, that is permanently shown (no collision detection).
    /** @param name The naming object. */
    void addPermanent(Name* name);

    /// Removes a permanent naming object.
    /** @param name The naming object. */
    void removePermanent(Name* name);

    /// Updates position, collision and visibility computation for the naming objects.
    void update(const ramses_internal::Matrix44f& viewProjectionMatrix);

    /// Computes the projected xy coordinates of a 3D point onto the screen and checks if clipped by the frustum.
    /** @param p The point in 3D to be projected.
     *  @param p2d The projected screen coordinates.
     *  @param xyVisible Flag, if the point is visible(not clipped) by the X, Y clipping planes.
     *  @return "false", when clipped by the near plane and "true" otherwise. */
    bool projectCheckXY(const ramses_internal::Vector3& p, ramses_internal::Vector2& p2d, bool& xyVisible);

    /// Computes the projected xy coordinates of a 3D point onto the screen.
    /** @param p The point in 3D to be projected.
     *  @param p2d The projected screen coordinates.
     *  @return "true", if the point can be projected and "false" if the point is behind the viewer. */
    bool project(const ramses_internal::Vector3& p, ramses_internal::Vector2& p2d);

    /// Makes the mesh geometry of the name visible and adds it to the list of visible names.
    /** @param name The name to make visible. */
    void addToVisibleNames(Name2D* name);

    /// Makes the mesh geometry of the name invisible and adds it to the list of invisible names.
    /** @param name The name to make invisible. */
    void addToInvisibleNames(Name2D* name);

    /// Checks if a name collides with a name from the collider list.
    /** @param name The name to check.
     *  @return "true", when name collides with another name and "false", otherwise. */
    bool checkCollision(Name2D* name);

    /// Adds a name to the list of collider names (names to check collision against).
    /** @param name The name to be added. */
    void addToColliders(Name2D* name);

protected:
    /// Creates a node, that translates the name screen coordinates into the 3D camera coordinate system.
    /** Origin of the name coordinate system is the left, bottom corner of the screen.
     *  (width/height) maps to the rightmost/top corner.
     *  @param width Width of the window in pixel.
     *  @param height Height of the window in pixel.
     *  @param fovy Vertical field of view in degrees. */
    ramses::Node* createOrthoCamera(float width, float height, float fovy);

    /// Create the appearance to be used in name text
    // ramses::Appearance* createNameAppearance();

    /// The camera node, that does the translation from screen coordinates to the 3D camera.
    ramses::Node* m_camera = nullptr;

    /// Parent node, under which all permanent names are added.
    ramses::Node* m_permanentNames;

    /// List of all name labels.
    std::vector<Name2D*> m_names;

    /// The current view-projection matrix for transforming names into screen coordinates.
    ramses_internal::Matrix44f m_viewProjectionMatrix;

    /// Half size of the window in pixel.
    ramses_internal::Vector2 m_halfSize;

    /// List of currently visible names.
    /** Also includes partly visible names, that currenly collide and are faded out. */
    std::list<Name2D*> m_visibleNames;

    /// List of currently invisible names.
    std::list<Name2D*> m_invisibleNames;

    /// List of names to check collision against.
    std::vector<Name2D*> m_colliderNames;

    /// The effect for rendering the names.
    ramses::Effect*        m_effect = nullptr;
    ramses::AttributeInput m_positionInput;
    ramses::AttributeInput m_texCoordInput;
    ramses::UniformInput   m_textureInput;

    ramses::Scene& m_scene;

    /// Flag, whether names should slowly fade in vs. be shown directly first frame
    bool m_useFading = true;

    /// The RAMSES font registry.
    ramses::FontRegistry m_fontRegistry;

    /// The font instance.
    ramses::FontInstanceId m_fontInstance;

    /// The text cache.
    ramses::TextCache m_textCache;
};

#endif
