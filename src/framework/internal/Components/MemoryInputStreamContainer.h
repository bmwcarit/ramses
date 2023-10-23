//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/InputStreamContainer.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include <memory>

namespace ramses::internal
{
    class MemoryInputStreamContainer : public IInputStreamContainer
    {
    public:
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        explicit MemoryInputStreamContainer(std::unique_ptr<std::byte[], void(*)(const std::byte*)> data)
            : m_data(std::move(data))
            , m_stream(m_data.get())
        {
        }

        IInputStream& getStream() override
        {
            return m_stream;
        }

    private:
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        const std::unique_ptr<std::byte[], void(*)(const std::byte*)> m_data;
        BinaryInputStream m_stream;
    };
}
