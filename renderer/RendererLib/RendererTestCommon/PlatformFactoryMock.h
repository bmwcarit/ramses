//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMFACTORYMOCK_H
#define RAMSES_PLATFORMFACTORYMOCK_H

#include "renderer_common_gmock_header.h"
#include "RendererAPI/IPlatformFactory.h"
#include "PlatformMock.h"

namespace ramses_internal
{
    template <template<typename> class MOCK_TYPE>
    class PlatformFactoryMock : public IPlatformFactory
    {
    public:
        PlatformFactoryMock()
        {
            ON_CALL(*this, createPlatform(_, _)).WillByDefault(Invoke([](){
                return std::make_unique<PlatformMock<MOCK_TYPE>>();
            }));
        }
        MOCK_METHOD(std::unique_ptr<IPlatform>, createPlatform, (const RendererConfig& rendererConfig, const DisplayConfig& dispConfig), (override));
    };

    using PlatformFactoryNiceMock = PlatformFactoryMock< ::testing::NiceMock>;
    using PlatformFactoryStrictMock = PlatformFactoryMock< ::testing::StrictMock>;
}

#endif
