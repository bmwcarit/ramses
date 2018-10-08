//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FORCEFALLBACKIMAGE_H
#define RAMSES_FORCEFALLBACKIMAGE_H

#include "Ramsh/RamshCommandArguments.h"
#include "Collections/String.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Common/Cpp11Macros.h"

namespace ramses
{
    class RamsesClientImpl;
}

namespace ramses_internal
{
    class RamsesObject;

    class ForceFallbackImage : public RamshCommandArgs<UInt32, ramses::sceneId_t, String>
    {
    public:
        explicit ForceFallbackImage(ramses::RamsesClientImpl& client);
        virtual Bool execute(UInt32& forceFallback, ramses::sceneId_t& sceneId, String& streamTextureName) const override;

    private:
        ramses::RamsesClientImpl& m_client;
    };
}

#endif //RAMSES_FORCEFALLBACKIMAGE_H
