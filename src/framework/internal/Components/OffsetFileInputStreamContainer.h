//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/InputStreamContainer.h"
#include "internal/Core/Utils/BinaryOffsetFileInputStream.h"

namespace ramses::internal
{
    class OffsetFileInputStreamContainer : public IInputStreamContainer
    {
    public:
        explicit OffsetFileInputStreamContainer(int fd, size_t offset, size_t length)
            : m_stream(fd, offset, length)
        {}

        IInputStream& getStream() override
        {
            return m_stream;
        }

    private:
        BinaryOffsetFileInputStream m_stream;
    };
}
