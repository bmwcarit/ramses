//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "Animation/ActionCollectingAnimationSystem.h"
#include "Scene/Scene.h"
#include "Scene/SceneDataBinding.h"
#include "Scene/SceneActionCollection.h"

using namespace testing;

namespace ramses_internal
{
    class AnActionCollectingAnimationSystem : public testing::Test
    {
    public:
        AnActionCollectingAnimationSystem()
            : m_animSystem(EAnimationSystemFlags_FullProcessing, m_actionCollection, AnimationSystemSizeInformation())
        {
            m_animSystem.setHandle(AnimationSystemHandle(0));
        }

        SceneId                         m_sceneId;
        SceneActionCollection           m_actionCollection;
        ActionCollectingAnimationSystem m_animSystem;

        void expectAction(uint32_t count = 1u)
        {
            EXPECT_EQ(count, m_actionCollection.numberOfActions());
            m_actionCollection.clear();
        }

        SplineHandle createSpline()
        {
            const SplineHandle handle = m_animSystem.allocateSpline(ESplineKeyType_Basic, EDataTypeID_Vector4f);
            expectAction();
            return handle;
        }

        DataBindHandle createDataBind()
        {
            Scene scene;
            static const TDataBindID dataBindID = DataBindContainerToTraitsSelector<IScene>::ContainerTraitsClassType::DataField_Vector4f;
            const DataBindHandle handle = m_animSystem.allocateDataBinding(scene, dataBindID, 1u, 2u);
            expectAction();
            return handle;
        }

        AnimationInstanceHandle createAnimationInstance()
        {
            const SplineHandle splineHandle = createSpline();
            const AnimationInstanceHandle handle = m_animSystem.allocateAnimationInstance(splineHandle, EInterpolationType_Linear, EVectorComponent_All);
            expectAction();
            return handle;
        }

        AnimationHandle createAnimation()
        {
            const AnimationInstanceHandle animInstHandle = createAnimationInstance();
            const AnimationHandle handle = m_animSystem.allocateAnimation(animInstHandle);
            expectAction();
            return handle;
        }
    };

    TEST_F(AnActionCollectingAnimationSystem, onSetTime)
    {
        const AnimationTime globalTime(999);
        m_animSystem.setTime(globalTime);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, allocateSpline)
    {
        createSpline();
    }

    TEST_F(AnActionCollectingAnimationSystem, onAllocateDataBinding)
    {
        createDataBind();
    }

    TEST_F(AnActionCollectingAnimationSystem, onAllocateAnimationInstance)
    {
        createAnimationInstance();
    }

    TEST_F(AnActionCollectingAnimationSystem, onAllocateAnimation)
    {
        createAnimation();
    }

    TEST_F(AnActionCollectingAnimationSystem, onAddDataBindingToAnimationInstance)
    {
        const AnimationInstanceHandle animInstHandle = createAnimationInstance();
        const DataBindHandle dataBindHandle = createDataBind();

        m_animSystem.addDataBindingToAnimationInstance(animInstHandle, dataBindHandle);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, setSplineKeyBasicVector4f)
    {
        const SplineHandle handle = createSpline();

        const SplineTimeStamp timeStamp = 10u;
        const Vector4 val(1, 2, 3, 4);
        m_animSystem.setSplineKeyBasicVector4f(handle, timeStamp, val);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, removeSplineKey)
    {
        const SplineHandle handle = createSpline();

        const SplineTimeStamp timeStamp = 10u;
        const Vector4 val(11.f, 22.f, 33.f, 44.f);
        m_animSystem.setSplineKeyBasicVector4f(handle, timeStamp, val);
        m_animSystem.setSplineKeyBasicVector4f(handle, timeStamp * 2, val);
        expectAction(2u);

        m_animSystem.removeSplineKey(handle, 0u);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, onSetAnimationStartTime)
    {
        const AnimationHandle animHandle = createAnimation();
        const AnimationTime startTime(100u);
        m_animSystem.setAnimationStartTime(animHandle, startTime);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, onSetAnimationStopTime)
    {
        const AnimationHandle animHandle = createAnimation();
        const AnimationTime stopTime(100u);
        m_animSystem.setAnimationStopTime(animHandle, stopTime);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, onSetAnimationProperties)
    {
        const AnimationHandle animHandle = createAnimation();
        m_animSystem.setAnimationProperties(animHandle, 3.f, 99u, 66u, 1000u);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, onStopAnimationAndRollback)
    {
        const AnimationHandle animHandle = createAnimation();
        m_animSystem.stopAnimationAndRollback(animHandle);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, removeDataBinding)
    {
        const DataBindHandle dataBindHandle = createDataBind();
        m_animSystem.removeDataBinding(dataBindHandle);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, removeSpline)
    {
        const SplineHandle handle = createSpline();
        m_animSystem.removeSpline(handle);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, removeAnimationInstance)
    {
        const AnimationInstanceHandle handle = createAnimationInstance();
        m_animSystem.removeAnimationInstance(handle);
        expectAction();
    }

    TEST_F(AnActionCollectingAnimationSystem, removeAnimation)
    {
        const AnimationHandle handle = createAnimation();
        m_animSystem.removeAnimation(handle);
        expectAction();
    }
}
