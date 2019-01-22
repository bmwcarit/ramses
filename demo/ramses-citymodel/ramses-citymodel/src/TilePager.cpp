//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/TilePager.h"
#include "ramses-citymodel/Tile.h"

TilePager::TilePager()
    : m_thread("TilePager")
{
    m_thread.start(*this);
}

TilePager::~TilePager()
{
    m_thread.cancel();
    m_nonEmptyCondition.signal();
    m_thread.join();
}

void TilePager::add(std::vector<Tile*> tiles)
{
    if (!tiles.empty())
    {
        m_mutex.lock();
        bool wasEmpty = m_queue.empty();
        for (uint32_t i = 0; i < tiles.size(); i++)
        {
            m_queue.push_front(tiles[i]);
        }
        if (wasEmpty)
        {
            m_nonEmptyCondition.signal();
        }
        m_mutex.unlock();
    }
}

void TilePager::remove(std::vector<Tile*> tiles)
{
    if (!tiles.empty())
    {
        m_mutex.lock();
        for (uint32_t i = 0; i < tiles.size(); i++)
        {
            remove(tiles[i]);
        }
        m_mutex.unlock();
    }
}

void TilePager::remove(Tile* tile)
{
    for (uint32_t i = 0; i < m_queue.size(); i++)
    {
        if (m_queue[i] == tile)
        {
            m_queue.erase(m_queue.begin() + i);
            return;
        }
    }
}

void TilePager::get(std::vector<Tile*>& tiles)
{
    m_mutex.lock();
    tiles = m_readTiles;
    m_readTiles.clear();
    m_mutex.unlock();
}

uint32_t TilePager::getNumTilesToLoad()
{
    m_mutex.lock();
    unsigned int numTiles = static_cast<unsigned int>(m_queue.size());
    m_mutex.unlock();
    return numTiles;
}

void TilePager::run()
{
    m_mutex.lock();
    for (;;)
    {
        while (m_queue.empty())
        {
            if (isCancelRequested())
            {
                m_mutex.unlock();
                return;
            }
            m_nonEmptyCondition.wait(&m_mutex);
        }

        Tile* tile = m_queue.back();
        m_queue.pop_back();
        m_mutex.unlock();

        tile->doReadNode();

        m_mutex.lock();
        m_readTiles.push_back(tile);
    }
}
