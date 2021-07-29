//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmStatusMessage.h"
#include "DcsmStatusMessageImpl.h"
#include "Utils/LogMacros.h"

namespace ramses
{
    DcsmStatusMessage::DcsmStatusMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl_)
        : impl(std::move(impl_))
    {
    }

    DcsmStatusMessage::~DcsmStatusMessage()
    {
    }

    StreamStatusMessage const* DcsmStatusMessage::getAsStreamStatus() const
    {
        if (DcsmStatusMessageImpl::Type::StreamStatus == impl->getType())
            return static_cast<StreamStatusMessage const*>(this);

        return nullptr;
    }

    ActiveLayoutMessage const* DcsmStatusMessage::getAsActiveLayout() const
    {
        if (DcsmStatusMessageImpl::Type::ActiveLayout == impl->getType())
            return static_cast<ActiveLayoutMessage const*>(this);

        return nullptr;
    }

    WidgetFocusStatusMessage const* DcsmStatusMessage::getAsWidgetFocusStatus() const
    {
        if (DcsmStatusMessageImpl::Type::WidgetFocusStatus == impl->getType())
            return static_cast<WidgetFocusStatusMessage const*>(this);

        return nullptr;
    }


    // ------------------------------------------------------------------------------

    StreamStatusMessage::StreamStatusMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl_)
        : DcsmStatusMessage(std::move(impl_))
    {
    }

    StreamStatusMessage::StreamStatusMessage(Status status)
        : DcsmStatusMessage(std::make_unique<DcsmStatusMessageImpl>(DcsmStatusMessageImpl::Type::StreamStatus, sizeof(StreamStatusMessage::Status)))
    {
        auto stream = impl->getOStream();
        stream << status;
    }

    StreamStatusMessage::Status StreamStatusMessage::getStreamStatus() const
    {
        StreamStatusMessage::Status status;
        auto stream = impl->getIStream();
        stream >> status;
        return status;
    }


    // ------------------------------------------------------------------------------

    ActiveLayoutMessage::ActiveLayoutMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl_)
        : DcsmStatusMessage(std::move(impl_))
    {
    }

    ActiveLayoutMessage::ActiveLayoutMessage(Layout layout)
        : DcsmStatusMessage(std::make_unique<DcsmStatusMessageImpl>(DcsmStatusMessageImpl::Type::ActiveLayout, sizeof(ActiveLayoutMessage::Layout)))
    {
        auto stream = impl->getOStream();
        stream << layout;
    }

    ActiveLayoutMessage::Layout ActiveLayoutMessage::getLayout() const
    {
        ActiveLayoutMessage::Layout layout;
        auto stream = impl->getIStream();
        stream >> layout;
        return layout;
    }


    // ------------------------------------------------------------------------------

    WidgetFocusStatusMessage::WidgetFocusStatusMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl_)
        : DcsmStatusMessage(std::move(impl_))
    {
    }

    WidgetFocusStatusMessage::WidgetFocusStatusMessage(Status status)
        : DcsmStatusMessage(std::make_unique<DcsmStatusMessageImpl>(DcsmStatusMessageImpl::Type::WidgetFocusStatus, sizeof(WidgetFocusStatusMessage::Status)))
    {
        auto stream = impl->getOStream();
        stream << status;
    }

    WidgetFocusStatusMessage::Status WidgetFocusStatusMessage::getWidgetFocusStatus() const
    {
        WidgetFocusStatusMessage::Status status;
        auto stream = impl->getIStream();
        stream >> status;
        return status;
    }

}


