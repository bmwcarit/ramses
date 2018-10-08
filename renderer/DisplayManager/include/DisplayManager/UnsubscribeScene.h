//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYMANAGER_UNSUBSCRIBESCENE_H
#define RAMSES_DISPLAYMANAGER_UNSUBSCRIBESCENE_H

#include "Ramsh/RamshCommandArguments.h"
#include "DisplayManager/IDisplayManager.h"

namespace ramses_display_manager
{
    class UnsubscribeScene : public ramses_internal::RamshCommandArgs < uint64_t >
    {
    public:
        UnsubscribeScene(IDisplayManager& displayManager);

        virtual bool execute(uint64_t& sceneId) const override;

    private:
        IDisplayManager& m_displayManager;
    };

}

#endif
