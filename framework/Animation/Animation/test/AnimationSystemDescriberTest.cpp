//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "SceneAPI/IScene.h"
#include "Scene/Scene.h"
#include "Scene/SceneDataBinding.h"
#include "Scene/SceneActionApplierHelper.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "Animation/AnimationSystem.h"
#include "Animation/AnimationSystemDescriber.h"

using namespace testing;

namespace ramses_internal
{
    class AnAnimationSystemDescriber : public testing::TestWithParam<UInt64>
    {
    public:
        AnAnimationSystemDescriber()
            : m_animSystemSource(*new AnimationSystem(EAnimationSystemFlags_Default, AnimationSystemSizeInformation()))
            , m_animSystemTarget(*new AnimationSystem(EAnimationSystemFlags_Default, AnimationSystemSizeInformation()))
            , m_scene(*new Scene())
        {
        }

        virtual ~AnAnimationSystemDescriber()
        {
            delete &m_scene;
        }

        void copyAnimationSystem(AnimationSystemHandle handle)
        {
            SceneActionCollection collection;
            SceneActionCollectionCreator creator(collection);
            AnimationSystemDescriber::DescribeAnimationSystem(m_animSystemSource, creator, handle);
            SceneActionApplierHelper applierListener(m_scene);
            applierListener.applyActionsOnScene(collection);
        }

        IAnimationSystem& m_animSystemSource;
        IAnimationSystem& m_animSystemTarget;
        IScene& m_scene;
    };

    INSTANTIATE_TEST_SUITE_P(
        TestCopyAtDifferentTimeStamps,
        AnAnimationSystemDescriber,
        testing::Values(0u, 2000u, 5000u, 7000u, 10000u, 50000u, 99000u, 99500u, 100000u, 101000u));

