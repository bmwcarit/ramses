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
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"

namespace ramses
{
    DataObject::DataObject(DataObjectImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    DataObject::~DataObject() = default;

    EDataType DataObject::getDataType() const
    {
        return impl.getDataType();
    }

    template <typename T>
    status_t DataObject::setValueInternal(T&& value)
    {
        using RawType = std::remove_cv_t<std::remove_reference_t<T>>;

        if constexpr (std::is_same_v<RawType, int32_t> || std::is_same_v<RawType, float>)
            return impl.setValue(value);

        if constexpr (std::is_same_v<RawType, vec2i>)
            return impl.setValue(ramses_internal::Vector2i{ value[0], value[1] });
        if constexpr (std::is_same_v<RawType, vec3i>)
            return impl.setValue(ramses_internal::Vector3i{ value[0], value[1], value[2] });
        if constexpr (std::is_same_v<RawType, vec4i>)
            return impl.setValue(ramses_internal::Vector4i{ value[0], value[1], value[2], value[3] });
        if constexpr (std::is_same_v<RawType, vec2f>)
            return impl.setValue(ramses_internal::Vector2{ value[0], value[1] });
        if constexpr (std::is_same_v<RawType, vec3f>)
            return impl.setValue(ramses_internal::Vector3{ value[0], value[1], value[2] });
        if constexpr (std::is_same_v<RawType, vec4f>)
            return impl.setValue(ramses_internal::Vector4{ value[0], value[1], value[2], value[3] });
        if constexpr (std::is_same_v<RawType, matrix22f>)
            return impl.setValue(ramses_internal::Matrix22f{value});
        if constexpr (std::is_same_v<RawType, matrix33f>)
            return impl.setValue(ramses_internal::Matrix33f{value});
        if constexpr (std::is_same_v<RawType, matrix44f>)
            return impl.setValue(ramses_internal::Matrix44f{value});
    }

    template <typename T>
    status_t DataObject::getValueInternal(T& value) const
    {
        if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, float>)
            return impl.getValue(value);

        // TODO vaclav unify public and internal math types to avoid these casts
        if constexpr (std::is_same_v<T, vec2i>)
            return impl.getValue(reinterpret_cast<ramses_internal::Vector2i&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr (std::is_same_v<T, vec3i>)
            return impl.getValue(reinterpret_cast<ramses_internal::Vector3i&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr (std::is_same_v<T, vec4i>)
            return impl.getValue(reinterpret_cast<ramses_internal::Vector4i&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr (std::is_same_v<T, vec2f>)
            return impl.getValue(reinterpret_cast<ramses_internal::Vector2&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr (std::is_same_v<T, vec3f>)
            return impl.getValue(reinterpret_cast<ramses_internal::Vector3&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr (std::is_same_v<T, vec4f>)
            return impl.getValue(reinterpret_cast<ramses_internal::Vector4&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr (std::is_same_v<T, matrix22f>)
            return impl.getValue(reinterpret_cast<ramses_internal::Matrix22f&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr (std::is_same_v<T, matrix33f>)
            return impl.getValue(reinterpret_cast<ramses_internal::Matrix33f&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr (std::is_same_v<T, matrix44f>)
            return impl.getValue(reinterpret_cast<ramses_internal::Matrix44f&>(value)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
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
