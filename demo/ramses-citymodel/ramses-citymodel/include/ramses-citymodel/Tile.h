//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_TILE_H
#define RAMSES_CITYMODEL_TILE_H

#include "BoundingBox.h"
#include "Reader.h"
#include "TileResourceContainer.h"

class Citymodel;

namespace ramses
{
    class Node;
}

/// Tile meta data class.
class Tile
{
public:
    /// Constructor.
    /** @param boundingBox The bounding box.
     *  @param citymodel The citymodel main class.
     *  @param index Index of the tile. */
    Tile(const BoundingBox& boundingBox, Citymodel& citymodel, uint32_t index);

    /// Returns the center of the bounding box.
    /** @return The center of the bounding box. */
    const ramses_internal::Vector3& center() const;

    /// Returns the bounding box.
    /** @return The bounding box. */
    const BoundingBox& boundingBox() const;

    /// Makes the tile visible or invisible.
    /** Queues the tile data for visible tiles to be read by the pager worker thread, when not already read.
     *  Add tiles to the delete list, when invisible. */
    void setVisible(bool v);

    /// Reads the tile geometry from the ".rex" archive file.
    /** Called by the worker thread of the pager, so don't access other members than mLoadedNode. */
    void doReadNode();

    /// Called for newly read tiles from the pager. Called by the main thread.
    void loaded();

    /// Called each frame, when the tile is not visible anymore. Decrements the delete counter and deletes the tile
    /// geometry, when counted down.
    void decDeleteCounter();

    /// Computes the intersection of a ray with the geometry of the tile.
    /** @param p Start point of the ray.
     *  @param d Direction of the ray.
     *  @param r Intersection parameter, updated when there is a nearer intersection. */
    void computeIntersection(const ramses_internal::Vector3& p, const ramses_internal::Vector3& d, float& r);

private:
    /// Adds the tile to the read queue of the pager worker thread.
    void addTileToRead();

    /// Removes the tile from the read queue.
    void removeTileToRead();

    /// Adds the tile to the delete list.
    void addTileToDelete();

    /// Removes tile from the delete list.
    void removeTileToDelete();

    /// The root node of this tile, loaded by the pager thread.
    ramses::Node* m_loadedNode = nullptr;

    /// Bounding box of the tile
    BoundingBox m_boundingBox;

    /// The citymodel main class.
    Citymodel& m_citymodel;

    /// The root node of this tile.
    ramses::Node* m_rootNode = nullptr;

    /// Flag, if the tile is currently visible or not.
    bool m_visible = false;

    /// Flag, if the tile is currently queued to be loaded by the pager worker thread.
    bool m_queuedToLoad = false;

    /// Delete counter, called each frame the tile is invisible. When counted down, the tile geometry data is deleted.
    uint32_t m_deleteCounter = 0;

    /// Center of the bounding box.
    ramses_internal::Vector3 m_center;

    /// The parent node, where to hang in this tile node, when visible.
    ramses::Node* m_parent = nullptr;

    /// All created resources used by the tile
    TileResourceContainer m_loadedRamsesResources;

    /// Index of the tile in the archive file.
    uint32_t m_index = 0;
};

#endif
