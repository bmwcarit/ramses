//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"
#include "RamsesTestUtils.h"
#include "SerializationTestUtils.h"
#include "WithTempDirectory.h"

#include "ramses-logic/SkinBinding.h"
#include "impl/LogicEngineImpl.h"
#include "impl/SkinBindingImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "impl/RamsesAppearanceBindingImpl.h"
#include "internals/DeserializationMap.h"

#include "generated/SkinBindingGen.h"
#include "glm/gtc/type_ptr.hpp"

namespace rlogic::internal
{
    class ASkinBinding : public ALogicEngine
    {
    public:
        ASkinBinding()
            : m_appearance{ m_scene->createAppearance(createTestEffect()) }
        {
            m_appearance->getEffect().findUniformInput("jointMat", m_uniform);

            // add some transformations to the joints before calculating inverse mats and creating skin
            m_jointNodes[0]->setTranslation({1.f, 2.f, 3.f});
            m_jointNodes[1]->setRotation({10.f, 20.f, 30.f});

            m_skin = createSkinBinding();
        }

    protected:
        const ramses::Effect& createTestEffect()
        {
            ramses::EffectDescription effectDesc;
            effectDesc.setVertexShader(m_vertShader.data());
            effectDesc.setFragmentShader(m_fragShader.data());
            return *m_scene->createEffect(effectDesc);
        }

        SkinBinding* createSkinBinding()
        {
            std::vector<matrix44f> inverseMats;
            inverseMats.resize(2u);

            m_jointNodes[0]->getInverseModelMatrix(inverseMats[0]);
            m_jointNodes[1]->getInverseModelMatrix(inverseMats[1]);

            return m_logicEngine.createSkinBinding(m_joints, inverseMats, *m_appearanceBinding, m_uniform, "skin");
        }

        // this VS is not capable of vertex skinning but that is not needed for the tests here
        const std::string_view m_vertShader = R"(
                #version 100

                uniform highp float floatUniform;
                uniform highp mat4 jointMat[2];
                attribute vec3 a_position;

                void main()
                {
                    gl_Position = floatUniform * vec4(a_position, 1.0) * jointMat[1];
                })";

