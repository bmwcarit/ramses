//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_OBJECTREGISTRYTEST_H
#define RAMSES_OBJECTREGISTRYTEST_H

#include "PerformanceTestBase.h"
#include "Collections/Vector.h"
#include "RamsesObjectRegistry.h"

namespace ramses
{
    class SceneObject;
    class Scene;
}

class ObjectRegistryTest : public PerformanceTestBase
{
public:

    enum
    {
        ObjectRegistryTest_Add = 0,
        ObjectRegistryTest_Delete,
        ObjectRegistryTest_FindByName,
    };

    ObjectRegistryTest(ramses_internal::String testName, uint32_t testState);

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void preUpdate() override;
    virtual void update() override;

private:
    void addObjects();
    void removeObjects();

    std::vector<ramses::SceneObject*> m_objects;
    std::vector<ramses::SceneObject*> m_objectsShuffled;
    ramses::RamsesObjectRegistry m_registry;
};
#endif

