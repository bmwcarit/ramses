//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayManager/LinkData.h"

namespace ramses_display_manager
{
    LinkData::LinkData(IDisplayManager& displayManager)
        : m_displayManager(displayManager)
    {
        description = "Link scene data on renderer";
        registerKeyword("linkData");

        getArgument<0>()
            .registerKeyword("providerSceneId")
            .setDescription("Provider scene id");

        getArgument<1>()
            .registerKeyword("providerId")
            .setDescription("Provider data id");

        getArgument<2>()
            .registerKeyword("consumerSceneId")
            .setDescription("Consumer scene id");

        getArgument<3>()
            .registerKeyword("consumerId")
            .setDescription("Consumer data id");
    }

    bool LinkData::execute(uint64_t& providerSceneId, uint32_t& providerId, uint64_t& consumerSceneId, uint32_t& consumerId) const
    {
        m_displayManager.linkData(providerSceneId, providerId, consumerSceneId, consumerId);

        return true;
    }
}
