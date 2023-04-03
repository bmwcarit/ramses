//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/AnimationTypes.h"

namespace rlogic::internal
{
    class AnimationNodeConfigImpl
    {
    public:
        bool addChannel(const AnimationChannel& channelData);
        [[nodiscard]] const AnimationChannels& getChannels() const;

        bool setExposingOfChannelDataAsProperties(bool enabled);
        [[nodiscard]] bool getExposingOfChannelDataAsProperties() const;

    private:
        AnimationChannels m_channels;
        bool m_exposeChannelDataAsProperties = false;
    };
}
