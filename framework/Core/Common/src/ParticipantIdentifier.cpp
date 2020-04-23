//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Common/ParticipantIdentifier.h"

namespace ramses_internal
{
    ParticipantIdentifier::ParticipantIdentifier(const Guid& participantId, const String& participantName)
        : m_participantId(participantId)
        , m_participantName(participantName)
    {
    }

    ParticipantIdentifier::ParticipantIdentifier()
        : m_participantName("<unknown>")
    {
    }

    ParticipantIdentifier::~ParticipantIdentifier()
    {
    }
}
