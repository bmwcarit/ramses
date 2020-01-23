//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ClientTestUtils.h"
#include "RamsesClientTypesImpl.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/EffectDescription.h"
#include "ResourceMock.h"
#include "RamsesObjectTestTypes.h"

namespace ramses
{
    class AResourceFileDescriptionTest : public LocalTestClientWithScene, public ::testing::Test
    {
    };

    template <typename T>
    class AResourceFileDescriptionTestTyped : public LocalTestClientWithScene, public ::testing::Test
    {
    };

    TYPED_TEST_CASE(AResourceFileDescriptionTestTyped, ResourceTypes);

    TEST_F(AResourceFileDescriptionTest, FilenameIsSet)
    {
        ramses_internal::String firstPath("/home/carit/res/building.ramres");
        ramses_internal::String secondPath("res/car.ramres");

        ResourceFileDescription someFileDescription( firstPath.c_str() );
        ResourceFileDescription otherFileDescription( secondPath.c_str() );

        EXPECT_STREQ( firstPath.c_str(), someFileDescription.getFilename() );
        EXPECT_STREQ( secondPath.c_str(), otherFileDescription.getFilename() );
    }


    TYPED_TEST(AResourceFileDescriptionTestTyped, CanAddResources)
    {
        const TypeParam* ramsesObject = &this->template createObject<TypeParam>();

        ResourceFileDescription fileDescription( "res/car.ramres" );

        fileDescription.add( ramsesObject );

        EXPECT_EQ(1u, fileDescription.getNumberOfResources());
        EXPECT_EQ( ramsesObject, &fileDescription.getResource(0) );

    }

    TYPED_TEST(AResourceFileDescriptionTestTyped, ContainsChecksForResources)
    {
        const TypeParam* firstRamsesObject  = &this->template createObject<TypeParam>("a");
        const TypeParam* secondRamsesObject = &this->template createObject<TypeParam>("b");
        const TypeParam* thirdRamsesObject  = &this->template createObject<TypeParam>("c");

        ResourceFileDescription fileDescription( "res/car.ramres" );

        fileDescription.add( firstRamsesObject );
        fileDescription.add( secondRamsesObject );

        EXPECT_TRUE(  fileDescription.contains( firstRamsesObject ) );
        EXPECT_TRUE(  fileDescription.contains( secondRamsesObject ) );
        EXPECT_FALSE( fileDescription.contains( thirdRamsesObject ) );
    }

    TEST(SimpleResourceIDStreamOperatorTest, Zeros)
    {
        ramses_internal::StringOutputStream zeros;
        zeros << resourceId_t::Invalid();
        EXPECT_STREQ(zeros.c_str(), "0x0000000000000000:0000000000000000");
    }

    TEST(SimpleResourceIDStreamOperatorTest, Values)
    {
        resourceId_t val;
        val.highPart = 0x0123456789ABCDEF;
        val.lowPart  = 0xFEDCBA9876543210;
        ramses_internal::StringOutputStream values;
        values << val;
        EXPECT_STREQ(values.c_str(), "0x0123456789ABCDEF:FEDCBA9876543210");
    }

    TEST_F(AResourceFileDescriptionTest, emptyDescriptionUtilsTest)
    {
        ramses_internal::String firstPath("/home/carit/res/building.ramres");
        ramses_internal::String secondPath("res/car.ramres");

        ResourceFileDescription someFileDescription(firstPath.c_str());
        ResourceFileDescription otherFileDescription(secondPath.c_str());

        EXPECT_EQ(ramses_internal::StringOutputStream::ToString(someFileDescription),
            ramses_internal::String("Filename: ") + someFileDescription.getFilename() + " ; Resource count 0: [  ]");
        EXPECT_EQ(ramses_internal::StringOutputStream::ToString(otherFileDescription),
            ramses_internal::String("Filename: ") + otherFileDescription.getFilename() + " ; Resource count 0: [  ]");
    }

    TYPED_TEST(AResourceFileDescriptionTestTyped, multiResourceDescriptionUtilsTest)
    {
        const TypeParam* firstRamsesObject = &this->template createObject<TypeParam>("a");
        const TypeParam* secondRamsesObject = &this->template createObject<TypeParam>("b");

        ResourceFileDescription fileDescription("res/car.ramres");

        fileDescription.add(firstRamsesObject);
        fileDescription.add(secondRamsesObject);

        ramses_internal::StringOutputStream verifyStream;
        verifyStream << "[ " << firstRamsesObject->getResourceId() << " ; " << secondRamsesObject->getResourceId() << " ]";

        EXPECT_EQ(ramses_internal::StringOutputStream::ToString(fileDescription),
            ramses_internal::String("Filename: res/car.ramres ; Resource count 2: ") + verifyStream.c_str());
    }

    TEST_F(AResourceFileDescriptionTest, emptySetMultiResourceDescriptionUtilsTest)
    {
        ResourceFileDescriptionSet descSet;
        EXPECT_EQ(ramses_internal::StringOutputStream::ToString(descSet), ramses_internal::String("0 Resource File Descriptions: [  ]"));
    }

    TYPED_TEST(AResourceFileDescriptionTestTyped, multiEntrySetMultiResourceDescriptionUtilsTest)
    {
        ramses_internal::String firstPath("/home/carit/res/building.ramres");
        ramses_internal::String secondPath("res/car.ramres");

        const TypeParam* firstRamsesObject = &this->template createObject<TypeParam>("a");
        const TypeParam* secondRamsesObject = &this->template createObject<TypeParam>("b");

        ResourceFileDescriptionSet descSet;
        ResourceFileDescription someFileDescription(firstPath.c_str());
        ResourceFileDescription otherFileDescription(secondPath.c_str());

        someFileDescription.add(firstRamsesObject);
        someFileDescription.add(secondRamsesObject);

        descSet.add(someFileDescription);
        descSet.add(otherFileDescription);

        ramses_internal::StringOutputStream verifyStream;
        verifyStream << "[ " << firstRamsesObject->getResourceId() << " ; " << secondRamsesObject->getResourceId() << " ]";

        EXPECT_EQ(ramses_internal::StringOutputStream::ToString(descSet),
            ramses_internal::String("2 Resource File Descriptions: [ Filename: /home/carit/res/building.ramres ; Resource count 2: ")
                    + verifyStream.c_str() + " ; Filename: res/car.ramres ; Resource count 0: [  ] ]");
    }

    TYPED_TEST(AResourceFileDescriptionTestTyped, DuplicateResourcesAreNotAddedTwice)
    {
        const TypeParam* firstRamsesObject = &this->template createObject<TypeParam>();
        const TypeParam* secondRamsesObject = &this->template createObject<TypeParam>();

        ResourceFileDescription fileDescription("res/car.ramres");

        fileDescription.add(firstRamsesObject);
        fileDescription.add(secondRamsesObject);

        EXPECT_EQ(1u, fileDescription.getNumberOfResources());
    }
}
