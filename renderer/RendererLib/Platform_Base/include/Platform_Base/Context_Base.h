//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTEXT_BASE_H
#define RAMSES_CONTEXT_BASE_H

#include "RendererAPI/IContext.h"

#include "Utils/StringUtils.h"
#include "Platform_Base/DeviceResourceMapper.h"

namespace ramses_internal
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
        static void ParseContextExtensionsHelper(const Char* extensionNativeString, StringSet& extensionsOut);

    protected:
        void parseContextExtensions(const Char* extensionNativeString);
        [[nodiscard]] Bool isContextExtensionAvailable(const String& extensionName) const;

    protected:
        DeviceResourceMapper m_resources;
        StringSet m_contextExtensions;
        StringSet m_apiExtensions;
    };
}

#endif
