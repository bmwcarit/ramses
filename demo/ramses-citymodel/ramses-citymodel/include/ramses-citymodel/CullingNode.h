//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CULLINGNODE_H
#define RAMSES_CITYMODEL_CULLINGNODE_H

#include "Math3d/Vector2.h"

#include "ramses-citymodel/Frustum.h"
#include "ramses-citymodel/Tile.h"

#include "vector"
#include "BoundingBox.h"

class SortedTileSet;
class Citymodel;

/// Node class for culling tree.
class CullingNode
{
public:
    /// Creates the root node of the culling tree, along it's child nodes.
    /** @param tiles All tiles for building the tree.
     *  @param citymodel The citymodel client main class. */
    CullingNode(const std::vector<Tile*>& tiles, Citymodel* citymodel);

    /// Returns the bounding box of this node.
    /** @return The bounding box. */
    const BoundingBox& boundingBox() const;

    /// Computes if this node is visible or not by clipping against the frustum.
    /** When visible, the child nodes are recursively checked, otherwise they
     *  are all set to invisible.
     *  @param clipMask Masks the clipping planes of the frustum, which have to be checked.
     *  @param frustum The viewing frustum. */
    void computeVisible(uint32_t clipMask, const Frustum& frustum);

private:
    /// Creates a child node of the culling tree, along it's child nodes.
    /** @param tiles All tiles for building the sub-tree.
     *  @param level Level of the node (root has level 0).
     *  @param citymodel The citymodel client main class. */
    CullingNode(const SortedTileSet& tiles, uint32_t level, Citymodel* citymodel);

    /// Creates the child nodes, when necessary and subdivides the tiles list into the childs.
    /** @param tiles The list of tiles to be inserted. */
    void subdivide(const SortedTileSet& tiles);

    /// Computes the median index for a number of elements.
    /** @param n Number of elements.
     *  @return The index of the median. */
    uint32_t medianIndex(uint32_t n);

    /// Splits a set of tiles in the X direction, using the median X value as criteria.
    /** @param tiles The set of tiles to be split.
     *  @param tilesLower Resulting tiles on the lower side (lesser X value than the median).
     *  @param tilesUpper Resulting tiles on the upper side (greater X value than the median). */
    void splitX(const SortedTileSet& tiles, SortedTileSet& tilesLower, SortedTileSet& tilesUpper);

    /// Splits a set of tiles in the Y direction, using the median Y value as criteria.
    /** @param tiles The set of tiles to be split.
     *  @param tilesLower Resulting tiles on the lower side (lesser X value than the median).
     *  @param tilesUpper Resulting tiles on the upper side (greater X value than the median). */
    void splitY(const SortedTileSet& tiles, SortedTileSet& tilesLower, SortedTileSet& tilesUpper);

    /// Makes this node and all tiles in the node visible/invisible.
    /** When set to invisible, also all child nodes are set invisible.
     *  @param v Flag, if the node shall be set to visible or not. */
    void setVisible(bool v);

    /// Computes the bounding box of a set of tiles.
    /** @param tiles The set of tiles. */
    BoundingBox computeBoundingBox(const std::vector<Tile*>& tiles);

    /// Checks whether the list of tiles is worth to be further split in sub nodes.
    /** @return Returns "true", when there is more than one bounding box. */
    bool isWorthToSplit(std::vector<BoundingBox>& boxes);

    /// List of tiles to be rendered, when this node is visible.
    std::vector<Tile*> m_tiles;

    /// The level of the node (the root has level 0).
    uint32_t m_level = 0;

    /// The citymodel main class.
    Citymodel* m_citymodel = nullptr;

    /// Flag, if the node is currently visible.
    bool m_visible = false;

    /// The child nodes, when further subdivided.
    std::vector<CullingNode*> mChilds;

    /// Bounding box of the child nodes and the tiles.
    BoundingBox mBoundingBox;
};

#endif
