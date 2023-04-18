//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAOBJECT_H
#define RAMSES_DATAOBJECT_H

#include "ramses-client-api/SceneObject.h"
#include "ramses-framework-api/EDataType.h"
#include "ramses-framework-api/DataTypes.h"

namespace ramses
{
    /**
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
         * @brief   Sets/updates the stored value.
         * @details Type of \c value must match #getDataType (see #ramses::GetEDataType).
         *
         * @param[in] value new value.
         * @return status == 0 for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        template <typename T>
        status_t setValue(T&& value);

        /**
         * @brief Gets the stored value.
         * @details Type of \c value must match #getDataType (see #ramses::GetEDataType).
         *
         * @param[out] value stored value.
         * @return status == 0 for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        template <typename T>
        status_t getValue(T& value) const;

        /**
        * Stores internal data for implementation specifics
        */
        class DataObjectImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating DataObject instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of DataObject
        *
        * @param[in] pimpl Internal data for implementation specifics of DataObject
        */
        explicit DataObject(DataObjectImpl& pimpl);

        /**
        * @brief Destructor for a DataObject
        */
        ~DataObject() override;

    private:
        /// Internal implementation of #setValue
        template <typename T> RAMSES_API status_t setValueInternal(T&& value);
        /// Internal implementation of #getValue
        template <typename T> RAMSES_API status_t getValueInternal(T& value) const;
    };

    template <typename T> status_t DataObject::setValue(T&& value)
    {
        static_assert(IsUniformInputDataType<T>(), "Unsupported data type!");
        return setValueInternal(std::forward<T>(value));
    }

    template <typename T> status_t DataObject::getValue(T& value) const
    {
        static_assert(IsUniformInputDataType<T>(), "Unsupported data type!");
        return getValueInternal<T>(value);
    }
}

#endif