    TEST_P(AnAnimationSystemDescriber, canDecomposeAnimationSystemIntoSceneActions)
    {
        m_scene.addAnimationSystem(&m_animSystemSource);

        const NodeHandle nodeHandle1 = m_scene.allocateNode(0, NodeHandle{ 1u });
        const NodeHandle nodeHandle2 = m_scene.allocateNode(0, NodeHandle{ 2u });
        const TransformHandle transHandle1 = m_scene.allocateTransform(nodeHandle1, TransformHandle{ 3u });
        const TransformHandle transHandle2 = m_scene.allocateTransform(nodeHandle2, TransformHandle{ 4u });

        const SplineHandle splineHandle1 = m_animSystemSource.allocateSpline(ESplineKeyType_Basic, EDataTypeID_Vector3f, SplineHandle{ 5u });
        const SplineHandle splineHandle2 = m_animSystemSource.allocateSpline(ESplineKeyType_Tangents, EDataTypeID_Float, SplineHandle{ 6u });
        m_animSystemSource.setSplineKeyBasicVector3f(splineHandle1, 99u, Vector3(111.f, -999.f, 66.f));
        m_animSystemSource.setSplineKeyBasicVector3f(splineHandle1, 199u, Vector3(11.f, -99.f, 666.f));
        m_animSystemSource.setSplineKeyTangentsFloat(splineHandle2, 11u, 55.f, Vector2(111.f, -999.f), Vector2(11.f, -99.f));
        m_animSystemSource.setSplineKeyTangentsFloat(splineHandle2, 99u, -88.f, Vector2(11.f, -99.f), Vector2(111.f, -999.f));

        using ContainerTraitsClass = DataBindContainerToTraitsSelector<IScene>::ContainerTraitsClassType;
        const DataBindHandle dataBindHandle1 = m_animSystemSource.allocateDataBinding(m_scene, ContainerTraitsClass::TransformNode_Rotation, transHandle1.asMemoryHandle(), InvalidMemoryHandle, DataBindHandle{ 7u });
        const DataBindHandle dataBindHandle2 = m_animSystemSource.allocateDataBinding(m_scene, ContainerTraitsClass::TransformNode_Translation, transHandle2.asMemoryHandle(), InvalidMemoryHandle, DataBindHandle{ 8u });

        const AnimationInstanceHandle animInstHandle1 = m_animSystemSource.allocateAnimationInstance(splineHandle1, EInterpolationType_Linear, EVectorComponent_All, AnimationInstanceHandle{ 9u });
        const AnimationInstanceHandle animInstHandle2 = m_animSystemSource.allocateAnimationInstance(splineHandle2, EInterpolationType_Bezier, EVectorComponent_Y, AnimationInstanceHandle{ 10u });
        m_animSystemSource.addDataBindingToAnimationInstance(animInstHandle1, dataBindHandle1);
        m_animSystemSource.addDataBindingToAnimationInstance(animInstHandle2, dataBindHandle2);

        const AnimationHandle animHandle1 = m_animSystemSource.allocateAnimation(animInstHandle1, AnimationHandle{ 11u });
        const AnimationHandle animHandle2 = m_animSystemSource.allocateAnimation(animInstHandle2, AnimationHandle{ 12u });
        m_animSystemSource.setAnimationProperties(animHandle1, 2.f, Animation::EAnimationFlags_Relative, 0u, 0u);
        m_animSystemSource.setAnimationProperties(animHandle2, 99.f, Animation::EAnimationFlags_Looping, 1000u, 0u);
        m_animSystemSource.setAnimationStartTime(animHandle1, 5000u);
        m_animSystemSource.setAnimationStartTime(animHandle2, 99000u);
        m_animSystemSource.setAnimationStopTime(animHandle1, 10000u);
        m_animSystemSource.setAnimationStopTime(animHandle2, 100000u);

        m_animSystemSource.setTime(GetParam());

        auto hdl = m_scene.addAnimationSystem(&m_animSystemTarget);
        copyAnimationSystem(hdl);

        for (SplineHandle handle(0u); handle < m_animSystemSource.getTotalSplineCount(); ++handle)
        {
            EXPECT_EQ(m_animSystemSource.containsSpline(handle), m_animSystemTarget.containsSpline(handle));
            if (m_animSystemSource.containsSpline(handle))
            {
                const SplineBase* const splineSrc = m_animSystemSource.getSpline(handle);
                const SplineBase* const splineTgt = m_animSystemTarget.getSpline(handle);
                EXPECT_EQ(splineSrc->getKeyType(), splineTgt->getKeyType());
                EXPECT_EQ(splineSrc->getDataType(), splineTgt->getDataType());
                EXPECT_EQ(splineSrc->getNumKeys(), splineTgt->getNumKeys());
                for (UInt32 k = 0u; k < splineSrc->getNumKeys(); ++k)
                {
                    EXPECT_EQ(splineSrc->getTimeStamp(k), splineTgt->getTimeStamp(k));
                }
            }
        }

        for (DataBindHandle handle(0u); handle < m_animSystemSource.getTotalDataBindCount(); ++handle)
        {
            EXPECT_EQ(m_animSystemSource.containsDataBinding(handle), m_animSystemTarget.containsDataBinding(handle));
            if (m_animSystemSource.containsDataBinding(handle))
            {
                const AnimationDataBindBase* const dataBindSrc = m_animSystemSource.getDataBinding(handle);
                const AnimationDataBindBase* const dataBindTgt = m_animSystemTarget.getDataBinding(handle);
                EXPECT_EQ(dataBindSrc->getContainerType(), dataBindTgt->getContainerType());
                EXPECT_EQ(dataBindSrc->getAccessorType(), dataBindTgt->getAccessorType());
                EXPECT_EQ(dataBindSrc->getDataType(), dataBindTgt->getDataType());
                EXPECT_EQ(dataBindSrc->getBindID(), dataBindTgt->getBindID());
                EXPECT_EQ(dataBindSrc->getHandle(), dataBindTgt->getHandle());
                EXPECT_EQ(dataBindSrc->getHandle2(), dataBindTgt->getHandle2());
            }
        }

        for (AnimationInstanceHandle handle(0u); handle < m_animSystemSource.getTotalAnimationInstanceCount(); ++handle)
        {
            EXPECT_EQ(m_animSystemSource.containsAnimationInstance(handle), m_animSystemTarget.containsAnimationInstance(handle));
            if (m_animSystemSource.containsAnimationInstance(handle))
            {
                const AnimationInstance& animInstSrc = m_animSystemSource.getAnimationInstance(handle);
                const AnimationInstance& animInstTgt = m_animSystemTarget.getAnimationInstance(handle);
                EXPECT_EQ(animInstSrc.getSplineHandle(), animInstTgt.getSplineHandle());
                EXPECT_EQ(animInstSrc.getInterpolationType(), animInstTgt.getInterpolationType());
                EXPECT_EQ(animInstSrc.getVectorComponentFlag(), animInstTgt.getVectorComponentFlag());

                const DataBindHandleVector& dataBindsSrc = animInstSrc.getDataBindings();
                const DataBindHandleVector& dataBindsTgt = animInstTgt.getDataBindings();
                EXPECT_EQ(dataBindsSrc.size(), dataBindsTgt.size());
                for (UInt db = 0u; db < dataBindsSrc.size(); ++db)
                {
                    EXPECT_EQ(dataBindsSrc[db], dataBindsTgt[db]);
                }
            }
        }

        for (AnimationHandle handle(0u); handle < m_animSystemSource.getTotalAnimationCount(); ++handle)
        {
            EXPECT_EQ(m_animSystemSource.containsAnimation(handle), m_animSystemTarget.containsAnimation(handle));
            if (m_animSystemSource.containsAnimation(handle))
            {
                const Animation& animSrc = m_animSystemSource.getAnimation(handle);
                const Animation& animTgt = m_animSystemTarget.getAnimation(handle);
                EXPECT_EQ(animSrc.m_animationInstanceHandle, animTgt.m_animationInstanceHandle);
                EXPECT_EQ(animSrc.m_flags, animTgt.m_flags);
                EXPECT_EQ(animSrc.m_loopDuration, animTgt.m_loopDuration);
                EXPECT_EQ(animSrc.m_paused, animTgt.m_paused);
                EXPECT_EQ(animSrc.m_playbackSpeed, animTgt.m_playbackSpeed);
                EXPECT_EQ(animSrc.m_startTime, animTgt.m_startTime);
                EXPECT_EQ(animSrc.m_stopTime, animTgt.m_stopTime);
            }
        }

        EXPECT_EQ(m_animSystemSource.getTime(), m_animSystemTarget.getTime());
    }
}
