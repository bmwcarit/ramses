//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/AnimatedSetter.h"
#include "ramses-client-api/AnimatedProperty.h"
#include "AnimationSystemImpl.h"
#include "AnimationAPI/IAnimationSystem.h"
#include "Animation/Spline.h"
#include "Animation/SplineKey.h"
#include "Scene/SceneDataBinding.h"
#include "Scene/Scene.h"

// for templated code instantiated internally
#include "AnimatedSetterImpl.cpp"

using namespace testing;

namespace ramses
{
    class AnimatedSetterTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
        virtual void SetUp()
        {
            m_animatedNode = this->m_scene.createNode();
            m_animatedNode->setTranslation(10.f, 50.f, -100.f);
            m_animPropertyFloat = this->animationSystem.createAnimatedProperty(*m_animatedNode, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X, "prop");
            m_animatedSetterFloat = this->animationSystem.createAnimatedSetter(*m_animPropertyFloat, "animatedSetter");
            m_animPropertyVec3f = this->animationSystem.createAnimatedProperty(*m_animatedNode, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All, "prop");
            m_animatedSetterVec3f = this->animationSystem.createAnimatedSetter(*m_animPropertyVec3f, "animatedSetter");

            setterDelay = this->animationSystem.impl.getDelayForAnimatedSetters();
            setCurrentTime(9999u);
        }

    protected:
        typedef ramses_internal::Spline<ramses_internal::SplineKey, ramses_internal::Vector3> SplineType;

        const SplineType& getInternalSpline() const
        {
            const ramses_internal::SplineHandle splineHandle = m_animatedSetterVec3f->impl.getSplineHandle();
            const ramses_internal::SplineBase* const splineBase = this->animationSystem.impl.getIAnimationSystem().getSpline(splineHandle);
            return static_cast<const SplineType&>(*splineBase);
        }

        uint32_t getNumSplineKeys() const
        {
            return getInternalSpline().getNumKeys();
        }

        splineTimeStamp_t getSplineKeyTimeStamp(uint32_t key) const
        {
            return getInternalSpline().getTimeStamp(key);
        }

        globalTimeStamp_t getStartTime(const AnimatedSetter& setter) const
        {
            const ramses::AnimationImpl& animation = setter.impl.getAnimation();
            return animation.getStartTime();
        }

        globalTimeStamp_t getStopTime(const AnimatedSetter& setter) const
        {
            const ramses::AnimationImpl& animation = setter.impl.getAnimation();
            return animation.getStopTime();
        }

        globalTimeStamp_t getCurrentTime() const
        {
            return this->animationSystem.getTime();
        }

        void setCurrentTime(globalTimeStamp_t time)
        {
            this->animationSystem.setTime(time);
        }

        void expectValue(const ramses_internal::Vector3& value)
        {
            ramses_internal::Vector3 translation;
            EXPECT_EQ(StatusOK, m_animatedNode->getTranslation(translation.x, translation.y, translation.z));
            EXPECT_FLOAT_EQ(value.x, translation.x);
            EXPECT_FLOAT_EQ(value.y, translation.y);
            EXPECT_FLOAT_EQ(value.z, translation.z);
        }

        Node* m_animatedNode;
        AnimatedProperty* m_animPropertyFloat;
        AnimatedSetter* m_animatedSetterFloat;
        AnimatedProperty* m_animPropertyVec3f;
        AnimatedSetter* m_animatedSetterVec3f;

