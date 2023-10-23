//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IContext.h"

#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/RendererLib/PlatformBase/DeviceResourceMapper.h"

#include <string>

namespace ramses::internal
{
    class DisplayConfig;

    class Context_Base : public IContext
    {
    public:
        Context_Base();

        DeviceResourceMapper& getResources() override;

        // TODO Violin this is not beautiful, but is needed because windows parses
        // extensions non-conform to EGL standard (read up about WGL on the net,
        // search terms "WGL extensions")
        static void ParseContextExtensionsHelper(const char* extensionNativeString, HashSet<std::string>& extensionsOut);

    protected:
        void parseContextExtensions(const char* extensionNativeString);
        [[nodiscard]] bool isContextExtensionAvailable(const std::string& extensionName) const;

    protected:
        DeviceResourceMapper m_resources;
        HashSet<std::string> m_contextExtensions;
        HashSet<std::string> m_apiExtensions;
    };
}