        const std::string_view m_fragShader = R"(
                #version 100
                void main(void)
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                })";

        std::vector<ramses::Node*> m_jointNodes{ m_scene->createNode(), m_scene->createNode() };
        std::vector<const RamsesNodeBinding*> m_joints{ m_logicEngine.createRamsesNodeBinding(*m_jointNodes[0]), m_logicEngine.createRamsesNodeBinding(*m_jointNodes[1]) };

        ramses::Appearance* m_appearance{ m_scene->createAppearance(createTestEffect()) };
        RamsesAppearanceBinding* m_appearanceBinding{ m_logicEngine.createRamsesAppearanceBinding(*m_appearance) };
        ramses::UniformInput m_uniform;

        SkinBinding* m_skin = nullptr;
    };

    TEST_F(ASkinBinding, RefersToGivenRamsesObjects)
    {
        EXPECT_EQ(m_appearanceBinding, &m_skin->getAppearanceBinding());
        const auto& skinConst = *m_skin;
        EXPECT_EQ(m_appearanceBinding, &skinConst.getAppearanceBinding());
        const auto& skinImplConst = m_skin->m_skinBinding;
        EXPECT_EQ(&m_appearanceBinding->m_appearanceBinding, &skinImplConst.getAppearanceBinding());

        EXPECT_EQ(ramses::EDataType::Matrix44F, *m_skin->getAppearanceUniformInput().getDataType());
        EXPECT_EQ(2u, m_skin->getAppearanceUniformInput().getElementCount());
    }

    TEST_F(ASkinBinding, HasNoInputNorOutputs)
    {
        EXPECT_EQ(nullptr, m_skin->getInputs());
        EXPECT_EQ(nullptr, m_skin->getOutputs());
    }

    TEST_F(ASkinBinding, SetsBoundUniformFromInitialJoints)
    {
        EXPECT_TRUE(m_logicEngine.update());

        // initially inverse binding mats are equal to the inverse transformation mats of joints
        // so result should be identity matrices
        const matrix44f expectedMat = {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f
        };

        std::array<ramses::matrix44f, 2u> uniformData;
        m_appearance->getInputValue(m_uniform, 2u, uniformData.data());
        const matrix44f mat1 = uniformData[0];
        const matrix44f mat2 = uniformData[1];

        for (glm::length_t i = 0u; i < 16; ++i)
            EXPECT_NEAR(expectedMat[i/4][i%4], mat1[i/4][i%4], 1e-7f) << i;

        for (glm::length_t i = 0u; i < 16; ++i)
            EXPECT_NEAR(expectedMat[i/4][i%4], mat2[i/4][i%4], 1e-7f) << i;
    }

    TEST_F(ASkinBinding, UpdatesBoundUniformOnJointChange)
    {
        m_jointNodes[0]->setRotation({-1.f, -2.f, -3.f});
        m_jointNodes[1]->setTranslation({-1.f, -2.f, -3.f});
        EXPECT_TRUE(m_logicEngine.update());

        const matrix44f expectedMat1 = {
            0.998f, -0.0523f, 0.0349f, 0.f,
            0.0529f, 0.9984f, -0.0174f, 0.f,
            -0.0339f, 0.01925f, 0.9992f, 0.f,
            -0.00209f, -0.00235f, 0.00227f, 1.f
        };
        const matrix44f expectedMat2 = {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            -1.f, -2.f, -3.f, 1.f
        };

        std::array<ramses::matrix44f, 2u> uniformData;
        m_appearance->getInputValue(m_uniform, 2u, uniformData.data());
        const matrix44f mat1 = uniformData[0];
        const matrix44f mat2 = uniformData[1];

        for (glm::length_t i = 0u; i < 16; ++i)
            EXPECT_NEAR(expectedMat1[i/4][i%4], mat1[i/4][i%4], 1e-4f) << i;

        for (glm::length_t i = 0u; i < 16; ++i)
            EXPECT_NEAR(expectedMat2[i/4][i%4], mat2[i/4][i%4], 1e-4f) << i;
    }

    class ASkinBinding_SerializationLifecycle : public ASkinBinding
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            MissingName,
            MissingJoints,
            NumJointsMismatchMatrices,
            UnresolvedJointNodeBinding,
            UnresolvedAppearanceBinding,
            InvalidUniform
        };

        std::unique_ptr<SkinBindingImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            {
                const auto fbLogicObject = rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    (issue == ESerializationIssue::MissingName ? 0 : m_flatBufferBuilder.CreateString("name")), 1u, 0u, 0u);

                const std::vector<uint64_t> jointIds{ 0u, 1u };

                std::vector<float> inverseBindMatData;
                inverseBindMatData.resize((issue == ESerializationIssue::NumJointsMismatchMatrices ? 16u : 32u), 0.f);

                auto fbSkinBinding = rlogic_serialization::CreateSkinBinding(m_flatBufferBuilder,
                    fbLogicObject,
                    (issue == ESerializationIssue::MissingJoints ? 0u : m_flatBufferBuilder.CreateVector(jointIds)),
                    m_flatBufferBuilder.CreateVector(inverseBindMatData),
                    2u,
                    (issue == ESerializationIssue::InvalidUniform ? m_flatBufferBuilder.CreateString("wrongUniform") : m_flatBufferBuilder.CreateString("jointMat")));
                m_flatBufferBuilder.Finish(fbSkinBinding);
            }

            DeserializationMap deserializationMap;
            deserializationMap.storeLogicObject(0u, m_joints[0]->m_nodeBinding);
            if (issue != ESerializationIssue::UnresolvedJointNodeBinding)
                deserializationMap.storeLogicObject(1u, m_joints[1]->m_nodeBinding);
            if (issue != ESerializationIssue::UnresolvedAppearanceBinding)
                deserializationMap.storeLogicObject(2u, m_appearanceBinding->m_appearanceBinding);

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::SkinBinding>(m_flatBufferBuilder.GetBufferPointer());
            return SkinBindingImpl::Deserialize(serialized, m_errorReporting, deserializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        ErrorReporting m_errorReporting;
    };

    TEST_F(ASkinBinding_SerializationLifecycle, CanSerializeWithNoIssue)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ASkinBinding_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_F(ASkinBinding_SerializationLifecycle, ReportsSerializationError_MissingName)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ASkinBinding_SerializationLifecycle::ESerializationIssue::MissingName));
        ASSERT_EQ(2u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LogicObject base from serialized data: missing name!");
        EXPECT_EQ(m_errorReporting.getErrors()[1].message, "Fatal error during loading of SkinBinding from serialized data: missing name and/or ID!");
    }

    TEST_F(ASkinBinding_SerializationLifecycle, ReportsSerializationError_MissingJoints)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ASkinBinding_SerializationLifecycle::ESerializationIssue::MissingJoints));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of SkinBinding from serialized data: missing or corrupted joints and/or inverse matrices data!");
    }

    TEST_F(ASkinBinding_SerializationLifecycle, ReportsSerializationError_NumJointsMismatchMatrices)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ASkinBinding_SerializationLifecycle::ESerializationIssue::NumJointsMismatchMatrices));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of SkinBinding from serialized data: missing or corrupted joints and/or inverse matrices data!");
    }

    TEST_F(ASkinBinding_SerializationLifecycle, ReportsSerializationError_UnresolvedJointNodeBinding)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ASkinBinding_SerializationLifecycle::ESerializationIssue::UnresolvedJointNodeBinding));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of SkinBinding from serialized data: could not resolve referenced node binding!");
    }

    TEST_F(ASkinBinding_SerializationLifecycle, ReportsSerializationError_UnresolvedAppearanceBinding)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ASkinBinding_SerializationLifecycle::ESerializationIssue::UnresolvedAppearanceBinding));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of SkinBinding from serialized data: could not resolve referenced appearance binding!");
    }

    TEST_F(ASkinBinding_SerializationLifecycle, ReportsSerializationError_InvalidUniform)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ASkinBinding_SerializationLifecycle::ESerializationIssue::InvalidUniform));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of SkinBinding from serialized data: invalid or mismatching uniform input!");
    }

    TEST_F(ASkinBinding_SerializationLifecycle, FailsToLoadWhenNoSceneProvided)
    {
        {
            createSkinBinding();
            ASSERT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "binding.bin"));
        }

        {
            EXPECT_FALSE(m_logicEngine.loadFromFile("binding.bin"));
            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Fatal error during loading from file! File contains references to Ramses objects but no Ramses scene was provided!");
        }
    }
}
