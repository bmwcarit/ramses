//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ramses-client-api/RamsesObject.h"
#include "RamsesObjectImpl.h"
#include "SerializationHelper.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "Collections/StringOutputStream.h"
#include "Collections/Vector.h"
#include "Collections/HashSet.h"
#include "ClientTestUtils.h"
#include "ramses-capu/os/StringUtils.h"

#include <cstdio>

namespace ramses
{
    ::testing::AssertionResult AssertArraysEqual( const char *m, const char *n, uint32_t size)
    {
        char mHexValue[5];
        char nHexValue[5];

        for(uint32_t i = 0; i < size; ++i)
        {
            if(m[i] != n[i])
            {
                ramses_capu::StringUtils::Sprintf(mHexValue, 5, "0x%02x", m[i]);
                ramses_capu::StringUtils::Sprintf(nHexValue, 5, "0x%02x", n[i]);
                return ::testing::AssertionFailure()
                    << "Arrays differ at position " << i << ": "
                    << mHexValue << " != " << nHexValue << "\n";
            }
        }

        return ::testing::AssertionSuccess();
    }

    class RamsesObjectImplDummy : public RamsesObjectImpl
    {
    public:
        RamsesObjectImplDummy(ERamsesObjectType type, const char* name)
            : RamsesObjectImpl(type, name)
        {
        }

        virtual void deinitializeFrameworkData() final
        {
        }
    };

    class RamsesObjectDummy : public RamsesObject
    {
    public:
        RamsesObjectDummy(RamsesObjectImpl& _impl)
            : RamsesObject(_impl)
            , impl(static_cast<RamsesObjectImplDummy&>(_impl))
        {
        }

        RamsesObjectImplDummy& impl;
    };

    class SerializationHelperTestBase : public testing::Test
    {
    public:
        SerializationHelperTestBase()
        {
            m_impls[0] = new RamsesObjectImplDummy(ERamsesObjectType_RamsesObject, "one");
            m_impls[1] = new RamsesObjectImplDummy(ERamsesObjectType_RamsesObject, "two");
            m_impls[2] = new RamsesObjectImplDummy(ERamsesObjectType_RamsesObject, "three");
            m_impls[3] = new RamsesObjectImplDummy(ERamsesObjectType_RamsesObject, "four");

            m_objects[0] = new RamsesObjectDummy(*m_impls[0]);
            m_objects[1] = new RamsesObjectDummy(*m_impls[1]);
            m_objects[2] = new RamsesObjectDummy(*m_impls[2]);
            m_objects[3] = new RamsesObjectDummy(*m_impls[3]);
        }

        virtual ~SerializationHelperTestBase()
        {
            delete static_cast<RamsesObjectDummy*>(m_objects[0]);
            delete static_cast<RamsesObjectDummy*>(m_objects[1]);
            delete static_cast<RamsesObjectDummy*>(m_objects[2]);
            delete static_cast<RamsesObjectDummy*>(m_objects[3]);
        }

    protected:

        static const char* firstElementSerialString;

        RamsesObject*                       m_objects[4];
        RamsesObjectImpl*                   m_impls[4];
        SerializationContext                m_serializationContext;
        DeserializationContext              m_deserializationContext;
        ramses_internal::BinaryOutputStream m_outStream;
    };

    const char* SerializationHelperTestBase::firstElementSerialString = "\x01\x00\x00\x00\x01\x00\x00\x00";


    template<typename T >
    class SerializationHelperObjectTest : public SerializationHelperTestBase
    {
    protected:
        T m_originalContainer;
        T m_deserializationContainer;
    };

    template<typename T >
    class SerializationHelperImplTest : public SerializationHelperTestBase
    {
    protected:
        T m_originalContainer;
        T m_deserializationContainer;
    };

    typedef ::testing::Types< std::vector<RamsesObject*>, ramses_internal::HashSet<RamsesObject*> > ContainerObjectTypes;
    TYPED_TEST_CASE(SerializationHelperObjectTest, ContainerObjectTypes);

    typedef ::testing::Types< std::vector<RamsesObjectImpl*>, ramses_internal::HashSet<RamsesObjectImpl*> > ContainerImplTypes;
    TYPED_TEST_CASE(SerializationHelperImplTest, ContainerImplTypes);

    TYPED_TEST(SerializationHelperObjectTest, CanGetCount)
    {
        EXPECT_EQ( 0u, SerializationHelper::GetContainerSize(this->m_originalContainer) );
        SerializationHelper::AddElement(this->m_originalContainer, this->m_objects[0]);
        EXPECT_EQ( 1u, SerializationHelper::GetContainerSize(this->m_originalContainer) );
        SerializationHelper::AddElement(this->m_originalContainer, this->m_objects[1]);
        EXPECT_EQ( 2u, SerializationHelper::GetContainerSize(this->m_originalContainer) );
    }

    TYPED_TEST(SerializationHelperImplTest, CanGetCount)
    {
        EXPECT_EQ( 0u, SerializationHelper::GetContainerSize(this->m_originalContainer) );
        SerializationHelper::AddElement(this->m_originalContainer, this->m_impls[0]);
        EXPECT_EQ( 1u, SerializationHelper::GetContainerSize(this->m_originalContainer) );
        SerializationHelper::AddElement(this->m_originalContainer, this->m_impls[1]);
        EXPECT_EQ( 2u, SerializationHelper::GetContainerSize(this->m_originalContainer) );
    }

    TYPED_TEST(SerializationHelperObjectTest, CanSerializeContainer)
    {
        SerializationHelper::AddElement(this->m_originalContainer, this->m_objects[0]);

        SerializationHelper::SerializeContainerIDs(this->m_outStream, this->m_serializationContext, this->m_originalContainer);

        const char* data = this->m_outStream.getData();
        uint32_t size    = this->m_outStream.getSize();

        EXPECT_TRUE(AssertArraysEqual(SerializationHelperTestBase::firstElementSerialString, data, size));
    }

    TYPED_TEST(SerializationHelperImplTest, CanSerializeContainer)
    {
        SerializationHelper::AddElement(this->m_originalContainer, this->m_impls[0]);

        SerializationHelper::SerializeContainerImplIDs(this->m_outStream, this->m_serializationContext, this->m_originalContainer);

        const char* data = this->m_outStream.getData();
        uint32_t size    = this->m_outStream.getSize();

        EXPECT_TRUE(AssertArraysEqual(SerializationHelperTestBase::firstElementSerialString, data, size));
    }

    TYPED_TEST(SerializationHelperObjectTest, CanDeserializeIdsInContainer)
    {
        ramses_internal::BinaryInputStream inputStream(SerializationHelperTestBase::firstElementSerialString);

        SerializationHelper::ReadDependentPointerAndStoreIDInContainer(inputStream, this->m_deserializationContainer);

        ObjectIDType id = static_cast<ObjectIDType>(reinterpret_cast<size_t>(*this->m_deserializationContainer.begin()));

        EXPECT_EQ( 1u, id );
    }

    TYPED_TEST(SerializationHelperImplTest, CanDeserializeIdsInContainer)
    {
        ramses_internal::BinaryInputStream inputStream(SerializationHelperTestBase::firstElementSerialString);

        SerializationHelper::ReadDependentPointerAndStoreIDInContainer(inputStream, this->m_deserializationContainer);

        ObjectIDType id = static_cast<ObjectIDType>(reinterpret_cast<size_t>(*this->m_deserializationContainer.begin()));

        EXPECT_EQ( 1u, id );
    }
}
