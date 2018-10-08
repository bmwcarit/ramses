//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_IWAYLANDOUTPUTGLOBAL_H
#define RAMSES_IWAYLANDOUTPUTGLOBAL_H


namespace ramses_internal
{
    class IWaylandClient;
    class IWaylandDisplay;

    class IWaylandOutputGlobal
    {
    public:
        virtual ~IWaylandOutputGlobal(){}
        virtual bool init(IWaylandDisplay& serverDisplay) = 0;
        virtual void destroy() = 0;
        virtual void getResolution(int32_t& width, int32_t& height) const = 0;
        virtual int32_t getRefreshRate() const = 0;
        virtual void outputBind(IWaylandClient& client, uint32_t version, uint32_t id) = 0;
    };
}

#endif
