//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/DataArray.h"
#include "impl/DataArrayImpl.h"

namespace rlogic
{
    DataArray::DataArray(std::unique_ptr<internal::DataArrayImpl> impl) noexcept
        : LogicObject(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_impl{ static_cast<internal::DataArrayImpl&>(*LogicObject::m_impl) }
    {
    }

    DataArray::~DataArray() noexcept = default;

    EPropertyType DataArray::getDataType() const
    {
        return m_impl.getDataType();
    }

    template <typename T>
    const std::vector<T>* DataArray::getDataInternal() const
    {
        return m_impl.getData<T>();
    }

    size_t DataArray::getNumElements() const
    {
        return m_impl.getNumElements();
    }

    template RAMSES_API const std::vector<float>* DataArray::getDataInternal() const;
    template RAMSES_API const std::vector<vec2f>* DataArray::getDataInternal() const;
    template RAMSES_API const std::vector<vec3f>* DataArray::getDataInternal() const;
    template RAMSES_API const std::vector<vec4f>* DataArray::getDataInternal() const;
    template RAMSES_API const std::vector<int32_t>* DataArray::getDataInternal() const;
    template RAMSES_API const std::vector<vec2i>* DataArray::getDataInternal() const;
    template RAMSES_API const std::vector<vec3i>* DataArray::getDataInternal() const;
    template RAMSES_API const std::vector<vec4i>* DataArray::getDataInternal() const;
    template RAMSES_API const std::vector<std::vector<float>>* DataArray::getDataInternal() const;
}
