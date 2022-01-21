//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"

#include "gmock/gmock.h"
#include "Math3d/Matrix44f.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneId.h"
#include "Scene/SceneActionCollection.h"
#include "TransportCommon/SomeIPStackCommon.h"

namespace ramses_internal
{
    void PrintTo(const ramses_internal::Matrix44f& matrix, ::std::ostream* os)
    {
        *os << "[" << matrix.m(0, 0) << "," << matrix.m(0, 1) << "," << matrix.m(0, 2) << "," << matrix.m(0, 3) << "]";
        *os << "[" << matrix.m(1, 0) << "," << matrix.m(1, 1) << "," << matrix.m(1, 2) << "," << matrix.m(1, 3) << "]";
        *os << "[" << matrix.m(2, 0) << "," << matrix.m(2, 1) << "," << matrix.m(2, 2) << "," << matrix.m(2, 3) << "]";
        *os << "[" << matrix.m(3, 0) << "," << matrix.m(3, 1) << "," << matrix.m(3, 2) << "," << matrix.m(3, 3) << "]";
    }

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

    void PrintTo(const SomeIPMsgHeader& hdr, ::std::ostream* os)
    {
        *os << fmt::to_string(hdr);
    }
}
