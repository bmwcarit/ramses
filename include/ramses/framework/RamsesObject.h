//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/APIExport.h"
#include "ramses/framework/RamsesObjectTypes.h"
#include "ramses/framework/ValidationReport.h"

#include <string_view>

namespace ramses
{
    namespace internal
    {
        class RamsesObjectImpl;
    }

    /**
    * @ingroup CoreAPI
    * @brief The RamsesObject is a base class for all client API objects owned by the framework.
    */
    class RAMSES_API RamsesObject
    {
    public:
        /**
        * @brief Returns the name of the object.
        *
        * @return Name of the object
        */
        [[nodiscard]] std::string_view getName() const;

        /**
        * @brief Changes the name of the object.
        *
        * @param name New name of the object
        * @return true for success, false otherwise
        */
        bool setName(std::string_view name);

        /**
        * @brief Gets type of the object.
        *
        * @return Type of the object, see ERamsesObjectType enum for possible values.
        */
        [[nodiscard]] ERamsesObjectType getType() const;

        /**
        * @brief Checks if the object is of given type.
        *
        * @param[in] type Type to check against.
        * @return True if object is of given type, ie. it can be converted to given type.
        */
        [[nodiscard]] bool isOfType(ERamsesObjectType type) const;

        /**
        * Set user ID for this object.
        * User ID is an optional identifier of #RamsesObject stored as number of up to 128 bits.
        * User ID is often logged together with name (#getName) when referring to this object and is serialized, thus persistent.
        * Note that user IDs do not have to be unique, it is user's choice and responsibility if uniqueness is desired.
        * User ID is logged only if other than [0,0] was set and the format is hexadecimal 'highId|lowId' with all digits printed.
        *
        * @param highId high 64 bits of user ID to set
        * @param lowId low 64 bits of user ID to set
        * @return true if successful, false if failed
        */
        bool setUserId(uint64_t highId, uint64_t lowId);

        /**
        * Returns the user ID set using #setUserId.
        *
        * @return the user ID [highId, lowId] or [0, 0] if no user ID was set
        */
        [[nodiscard]] std::pair<uint64_t, uint64_t> getUserId() const;

        /**
        * @brief Performs a (potentially slow!) validation of this object and its dependencies
        *
        * validate() may append issues to the provided report object, classified by warning or error:
        * - errors need to be fixed, otherwise the object's behaviour will be undefined
        * - warnings indicate issues that are undesireable, but not necessarily cause problems
        *   (e.g. unused resources, performance issues)
        *
        * @note validate() will skip objects that are already part of the provided report. (Provide an empty report to force re-validation)
        *
        * @param report The report that the object writes to
        */
        void validate(ValidationReport& report) const;

        /**
        * Casts this object to given type.
        * Has same behavior as \c dynamic_cast, will return nullptr (without error) if given type does not match this object.
        *
        * @return ramses object cast to given type or nullptr if wrong type provided
        */
        template <typename T>
        [[nodiscard]] const T* as() const;

        /**
        * @copydoc as() const
        */
        template <typename T>
        [[nodiscard]] T* as();

        /**
         * Get the internal data for implementation specifics of RamsesObject.
         */
        [[nodiscard]] internal::RamsesObjectImpl& impl();

        /**
         * Get the internal data for implementation specifics of RamsesObject.
         */
        [[nodiscard]] const internal::RamsesObjectImpl& impl() const;

        /**
         * @brief Deleted copy constructor
         */
        RamsesObject(const RamsesObject&) = delete;

        /**
         * @brief Deleted move constructor
         */
        RamsesObject(RamsesObject&&) = delete;

        /**
         * @brief Deleted copy assignment
         * @return unused
         */
        RamsesObject& operator=(const RamsesObject&) = delete;

        /**
         * @brief Deleted move assignment
         * @return unused
         */
        RamsesObject& operator=(RamsesObject&&) = delete;

    protected:
        /**
        * @brief Constructor for RamsesObject.
        *
        * @param[in] impl Internal data for implementation specifics of RamsesObject (sink - instance becomes owner)
        */
        explicit RamsesObject(std::unique_ptr<internal::RamsesObjectImpl> impl);

        /**
        * @brief Destructor of the RamsesObject
        */
        virtual ~RamsesObject();

        /**
        * Internal implementation of #as<T> with check that T is the correct type
        */
        template <typename T>
        [[nodiscard]] const T* internalCast() const;

        /**
        * @copydoc internalCast() const
        */
        template <typename T>
        [[nodiscard]] T* internalCast();

        /**
        * Stores internal data for implementation specifics of RamsesObject.
        */
        std::unique_ptr<internal::RamsesObjectImpl> m_impl;
    };

    template <typename T> const T* RamsesObject::as() const
    {
        static_assert(std::is_base_of<RamsesObject, T>::value, "T in as<T> must be a subclass of RamsesObject!");
        return internalCast<T>();
    }

    template <typename T> T* RamsesObject::as()
    {
        static_assert(std::is_base_of<RamsesObject, T>::value, "T in as<T> must be a subclass of RamsesObject!");
        return internalCast<T>();
    }

    /**
     * Returns the given object cast to type T if the object is convertible to T
     * Otherwise a nullptr is returned
     */
    template <typename T> T object_cast(RamsesObject* obj)
    {
        using ObjType = typename std::remove_cv_t<typename std::remove_pointer_t<T>>;
        static_assert(std::is_base_of_v<RamsesObject, ObjType>, "T must be a subclass of RamsesObject!");
        return (obj != nullptr) ? obj->as<ObjType>() : nullptr;
    }

    /**
     * @copydoc object_cast()
     */
    template <typename T> T object_cast(const RamsesObject* obj)
    {
        using ObjType = typename std::remove_cv_t<typename std::remove_pointer_t<T>>;
        static_assert(std::is_base_of_v<RamsesObject, ObjType>, "T must be a subclass of RamsesObject!");
        return (obj != nullptr) ? obj->as<ObjType>() : nullptr;
    }
}
