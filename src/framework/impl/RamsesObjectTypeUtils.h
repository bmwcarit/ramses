//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesObjectTypes.h"
#include "ramses/framework/RamsesObject.h"
#include "impl/RamsesObjectTypeTraits.h"
#include <cassert>

namespace ramses::internal
{
    class RamsesObjectTypeUtils final
    {
    public:
        static const char* GetRamsesObjectTypeName(ERamsesObjectType type);

        static constexpr bool IsTypeMatchingBaseType(ERamsesObjectType type, ERamsesObjectType baseType)
        {
            while (type != ERamsesObjectType::Invalid)
            {
                if (type == baseType)
                {
                    return true;
                }
                const auto index = static_cast<int>(type);
                assert(RamsesObjectTraits[index].typeID == type && "Wrong order of RamsesObject traits!");
                type = RamsesObjectTraits[index].baseClassTypeID;
            }

            return false;
        }

        static constexpr bool IsConcreteType(ERamsesObjectType type)
        {
            const auto index = static_cast<int>(type);
            assert(RamsesObjectTraits[index].typeID == type && "Wrong order of RamsesObject traits!");
            return RamsesObjectTraits[index].isConcreteType;
        }

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
