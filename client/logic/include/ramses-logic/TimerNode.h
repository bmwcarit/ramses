//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicNode.h"
#include <memory>

namespace rlogic::internal
{
    class TimerNodeImpl;
}

namespace rlogic
{
    /**
    * Timer node can be used to provide timing information to animation nodes (#rlogic::AnimationNode) or any other logic nodes.
    * - Property inputs:
    *     - ticker_us (int64)  - (optional) user provided ticker in microseconds (by default, see below to learn how to use arbitrary time units).
    *                          - if the input is 0 (default) then this TimerNode uses system clock to generate ticker by itself,
    *                            this is recommended for simple use cases when application does not need more advanced timing control.
    * - Property outputs:
    *     - ticker_us (int64)  - in auto-generate mode ('ticker_us' input stays 0) this will output system clock time since epoch in microseconds
    *                          - in case of user provided ticker (non-zero 'ticker_us' input) this output will contain the same value
    *                            (user ticker is just passed through, this way time units are user defined).
    *
    * Timer node works in one of two modes - it generates ticker by itself or uses user provided ticker. This allows quick and easy switch
    * between stages of the development, e.g. prototyping, testing or production, where for some use cases auto-generated time is desired
    * and other require well specified timing provided by application.
    * Due to the auto-generate mode both the input and output has defined time units, however timer node can also be used in a fully
    * time unit agnostic mode, see inputs/outputs description above for details.
    * Note that unlike other logic nodes a TimerNode is always updated on every #rlogic::LogicEngine::update call regardless of if any of its
    * inputs were modified or not.
    */
    class TimerNode : public LogicNode
    {
    public:
        /**
        * Constructor of TimerNode. User is not supposed to call this - TimerNodes are created by other factory classes
        *
        * @param impl implementation details of the TimerNode
        */
        explicit TimerNode(std::unique_ptr<internal::TimerNodeImpl> impl) noexcept;

        /**
        * Destructor of TimerNode.
        */
        ~TimerNode() noexcept override;

        /**
        * Copy Constructor of TimerNode is deleted because TimerNodes are not supposed to be copied
        */
        TimerNode(const TimerNode&) = delete;

        /**
        * Move Constructor of TimerNode is deleted because TimerNodes are not supposed to be moved
        */
        TimerNode(TimerNode&&) = delete;

        /**
        * Assignment operator of TimerNode is deleted because TimerNodes are not supposed to be copied
        */
        TimerNode& operator=(const TimerNode&) = delete;

        /**
        * Move assignment operator of TimerNode is deleted because TimerNodes are not supposed to be moved
        */
        TimerNode& operator=(TimerNode&&) = delete;

        /**
        * Implementation of TimerNode
        */
        internal::TimerNodeImpl& m_timerNodeImpl;
    };
}
