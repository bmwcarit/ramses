//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/TaskFramework/RefCounted.h"

namespace ramses::internal
{
    /**
     * Interface for a Task which executable.
     */
    class ITask : public RefCounted
    {
    public:
        /**
         * Execute this Task.
         * @return  true if the execution was successful or false otherwise.
         */
        virtual void execute() = 0;

    };
}
