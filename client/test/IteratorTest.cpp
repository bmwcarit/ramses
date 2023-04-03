//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include <gtest/gtest.h>
#include "ClientTestUtils.h"

#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/MeshNode.h"

#include "IteratorImpl.h"
#include "RamsesObjectVector.h"

namespace ramses
{
    class IteratorTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        IteratorTest() : LocalTestClientWithScene()
        {
            obj1 = &this->createObject<Node>("object1");
            obj2 = &this->createObject<Appearance>("object2");
            obj3 = &this->createObject<MeshNode>("object3");
            obj4 = &this->createObject<Node>("object4");
            obj5 = &this->createObject<Appearance>("object5");
            obj6 = &this->createObject<MeshNode>("object6");

            objects.push_back(obj1);
            objects.push_back(obj2);
            objects.push_back(obj3);
            objects.push_back(obj4);
            objects.push_back(obj5);
            objects.push_back(obj6);
        }

        RamsesObject* obj1;
        RamsesObject* obj2;
        RamsesObject* obj3;
        RamsesObject* obj4;
        RamsesObject* obj5;
        RamsesObject* obj6;

        RamsesObjectVector objects;
    };

    TEST_F(IteratorTest, iteratesOverAllObjects)
    {
        IteratorImpl<RamsesObject*> iterator(objects);

        EXPECT_EQ(obj1, iterator.getNext());
        EXPECT_EQ(obj2, iterator.getNext());
        EXPECT_EQ(obj3, iterator.getNext());
        EXPECT_EQ(obj4, iterator.getNext());
        EXPECT_EQ(obj5, iterator.getNext());
        EXPECT_EQ(obj6, iterator.getNext());
        EXPECT_EQ(nullptr, iterator.getNext());
    }
}
