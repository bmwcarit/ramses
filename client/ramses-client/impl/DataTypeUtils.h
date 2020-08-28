//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENT_DATATYPEUTILS_H
#define RAMSES_CLIENT_DATATYPEUTILS_H

#include "ramses-client-api/EDataType.h"
#include "SceneAPI/EDataType.h"
#include <assert.h>

namespace ramses
{
    class DataTypeUtils
    {
    public:
        static ramses_internal::EDataType GetDataTypeInternal(EDataType dataType)
        {
            switch (dataType)
            {
            case EDataType::UInt16:
                return ramses_internal::EDataType::UInt16;
            case EDataType::UInt32:
                return ramses_internal::EDataType::UInt32;
            case EDataType::Float:
                return ramses_internal::EDataType::Float;
            case EDataType::Vector2F:
                return ramses_internal::EDataType::Vector2F;
            case EDataType::Vector3F:
                return ramses_internal::EDataType::Vector3F;
            case EDataType::Vector4F:
                return ramses_internal::EDataType::Vector4F;
            default:
                assert(false);
            }

            return ramses_internal::EDataType::Invalid;
        }

        static EDataType GetDataTypeFromInternal(ramses_internal::EDataType dataType)
        {
            switch (dataType)
            {
            case ramses_internal::EDataType::UInt16:
                return EDataType::UInt16;
            case ramses_internal::EDataType::UInt32:
                return EDataType::UInt32;
            case ramses_internal::EDataType::Float:
                return EDataType::Float;
            case ramses_internal::EDataType::Vector2F:
                return EDataType::Vector2F;
            case ramses_internal::EDataType::Vector3F:
                return EDataType::Vector3F;
            case ramses_internal::EDataType::Vector4F:
                return EDataType::Vector4F;
            default:
                assert(false);
                return EDataType::Vector4F;
            }
        }
    };
}

#endif
