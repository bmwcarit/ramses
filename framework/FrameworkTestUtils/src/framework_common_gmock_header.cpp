//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"

#include "gmock/gmock.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneId.h"
#include "Scene/SceneActionCollection.h"

namespace glm
{
    void PrintTo(const mat4& matrix, ::std::ostream* os)
    {
        *os << "[" << matrix[0][0] << "," << matrix[1][0] << "," << matrix[2][0] << "," << matrix[3][0] << "]";
        *os << "[" << matrix[0][1] << "," << matrix[1][1] << "," << matrix[2][1] << "," << matrix[3][1] << "]";
        *os << "[" << matrix[0][2] << "," << matrix[1][2] << "," << matrix[2][2] << "," << matrix[3][2] << "]";
        *os << "[" << matrix[0][3] << "," << matrix[1][3] << "," << matrix[2][3] << "," << matrix[3][3] << "]";
    }

    void PrintTo(const vec3& value, ::std::ostream* os)
    {
        *os << "xyz(" << value.x;
        *os << "," << value.y;
        *os << "," << value.z << ")";
    }

    void PrintTo(const vec4& value, ::std::ostream* os)
    {
        *os << "xyzw(" << value.x;
        *os << "," << value.y;
        *os << "," << value.z;
        *os << "," << value.w << ")";
    }
}

namespace ramses_internal
{
    void PrintTo(const SceneActionCollection& actions, std::ostream* os)
    {
        *os << "SceneActionCollection numberOfActions=" << actions.numberOfActions() << ", dataSize=" << actions.collectionData().size();
    }

    void PrintTo(const String& string, std::ostream* os)
    {
        *os << string.c_str();
    }

    void PrintTo(const DataFieldHandle& field, ::std::ostream* os)
    {
        *os << field.asMemoryHandle();
    }

    void PrintTo(const Guid& guid, ::std::ostream* os)
    {
        *os << guid.toString().c_str();
    }

    void PrintTo(const SceneInfo& si, ::std::ostream* os)
    {
        *os << "ScenInfo(sceneId " << si.sceneID.getValue() << ", name " << si.friendlyName.c_str() << ")";
    }
}
