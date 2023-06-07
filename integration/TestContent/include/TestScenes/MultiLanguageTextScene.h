//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTILANGUAGETEXTSCENE_H
#define RAMSES_MULTILANGUAGETEXTSCENE_H

#if defined(RAMSES_TEXT_ENABLED)

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

        MultiLanguageTextScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);
    };
}

#endif
#endif
