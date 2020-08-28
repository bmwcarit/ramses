//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEDATAPOOLPERSISTATIONTEST_H
#define RAMSES_RESOURCEDATAPOOLPERSISTATIONTEST_H

#include <gtest/gtest.h>
#include "ramses-client-api/RamsesClient.h"
#include "RamsesClientImpl.h"
#include "CreationHelper.h"
#include "ScenePersistationTest.h"
#include "ramses-hmi-utils.h"

namespace ramses
{
    class ResourceDataPoolPersistation : public ASceneAndAnimationSystemLoadedFromFile
    {
    public:
        template<typename ResourceType>
        ResourceType* createResource(const char* name)
        {
            return m_creationHelper.createObjectOfType<ResourceType>(name);
        }

        void doWriteReadCycle(std::string const& fileName)
        {
            EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(m_scene, fileName, false));
            EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).addResourceDataFile(fileName));
        }

        template<typename ResourceType>
        const ResourceType* getObjectForTesting(resourceId_t const& id, const char* name)
        {
            if (!m_sceneLoaded)
                m_sceneLoaded = m_clientForLoading.createScene(sceneId_t{ 0xf00b42 });
            RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).createResourceForScene(*m_sceneLoaded, id);
            Resource* objectForId = this->m_sceneLoaded->getResource(id);
            EXPECT_TRUE(objectForId);
            RamsesObject* objectPerName = this->m_sceneLoaded->findObjectByName(name);
            auto type = TYPE_ID_OF_RAMSES_OBJECT<ResourceType>::ID;
            EXPECT_EQ(objectPerName->getType(), type);
            EXPECT_EQ(objectPerName, objectForId);
            EXPECT_TRUE(objectPerName);
            if (!objectPerName)
                return nullptr;

            const ResourceType* specificObject = RamsesUtils::TryConvert<ResourceType>(*objectPerName);
            EXPECT_TRUE(specificObject);
            return specificObject;
        }
    };
}

#endif
