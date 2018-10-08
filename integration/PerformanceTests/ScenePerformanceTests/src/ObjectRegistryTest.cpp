//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ObjectRegistryTest.h"
#include "PerformanceTestUtils.h"
#include "ramses-client-api/TransformationNode.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RamsesObject.h"
#include "RamsesObjectImpl.h"

ObjectRegistryTest::ObjectRegistryTest(ramses_internal::String testName, uint32_t testState) : PerformanceTestBase(testName, testState) {};

void ObjectRegistryTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);

    const uint32_t ObjectCount = 10 * 1000;

    m_objects.reserve(ObjectCount);
    PerformanceTestUtils::BuildNodesOfVariousTypes(m_objects, scene, ObjectCount);

    m_objectsShuffled = m_objects;

    PerformanceTestUtils::ShuffleObjectList(m_objectsShuffled);

    switch (m_testState)
    {
    case ObjectRegistryTest_Add:
    case ObjectRegistryTest_FindByName:
    {
        addObjects();
        break;
    }
    default:
    {
        break;
    }
    }
}

void ObjectRegistryTest::preUpdate()
{
    switch (m_testState)
    {
    case ObjectRegistryTest_Add:
    {
        removeObjects(); // Clear items, so (re)adding can be tested
        break;
    }
    case ObjectRegistryTest_Delete:
    {
        addObjects(); // Add items, so removal can be tested
        break;
    }
    case ObjectRegistryTest_FindByName:
    {
        assert(!m_objects.empty());
        break;
    }
    default:
    {
        assert(false);
        break;
    }
    }
}

void ObjectRegistryTest::update()
{
    switch (m_testState)
    {
    case ObjectRegistryTest_Add:
    {
        addObjects();
        break;
    }
    case ObjectRegistryTest_Delete:
    {
        removeObjects();
        break;
    }
    case ObjectRegistryTest_FindByName:
    {
        for (auto it : m_objectsShuffled)
        {
            ramses::RamsesObject* obj = it;

            // This is just here to make sure the compiler doesn't optimize things away. Will never be executed.
            if (m_registry.findObjectByName(obj->getName()) == nullptr)
            {
                assert(false);
                m_registry.removeObject(*obj);
            }
        }

        break;
    }
    default:
    {
        assert(false);
        break;
    }
    }
}

void ObjectRegistryTest::addObjects()
{
    for (auto it : m_objects)
    {
        m_registry.addObject(*it);
    }
}

void ObjectRegistryTest::removeObjects()
{
    // Removed objects in shuffled order compared to how they were added.
    for (auto it : m_objectsShuffled)
    {
        m_registry.removeObject(*it);
    }
}
