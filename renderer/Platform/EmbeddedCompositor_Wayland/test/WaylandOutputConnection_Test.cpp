//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandOutputConnection.h"
#include "WaylandClientMock.h"
#include "WaylandOutputResourceMock.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    using namespace testing;

    class AWaylandOutputConnection : public Test
    {
    protected:
        StrictMock<WaylandClientMock> m_client;
    };


    TEST_F(AWaylandOutputConnection, SendsGeometryAndModeWhenBoundForInterfaceVersion1)
    {
        const uint32_t interfaceVersion = 1;
        const uint32_t id               = 1;
        const uint32_t width            = 200;
        const uint32_t height           = 100;
        const uint32_t refresh          = 30;

        WaylandOutputResourceMock* outputResource = new StrictMock<WaylandOutputResourceMock>;

        InSequence s;

        EXPECT_CALL(m_client, outputResourceCreate(&wl_output_interface, interfaceVersion, id))
            .WillOnce(Return(outputResource));

        EXPECT_CALL(*outputResource, setImplementation(_, _, _));
        EXPECT_CALL(*outputResource,
                    outputSendGeometry(0,
                                       0,
                                       width / 6,
                                       height / 6,
                                       WL_OUTPUT_SUBPIXEL_UNKNOWN,
                                       StrEq("unknown"),
                                       StrEq("unknown"),
                                       WL_OUTPUT_TRANSFORM_NORMAL));
        EXPECT_CALL(*outputResource,
                    outputSendMode(WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED, width, height, refresh));

        WaylandOutputConnection waylandOutputConnection(m_client, interfaceVersion, id, width, height, refresh);

        EXPECT_CALL(*outputResource, setImplementation(_, _, nullptr));
    }

    TEST_F(AWaylandOutputConnection, SendsGeometryModeScaleAndDoneWhenBoundForInterfaceVersion2)
    {
        const uint32_t interfaceVersion = 2;
        const uint32_t id               = 1;
        const uint32_t width            = 200;
        const uint32_t height           = 100;
        const uint32_t refresh          = 30;

        WaylandOutputResourceMock* outputResource = new StrictMock<WaylandOutputResourceMock>;

        InSequence s;

        EXPECT_CALL(m_client, outputResourceCreate(&wl_output_interface, interfaceVersion, id))
            .WillOnce(Return(outputResource));

        EXPECT_CALL(*outputResource, setImplementation(_, _, _));
        EXPECT_CALL(*outputResource,
                    outputSendGeometry(0,
                                       0,
                                       width / 6,
                                       height / 6,
                                       WL_OUTPUT_SUBPIXEL_UNKNOWN,
                                       StrEq("unknown"),
                                       StrEq("unknown"),
                                       WL_OUTPUT_TRANSFORM_NORMAL));
        EXPECT_CALL(*outputResource,
                    outputSendMode(WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED, width, height, refresh));
        EXPECT_CALL(*outputResource, outputSendScale(1));
        EXPECT_CALL(*outputResource, outputSendDone());
        WaylandOutputConnection waylandOutputConnection(m_client, interfaceVersion, id, width, height, refresh);

        EXPECT_CALL(*outputResource, setImplementation(_, _, nullptr));
    }

    TEST_F(AWaylandOutputConnection, DisownsNativeWaylandResourceWhenResourceDestroyed)
    {
        const uint32_t interfaceVersion = 1;
        const uint32_t id               = 1;
        const uint32_t width            = 200;
        const uint32_t height           = 100;
        const uint32_t refresh          = 30;

        WaylandOutputResourceMock* outputResource = new StrictMock<WaylandOutputResourceMock>;

        InSequence s;

        EXPECT_CALL(m_client, outputResourceCreate(&wl_output_interface, interfaceVersion, id))
            .WillOnce(Return(outputResource));

        EXPECT_CALL(*outputResource, setImplementation(_, _, _));
        EXPECT_CALL(*outputResource,
                    outputSendGeometry(0,
                                       0,
                                       width / 6,
                                       height / 6,
                                       WL_OUTPUT_SUBPIXEL_UNKNOWN,
                                       StrEq("unknown"),
                                       StrEq("unknown"),
                                       WL_OUTPUT_TRANSFORM_NORMAL));
        EXPECT_CALL(*outputResource,
                    outputSendMode(WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED, width, height, refresh));


        WaylandOutputConnection waylandOutputConnection(m_client, interfaceVersion, id, width, height, refresh);

        EXPECT_CALL(*outputResource, disownWaylandResource());
        waylandOutputConnection.resourceDestroyed();

        EXPECT_CALL(*outputResource, setImplementation(_, _, nullptr));
    }
}
