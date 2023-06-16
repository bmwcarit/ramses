//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/IResourceUploader.h"
#include "ResourceUploaderMock.h"

using namespace testing;

namespace ramses_internal
{
    const DeviceResourceHandle ResourceUploaderMock::FakeResourceDeviceHandle(996699u);

    ResourceUploaderMock::ResourceUploaderMock()
    {
        ON_CALL(*this, uploadResource(_, _, _)).WillByDefault(Return(FakeResourceDeviceHandle));
    }
};
