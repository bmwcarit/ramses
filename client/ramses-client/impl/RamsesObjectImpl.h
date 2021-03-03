//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTIMPL_H
#define RAMSES_RAMSESOBJECTIMPL_H

// API
#include "ramses-client-api/RamsesObjectTypes.h"

// internal
#include "StatusObjectImpl.h"
#include "RamsesObjectHandle.h"

// framework
#include "Collections/String.h"


namespace ramses_internal
{
    class IOutputStream;
    class IInputStream;
}

namespace ramses
{
    class SerializationContext;
    class DeserializationContext;
    class RamsesObject;
    class IRamsesObjectRegistry;

    class RamsesObjectImpl : public StatusObjectImpl
    {
    public:
        explicit RamsesObjectImpl(ERamsesObjectType type, const char* name);
        virtual ~RamsesObjectImpl() override;

        void setObjectRegistry(IRamsesObjectRegistry& objectRegistry);
        void setObjectRegistryHandle(RamsesObjectHandle handle);
        RamsesObjectHandle getObjectRegistryHandle() const;

        ERamsesObjectType      getType() const;
        bool                   isOfType(ERamsesObjectType type) const;
        const ramses_internal::String& getName() const;
        virtual status_t       setName(RamsesObject& object, const char* name);
        const RamsesObject&    getRamsesObject() const;
        RamsesObject&          getRamsesObject();
        void                   setRamsesObject(RamsesObject& ramsesObject);

        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext);
        virtual status_t resolveDeserializationDependencies(DeserializationContext& serializationContext);

        virtual void deinitializeFrameworkData() = 0;

        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

    private:
        ERamsesObjectType       m_type;
        ramses_internal::String m_name;

        RamsesObject*           m_ramsesObject = nullptr;
        IRamsesObjectRegistry*  m_objectRegistry = nullptr;
        RamsesObjectHandle      m_objectRegistryHandle;
    };
}

#endif
