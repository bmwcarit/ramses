//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataMatrix44f.h"

#include "Collections/Vector.h"

#include "ClientTestUtils.h"
#include "EffectInputImpl.h"
#include "AnimatedPropertyImpl.h"
#include "AnimationSystemImpl.h"
#include "TestEffectCreator.h"
#include "RamsesObjectTestTypes.h"

using namespace testing;

namespace ramses
{
    class AnimatedProperty;

    static TestEffectCreator* sharedEffectCreator = nullptr;
    static AnimationSystem* sharedAnimationSystem = nullptr;

    template <typename AnimatableType>
    const AnimatedProperty* createAnimatedProperty(AnimationSystem& animSystem, AnimatableType& animatable, EAnimatedPropertyComponent propertyComponent, EAnimatedProperty property, Appearance&)
    {
        return animSystem.createAnimatedProperty(animatable, property, propertyComponent);
    }

    template <>
    const AnimatedProperty* createAnimatedProperty<DataVector2f>(AnimationSystem& animSystem, DataVector2f& animatable, EAnimatedPropertyComponent propertyComponent, EAnimatedProperty, Appearance&)
    {
        return animSystem.createAnimatedProperty(animatable, propertyComponent);
    }

    template <>
    const AnimatedProperty* createAnimatedProperty<DataVector3f>(AnimationSystem& animSystem, DataVector3f& animatable, EAnimatedPropertyComponent propertyComponent, EAnimatedProperty, Appearance&)
    {
        return animSystem.createAnimatedProperty(animatable, propertyComponent);
    }

    template <>
    const AnimatedProperty* createAnimatedProperty<UniformInput>(AnimationSystem& animSystem, UniformInput& animatable, EAnimatedPropertyComponent propertyComponent, EAnimatedProperty, Appearance& appearance)
    {
        return animSystem.createAnimatedProperty(animatable, appearance, propertyComponent);
    }

    template <typename TestType>
    class AnimatedPropertyTest : public testing::Test
    {
    public:
        typedef std::vector<EAnimatedPropertyComponent> ComponentVector;
        typedef std::vector<ComponentVector> PropertyComponentVector;
        typedef std::vector<EAnimatedProperty> PropertyVector;

        static void SetUpTestCase()
        {
            sharedEffectCreator = new TestEffectCreator;
            sharedAnimationSystem = sharedEffectCreator->getScene().createAnimationSystem();
        }

        static void TearDownTestCase()
        {
            sharedAnimationSystem = NULL;
            delete sharedEffectCreator;
            sharedEffectCreator = NULL;
        }

        void SetUp()
        {
            EXPECT_TRUE(sharedEffectCreator != NULL);
            EXPECT_TRUE(sharedAnimationSystem != NULL);
        }

        AnimatedPropertyTest()
            : animatableProperties({ EAnimatedProperty_Translation, EAnimatedProperty_Rotation, EAnimatedProperty_Scaling })
        {
            // define all possible valid combinations for every property

            ComponentVector compsScalar;
            compsScalar.push_back(EAnimatedPropertyComponent_All);
            ComponentVector compsVec3;
            compsVec3.push_back(EAnimatedPropertyComponent_All);
            compsVec3.push_back(EAnimatedPropertyComponent_X);
            compsVec3.push_back(EAnimatedPropertyComponent_Y);
            compsVec3.push_back(EAnimatedPropertyComponent_Z);
            ComponentVector compsVec4;
            compsVec4.push_back(EAnimatedPropertyComponent_All);
            compsVec4.push_back(EAnimatedPropertyComponent_X);
            compsVec4.push_back(EAnimatedPropertyComponent_Y);
            compsVec4.push_back(EAnimatedPropertyComponent_Z);
            compsVec4.push_back(EAnimatedPropertyComponent_W);

            propertiesDefaultScalar.push_back(compsScalar);
            propertiesDefaultVec3.push_back(compsVec3);
            propertiesDefaultVec4.push_back(compsVec4);
        }

        template <typename AnimatableType>
        void checkPropertyCreation(AnimatableType& animatable, const PropertyComponentVector& comps)
        {
            for (const auto& propComp : comps)
            {
                for (const auto comp : propComp)
                {
                    for (const auto property : animatableProperties)
                    {
                        const AnimatedProperty* prop = createAnimatedProperty(*sharedAnimationSystem, animatable, comp, property, *sharedEffectCreator->appearance);
                        EXPECT_FALSE(prop == NULL);
                    }
                }
            }
        }

        template <typename AnimatableType>
        void checkPropertyCreationFail(AnimatableType& animatable)
        {
            for (const auto property : animatableProperties)
            {
                const AnimatedProperty* prop = createAnimatedProperty(*sharedAnimationSystem, animatable, EAnimatedPropertyComponent_W, property, *sharedEffectCreator->appearance);
                EXPECT_TRUE(prop == NULL);
            }
        }

        void getValidUniformInput(UniformInput& uniform, const char* inputName)
        {
            EXPECT_EQ(StatusOK, sharedEffectCreator->effect->findUniformInput(inputName, uniform));
            EXPECT_TRUE(uniform.isValid());
        }

    protected:
        PropertyComponentVector propertiesDefaultScalar;
        PropertyComponentVector propertiesDefaultVec3;
        PropertyComponentVector propertiesDefaultVec4;
        PropertyVector animatableProperties;
    };

    // dummy type instantiation for tests with explicit animatable type within the test case itself
    using AnAnimatedProperty = AnimatedPropertyTest<int>;

