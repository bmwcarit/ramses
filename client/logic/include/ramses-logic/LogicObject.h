//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include <string>
#include <memory>

namespace rlogic::internal
{
    class LogicObjectImpl;
}

namespace rlogic
{
    /**
    * A base class for all rlogic API objects
    */
    class LogicObject
    {
    public:
        /**
        * Returns the name of this object.
        *
        * @return the name of this object
        */
        [[nodiscard]] RAMSES_API std::string_view getName() const;

        /**
        * Sets the name of this object.
        *
        * @param name new name of the object
        * @return true if setting the name was successful, false if the object name can't be changed (e.g. interface objects)
        */
        RAMSES_API bool setName(std::string_view name);

        /**
        * Returns the id of this object. Every object gets a unique, immutable id assigned on object creation.
        * The id is serialized and thus persisted on load.
        *
        * @return the id of this object
        */
        [[nodiscard]] RAMSES_API uint64_t getId() const;

        /**
        * Set user ID for this object.
        * User IDs are optional identifiers of logic objects stored as number of up to 128 bits.
        * User IDs will be logged together with name (#getName) and unique ID (#getId) and are serialized, thus persistent.
        * Note that user IDs do not have to be unique, it is user's choice and responsibility if uniqueness is desired.
        * User ID is logged only if other than [0,0] was set and the format is hexadecimal 'highId|lowId' with all digits printed.
        *
        * @param highId high 64 bits of user ID to set
        * @param lowId low 64 bits of user ID to set
        * @return true if successful, false if failed
        */
        RAMSES_API bool setUserId(uint64_t highId, uint64_t lowId);

        /**
        * Returns the user ID set using #setUserId.
        *
        * @return the user ID [highId, lowId] or [0, 0] if no user ID was set
        */
        [[nodiscard]] RAMSES_API std::pair<uint64_t, uint64_t> getUserId() const;

        /**
        * Casts this object to given type.
        * Has same behavior as \c dynamic_cast, will return nullptr (without error) if given type does not match this object.
        *
        * @return logic object cast to given type or nullptr if wrong type provided
        */
        template <typename T>
        [[nodiscard]] const T* as() const;

        /**
        * @copydoc as() const
        */
        template <typename T>
        [[nodiscard]] T* as();

        /**
        * Deleted copy constructor
        */
        LogicObject(const LogicObject&) = delete;

        /**
        * Deleted assignment operator
        */
        LogicObject& operator=(const LogicObject&) = delete;

        /**
        * Destructor of #LogicObject
        */
        virtual ~LogicObject() noexcept;

        std::unique_ptr<internal::LogicObjectImpl> m_impl;

    protected:
        /**
        * Constructor of #LogicObject. User is not supposed to call this - LogcNodes are created by subclasses
        *
        * @param impl implementation details of the #LogicObject
        */
        explicit LogicObject(std::unique_ptr<internal::LogicObjectImpl> impl) noexcept;

        /**
        * Internal implementation of #as<T> with check that T is the correct type
        */
        template <typename T>
        [[nodiscard]] RAMSES_API const T* internalCast() const;

        /**
        * @copydoc internalCast() const
        */
        template <typename T>
        [[nodiscard]] RAMSES_API T* internalCast();
    };

    template <typename T> const T* LogicObject::as() const
    {
        static_assert(std::is_base_of<LogicObject, T>::value, "T in as<T> must be a subclass of LogicObject!");
        return internalCast<T>();
    }

    template <typename T> T* LogicObject::as()
    {
        static_assert(std::is_base_of<LogicObject, T>::value, "T in as<T> must be a subclass of LogicObject!");
        return internalCast<T>();
    }
}
