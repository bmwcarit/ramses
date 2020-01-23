//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTEVENTHANDLERMOCK_H
#define RAMSES_CLIENTEVENTHANDLERMOCK_H

#include "gmock/gmock.h"
#include "ramses-client-api/IClientEventHandler.h"

namespace ramses
{
    class ClientEventHandlerMock : public IClientEventHandler
    {
    public:
        ClientEventHandlerMock();
        virtual ~ClientEventHandlerMock();

        MOCK_METHOD1(resourceFileLoadFailed, void(const char* filename));
        MOCK_METHOD1(resourceFileLoadSucceeded, void(const char* filename));
        MOCK_METHOD1(sceneFileLoadFailed, void(const char* filename));
        MOCK_METHOD2(sceneFileLoadSucceeded, void(const char* filename, Scene* loadedScene));
    };
}

#endif
