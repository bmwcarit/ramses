//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/CitymodelScene.h"
#include "ramses-client-api/Scene.h"

CitymodelScene::CitymodelScene(ramses::Scene& scene)
{
    m_carPosData         = scene.createDataVector3f("carPos");
    m_lightConeScaleData = scene.createDataFloat("lightConeScale");
}

void CitymodelScene::addTile(Tile* tile)
{
    m_tiles.push_back(tile);
}

const std::vector<Tile*>& CitymodelScene::getTiles() const
{
    return m_tiles;
}

void CitymodelScene::setCarsor(ramses::Node* node)
{
    m_carsor = node;
}

ramses::Node* CitymodelScene::getCarsor() const
{
    return m_carsor;
}

void CitymodelScene::setCarPos(const ramses_internal::Vector3& carPos)
{
    m_carPosData->setValue(carPos.x, carPos.y, carPos.z);
}


void CitymodelScene::setLightConeFactor(float v)
{
    m_lightConeScaleData->setValue(v);
}

const ramses::DataVector3f& CitymodelScene::getDataVectorOfCarPosition() const
{
    return *m_carPosData;
}

const ramses::DataFloat& CitymodelScene::getDataOfLightConeScale() const
{
    return *m_lightConeScaleData;
}

AnimationPath& CitymodelScene::getAnimationPath()
{
    return m_animationPath;
}

std::vector<std::string>& CitymodelScene::getNames()
{
    return m_names;
}

std::vector<ramses_internal::Vector3>& CitymodelScene::getNamePoints()
{
    return m_namePoints;
}

std::vector<ramses_internal::Vector3>& CitymodelScene::getRoutePoints()
{
    return m_routePoints;
}
