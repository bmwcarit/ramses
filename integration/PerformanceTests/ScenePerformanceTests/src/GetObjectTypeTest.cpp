//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "GetObjectTypeTest.h"
#include "ramses-client-api/TransformationNode.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include "ramses-client-api/Node.h"

GetObjectTypeTest::GetObjectTypeTest(ramses_internal::String testName, uint32_t testState) : PerformanceTestBase(testName, testState) {};

void GetObjectTypeTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);

    m_node = scene.createTransformationNode();
}

void GetObjectTypeTest::preUpdate()
{
}

void GetObjectTypeTest::update()
{
    // Lots of possible types to test against (all false, as the target node is TransformationNode)
    static ramses::ERamsesObjectType Types[] =
    {
        ramses::ERamsesObjectType_DataVector2i,
        ramses::ERamsesObjectType_DataVector3i,
        ramses::ERamsesObjectType_DataVector4i,
        ramses::ERamsesObjectType_StreamTexture,
        ramses::ERamsesObjectType_RenderGroup,
        ramses::ERamsesObjectType_RenderPass,
        ramses::ERamsesObjectType_BlitPass,
        ramses::ERamsesObjectType_TextureSampler,
        ramses::ERamsesObjectType_RemoteCamera,
        ramses::ERamsesObjectType_LocalCamera,
        ramses::ERamsesObjectType_PerspectiveCamera,
        ramses::ERamsesObjectType_OrthographicCamera,
        ramses::ERamsesObjectType_SplineStepBool,
        ramses::ERamsesObjectType_SplineStepFloat,
        ramses::ERamsesObjectType_SplineStepInt32,
        ramses::ERamsesObjectType_SplineStepVector2f,
        ramses::ERamsesObjectType_SplineStepVector3f,
        ramses::ERamsesObjectType_SplineStepVector4f,
        ramses::ERamsesObjectType_SplineStepVector2i,
        ramses::ERamsesObjectType_SplineStepVector3i,
        ramses::ERamsesObjectType_SplineStepVector4i,
        ramses::ERamsesObjectType_SplineLinearFloat,
        ramses::ERamsesObjectType_SplineLinearInt32,
        ramses::ERamsesObjectType_SplineLinearVector2f,
        ramses::ERamsesObjectType_SplineLinearVector3f,
        ramses::ERamsesObjectType_SplineLinearVector4f,
        ramses::ERamsesObjectType_SplineLinearVector2i,
        ramses::ERamsesObjectType_SplineLinearVector3i,
        ramses::ERamsesObjectType_SplineLinearVector4i,
        ramses::ERamsesObjectType_SplineBezierFloat,
        ramses::ERamsesObjectType_SplineBezierInt32,
        ramses::ERamsesObjectType_SplineBezierVector2f,
        ramses::ERamsesObjectType_SplineBezierVector3f,
        ramses::ERamsesObjectType_SplineBezierVector4f,
        ramses::ERamsesObjectType_SplineBezierVector2i,
        ramses::ERamsesObjectType_SplineBezierVector3i,
        ramses::ERamsesObjectType_SplineBezierVector4i,
    };

    const uint32_t TypesLength = sizeof(Types) / sizeof(uint32_t);

    bool result = false;

    switch (m_testState)
    {
    case GetObjectTypeTest_GetType:
    {
        for (uint32_t i = 0; i < TypesLength; i++)
        {
            result = m_node->getType() == Types[i];

            if (result)
            {
                break;
            }
        }

        break;
    }
    case GetObjectTypeTest_IsOfType:
    {
        for (uint32_t i = 0; i < TypesLength; i++)
        {
            result = m_node->isOfType(ramses::ERamsesObjectType_BlitPass);

            if (result)
            {
                break;
            }
        }

        break;
    }
    default:
    {
        assert(false);
        break;
    }
    }


    if (result)
    {
        // Will never be executed. This is just here to avoid the compiler optimizing everything away.
        m_node->removeParent();
    }
}
