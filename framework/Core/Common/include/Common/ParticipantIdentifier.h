//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PARTICIPANTIDENTIFIER_H
#define RAMSES_PARTICIPANTIDENTIFIER_H

#include "Collections/Guid.h"

#include <string>
#include <string_view>

namespace ramses_internal
{
    class ParticipantIdentifier
    {
    public:
        ParticipantIdentifier();
        ParticipantIdentifier(const Guid& participantId, std::string_view participantName);

        virtual ~ParticipantIdentifier();

        [[nodiscard]] const Guid& getParticipantId() const;
        [[nodiscard]] const std::string& getParticipantName() const;

        bool operator==(const ParticipantIdentifier& other) const;

    private:
        Guid m_participantId;
        std::string m_participantName;
    };

    inline const Guid& ParticipantIdentifier::getParticipantId() const
    {
        return m_participantId;
    }

    inline const std::string& ParticipantIdentifier::getParticipantName() const
    {
        return m_participantName;
    }

    inline bool ParticipantIdentifier::operator==(const ParticipantIdentifier& other) const
    {
        return m_participantId == other.m_participantId;
    }
}

#endif
