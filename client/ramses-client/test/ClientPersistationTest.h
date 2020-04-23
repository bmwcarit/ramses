//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTPERSISTATIONTEST_H
#define RAMSES_CLIENTPERSISTATIONTEST_H

#include <gtest/gtest.h>
#include "ramses-client-api/RamsesClient.h"
#include "RamsesClientImpl.h"
#include "CreationHelper.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ScenePersistationTest.h"

namespace ramses
{
    class ClientPersistation : public ASceneAndAnimationSystemLoadedFromFile
    {
    public:
        template<typename ResourceType>
        ResourceType* createResource(const char* name)
        {
            return m_creationHelper.createObjectOfType<ResourceType>(name);
        }

        template<typename T>
        void doWriteReadCycle(const T& fileDescription)
        {
            status_t status = client.saveResources(fileDescription, false);
            EXPECT_EQ(StatusOK, status);

            status = m_clientForLoading.loadResources(fileDescription);
            EXPECT_EQ(StatusOK, status);
        }


        template<typename ResourceType>
        const ResourceType* getObjectForTesting(const char* name)
        {
            const RamsesObject* objectPerName = this->m_clientForLoading.findObjectByName(name);
            EXPECT_TRUE(objectPerName != nullptr);
            if (objectPerName != nullptr)
            {
                EXPECT_STREQ(name, objectPerName->getName());
            }

            const ResourceType* specificObject = RamsesUtils::TryConvert<ResourceType>(*objectPerName);
            EXPECT_TRUE(nullptr != specificObject);
            return specificObject;
        }
    };
}

#endif
