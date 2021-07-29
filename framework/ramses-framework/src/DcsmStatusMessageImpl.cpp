//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmStatusMessageImpl.h"

namespace ramses
{
    DcsmStatusMessageImpl::DcsmStatusMessageImpl(Type type, size_t size)
        : m_type(type)
        , m_data(size)
    {
        assert(m_type < Type::NumElements);
    }

    DcsmStatusMessageImpl::DcsmStatusMessageImpl(Type type, absl::Span<const ramses_internal::Byte> message)
        : m_type(type)
        , m_data(message.size())
    {
        assert(m_type < Type::NumElements);
        std::memcpy(m_data.data(), message.data(), message.size());
    }

    std::unique_ptr<DcsmStatusMessage> DcsmStatusMessageImpl::CreateMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl)
    {
        switch (impl->getType())
        {
        case Type::StreamStatus:
            return std::make_unique<StreamStatusMessage>(std::move(impl));
        case Type::ActiveLayout:
            return std::make_unique<ActiveLayoutMessage>(std::move(impl));
        case Type::WidgetFocusStatus:
            return std::make_unique<WidgetFocusStatusMessage>(std::move(impl));
        case Type::NumElements:
            assert(false);
        }
        return nullptr;
    }

    DcsmStatusMessageImpl::Type DcsmStatusMessageImpl::getType() const
    {
        return m_type;
    }

    std::vector<ramses_internal::Byte> const& DcsmStatusMessageImpl::getData() const
    {
        return m_data;
    }

    ramses_internal::RawBinaryOutputStream DcsmStatusMessageImpl::getOStream()
    {
        return ramses_internal::RawBinaryOutputStream(m_data.data(), m_data.size());
    }

    ramses_internal::BinaryInputStream DcsmStatusMessageImpl::getIStream()
    {
        return ramses_internal::BinaryInputStream(m_data.data());
    }
}
