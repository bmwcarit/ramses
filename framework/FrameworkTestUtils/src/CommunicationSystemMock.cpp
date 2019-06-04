//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "CommunicationSystemMock.h"

using namespace testing;

namespace ramses_internal
{
    CommunicationSystemMock::CommunicationSystemMock()
        : m_sendDataSizes(CommunicationSendDataSizes{1, 512, 100, 1, 1, 1})
    {
    }

    CommunicationSystemMock::~CommunicationSystemMock()
    {
    }

    CommunicationSendDataSizes CommunicationSystemMock::getSendDataSizes() const
    {
        return m_sendDataSizes;
    }

    void CommunicationSystemMock::setSendDataSizes(const CommunicationSendDataSizes& sizes)
    {
        m_sendDataSizes = sizes;
    }

    void CommunicationSystemMock::setResourceProviderServiceHandler(IResourceProviderServiceHandler*)
    {
    }

    void CommunicationSystemMock::setResourceConsumerServiceHandler(IResourceConsumerServiceHandler*)
    {
    }

    void CommunicationSystemMock::setSceneProviderServiceHandler(ISceneProviderServiceHandler*)
    {
    }

    void CommunicationSystemMock::setSceneRendererServiceHandler(ISceneRendererServiceHandler*)
    {
    }

    void CommunicationSystemMock::setDcsmProviderServiceHandler(IDcsmProviderServiceHandler*)
    {
    }

    void CommunicationSystemMock::setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler*)
    {
    }
}
