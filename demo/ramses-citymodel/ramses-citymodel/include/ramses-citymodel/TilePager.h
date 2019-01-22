//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_TILEPAGER_H
#define RAMSES_CITYMODEL_TILEPAGER_H

#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformConditionVariable.h"
#include "PlatformAbstraction/PlatformLock.h"

#include "vector"
#include "deque"

class Tile;

/// Class for asynchron paging of tiles.
class TilePager : ramses_internal::Runnable
{
public:
    /// Constructor.
    TilePager();

    /// Destructor.
    ~TilePager();

    /// Adds a set of tiles to the list to be loaded.
    /** @param tiles The tiles to be added. */
    void add(std::vector<Tile*> tiles);

    /// Removes a set of tiles from the list to be loaded.
    /** @param tiles The tiles to be removed. */
    void remove(std::vector<Tile*> tiles);

    /// Returns the set of tiles, that were newly loaded.
    /** @param tiles The set of tiles. */
    void get(std::vector<Tile*>& tiles);

    /// Returns the number of tiles which have been added but not yet loaded.
    /** @return The number of tiles which have been added but not yet loaded. */
    uint32_t getNumTilesToLoad();

private:
    /// Does the paging work. Called from a worker thread.
    void run() override;

    /// Removes a tile from the list to be loaded.
    /** @param tile The tile to be removed. */
    void remove(Tile* tile);

    /// The worker thread for doing the tile loading.
    ramses_internal::PlatformThread m_thread;

    /// Condition to wake up the worker thread, when new tiles are added for loading.
    ramses_internal::PlatformConditionVariable m_nonEmptyCondition;

    /// Mutex variable to synchronize access through the interface functions and the worker thread.
    ramses_internal::PlatformLightweightLock m_mutex;

    /// Queue of tiles to be read by the worker thread.
    std::deque<Tile*> m_queue;

    /// Vector of tiles, that were newly read and which are delivered by the get() function.
    std::vector<Tile*> m_readTiles;
};

#endif
