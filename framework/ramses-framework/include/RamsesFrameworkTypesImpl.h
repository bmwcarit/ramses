//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKTYPESIMPL_H
#define RAMSES_RAMSESFRAMEWORKTYPESIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-capu/container/Hash.h"
#include "Collections/StringOutputStream.h"
#include "Common/StronglyTypedValue.h"
#include "Utils/StringOutputSpecialWrapper.h"

namespace ramses
{
    // StringOutputStream operators
    DEFINE_SPECIAL_STRINGOUTPUTSTREAM_OPERATOR(ramses::ContentID, ramses::ContentID);
    DEFINE_SPECIAL_STRINGOUTPUTSTREAM_OPERATOR(ramses::Category, ramses::Category);
    DEFINE_SPECIAL_STRINGOUTPUTSTREAM_OPERATOR(ramses::sceneId_t, ramses::sceneId_t);
    DEFINE_STRINGOUTPUTSTREAM_OPERATOR(ramses::TechnicalContentDescriptor);
    DEFINE_STRINGOUTPUTSTREAM_OPERATOR(ramses::pickableObjectId_t);
    DEFINE_STRINGOUTPUTSTREAM_OPERATOR(ramses::streamSource_t);
    DEFINE_STRINGOUTPUTSTREAM_OPERATOR(ramses::displayId_t);
    DEFINE_STRINGOUTPUTSTREAM_OPERATOR(ramses::displayBufferId_t);
    DEFINE_STRINGOUTPUTSTREAM_OPERATOR(ramses::resourceCacheFlag_t);
    DEFINE_STRINGOUTPUTSTREAM_OPERATOR(ramses::binaryShaderFormatId_t);
}

namespace std
{
    template<>
    struct hash<::ramses::resourceId_t>
    {
        size_t operator()(const ::ramses::resourceId_t& rid)
        {
            return ramses_capu::HashValue(rid.lowPart, rid.highPart);
        }
    };
}

#endif
