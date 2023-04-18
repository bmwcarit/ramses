//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace rlogic
{
    class Property;

    /**
     * PropertyLink describes a data link between two properties, each belonging to a #rlogic::LogicNode.
     * Data flows always from \c source (some #rlogic::LogicNode's output) to \c target (another #rlogic::LogicNode's input),
     * see #rlogic::LogicEngine::link and #rlogic::LogicEngine::update for more details.
     * PropertLink can be obtained using #rlogic::Property::getIncomingLink, #rlogic::Property::getOutgoingLink
     * or #rlogic::LogicEngine::getPropertyLinks.
     */
    struct PropertyLink
    {
        /// data flow source (some #rlogic::LogicNode's output)
        const Property* source = nullptr;
        /// data flow target (another #rlogic::LogicNode's input)
        const Property* target = nullptr;
        /// \c true if this is a weak link - see #rlogic::LogicEngine::linkWeak
        bool isWeakLink = false;
    };
}
