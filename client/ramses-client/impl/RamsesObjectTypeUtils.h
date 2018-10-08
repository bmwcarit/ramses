//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTTYPEUTILS_H
#define RAMSES_RAMSESOBJECTTYPEUTILS_H

#include "ramses-client-api/RamsesObjectTypes.h"
#include "ramses-client-api/RamsesObject.h"
#include "RamsesObjectTypeTraits.h"
#include <assert.h>

namespace ramses
{
    class RamsesObjectTypeUtils
    {
    public:
        static const char* GetRamsesObjectTypeName( ERamsesObjectType type );
        static bool        IsTypeMatchingBaseType(ERamsesObjectType type, ERamsesObjectType baseType);
        static bool        IsConcreteType(ERamsesObjectType type);

        template <typename T>
        static const T& ConvertTo(const RamsesObject& obj)
        {
            assert(IsTypeMatchingBaseType(obj.getType(), TYPE_ID_OF_RAMSES_OBJECT<T>::ID));
            return static_cast<const T&>(obj);
        }

        template <typename T>
        static T& ConvertTo(RamsesObject& obj)
        {
            assert(IsTypeMatchingBaseType(obj.getType(), TYPE_ID_OF_RAMSES_OBJECT<T>::ID));
            return static_cast<T&>(obj);
        }
    };
}

#endif
