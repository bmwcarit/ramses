//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/DataObject.h"

//internal
#include "DataObjectImpl.h"

namespace ramses
{
    DataObject::DataObject(std::unique_ptr<DataObjectImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<DataObjectImpl&>(SceneObject::m_impl) }
    {
    }

    EDataType DataObject::getDataType() const
    {
        return m_impl.getDataType();
    }

    template <typename T>
    status_t DataObject::setValueInternal(T&& value)
    {
        return m_impl.setValue(value);

    }

    template <typename T>
    status_t DataObject::getValueInternal(T& value) const
    {
        return m_impl.getValue(value);
    }

    // const l-value instances
    template RAMSES_API status_t DataObject::setValueInternal<const int32_t&>(const int32_t&);
    template RAMSES_API status_t DataObject::setValueInternal<const float&>(const float&);
    template RAMSES_API status_t DataObject::setValueInternal<const vec2i&>(const vec2i&);
    template RAMSES_API status_t DataObject::setValueInternal<const vec3i&>(const vec3i&);
    template RAMSES_API status_t DataObject::setValueInternal<const vec4i&>(const vec4i&);
    template RAMSES_API status_t DataObject::setValueInternal<const vec2f&>(const vec2f&);
    template RAMSES_API status_t DataObject::setValueInternal<const vec3f&>(const vec3f&);
    template RAMSES_API status_t DataObject::setValueInternal<const vec4f&>(const vec4f&);
    template RAMSES_API status_t DataObject::setValueInternal<const matrix22f&>(const matrix22f&);
    template RAMSES_API status_t DataObject::setValueInternal<const matrix33f&>(const matrix33f&);
    template RAMSES_API status_t DataObject::setValueInternal<const matrix44f&>(const matrix44f&);

    // l-value instances
    template RAMSES_API status_t DataObject::setValueInternal<int32_t&>(int32_t&);
    template RAMSES_API status_t DataObject::setValueInternal<float&>(float&);
    template RAMSES_API status_t DataObject::setValueInternal<vec2i&>(vec2i&);
    template RAMSES_API status_t DataObject::setValueInternal<vec3i&>(vec3i&);
    template RAMSES_API status_t DataObject::setValueInternal<vec4i&>(vec4i&);
    template RAMSES_API status_t DataObject::setValueInternal<vec2f&>(vec2f&);
    template RAMSES_API status_t DataObject::setValueInternal<vec3f&>(vec3f&);
    template RAMSES_API status_t DataObject::setValueInternal<vec4f&>(vec4f&);
    template RAMSES_API status_t DataObject::setValueInternal<matrix22f&>(matrix22f&);
    template RAMSES_API status_t DataObject::setValueInternal<matrix33f&>(matrix33f&);
    template RAMSES_API status_t DataObject::setValueInternal<matrix44f&>(matrix44f&);

    // r-value instances
    template RAMSES_API status_t DataObject::setValueInternal<int32_t>(int32_t&&);
    template RAMSES_API status_t DataObject::setValueInternal<float>(float&&);
    template RAMSES_API status_t DataObject::setValueInternal<vec2i>(vec2i&&);
    template RAMSES_API status_t DataObject::setValueInternal<vec3i>(vec3i&&);
    template RAMSES_API status_t DataObject::setValueInternal<vec4i>(vec4i&&);
    template RAMSES_API status_t DataObject::setValueInternal<vec2f>(vec2f&&);
    template RAMSES_API status_t DataObject::setValueInternal<vec3f>(vec3f&&);
    template RAMSES_API status_t DataObject::setValueInternal<vec4f>(vec4f&&);
    template RAMSES_API status_t DataObject::setValueInternal<matrix22f>(matrix22f&&);
    template RAMSES_API status_t DataObject::setValueInternal<matrix33f>(matrix33f&&);
    template RAMSES_API status_t DataObject::setValueInternal<matrix44f>(matrix44f&&);

    template RAMSES_API status_t DataObject::getValueInternal<int32_t>(int32_t&) const;
    template RAMSES_API status_t DataObject::getValueInternal<float>(float&) const;
    template RAMSES_API status_t DataObject::getValueInternal<vec2i>(vec2i&) const;
    template RAMSES_API status_t DataObject::getValueInternal<vec3i>(vec3i&) const;
    template RAMSES_API status_t DataObject::getValueInternal<vec4i>(vec4i&) const;
    template RAMSES_API status_t DataObject::getValueInternal<vec2f>(vec2f&) const;
    template RAMSES_API status_t DataObject::getValueInternal<vec3f>(vec3f&) const;
    template RAMSES_API status_t DataObject::getValueInternal<vec4f>(vec4f&) const;
    template RAMSES_API status_t DataObject::getValueInternal<matrix22f>(matrix22f&) const;
    template RAMSES_API status_t DataObject::getValueInternal<matrix33f>(matrix33f&) const;
    template RAMSES_API status_t DataObject::getValueInternal<matrix44f>(matrix44f&) const;
}
