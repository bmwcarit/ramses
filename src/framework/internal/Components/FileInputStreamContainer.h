//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/InputStreamContainer.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "internal/Core/Utils/File.h"

#include <string_view>

namespace ramses::internal
{
    class FileInputStreamContainer : public IInputStreamContainer
    {
    public:
        explicit FileInputStreamContainer(std::string_view filename)
            : m_file(filename)
            , m_stream(m_file)
        {}

        IInputStream& getStream() override
        {
            return m_stream;
        }

    private:
        File m_file;
        BinaryFileInputStream m_stream;
    };
}
