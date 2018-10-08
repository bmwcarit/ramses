//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTILANGUAGETEXTSCENE_H
#define RAMSES_MULTILANGUAGETEXTSCENE_H

#include "TextScene_Base.h"

namespace ramses_internal
{
    class MultiLanguageTextScene : public TextScene_Base
    {
    public:
        enum EState
        {
            EState_INITIAL = 0
        };

        MultiLanguageTextScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);
    };
}

#endif
