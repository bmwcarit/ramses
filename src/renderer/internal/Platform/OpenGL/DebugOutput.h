//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/OpenGL/Device_GL_platform.h"

#include <cstdint>

namespace ramses::internal
{
    class IContext;

    class DebugOutput final
    {
    public:
        void enable();
        [[nodiscard]] static bool IsAvailable();
        [[nodiscard]] bool checkAndResetError() const;

    private:
        mutable bool m_errorOccured = false;
    };
}
