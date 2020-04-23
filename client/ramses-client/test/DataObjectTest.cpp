//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/DataMatrix22f.h"
#include "ramses-client-api/DataMatrix33f.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector3i.h"
#include "ramses-client-api/DataVector4i.h"
#include "ramses-utils.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    class ADataObject : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    protected:
        template <typename T>
        void checkConversion(T& dataObj)
        {
            DataObject* baseObj = RamsesUtils::TryConvert<DataObject>(dataObj);
            ASSERT_TRUE(baseObj != nullptr);
            T* concreteObj = RamsesUtils::TryConvert<T>(*baseObj);
            EXPECT_TRUE(concreteObj != nullptr);
            EXPECT_EQ(&dataObj, concreteObj);

            const T& constDataObj = dataObj;
            const DataObject* constBaseObj = RamsesUtils::TryConvert<DataObject>(constDataObj);
            ASSERT_TRUE(constBaseObj != nullptr);
            const T* constConcreteObj = RamsesUtils::TryConvert<T>(*constBaseObj);
            EXPECT_TRUE(constConcreteObj != nullptr);
            EXPECT_EQ(&constDataObj, constConcreteObj);
        }
    };


    // DataFloat

    TEST_F(ADataObject, isInitializedOnCreationOfDataFloat)
    {
        DataFloat* data = m_scene.createDataFloat("dataFloat");

        float getVal = 1.5f;
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        EXPECT_EQ(0.0f, getVal);
    }

    TEST_F(ADataObject, canSetAndGetAFloatValue)
    {
        DataFloat* data = m_scene.createDataFloat("dataFloat");
        float setVal = 3.1415f;
        ASSERT_EQ(StatusOK, data->setValue(setVal));

        float getVal = 0.0f;
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        EXPECT_EQ(setVal, getVal);
    }

    TEST_F(ADataObject, canConvertDataFloatToDataObjectAndBack)
    {
        DataFloat* data = m_scene.createDataFloat("dataFloat");
        DataObject* dataObj = RamsesUtils::TryConvert<DataObject>(*data);
        EXPECT_TRUE(dataObj);
        data = RamsesUtils::TryConvert<DataFloat>(*dataObj);
        EXPECT_TRUE(data);
    }

    // DataVector2f

    TEST_F(ADataObject, isInitializedOnCreationOfDataVector2f)
    {
        DataVector2f* data = m_scene.createDataVector2f("dataVector2f");

        float getVal[] = { 3.1415f, 1.732f };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1]));
        EXPECT_EQ(0.0f, getVal[0]);
        EXPECT_EQ(0.0f, getVal[1]);
    }

    TEST_F(ADataObject, canSetAndGetAVector2fValue)
    {
        DataVector2f* data = m_scene.createDataVector2f("dataVector2f");
        float setVal[] = { 3.1415f, 1.732f };
        ASSERT_EQ(StatusOK, data->setValue(setVal[0], setVal[1]));

        float getVal[] = { 0.0f, 0.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1]));
        EXPECT_EQ(setVal[0], getVal[0]);
        EXPECT_EQ(setVal[1], getVal[1]);
    }

    TEST_F(ADataObject, canConvertDataVector2fToDataObjectAndBack)
    {
        DataVector2f* data = m_scene.createDataVector2f("dataVector2f");
        this->checkConversion(*data);
    }

    // DataVector3f

    TEST_F(ADataObject, isInitializedOnCreationOfDataVector3f)
    {
        DataVector3f* data = m_scene.createDataVector3f("dataVector3f");

        float getVal[] = { 3.1415f, 1.732f, 2.71828f };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1], getVal[2]));
        EXPECT_EQ(0.0f, getVal[0]);
        EXPECT_EQ(0.0f, getVal[1]);
        EXPECT_EQ(0.0f, getVal[2]);
    }

    TEST_F(ADataObject, canSetAndGetAVector3fValue)
    {
        DataVector3f* data = m_scene.createDataVector3f("dataVector3f");
        float setVal[] = { 3.1415f, 1.732f, 2.71828f };
        ASSERT_EQ(StatusOK, data->setValue(setVal[0], setVal[1], setVal[2]));

        float getVal[] = { 0.0f, 0.0f, 0.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1], getVal[2]));
        EXPECT_EQ(setVal[0], getVal[0]);
        EXPECT_EQ(setVal[1], getVal[1]);
        EXPECT_EQ(setVal[2], getVal[2]);
    }

    TEST_F(ADataObject, canConvertDataVector3fToDataObjectAndBack)
    {
        DataVector3f* data = m_scene.createDataVector3f("dataVector3f");
        this->checkConversion(*data);
    }

    // DataVector4f

    TEST_F(ADataObject, isInitializedOnCreationOfDataVector4f)
    {
        DataVector4f* data = m_scene.createDataVector4f("dataVector4f");

        float getVal[] = { 3.1415f, 1.732f, 2.71828f, 1.41429f };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1], getVal[2], getVal[3]));
        EXPECT_EQ(0.0f, getVal[0]);
        EXPECT_EQ(0.0f, getVal[1]);
        EXPECT_EQ(0.0f, getVal[2]);
        EXPECT_EQ(0.0f, getVal[3]);
    }

    TEST_F(ADataObject, canSetAndGetAVector4fValue)
    {
        DataVector4f* data = m_scene.createDataVector4f("dataVector4f");
        float setVal[] = { 3.1415f, 1.732f, 2.71828f, 1.41429f };
        ASSERT_EQ(StatusOK, data->setValue(setVal[0], setVal[1], setVal[2], setVal[3]));

        float getVal[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1], getVal[2], getVal[3]));
        EXPECT_EQ(setVal[0], getVal[0]);
        EXPECT_EQ(setVal[1], getVal[1]);
        EXPECT_EQ(setVal[2], getVal[2]);
        EXPECT_EQ(setVal[3], getVal[3]);
    }

    TEST_F(ADataObject, canConvertDataVector4fToDataObjectAndBack)
    {
        DataVector4f* data = m_scene.createDataVector4f("dataVector4f");
        this->checkConversion(*data);
    }

    // DataMatrix22f

    TEST_F(ADataObject, isInitializedOnCreationOfDataMatrix22f)
    {
        DataMatrix22f* data = m_scene.createDataMatrix22f("dataMatrix22f");

        float getVal[] = { 1.0f, 2.0f, 3.0f, 4.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        for (uint32_t i = 0; i < 4u; i++)
        {
            EXPECT_EQ(0.0f, getVal[i]);
        }
    }

    TEST_F(ADataObject, canSetAndGetAMatrix22fValue)
    {
        DataMatrix22f* data = m_scene.createDataMatrix22f("dataMatrix22f");
        const float setVal[] = { 1.0f, 2.0f, 3.0f, 4.0f };
        ASSERT_EQ(StatusOK, data->setValue(setVal));

        float getVal[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        for (uint32_t i = 0; i < 4u; i++)
        {
            EXPECT_EQ(setVal[i], getVal[i]);
        }
    }

    TEST_F(ADataObject, canConvertDataMatrix22fToDataObjectAndBack)
    {
        DataMatrix22f* data = m_scene.createDataMatrix22f("dataMatrix22f");
        this->checkConversion(*data);
    }

    // DataMatrix33f

    TEST_F(ADataObject, isInitializedOnCreationOfDataMatrix33f)
    {
        DataMatrix33f* data = m_scene.createDataMatrix33f("dataMatrix33f");

        float getVal[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        for (uint32_t i = 0; i < 9u; i++)
        {
            EXPECT_EQ(0.0f, getVal[i]);
        }
    }

    TEST_F(ADataObject, canSetAndGetAMatrix33fValue)
    {
        DataMatrix33f* data = m_scene.createDataMatrix33f("dataMatrix33f");
        const float setVal[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };
        ASSERT_EQ(StatusOK, data->setValue(setVal));

        float getVal[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        for (uint32_t i = 0; i < 9u; i++)
        {
            EXPECT_EQ(setVal[i], getVal[i]);
        }
    }

    TEST_F(ADataObject, canConvertDataMatrix33fToDataObjectAndBack)
    {
        DataMatrix33f* data = m_scene.createDataMatrix33f("dataMatrix33f");
        this->checkConversion(*data);
    }

    // DataMatrix44f

    TEST_F(ADataObject, isInitializedOnCreationOfDataMatrix44f)
    {
        DataMatrix44f* data = m_scene.createDataMatrix44f("dataMatrix44f");

        float getVal[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        for (uint32_t i = 0; i < 16; i++)
        {
            EXPECT_EQ(0.0f, getVal[i]);
        }
    }

    TEST_F(ADataObject, canSetAndGetAMatrix44fValue)
    {
        DataMatrix44f* data = m_scene.createDataMatrix44f("dataMatrix44f");
        float setVal[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };
        ASSERT_EQ(StatusOK, data->setValue(setVal));

        float getVal[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        for (uint32_t i = 0; i < 16; i++)
        {
            EXPECT_EQ(setVal[i], getVal[i]);
        }
    }

    TEST_F(ADataObject, canConvertDataMatrix44fToDataObjectAndBack)
    {
        DataMatrix44f* data = m_scene.createDataMatrix44f("dataMatrix44f");
        this->checkConversion(*data);
    }

    // DataInt32

    TEST_F(ADataObject, isInitializedOnCreationOfDataInt32)
    {
        DataInt32* data = m_scene.createDataInt32("dataInt32");

        int32_t getVal = 1;
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        EXPECT_EQ(0.0f, getVal);
    }

    TEST_F(ADataObject, canSetAndGetAInt32Value)
    {
        DataInt32* data = m_scene.createDataInt32("dataInt32");
        int32_t setVal = 3;
        ASSERT_EQ(StatusOK, data->setValue(setVal));

        int32_t getVal = 0;
        ASSERT_EQ(StatusOK, data->getValue(getVal));
        EXPECT_EQ(setVal, getVal);
    }

    TEST_F(ADataObject, canConvertDataInt32ToDataObjectAndBack)
    {
        DataInt32* data = m_scene.createDataInt32("dataInt32");
        this->checkConversion(*data);
    }

    // DataVector2i

    TEST_F(ADataObject, isInitializedOnCreationOfDataVector2i)
    {
        DataVector2i* data = m_scene.createDataVector2i("dataVector2i");

        int32_t getVal[] = { 3, 1 };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1]));
        EXPECT_EQ(0.0f, getVal[0]);
        EXPECT_EQ(0.0f, getVal[1]);
    }

    TEST_F(ADataObject, canSetAndGetAVector2iValue)
    {
        DataVector2i* data = m_scene.createDataVector2i("dataVector2i");
        int32_t setVal[] = { 3, 2 };
        ASSERT_EQ(StatusOK, data->setValue(setVal[0], setVal[1]));

        int32_t getVal[] = { 0, 0 };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1]));
        EXPECT_EQ(setVal[0], getVal[0]);
        EXPECT_EQ(setVal[1], getVal[1]);
    }

    TEST_F(ADataObject, canConvertDataVector2iToDataObjectAndBack)
    {
        DataVector2i* data = m_scene.createDataVector2i("dataVector2i");
        this->checkConversion(*data);
    }

    // DataVector3i

    TEST_F(ADataObject, isInitializedOnCreationOfDataVector3i)
    {
        DataVector3i* data = m_scene.createDataVector3i("dataVector3i");

        int32_t getVal[] = { 1, 2, 3 };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1], getVal[2]));
        EXPECT_EQ(0.0f, getVal[0]);
        EXPECT_EQ(0.0f, getVal[1]);
        EXPECT_EQ(0.0f, getVal[2]);
    }

    TEST_F(ADataObject, canSetAndGetAVector3iValue)
    {
        DataVector3i* data = m_scene.createDataVector3i("dataVector3i");
        int32_t setVal[] = { 1, 2, 3 };
        ASSERT_EQ(StatusOK, data->setValue(setVal[0], setVal[1], setVal[2]));

        int32_t getVal[] = { 0, 0, 0 };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1], getVal[2]));
        EXPECT_EQ(setVal[0], getVal[0]);
        EXPECT_EQ(setVal[1], getVal[1]);
        EXPECT_EQ(setVal[2], getVal[2]);
    }

    TEST_F(ADataObject, canConvertDataVector3iToDataObjectAndBack)
    {
        DataVector3i* data = m_scene.createDataVector3i("dataVector3i");
        this->checkConversion(*data);
    }

    // DataVector4i

    TEST_F(ADataObject, isInitializedOnCreationOfDataVector4i)
    {
        DataVector4i* data = m_scene.createDataVector4i("dataVector4i");

        int32_t getVal[] = { 1, 2, 3, 4 };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1], getVal[2], getVal[3]));
        EXPECT_EQ(0.0f, getVal[0]);
        EXPECT_EQ(0.0f, getVal[1]);
        EXPECT_EQ(0.0f, getVal[2]);
        EXPECT_EQ(0.0f, getVal[3]);
    }

    TEST_F(ADataObject, canSetAndGetAVector4iValue)
    {
        DataVector4i* data = m_scene.createDataVector4i("dataVector4i");
        int32_t setVal[] = { 1, 2, 3, 4 };
        ASSERT_EQ(StatusOK, data->setValue(setVal[0], setVal[1], setVal[2], setVal[3]));

        int32_t getVal[] = { 0, 0, 0, 0 };
        ASSERT_EQ(StatusOK, data->getValue(getVal[0], getVal[1], getVal[2], getVal[3]));
        EXPECT_EQ(setVal[0], getVal[0]);
        EXPECT_EQ(setVal[1], getVal[1]);
        EXPECT_EQ(setVal[2], getVal[2]);
        EXPECT_EQ(setVal[3], getVal[3]);
    }

    TEST_F(ADataObject, canConvertDataVector4iToDataObjectAndBack)
    {
        DataVector4i* data = m_scene.createDataVector4i("dataVector4i");
        this->checkConversion(*data);
    }
}
