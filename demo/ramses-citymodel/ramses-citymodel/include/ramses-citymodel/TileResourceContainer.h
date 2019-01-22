//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_TILERESOURCECONTAINER_H
#define RAMSES_CITYMODEL_TILERESOURCECONTAINER_H

#include "Math3d/Vector3.h"
#include "set"

class Material;
class GeometryNode;

namespace ramses
{
    class RamsesClient;
    class Scene;
    class Resource;
    class SceneObject;
    class RenderGroup;
}

class TileResourceContainer
{
public:
    /// Adds a geometry node.
    /** @param geometryNode The geometry node. */
    void addGeometryNode(GeometryNode* geometryNode);

    /// Adds a material.
    /** @param material The material. */
    void addMaterial(Material* material);

    /// Adds a resource.
    /** @param resource The resource. */
    void addResource(const ramses::Resource* resource);

    /// Adds a scene object.
    /** @param sceneObject The scene object. */
    void addSceneObject(ramses::SceneObject* sceneObject);

    /// Destroys the stored objects.
    /** @param client The RAMSES client.
     *  @param scene The RAMSES scene.
     *  @param renderGroup The render group. */
    void destroy(ramses::RamsesClient& client, ramses::Scene& scene, ramses::RenderGroup& renderGroup);

    /// Computes the intersection of a ray with the geometry of the tile.
    /** @param p Start point of the ray.
     *  @param d Direction of the ray.
     *  @param r Intersection parameter, updated when there is a nearer intersection. */
    void computeIntersection(const ramses_internal::Vector3& p, const ramses_internal::Vector3& d, float& r);

private:
    /// Destroys the stored materials.
    void destroyMaterials();

    /// Destroys the stored geometry nodes.
    void destroyGeometryNodes();

    /// Destroys the stored resources.
    /** @param client The RAMSES client. */
    void destroyResources(ramses::RamsesClient& client);

    /// Destroys the stored scene objects and removes them from the render group.
    /** @param scene The RAMSES scene.
     *  @param renderGroup The render group. */
    void destroySceneObjects(ramses::Scene& scene, ramses::RenderGroup& renderGroup);

    /// Computes the intersection of a ray with a triangle.
    /** @param p Start point of the ray.
     *  @param d Direction of the ray.
     *  @param p0 First point of the triangle.
     *  @param p1 Second point of the triangle.
     *  @param p2 Third point of the triangle.
     *  @return The intersection parameter. */
    float ComputeIntersectionWithTriangle(const ramses_internal::Vector3& p,
                                          const ramses_internal::Vector3& d,
                                          const ramses_internal::Vector3& p0,
                                          const ramses_internal::Vector3& p1,
                                          const ramses_internal::Vector3& p2);

    /// Set of stored material.
    std::set<Material*> m_materials;

    /// Set of stored geometry nodes.
    std::set<GeometryNode*> m_geometryNodes;

    /// Set of stored resource objects.
    std::set<const ramses::Resource*> m_resources;

    /// Set of stored scene objects.
    std::set<ramses::SceneObject*> m_sceneObjects;
};

#endif
