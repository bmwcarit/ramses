//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/CullingNode.h"
#include "ramses-citymodel/Citymodel.h"

#include "algorithm"
#include "assert.h"

bool lessX(Tile* a, Tile* b)
{
    return a->center().x < b->center().x;
}

bool lessY(Tile* a, Tile* b)
{
    return a->center().y < b->center().y;
}

class SortedTileSet
{
public:
    SortedTileSet() {}
    SortedTileSet(const std::vector<Tile*>& tilesX, const std::vector<Tile*>& tilesY)
        : m_tilesX(tilesX)
        , m_tilesY(tilesY)
    {
    }
    std::vector<Tile*> m_tilesX;
    std::vector<Tile*> m_tilesY;
};

CullingNode::CullingNode(const std::vector<Tile*>& tiles, Citymodel* client)
    : m_citymodel(client)
{
    std::vector<Tile*> tilesX = tiles;
    std::sort(tilesX.begin(), tilesX.end(), lessX);

    std::vector<Tile*> tilesY = tiles;
    std::sort(tilesY.begin(), tilesY.end(), lessY);

    subdivide(SortedTileSet(tilesX, tilesY));
}

CullingNode::CullingNode(const SortedTileSet& tiles, uint32_t level, Citymodel* citymodel)
    : m_level(level)
    , m_citymodel(citymodel)
{
    subdivide(tiles);
}

BoundingBox CullingNode::computeBoundingBox(const std::vector<Tile*>& tiles)
{
    BoundingBox bb;
    for (uint32_t i = 0; i < tiles.size(); i++)
    {
        bb.add(tiles[i]->boundingBox());
    }
    return bb;
}

bool CullingNode::isWorthToSplit(std::vector<BoundingBox>& boxes)
{
    if (boxes.size() <= 1)
    {
        return false;
    }
    return true;
}

void CullingNode::subdivide(const SortedTileSet& tiles)
{
    SortedTileSet tilesLeft;
    SortedTileSet tilesRight;
    splitX(tiles, tilesLeft, tilesRight);

    SortedTileSet childTiles[4];
    splitY(tilesLeft, childTiles[0], childTiles[1]);

    splitY(tilesRight, childTiles[2], childTiles[3]);

    std::vector<BoundingBox> boxes;

    for (uint32_t i = 0; i < 4; i++)
    {
        if (childTiles[i].m_tilesX.size() != 0)
        {
            boxes.push_back(computeBoundingBox(childTiles[i].m_tilesX));
        }
    }

    if (isWorthToSplit(boxes))
    {
        /// Subdivide into child nodes
        for (uint32_t i = 0; i < 4; i++)
        {
            if (childTiles[i].m_tilesX.size() != 0)
            {
                mChilds.push_back(new CullingNode(childTiles[i], m_level + 1, m_citymodel));
            }
        }
    }
    else
    {
        m_tiles = tiles.m_tilesX;
    }

    for (uint32_t i = 0; i < m_tiles.size(); i++)
    {
        mBoundingBox.add(m_tiles[i]->boundingBox());
    }

    for (uint32_t i = 0; i < mChilds.size(); i++)
    {
        mBoundingBox.add(mChilds[i]->boundingBox());
    }
}

uint32_t CullingNode::medianIndex(uint32_t n)
{
    return (n - 1) / 2;
}

void CullingNode::splitX(const SortedTileSet& tiles, SortedTileSet& tilesLower, SortedTileSet& tilesUpper)
{
    assert(tiles.m_tilesX.size() == tiles.m_tilesY.size());
    if (tiles.m_tilesX.size() == 0)
    {
        return;
    }

    float median = tiles.m_tilesX[medianIndex(static_cast<uint32_t>(tiles.m_tilesX.size()))]->center().x;

    for (uint32_t i = 0; i < tiles.m_tilesX.size(); i++)
    {
        if (tiles.m_tilesX[i]->center().x <= median)
        {
            tilesLower.m_tilesX.push_back(tiles.m_tilesX[i]);
        }
        else
        {
            tilesUpper.m_tilesX.push_back(tiles.m_tilesX[i]);
        }
    }

    for (uint32_t i = 0; i < tiles.m_tilesY.size(); i++)
    {
        if (tiles.m_tilesY[i]->center().x <= median)
        {
            tilesLower.m_tilesY.push_back(tiles.m_tilesY[i]);
        }
        else
        {
            tilesUpper.m_tilesY.push_back(tiles.m_tilesY[i]);
        }
    }
}

void CullingNode::splitY(const SortedTileSet& tiles, SortedTileSet& tilesLower, SortedTileSet& tilesUpper)
{
    assert(tiles.m_tilesX.size() == tiles.m_tilesY.size());
    if (tiles.m_tilesY.size() == 0)
    {
        return;
    }

    float median = tiles.m_tilesY[medianIndex(static_cast<uint32_t>(tiles.m_tilesY.size()))]->center().y;
    for (uint32_t i = 0; i < tiles.m_tilesX.size(); i++)
    {
        if (tiles.m_tilesX[i]->center().y <= median)
        {
            tilesLower.m_tilesX.push_back(tiles.m_tilesX[i]);
        }
        else
        {
            tilesUpper.m_tilesX.push_back(tiles.m_tilesX[i]);
        }
    }

    for (uint32_t i = 0; i < tiles.m_tilesY.size(); i++)
    {
        if (tiles.m_tilesY[i]->center().y <= median)
        {
            tilesLower.m_tilesY.push_back(tiles.m_tilesY[i]);
        }
        else
        {
            tilesUpper.m_tilesY.push_back(tiles.m_tilesY[i]);
        }
    }
}

const BoundingBox& CullingNode::boundingBox() const
{
    return mBoundingBox;
}

void CullingNode::computeVisible(uint32_t clipMask, const Frustum& frustum)
{
    const bool visible = frustum.overlap(clipMask, mBoundingBox);

    setVisible(visible);

    if (visible)
    {
        for (uint32_t i = 0; i < mChilds.size(); i++)
        {
            mChilds[i]->computeVisible(clipMask, frustum);
        }
    }
}

void CullingNode::setVisible(bool v)
{
    if (m_visible != v)
    {
        m_visible = v;

        uint32_t n = static_cast<uint32_t>(m_tiles.size());
        for (uint32_t i = 0; i < n; i++)
        {
            m_tiles[i]->setVisible(v);
        }

        if (!v)
        {
            for (uint32_t i = 0; i < mChilds.size(); i++)
            {
                mChilds[i]->setVisible(false);
            }
        }
    }
}