        timeMilliseconds_t setterDelay;
    };

    TEST_F(AnimatedSetterTest, initialValues)
    {
        EXPECT_EQ(0u, getNumSplineKeys());
        const globalTimeStamp_t invalidTimeStamp = ramses_internal::AnimationTime::InvalidTimeStamp;
        EXPECT_EQ(invalidTimeStamp, getStartTime(*m_animatedSetterFloat));
        EXPECT_EQ(invalidTimeStamp, getStartTime(*m_animatedSetterVec3f));
        EXPECT_EQ(invalidTimeStamp, getStopTime(*m_animatedSetterFloat));
        EXPECT_EQ(invalidTimeStamp, getStopTime(*m_animatedSetterVec3f));
    }

    TEST_F(AnimatedSetterTest, failsToSetValueOfInvalidType)
    {
        EXPECT_NE(StatusOK, m_animatedSetterFloat->setValue(1.f, 2.f, 3.f));
    }

    TEST_F(AnimatedSetterTest, canSetFirstValue)
    {
        const ramses_internal::Vector3 value(1.f, 2.f, 3.f);
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(value.x, value.y, value.z));
        EXPECT_EQ(1u, getNumSplineKeys());
        EXPECT_EQ(getCurrentTime() + setterDelay,      getStartTime(*m_animatedSetterVec3f));
        EXPECT_EQ(getCurrentTime() + setterDelay * 2u, getStopTime(*m_animatedSetterVec3f));

        setCurrentTime(getCurrentTime() + setterDelay);
        setCurrentTime(getCurrentTime() + setterDelay);
        expectValue(value);
    }

    TEST_F(AnimatedSetterTest, canSetSubsequentValue)
    {
        // 1st set
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(1.f, 2.f, 3.f));
        const globalTimeStamp_t animStartTime = getStartTime(*m_animatedSetterVec3f);

        // 2nd set
        setCurrentTime(getCurrentTime() + setterDelay / 2u);

        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(2.f, 3.f, 4.f));
        EXPECT_EQ(2u, getNumSplineKeys());
        EXPECT_EQ(animStartTime, getStartTime(*m_animatedSetterVec3f)); // unchanged
        EXPECT_EQ(getCurrentTime() + setterDelay * 2u, getStopTime(*m_animatedSetterVec3f));

        // 3rd set
        setCurrentTime(getCurrentTime() + setterDelay / 2u);

        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(3.f, 4.f, 5.f));
        EXPECT_EQ(3u, getNumSplineKeys());
        EXPECT_EQ(animStartTime, getStartTime(*m_animatedSetterVec3f)); // unchanged
        EXPECT_EQ(getCurrentTime() + setterDelay * 2u, getStopTime(*m_animatedSetterVec3f));

        setCurrentTime(getCurrentTime() + setterDelay);
        setCurrentTime(getCurrentTime() + setterDelay);
        expectValue(ramses_internal::Vector3(3.f, 4.f, 5.f));
    }

    TEST_F(AnimatedSetterTest, resetsAnimationIfNextSetComesAfterThreshold)
    {
        // 1st set
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(1.f, 2.f, 3.f));
        // 2nd set
        setCurrentTime(getCurrentTime() + setterDelay / 2u);
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(2.f, 3.f, 4.f));

        // 3rd set after threshold which is equal to delay
        setCurrentTime(getCurrentTime() + setterDelay + 1u);

        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(3.f, 4.f, 5.f));
        EXPECT_EQ(1u, getNumSplineKeys());
        EXPECT_EQ(getCurrentTime() + setterDelay, getStartTime(*m_animatedSetterVec3f));
        EXPECT_EQ(getCurrentTime() + setterDelay * 2u, getStopTime(*m_animatedSetterVec3f));

        setCurrentTime(getCurrentTime() + setterDelay);
        setCurrentTime(getCurrentTime() + setterDelay);
        expectValue(ramses_internal::Vector3(3.f, 4.f, 5.f));
    }

    TEST_F(AnimatedSetterTest, resetsAnimationIfNextSetComesAfterStop)
    {
        // 1st set
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(1.f, 2.f, 3.f));
        // 2nd set
        setCurrentTime(getCurrentTime() + setterDelay / 2u);
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(2.f, 3.f, 4.f));

        m_animatedSetterVec3f->stop();

        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(3.f, 4.f, 5.f));
        EXPECT_EQ(1u, getNumSplineKeys());
        EXPECT_EQ(getCurrentTime() + setterDelay, getStartTime(*m_animatedSetterVec3f));
        EXPECT_EQ(getCurrentTime() + setterDelay * 2u, getStopTime(*m_animatedSetterVec3f));

        setCurrentTime(getCurrentTime() + setterDelay);
        setCurrentTime(getCurrentTime() + setterDelay);
        expectValue(ramses_internal::Vector3(3.f, 4.f, 5.f));
    }

    TEST_F(AnimatedSetterTest, canBeStopped)
    {
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(1.f, 2.f, 3.f));
        setCurrentTime(getCurrentTime() + setterDelay / 2u);

        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->stop());
        EXPECT_EQ(getCurrentTime(), getStopTime(*m_animatedSetterVec3f));
    }

    TEST_F(AnimatedSetterTest, canBeStoppedWithDelay)
    {
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(1.f, 2.f, 3.f));
        setCurrentTime(getCurrentTime() + setterDelay / 2u);

        const timeMilliseconds_t stopDelay = 33u;
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->stop(stopDelay));
        EXPECT_EQ(getCurrentTime() + stopDelay, getStopTime(*m_animatedSetterVec3f));
    }

    TEST_F(AnimatedSetterTest, canValidate)
    {
        const ramses_internal::Vector3 value(1.f, 2.f, 3.f);
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(value.x, value.y, value.z));
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->validate());
    }

    TEST_F(AnimatedSetterTest, failsToValidateIfAnimatedPropertyDestroyed)
    {
        const ramses_internal::Vector3 value(1.f, 2.f, 3.f);
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(value.x, value.y, value.z));
        this->animationSystem.destroy(*m_animPropertyVec3f);
        EXPECT_NE(StatusOK, m_animatedSetterVec3f->validate());
    }

    TEST_F(AnimatedSetterTest, failsToValidateIfAnimatedNodeDestroyed)
    {
        const ramses_internal::Vector3 value(1.f, 2.f, 3.f);
        EXPECT_EQ(StatusOK, m_animatedSetterVec3f->setValue(value.x, value.y, value.z));
        this->getScene().destroy(*m_animatedNode);
        EXPECT_NE(StatusOK, m_animatedSetterVec3f->validate());
    }

    typedef ::testing::Types
        <
        int32_t,
        float,
        ramses_internal::Vector2,
        ramses_internal::Vector3,
        ramses_internal::Vector4,
        ramses_internal::Vector2i,
        ramses_internal::Vector3i,
        ramses_internal::Vector4i
        > DataTypes;

    typedef ramses_internal::DataBindContainerToTraitsSelector<ramses_internal::IScene>::ContainerTraitsClassType ContainerTraitsClass;
    template <typename DATA>
    void InitializeAnimatedProperty(AnimatedPropertyImpl&)
    {
        assert(false);
    }

    ramses_internal::Scene gDummyScene;

    template <>
    void InitializeAnimatedProperty<int32_t>(AnimatedPropertyImpl& prop)
    {
        prop.initializeFrameworkData(gDummyScene, 0u, 0u, ContainerTraitsClass::DataField_Integer, ramses_internal::EVectorComponent_All, ramses_internal::DataTypeToDataIDSelector<int32_t>::DataTypeID);
    }

    template <>
    void InitializeAnimatedProperty<float>(AnimatedPropertyImpl& prop)
    {
        prop.initializeFrameworkData(gDummyScene, 0u, 0u, ContainerTraitsClass::DataField_Float, ramses_internal::EVectorComponent_All, ramses_internal::DataTypeToDataIDSelector<float>::DataTypeID);
    }

    template <>
    void InitializeAnimatedProperty<ramses_internal::Vector2>(AnimatedPropertyImpl& prop)
    {
        prop.initializeFrameworkData(gDummyScene, 0u, 0u, ContainerTraitsClass::DataField_Vector2f, ramses_internal::EVectorComponent_All, ramses_internal::DataTypeToDataIDSelector<ramses_internal::Vector2>::DataTypeID);
    }

    template <>
    void InitializeAnimatedProperty<ramses_internal::Vector3>(AnimatedPropertyImpl& prop)
    {
        prop.initializeFrameworkData(gDummyScene, 0u, 0u, ContainerTraitsClass::DataField_Vector3f, ramses_internal::EVectorComponent_All, ramses_internal::DataTypeToDataIDSelector<ramses_internal::Vector3>::DataTypeID);
    }

    template <>
    void InitializeAnimatedProperty<ramses_internal::Vector4>(AnimatedPropertyImpl& prop)
    {
        prop.initializeFrameworkData(gDummyScene, 0u, 0u, ContainerTraitsClass::DataField_Vector4f, ramses_internal::EVectorComponent_All, ramses_internal::DataTypeToDataIDSelector<ramses_internal::Vector4>::DataTypeID);
    }

    template <>
    void InitializeAnimatedProperty<ramses_internal::Vector2i>(AnimatedPropertyImpl& prop)
    {
        prop.initializeFrameworkData(gDummyScene, 0u, 0u, ContainerTraitsClass::DataField_Vector2i, ramses_internal::EVectorComponent_All, ramses_internal::DataTypeToDataIDSelector<ramses_internal::Vector2i>::DataTypeID);
    }

    template <>
    void InitializeAnimatedProperty<ramses_internal::Vector3i>(AnimatedPropertyImpl& prop)
    {
        prop.initializeFrameworkData(gDummyScene, 0u, 0u, ContainerTraitsClass::DataField_Vector3i, ramses_internal::EVectorComponent_All, ramses_internal::DataTypeToDataIDSelector<ramses_internal::Vector3i>::DataTypeID);
    }

    template <>
    void InitializeAnimatedProperty<ramses_internal::Vector4i>(AnimatedPropertyImpl& prop)
    {
        prop.initializeFrameworkData(gDummyScene, 0u, 0u, ContainerTraitsClass::DataField_Vector4i, ramses_internal::EVectorComponent_All, ramses_internal::DataTypeToDataIDSelector<ramses_internal::Vector4i>::DataTypeID);
    }

    template <typename DATA>
    class AnimatedSetterDataTypeTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
        AnimatedSetterDataTypeTest()
            : m_setterDelay(100u)
            , m_animatedProperty(LocalTestClientWithSceneAndAnimationSystem::animationSystem.impl, "")
            , m_animatedSetter(LocalTestClientWithSceneAndAnimationSystem::animationSystem.impl, m_setterDelay, "")
        {
        }

        virtual void SetUp()
        {
            InitializeAnimatedProperty<DATA>(m_animatedProperty);
            m_animatedSetter.initializeFrameworkData(m_animatedProperty);
        }

    protected:
        timeMilliseconds_t m_setterDelay;
        AnimatedPropertyImpl m_animatedProperty;
        AnimatedSetterImpl m_animatedSetter;
    };

    TYPED_TEST_CASE(AnimatedSetterDataTypeTest, DataTypes);

    TYPED_TEST(AnimatedSetterDataTypeTest, canSetValueOfType)
    {
        const TypeParam value(0);
        EXPECT_EQ(StatusOK, this->m_animatedSetter.setValueInternal(value));
    }

    TYPED_TEST(AnimatedSetterDataTypeTest, failsToSetValueOfInvalidDataType)
    {
        if (ramses_internal::DataTypeToDataIDSelector<TypeParam>::DataTypeID != ramses_internal::EDataTypeID_Float)
        {
            EXPECT_NE(StatusOK, this->m_animatedSetter.setValueInternal(1.f));
        }
    }
}
