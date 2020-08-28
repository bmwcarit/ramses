//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SERIALIZATIONHELPER_H
#define RAMSES_SERIALIZATIONHELPER_H

#include "SerializationContext.h"
#include "Collections/IOutputStream.h"
#include "RamsesObjectImpl.h"
#include "RamsesObjectRegistry.h"
#include "RamsesObjectRegistryIterator.h"
#include "RamsesObjectTypeUtils.h"
#include "Collections/Pair.h"
#include "Collections/Vector.h"

namespace ramses
{
    class SerializationHelper
    {
    public:

        static ObjectIDType DeserializeObjectID(ramses_internal::IInputStream& inStream)
        {
            ObjectIDType objectID = DeserializationContext::GetObjectIDNull();
            inStream >> objectID;
            return objectID;
        }

        static status_t DeserializeObjectImpl(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext, RamsesObjectImpl& impl, ObjectIDType& objectID)
        {
            objectID = SerializationHelper::DeserializeObjectID(inStream);
            return impl.deserialize(inStream, serializationContext);
        }

        template <typename ObjectsBaseType>
        static status_t SerializeObjectsInRegistry(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext, const RamsesObjectRegistry& registry)
        {
            const ERamsesObjectType baseType = TYPE_ID_OF_RAMSES_OBJECT<ObjectsBaseType>::ID;

            typedef std::pair<ERamsesObjectType, uint32_t> TypeCountPair;
            std::vector<TypeCountPair> typesToSerialize;
            typesToSerialize.reserve(ERamsesObjectType_NUMBER_OF_TYPES);
            uint32_t totalCount = 0u;

            for (uint32_t typeIdx = 0u; typeIdx < ERamsesObjectType_NUMBER_OF_TYPES; ++typeIdx)
            {
                const ERamsesObjectType type = static_cast<ERamsesObjectType>(typeIdx);
                if (RamsesObjectTypeUtils::IsConcreteType(type) &&
                    RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, baseType))
                {
                    const uint32_t count = registry.getNumberOfObjects(type);
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

                RamsesObjectRegistryIterator iter(registry, type);
                while (const ObjectsBaseType* obj = iter.getNext<ObjectsBaseType>())
                {
                    CHECK_RETURN_ERR(obj->impl.serialize(outStream, serializationContext));
                }
            }

            return StatusOK;
        }

        static void DeserializeNumberOfObjectTypes(ramses_internal::IInputStream& inStream, uint32_t& totalCount, uint32_t& typesCount)
        {
            inStream >> totalCount;
            inStream >> typesCount;
        }

        static ERamsesObjectType DeserializeObjectTypeAndCount(ramses_internal::IInputStream& inStream, uint32_t& count)
        {
            uint32_t typeInt = ERamsesObjectType_Invalid;
            inStream >> typeInt;
            inStream >> count;

            return static_cast<ERamsesObjectType>(typeInt);
        }
    };
}

#endif
