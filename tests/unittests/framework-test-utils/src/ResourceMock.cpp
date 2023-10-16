//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceMock.h"

namespace ramses::internal
{
    ResourceMock::ResourceMock(ResourceContentHash hash, EResourceType typeId)
        : m_hash(hash)
        , m_typeId(typeId)
    {
        ON_CALL(*this, getDecompressedDataSize()).WillByDefault(Return(100u));
    }

    ResourceMock::~ResourceMock() = default;
}
