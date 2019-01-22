//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CITYMODELSCENE_H
#define RAMSES_CITYMODEL_CITYMODELSCENE_H

#include "ramses-citymodel/AnimationPath.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataFloat.h"
#include "Math3d/Vector3.h"

#include "vector"
#include "string"

namespace ramses
{
    class Scene;
    class Node;
}

class Tile;

/// The citymodel scene object.
class CitymodelScene
{
public:
    /// Constructor.
    CitymodelScene(ramses::Scene& scene);

    /// Adds a tile meta-data to the list of tiles.
    void addTile(Tile* tile);

    /// Sets the carsor model.
    /** @param node The ramses node of the carsor model. */
    void setCarsor(ramses::Node* node);

    /// Returns the carsor model.
    /** @return The carsor model. */
    ramses::Node* getCarsor() const;

    /// Sets the car position uniform to all materials.
    /** @param carPos The car position. */
    void setCarPos(const ramses_internal::Vector3& carPos);

    /// Sets the light cone factor.
    /** @param f Factor for the light cone. */
    void setLightConeFactor(float f);

    /// Returns the data vector for passing the car position to the effects.
    /** @return The data vector. */
    const ramses::DataVector3f& getDataVectorOfCarPosition() const;

    /// Returns the data vector for passing the light cone scale to the effects.
    /** @return The data vector. */
    const ramses::DataFloat& getDataOfLightConeScale() const;

    /// Returns the list of tile meta-data.
    /** @return The list of tiles meta-data. */
    const std::vector<Tile*>& getTiles() const;

    /// Returns the animation path.
    /** @return The animation path. */
    AnimationPath& getAnimationPath();

    /// Returns the list of names.
    /** @return The list of names. */
    std::vector<std::string>& getNames();

    /// Returns the list of name points.
    /** @return The list of name points. */
    std::vector<ramses_internal::Vector3>& getNamePoints();

    /// Returns the list of route points.
    /** @return The list of route points. */
    std::vector<ramses_internal::Vector3>& getRoutePoints();

protected:
    ramses::DataVector3f* m_carPosData         = nullptr;
    ramses::DataFloat*    m_lightConeScaleData = nullptr;

    /// List of tiles meta-data.
    std::vector<Tile*> m_tiles;

    /// The carsor model.
    ramses::Node* m_carsor = nullptr;

    /// The animation path for car and carsor.
    AnimationPath m_animationPath;

    /// The list of street names.
    std::vector<std::string> m_names;

    /// The list of name points.
    std::vector<ramses_internal::Vector3> m_namePoints;

    /// The list of route points.
    std::vector<ramses_internal::Vector3> m_routePoints;
};

#endif
