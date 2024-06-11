//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IPlatformFactory.h"
#include "PlatformMock.h"

namespace ramses::internal
{
    template <template<typename> class MOCK_TYPE>
    class PlatformFactoryMock : public IPlatformFactory
    {
    public:
        template<typename=typename std::enable_if<std::is_same<PlatformFactoryMock<MOCK_TYPE>, PlatformFactoryMock<::testing::NiceMock>>::value>::type>
        PlatformFactoryMock()
        {
            // Add expectations for NiceMock to avoid "unexpected call" messages. StrictMock should still cause test failure for unexpected calls.
            EXPECT_CALL(*this, createPlatform(_, _)).Times(AnyNumber()).WillRepeatedly([](){
                return std::make_unique<PlatformMock<::testing::NiceMock>>();});
        }

        MOCK_METHOD(std::unique_ptr<IPlatform>, createPlatform, (const RendererConfigData& rendererConfig, const DisplayConfigData& dispConfig), (override));
    };

    using PlatformFactoryNiceMock = PlatformFactoryMock< ::testing::NiceMock>;
    using PlatformFactoryStrictMock = PlatformFactoryMock< ::testing::StrictMock>;
}
