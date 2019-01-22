//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_LINECONTAINER_H
#define RAMSES_CITYMODEL_LINECONTAINER_H

#include "Math3d/Vector3.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/UInt16Array.h"

#include "vector"

namespace ramses
{
    class RamsesClient;
}

/// Class for a line container.
/** All polylines in this container have the same style. */
class LineContainer
{
public:
    /// Line cap type.
    enum ECapType
    {
        ECapType_Round = 0,
        ECapType_Flat
    };

    /// Constructor.
    LineContainer(const ramses_internal::Vector3& color,
                  const ramses_internal::Vector3& colorInvisible,
                  float                           width,
                  const ramses_internal::Vector3& up,
                  ramses::Scene&                  scene,
                  ramses::RamsesClient&           client,
                  ramses::RenderGroup&            renderGroup);

    /// Activates rendering of invisible parts with a separate color.
    /** @param color The color for rendering the invisible parts. */
    void activateRenderingOfInvisibleParts(const ramses_internal::Vector3& color);

    /// Adds a polyline to this line container.
    /** @param points The points of the polyline.
     *  @param startCap Type of the start cap.
     *  @param endCap Type of the end cap. */
    void addPolyline(const std::vector<ramses_internal::Vector3>& points, ECapType startCap, ECapType endCap);

    /// Creates The ramses node for the line container.
    void updateGeometry();

private:
    /// Creates the effects needed for rendering.
    void createEffects();

    /// Create the geometry for rendering.
    void createGeometry();

    /// Adds a line segment to this container.
    /** @param p1 First point of the segment.
     *  @param p2 Second point of the segment.
     *  @param startCap Flag, whether the segment has a round start cap or not.
     *  @param endCap Flag, whether the segment has a round end cap or not.*/
    void addSegment(const ramses_internal::Vector3& p1, const ramses_internal::Vector3& p2, bool startCap, bool endCap);

    /// Vector of the line segments vertices.
    ramses_internal::Vector<float> m_vertex;

    /// Vector of the line segments anti aliasing parameters.
    ramses_internal::Vector<float> m_param;

    /// Fill color for the line.
    ramses_internal::Vector3 m_color;

    /// Color for rendering invisible parts.
    ramses_internal::Vector3 m_colorInvisible;

    /// Width of the line in pixel.
    float m_width = 0.0;

    /// Up vector.
    ramses_internal::Vector3 m_up;

    /// Flag, if the invisible parts shall be rendered. The color mColorInvisible is then used.
    bool m_renderInvisible = false;

    /// Scaling parameters for effect.
    /** mScale[0]: 1 / pixelToWorldScale
     *  mScale[1]: width / 2 [Pixel]
     *  mScale[2]: pixelToWorldScale * width / 2 [Pixel] */
    ramses_internal::Vector3 m_scale;

    /// Effect for building transparency in the alpha channel.
    ramses::Effect* m_effectLine = nullptr;

    /// Effect for filling the visible parts in the color buffer.
    ramses::Effect* m_effectLineFillVisible = nullptr;

    /// Effect for filling the invisible parts in the color buffer.
    ramses::Effect* m_effectLineFillInvisible = nullptr;

    /// The RAMSES client.
    ramses::RamsesClient& m_ramsesClient;

    /// The RAMSES scene.
    ramses::Scene& m_ramsesScene;

    /// The RAMSES render group.
    ramses::RenderGroup& m_renderGroup;

    /// Geometry binding for rendering the fill alpha pass.
    ramses::GeometryBinding* m_geometryFillAlpha;

    /// Geometry binding for rendering the visible part.
    ramses::GeometryBinding* m_geometryRenderVisible;

    /// Geometry binding for rendering the invisible part.
    ramses::GeometryBinding* m_geometryRenderInvisible;

    /// Mesh node for rendering the fill alpha pass.
    ramses::MeshNode* m_meshFillAlpha;

    /// Mesh node for rendering the visible part.
    ramses::MeshNode* m_meshRenderVisible;

    /// Mesh node for rendering the invisible part.
    ramses::MeshNode* m_meshRenderInvisible;

    /// Position vertex data.
    const ramses::Vector3fArray* m_positions = nullptr;

    /// Normals vertex data.
    const ramses::Vector3fArray* m_normals = nullptr;
};

#endif
