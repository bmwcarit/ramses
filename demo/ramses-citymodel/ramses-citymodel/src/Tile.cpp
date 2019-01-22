//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Tile.h"
#include "ramses-citymodel/Citymodel.h"
#include "ramses-citymodel/Reader.h"
#include "ramses-citymodel/Timer.h"
#include "ramses-client-api/Node.h"

#include "RamsesObjectTypeUtils.h"

Tile::Tile(const BoundingBox& boundingBox, Citymodel& citymodel, uint32_t index)
    : m_boundingBox(boundingBox)
    , m_citymodel(citymodel)
    , m_index(index)
{
    m_center = (m_boundingBox.getMinimumBoxCorner() + m_boundingBox.getMaximumBoxCorner()) * 0.5;
}

const BoundingBox& Tile::boundingBox() const
{
    return m_boundingBox;
}

const ramses_internal::Vector3& Tile::center() const
{
    return m_center;
}

void Tile::setVisible(bool v)
{
    assert(m_visible != v);
    m_visible = v;
    if (v)
    {
        if (!m_rootNode)
        {
            if (!m_queuedToLoad)
            {
                addTileToRead();
            }
        }
        else
        {
            m_rootNode->setVisibility(true);
            removeTileToDelete();
        }
    }
    else
    {
        if (m_rootNode)
        {
            m_rootNode->setVisibility(false);
            addTileToDelete();
        }
        else
        {
            removeTileToRead();
        }
    }
}

void Tile::loaded()
{
    m_rootNode   = m_loadedNode;
    m_loadedNode = 0;
    if (!m_visible)
    {
        m_rootNode->setVisibility(false);
        addTileToDelete();
    }
    m_queuedToLoad = false;
}

void Tile::addTileToRead()
{
    m_citymodel.addTileToRead(this);
    m_queuedToLoad = true;
}

void Tile::removeTileToRead()
{
    if (m_queuedToLoad)
    {
        m_citymodel.removeTileToRead(this);
        m_queuedToLoad = false;
    }
}

void Tile::addTileToDelete()
{
    m_deleteCounter = 100;
    m_citymodel.addTileToDelete(this);
}

void Tile::removeTileToDelete()
{
    m_citymodel.removeTileToDelete(this);
}

void Tile::decDeleteCounter()
{
    assert(m_deleteCounter > 0);
    m_deleteCounter--;
    if (m_deleteCounter == 0)
    {
        if (m_rootNode)
        {
            m_loadedRamsesResources.destroy(
                m_citymodel.getRamsesClient(), m_citymodel.getRamsesScene(), m_citymodel.getRenderGroup());
            m_rootNode = 0;
        }

        removeTileToDelete();
    }
}

void Tile::doReadNode()
{
    void*                 object       = m_citymodel.getReader().read(m_index + 1, m_loadedRamsesResources);
    ramses::RamsesObject* ramsesObject = static_cast<ramses::RamsesObject*>(object);

    if (!ramsesObject || !ramsesObject->isOfType(ramses::ERamsesObjectType_Node))
    {
        printf("CTile:readNode Could not read node !!!");
        exit(1);
    }

    m_loadedNode = &ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Node>(*ramsesObject);
}

void Tile::computeIntersection(const ramses_internal::Vector3& p, const ramses_internal::Vector3& d, float& r)
{
    if (m_visible)
    {
        m_loadedRamsesResources.computeIntersection(p, d, r);
    }
}
