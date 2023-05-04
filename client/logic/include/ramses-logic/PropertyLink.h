//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses
{
    class Property;

    /**
     * @ingroup LogicAPI
     * PropertyLink describes a data link between two properties, each belonging to a #ramses::LogicNode.
     * Data flows always from \c source (some #ramses::LogicNode's output) to \c target (another #ramses::LogicNode's input),
     * see #ramses::LogicEngine::link and #ramses::LogicEngine::update for more details.
     * PropertLink can be obtained using #ramses::Property::getIncomingLink, #ramses::Property::getOutgoingLink
     * or #ramses::LogicEngine::getPropertyLinks.
     */
    struct PropertyLink
    {
        /// data flow source (some #ramses::LogicNode's output)
        const Property* source = nullptr;
        /// data flow target (another #ramses::LogicNode's input)
        const Property* target = nullptr;
        /// \c true if this is a weak link - see #ramses::LogicEngine::linkWeak
        bool isWeakLink = false;
    };
}
