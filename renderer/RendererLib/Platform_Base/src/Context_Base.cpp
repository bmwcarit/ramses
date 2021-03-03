//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/Context_Base.h"
#include "Utils/StringUtils.h"

namespace ramses_internal
{
    Context_Base::Context_Base()
    {
    }

    DeviceResourceMapper& Context_Base::getResources()
    {
        return m_resources;
    }

    void Context_Base::ParseContextExtensionsHelper(const Char* extensionNativeString, StringSet& extensionsOut)
    {
        extensionsOut = StringSet();
        String extensionString = StringUtils::Trim(extensionNativeString);
        StringUtils::Tokenize(extensionString, extensionsOut);
    }

    void Context_Base::parseContextExtensions(const Char* extensionNativeString)
    {
        ParseContextExtensionsHelper(extensionNativeString, m_contextExtensions);
    }

    Bool Context_Base::isContextExtensionAvailable(const String& extensionName) const
    {
        // try out various prefixes; add more if required
        String nameEXT = "EGL_EXT_" + extensionName;
        String nameARB = "EGL_ARB_" + extensionName;

        return  m_contextExtensions.contains(nameEXT) ||
                m_contextExtensions.contains(nameARB);
    }
}