    TEST_F(AnAnimatedProperty, createAnimatedPropertyWithAllCombinationsOfValidParams_UniformInput_Vector4F)
    {
        UniformInput animatable;
        getValidUniformInput(animatable, "vec4fInput");
        checkPropertyCreation(animatable, propertiesDefaultVec4);
    }

    TEST_F(AnAnimatedProperty, createAnimatedPropertyWithAllCombinationsOfValidParams_DataObject_Vector3F)
    {
        DataVector3f& animatable = sharedEffectCreator->createObject<DataVector3f>("animatable");
        checkPropertyCreation(animatable, propertiesDefaultVec3);
    }

    TEST_F(AnAnimatedProperty, createAnimatedPropertyWithInvalidParams_UniformInput_Vector2F_Z_Animation)
    {
        UniformInput animatable;
        getValidUniformInput(animatable, "vec2fInput");
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, *sharedEffectCreator->appearance, EAnimatedPropertyComponent_Z);
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, createAnimatedPropertyWithInvalidParams_DataObject_Vector2F_Z_Animation)
    {
        DataVector2f& animatable = sharedEffectCreator->createObject<DataVector2f>("animatable");
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, EAnimatedPropertyComponent_Z);
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, failsToCreateAnimatedPropertyForInvalidUniformInput)
    {
        UniformInput animatable;
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, *sharedEffectCreator->appearance);
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, failsToCreateAnimatedPropertyForUniformInputBelongingToDifferentAppearance)
    {
        UniformInput animatable;
        getValidUniformInput(animatable, "vec4fInput");
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, sharedEffectCreator->createObject<Appearance>());
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, failsToCreateIfPropertyIsFromAnotherScene_Node)
    {
        Scene* otherScene = sharedEffectCreator->getClient().createScene(12u);
        Node& animatable = *otherScene->createNode();

        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, EAnimatedProperty_Translation);
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, failsToCreateIfPropertyIsFromAnotherScene_Uniform)
    {
        UniformInput animatable;
        getValidUniformInput(animatable, "vec4fInput");

        Scene* otherScene = sharedEffectCreator->getClient().createScene(13u);
        AnimationSystem* otherAnimationSystem = otherScene->createAnimationSystem();

        AnimatedProperty* prop = otherAnimationSystem->createAnimatedProperty(animatable, *sharedEffectCreator->appearance);
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, failsToCreateIfPropertyIsFromAnotherScene_DataObject)
    {
        DataVector3f& animatable = sharedEffectCreator->createObject<DataVector3f>("animatable");

        Scene* otherScene = sharedEffectCreator->getClient().createScene(14u);
        AnimationSystem* otherAnimationSystem = otherScene->createAnimationSystem();

        AnimatedProperty* prop = otherAnimationSystem->createAnimatedProperty(animatable);
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, failsToCreateAnimatedPropertyForUniformInputWithUnsupportedDataType)
    {
        UniformInput animatable;
        getValidUniformInput(animatable, "matrix44fInput");
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, *sharedEffectCreator->appearance);
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, failsToCreateAnimatedPropertyForDataObjectWithUnsupportedDataType)
    {
        DataMatrix44f& animatable = sharedEffectCreator->createObject<DataMatrix44f>("animatable");

        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable);
        EXPECT_TRUE(prop == NULL);
    }

    TEST_F(AnAnimatedProperty, removalOfAnimatedPropertyReleasesItsDataBind)
    {
        Node& animatable = sharedEffectCreator->createObject<Node>("animatable");
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);
        const ramses_internal::DataBindHandle dataBind = prop->impl.getDataBindHandle();
        EXPECT_TRUE(sharedAnimationSystem->impl.getIAnimationSystem().containsDataBinding(dataBind));

        sharedAnimationSystem->destroy(*prop);

        EXPECT_FALSE(sharedAnimationSystem->impl.getIAnimationSystem().containsDataBinding(dataBind));
    }

    TEST_F(AnAnimatedProperty, canBeValidated)
    {
        Node& animatable = sharedEffectCreator->createObject<Node>("animatable");
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);
        EXPECT_EQ(StatusOK, prop->validate());
    }

    TEST_F(AnAnimatedProperty, failsValidationWhenItsPropertyRemoved)
    {
        Node& animatable = sharedEffectCreator->createObject<Node>("animatable");
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);

        sharedEffectCreator->getScene().destroy(animatable);

        EXPECT_NE(StatusOK, prop->validate());
    }

    TEST_F(AnAnimatedProperty, failsValidationWhenUsedInMultipleAnimations)
    {
        Node& animatable = sharedEffectCreator->createObject<Node>("animatable");
        AnimatedProperty* prop = sharedAnimationSystem->createAnimatedProperty(animatable, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);

        Spline* spline = sharedAnimationSystem->createSplineLinearVector3f();
        ASSERT_TRUE(spline != NULL);

        sharedAnimationSystem->createAnimation(*prop, *spline);
        sharedAnimationSystem->createAnimation(*prop, *spline);

        EXPECT_NE(StatusOK, prop->validate());
    }

    // templated tests for all node types
    TYPED_TEST_CASE(AnimatedPropertyTest, NodeTypes);

    TYPED_TEST(AnimatedPropertyTest, createNodeAnimatedPropertyWithAllCombinationsOfValidParams)
    {
        TypeParam& animatable = sharedEffectCreator->createObject<TypeParam>("animatable");
        this->checkPropertyCreation(animatable, this->propertiesDefaultVec3);
    }

    TYPED_TEST(AnimatedPropertyTest, createNodeAnimatedPropertyWithInvalidParams)
    {
        TypeParam& animatable = sharedEffectCreator->createObject<TypeParam>("animatable");
        this->checkPropertyCreationFail(animatable);
    }
}
