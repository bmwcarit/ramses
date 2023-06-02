//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicObject.h"
#include "ramses-logic/EPropertyType.h"

#include <string>
#include <memory>
#include <vector>

namespace ramses::internal
{
    class DataArrayImpl;
}

namespace ramses
{
    /**
    * Storage for data - e.g. animation data like keyframes or timestamps.
    */
    class DataArray : public LogicObject
    {
    public:
        /**
        * Returns the type of data stored in this #DataArray.
        *
        * @return the data type
        */
        [[nodiscard]] RAMSES_API EPropertyType getDataType() const;

        /**
        * Returns the data stored in this #DataArray.
        * Make sure to use the right template type, query #getDataType
        * to see what data type is stored (see also #ramses::PropertyTypeToEnum traits
        * and #ramses::CanPropertyTypeBeStoredInDataArray).
        * When called with an unsupported type, a compile-time assert is triggered.
        * When called with a mismatching type (e.g. getData<float>() when the type
        * is vec4f) the method returns nullptr.
        *
        * @return vector of data or nullptr if incorrect template type is provided
        */
        template <typename T>
        [[nodiscard]] const std::vector<T>* getData() const;

        /**
        * Returns the number of elements stored in this #DataArray.
        *
        * @return the number of elements
        */
        [[nodiscard]] RAMSES_API size_t getNumElements() const;

        /**
        * Implementation detail of DataArray
        */
        internal::DataArrayImpl& m_impl;

    protected:
        /**
        * Constructor of DataArray. Use #ramses::LogicEngine::createDataArray.
        *
        * @param impl implementation details of the DataArray
        */
        explicit DataArray(std::unique_ptr<internal::DataArrayImpl> impl) noexcept;

    private:
        /**
        * Internal implementation of #getData.
        *
        * @return vector of data or nullptr if wrong template type provided
        */
        template <typename T>
        [[nodiscard]] RAMSES_API const std::vector<T>* getDataInternal() const;

        friend class internal::ApiObjects;
    };

    template <typename T>
    const std::vector<T>* DataArray::getData() const
    {
        static_assert(CanPropertyTypeBeStoredInDataArray(PropertyTypeToEnum<T>::TYPE),
            "Unsupported data type, see createDataArray API doc to see supported types.");
        return getDataInternal<T>();
    }
}
