//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceFileDescriptionUtils.h"

namespace ramses_internal
{
    StringOutputStream& operator<<(StringOutputStream& lhs, ramses::resourceId_t const& rhs)
    {
        lhs.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        lhs << rhs.highPart << ":" << rhs.lowPart;
        lhs.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_NoHex);
        return lhs;
    }

    String ResourceFileDescriptionUtils::MakeLoggingString(ramses::ResourceFileDescription const& rhs)
    {
        StringOutputStream os;
        const auto numResources = rhs.getNumberOfResources();
        os << "Filename: " << rhs.getFilename() << " ; Resource count " << numResources << ": [ ";
        for (uint32_t i = 0; i < numResources; ++i)
        {
            os << (i != 0 ? " ; " : "") << rhs.getResource(i).getResourceId() ;
        }
        os << " ]";
        return os.release();
    }

    String ResourceFileDescriptionUtils::MakeLoggingString(ramses::ResourceFileDescriptionSet const& rhs)
    {
        StringOutputStream os;
        const auto numDescriptions = rhs.getNumberOfDescriptions();
        os << numDescriptions << " Resource File Descriptions: [ ";
        for (uint32_t i = 0; i < numDescriptions; ++i)
        {
            os << (i != 0 ? " ; " : "") << MakeLoggingString(rhs.getDescription(i));
        }
        os << " ]";
        return os.release();
    }
}
