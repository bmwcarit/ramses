//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextureUploadingAdapterMock.h"

using namespace testing;

namespace ramses_internal
{
    TextureUploadingAdapterMock::TextureUploadingAdapterMock()
    {
    }

    TextureUploadingAdapterMock::~TextureUploadingAdapterMock()
    {
    }

    TextureUploadingAdapterMockWithDestructor::TextureUploadingAdapterMockWithDestructor()
    {
    }

    TextureUploadingAdapterMockWithDestructor::~TextureUploadingAdapterMockWithDestructor()
    {
        Die();
    }
}
