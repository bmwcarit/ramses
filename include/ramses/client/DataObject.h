//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/SceneObject.h"
#include "ramses/framework/EDataType.h"
#include "ramses/framework/DataTypes.h"

namespace ramses
{
    namespace internal
    {
        class DataObjectImpl;
    }

    /**
    * @ingroup CoreAPI
    * @brief   The DataObject is a data container for storing data within a scene.
    * @details The DataObject can be bound to some inputs of some object types
    *          (e.g. #ramses::Appearance::bindInput or #ramses::Camera::bindViewportOffset).
    *          When a data object is bound to an input the data object value overrides whatever was previously
    *          set to that input using its direct setter.
    *          A single #DataObject can be bound to multiple inputs (also to other #ramses::RamsesObject types
    *          where applicable) providing a way to distribute value across instances/inputs.
    *          Using #DataObject also allows use of data linking across scenes between data object provider and consumer
    *          (#ramses::Scene::createDataProvider and #ramses::Scene::createDataConsumer) see SDK examples for typical use cases.
    *          A value set to a #DataObject is then propagated everywhere it is bound to and it is linked to.
    */
    class RAMSES_API DataObject : public SceneObject
    {
    public:
        /**
        * @brief Returns the data type this #DataObject holds.
        *
        * @return data type held in this #DataObject
        */
        [[nodiscard]] EDataType getDataType() const;

        /**
         * Get the internal data for implementation specifics of DataObject.
         */
        [[nodiscard]] internal::DataObjectImpl& impl();

        /**
         * Get the internal data for implementation specifics of DataObject.
         */
        [[nodiscard]] const internal::DataObjectImpl& impl() const;

        /**
         * @brief   Sets/updates the stored value.
         * @details Type of \c value must match #getDataType (see #ramses::GetEDataType).
         *
         * @param[in] value new value.
         * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
         */
        template <typename T>
        bool setValue(T&& value);

        /**
         * @brief Gets the stored value.
         * @details Type of \c value must match #getDataType (see #ramses::GetEDataType).
         *
         * @param[out] value stored value.
         * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
         */
        template <typename T>
        bool getValue(T& value) const;

    protected:
        /**
        * @brief Scene is the factory for creating DataObject instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor of DataObject
        *
        * @param[in] impl Internal data for implementation specifics of DataObject
        */
        explicit DataObject(std::unique_ptr<internal::DataObjectImpl> impl);

        /**
        * Stores internal data for implementation specifics
        */
        internal::DataObjectImpl& m_impl;

    private:
        /// Internal implementation of #setValue
        template <typename T> bool setValueInternal(T&& value);
        /// Internal implementation of #getValue
        template <typename T> bool getValueInternal(T& value) const;
    };

    template <typename T> bool DataObject::setValue(T&& value)
    {
        static_assert(IsUniformInputDataType<T>(), "Unsupported data type!");
        return setValueInternal(std::forward<T>(value));
    }

    template <typename T> bool DataObject::getValue(T& value) const
    {
        static_assert(IsUniformInputDataType<T>(), "Unsupported data type!");
        return getValueInternal<T>(value);
    }
}
