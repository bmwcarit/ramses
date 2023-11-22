//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/SerializationContext.h"
#include "impl/SceneObjectImpl.h"
#include "impl/SceneObjectRegistry.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

namespace ramses::internal
{
    class SerializationHelper
    {
    public:

        static ObjectIDType DeserializeObjectID(ramses::internal::IInputStream& inStream)
        {
            ObjectIDType objectID = DeserializationContext::GetObjectIDNull();
            inStream >> objectID;
            return objectID;
        }

        template <typename ObjectsBaseType>
        static bool SerializeObjectsInRegistry(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext, const SceneObjectRegistry & registry)
        {
            const ERamsesObjectType baseType = TYPE_ID_OF_RAMSES_OBJECT<ObjectsBaseType>::ID;

            using TypeCountPair = std::pair<ERamsesObjectType, uint32_t>;
            std::vector<TypeCountPair> typesToSerialize;
            typesToSerialize.reserve(RamsesObjectTypeCount);
            uint32_t totalCount = 0u;

            for (uint32_t typeIdx = 0u; typeIdx < static_cast<uint32_t>(RamsesObjectTypeCount); ++typeIdx)
            {
                const auto type = static_cast<ERamsesObjectType>(typeIdx);
                if (RamsesObjectTypeUtils::IsConcreteType(type) &&
                    RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, baseType))
                {
                    const auto count = static_cast<uint32_t>(registry.getNumberOfObjects(type));
                    if (count > 0u)
                    {
                        const TypeCountPair typeCountEntry = { type, count };
                        typesToSerialize.push_back(typeCountEntry);
                        totalCount += count;
                    }
                }
            }

            outStream << totalCount;
            outStream << static_cast<uint32_t>(typesToSerialize.size());

            for (const auto& typeCountIter : typesToSerialize)
            {
                const ERamsesObjectType type = typeCountIter.first;

                outStream << static_cast<uint32_t>(type);
                outStream << typeCountIter.second;

                SceneObjectRegistryIterator iter(registry, type);
                while (const auto* obj = iter.getNext<ObjectsBaseType>())
                {
                    if (!obj->impl().serialize(outStream, serializationContext))
                        return false;
                }
            }

            return true;
        }

        static void DeserializeNumberOfObjectTypes(ramses::internal::IInputStream& inStream, uint32_t& totalCount, uint32_t& typesCount)
        {
            inStream >> totalCount;
            inStream >> typesCount;
        }

        static ERamsesObjectType DeserializeObjectTypeAndCount(ramses::internal::IInputStream& inStream, uint32_t& count)
        {
            auto typeInt = static_cast<uint32_t>(ERamsesObjectType::Invalid);
            inStream >> typeInt;
            inStream >> count;

            return static_cast<ERamsesObjectType>(typeInt);
        }
    };
}
