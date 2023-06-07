//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_SCENEID_H
#define RAMSES_SCENEAPI_SCENEID_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Common/StronglyTypedValue.h"
#include "Collections/Vector.h"
#include "Scene/EScenePublicationMode.h"

#include <string>
#include <string_view>

namespace ramses_internal
{
    struct SceneIdTag {};
    using SceneId = StronglyTypedValue<uint64_t, 0, SceneIdTag>;

    using SceneIdVector = std::vector<SceneId>;

    struct SceneInfo
    {
        SceneInfo() {}
        explicit SceneInfo(const SceneId& sceneID_, std::string_view friendlyName_ = {}, EScenePublicationMode mode = EScenePublicationMode_LocalAndRemote)
            : sceneID(sceneID_)
            , friendlyName(friendlyName_)
            , publicationMode(mode)
        {}

        friend bool operator==(const SceneInfo& a, const SceneInfo& b)
        {
            return a.sceneID == b.sceneID && a.friendlyName == b.friendlyName;
        }

        friend bool operator!=(const SceneInfo& a, const SceneInfo& b)
        {
            return !(a == b);
        }

        SceneId sceneID;
        std::string friendlyName;
        EScenePublicationMode publicationMode = EScenePublicationMode_Unpublished;
    };
    using SceneInfoVector = std::vector<SceneInfo>;
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::SceneId)

#endif
