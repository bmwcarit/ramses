//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONDATATEST_H
#define RAMSES_ANIMATIONDATATEST_H

#include "framework_common_gmock_header.h"
#include "Animation/AnimationData.h"
#include "Animation/AnimationLogic.h"
#include "SplineTestUtils.h"
#include "AnimationDataBindTestUtils.h"
#include "AnimationTestTypes.h"

namespace ramses_internal
{
    class AnimationDataTest : public testing::Test
    {
    public:
        AnimationDataTest()
            : m_dataBind(m_container, 0, EDataBindAccessorType_Handles_1)
        {
            m_animationData.addListener(&m_dataListener);
        }

        template <typename EDataType>
        void testAnimationManagerDataBindingTypes(MemoryHandle handle, const EDataType constainerInitVal, const EDataType setVal)
        {
            typedef AnimationDataBind<AnimationDataBindTestContainer<EDataType>, EDataType, MemoryHandle> DataBindTypeLocal;
            AnimationDataBindTestContainer<EDataType> container;
            DataBindTypeLocal dataBind(container, handle, EDataBindAccessorType_Handles_1);

            const DataBindHandle dataBindHandle = m_animationData.allocateDataBinding(dataBind);
            container.setVal1(handle, constainerInitVal);
            const DataBindTypeLocal& dataBindAcquired = *(static_cast<const DataBindTypeLocal*>(m_animationData.getDataBinding(dataBindHandle)));
            EXPECT_EQ(container.getVal1(handle), dataBindAcquired.getValue());

            const_cast<DataBindTypeLocal&>(dataBindAcquired).setValue(setVal);
            EXPECT_EQ(container.getVal1(handle), dataBindAcquired.getValue());
        }

    protected:
        SplineVec3 m_spline;
        ContainerType m_container;
        DataBindType m_dataBind;

        AnimationData m_animationData;
        testing::NiceMock<MockAnimationDataListener> m_dataListener;
    };
}

#endif
