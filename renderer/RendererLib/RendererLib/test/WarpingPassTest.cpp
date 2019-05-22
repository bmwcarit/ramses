//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/WarpingPass.h"

#include "DeviceMock.h"
#include "RendererLib/WarpingMeshData.h"

using namespace testing;
using namespace ramses_internal;

class AWarpingPass : public ::testing::Test
{
public:
    AWarpingPass()
    {
        EXPECT_CALL(device, uploadShader(_));
        EXPECT_CALL(device, allocateIndexBuffer(_, _));
        EXPECT_CALL(device, uploadIndexBufferData(_, _, _));
        EXPECT_CALL(device, allocateVertexBuffer(_, _)).Times(2);
        EXPECT_CALL(device, uploadVertexBufferData(_, _, _)).Times(2);

        ramses_internal::WarpingMeshData meshData;
        pass = new WarpingPass(device, meshData);
    }

    ~AWarpingPass()
    {
        EXPECT_CALL(device, deleteIndexBuffer(_));
        EXPECT_CALL(device, deleteVertexBuffer(_)).Times(2);
        EXPECT_CALL(device, deleteShader(_));

        delete pass;
    }

protected:
    StrictMock<DeviceMock> device;
    WarpingPass* pass;
};


TEST_F(AWarpingPass, ValidRenderBackendCallsOnExecute)
{
    const DeviceResourceHandle inputColorBuffer(1);

    InSequence sequenceEnforcer;
    EXPECT_CALL(device, activateShader(Ne(DeviceResourceHandle::Invalid())));

    EXPECT_CALL(device, activateTexture(inputColorBuffer, Ne(DataFieldHandle::Invalid())));
    const UInt32 isotropicFilteringLevel = 1;
    EXPECT_CALL(device, setTextureSampling(Ne(DataFieldHandle::Invalid()), EWrapMethod::Clamp, EWrapMethod::Clamp, EWrapMethod::Clamp, ESamplingMethod::Linear, ESamplingMethod::Linear, isotropicFilteringLevel));

    EXPECT_CALL(device, activateIndexBuffer(Ne(DeviceResourceHandle::Invalid())));
    EXPECT_CALL(device, activateVertexBuffer(Ne(DeviceResourceHandle::Invalid()), Ne(DataFieldHandle::Invalid()), 0u)).Times(2);
    EXPECT_CALL(device, drawIndexedTriangles(0, 6, 1u)); // element count for default WarpingMeshData

    pass->execute(inputColorBuffer);
}
