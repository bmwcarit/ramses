//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2011 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <atomic>

namespace ramses::internal
{
    class Runnable
    {
    public:
        Runnable() = default;
        virtual ~Runnable() = default;

        virtual void run() = 0;

        virtual void cancel()
        {
            mCancel = true;
        }

        void resetCancel()
        {
            mCancel = false;
        }

        bool isCancelRequested()
        {
            return mCancel;
        }

        Runnable(const Runnable& other) = delete;
        Runnable& operator=(const Runnable& other) = delete;

    private:
        std::atomic<bool> mCancel{false};
    };
}
