//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_COMMON_GMOCK_HEADER_H
#define RAMSES_FRAMEWORK_COMMON_GMOCK_HEADER_H

#include "SceneAPI/Handles.h"
#include <iosfwd>
#include "DataTypesImpl.h"

namespace ramses_internal
{
    // These are custom printers, which shows ramses type values in mocks and tests
    // It needs to be in a separate file visible by ALL compilation units (!!!)
    // The reason for this is that PrintTo has a template version in gmock/gtest
    // which violates the one definition rule (http://en.wikipedia.org/wiki/One_Definition_Rule)
    // Can also read more about it here: https://groups.google.com/forum/#!topic/googletestframework/mBlPZtprtr8

    class SceneActionCollection;
    class String;
    class RamshInput;
    class Guid;
    struct SceneInfo;

    void PrintTo(const SceneActionCollection& actions, std::ostream* os);
    void PrintTo(const String& string, std::ostream* os);
    void PrintTo(const DataFieldHandle& field, ::std::ostream* os);
    void PrintTo(const Guid& guid, ::std::ostream* os);
    void PrintTo(const SceneInfo&, ::std::ostream* os);
    void PrintTo(const RamshInput&, ::std::ostream* os);
}

namespace glm
{
    void PrintTo(const mat4& matrix, ::std::ostream* os);
    void PrintTo(const vec3& value, ::std::ostream* os);
    void PrintTo(const vec4& value, ::std::ostream* os);
}


#endif
