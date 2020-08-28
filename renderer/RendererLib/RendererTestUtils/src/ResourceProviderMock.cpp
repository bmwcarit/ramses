//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceProviderMock.h"

namespace ramses_internal {

constexpr ResourceContentHash ResourceProviderMock::FakeEffectHash;
constexpr ResourceContentHash ResourceProviderMock::FakeVertArrayHash;
constexpr ResourceContentHash ResourceProviderMock::FakeVertArrayHash2;
constexpr ResourceContentHash ResourceProviderMock::FakeIndexArrayHash;
constexpr ResourceContentHash ResourceProviderMock::FakeIndexArrayHash2;
constexpr ResourceContentHash ResourceProviderMock::FakeIndexArrayHash3;
constexpr ResourceContentHash ResourceProviderMock::FakeTextureHash;
constexpr ResourceContentHash ResourceProviderMock::FakeTextureHash2;

ResourceProviderMock::ResourceProviderMock()
{
    ON_CALL(*this, requestResourceAsyncronouslyFromFramework(_, _, _))
        .WillByDefault(Invoke([&](const auto& ids, auto, auto) {
                                  requestedResources.insert(requestedResources.end(), ids.begin(), ids.end());
                              }));
    EXPECT_CALL(*this, popArrivedResources(_)).Times(AnyNumber()).WillRepeatedly(Invoke(this, &ResourceProviderMock::fakePopArrivedResources));
}

ResourceProviderMock::~ResourceProviderMock() = default;
}
