//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMSTATUSMESSAGEIMPL_H
#define RAMSES_DCSMSTATUSMESSAGEIMPL_H

#include "ramses-framework-api/DcsmStatusMessage.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/RawBinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"

#include <vector>
#include "absl/types/span.h"

namespace ramses
{
    class DcsmStatusMessageImpl
    {
    public:
        enum class Type : uint64_t
        {
            StreamStatus,

            NumElements
        };

        DcsmStatusMessageImpl(Type type, size_t size);
        DcsmStatusMessageImpl(uint64_t messageID, absl::Span<const ramses_internal::Byte> message);

        static std::unique_ptr<DcsmStatusMessage> CreateMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl);

        Type getType() const;
        std::vector<ramses_internal::Byte> const& getData() const;

        ramses_internal::RawBinaryOutputStream getOStream();
        ramses_internal::BinaryInputStream getIStream();

    private:
        Type m_type;
        std::vector<ramses_internal::Byte> m_data;
    };
}

#endif
