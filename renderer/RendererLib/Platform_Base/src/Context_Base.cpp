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

    void Context_Base::ParseContextExtensionsHelper(const char* extensionNativeString, HashSet<std::string>& extensionsOut)
    {
        extensionsOut = StringUtils::TokenizeToSet(StringUtils::TrimView(extensionNativeString));
    }

    void Context_Base::parseContextExtensions(const char* extensionNativeString)
    {
        ParseContextExtensionsHelper(extensionNativeString, m_contextExtensions);
    }

    bool Context_Base::isContextExtensionAvailable(const std::string& extensionName) const
    {
        // try out various prefixes; add more if required
        std::string nameEXT = "EGL_EXT_" + extensionName;
        std::string nameARB = "EGL_ARB_" + extensionName;

        return  m_contextExtensions.contains(nameEXT) ||
                m_contextExtensions.contains(nameARB);
    }
}

